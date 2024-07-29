#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <span>
#include <set>
#include <deque>

#include "Types.h"

constexpr int MAX_SIGNAL_VALUE{ 15 };
constexpr int MIN_SIGNAL_VALUE{ 0 };

bool isSignalValid(int signal) {
	if (signal > MAX_SIGNAL_VALUE || signal < MIN_SIGNAL_VALUE) {
		return false;
	}
	return true;
}

int max(std::span<int> signalInputs) {
	int largestSignal{};
	for (const auto signal : signalInputs) {
		assert(isSignalValid(signal));
		if (signal > largestSignal) {
			largestSignal = signal;
		}
	}

	return largestSignal;
}

// output behavior function
namespace OBF {
	int torch(const int state) {
		if (state == MIN_SIGNAL_VALUE) {
			return MAX_SIGNAL_VALUE;
		}
		return MIN_SIGNAL_VALUE;
	}
	int lever(const int state) {
		return state;
	}
}  // namespace OBF

// state behavior function
namespace SBF {
	int torch(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		assert(isSignalValid(signalInput));

		if (signalInput == 0) {
			return MIN_SIGNAL_VALUE;
		}
		return MAX_SIGNAL_VALUE;
	}

	int lever(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		assert(isSignalValid(state));
		return state;
	}

}  // namespace SBF

namespace {
	std::function<
		int(const Graph&, const int vertex, const int, const int, const int)>
		stateBehaviors[]{ SBF::torch, SBF::lever };

	std::function<int(const int)> outputBehaviors[]{ OBF::torch, OBF::lever };
}  // namespace

std::vector<int> propegateBack(int vertex, const Graph& graph) {
	assert(vertex < graph.vertexComponentMapping.size());

	std::vector<int> verticesFound{};
	for (const auto& edge : graph.edges) {
		if (edge.head == vertex) {
			verticesFound.emplace_back(edge.tail);
		}
	}
	return verticesFound;
}

int getVertexState(
	const int vertex,
	const int t,
	const std::vector<int>& initialState,
	const Graph& graph,
	std::vector<VertexState>& memoizedState,
	std::unordered_map<int, SlidingBuffer<VertexState>>& memoizedCycleState
) {
	assert(vertex < graph.vertexComponentMapping.size());
	assert(initialState.size() == graph.vertexComponentMapping.size());

	if (t <= 0) {
		return initialState[vertex];
	}

	if (memoizedCycleState.contains(vertex)) {
		for (const auto& vertex : memoizedCycleState[vertex].data()) {
			if (vertex.t == t) {
				return vertex.signal;
			}
		}
	} else {
		VertexState memoizedVertexState{ memoizedState[vertex] };
		if (memoizedVertexState.t == t) {
			return memoizedVertexState.signal;
		}
	}

	std::vector<int> inputVertices{ propegateBack(vertex, graph) };

	int inputSignal{};

	for (const auto& inVertex : inputVertices) {
		Component inComponent{ graph.vertexComponentMapping[inVertex] };
		int inVertexState{ getVertexState(
			inVertex,
			t - 1,
			initialState,
			graph,
			memoizedState,
			memoizedCycleState
		) };
		int inSignal{ outputBehaviors[inComponent](inVertexState) -
					  graph.dropoffs[inVertex].backInputDropoff };

		if (inSignal > inputSignal) {
			inputSignal = inSignal;
		}
	}

	int currentState{ -1 };
	if (memoizedCycleState.contains(vertex)) {
		for (const auto& vertex : memoizedCycleState[vertex].data()) {
			if (vertex.t == t - 1) {
				currentState = vertex.signal;
				break;
			}
		}
	} else {
		VertexState memoizedVertexState{ memoizedState[vertex] };
		if (memoizedVertexState.t == t - 1) {
			currentState = memoizedVertexState.signal;
		}

		if (currentState == -1) {
			currentState = getVertexState(
				vertex,
				t - 1,
				initialState,
				graph,
				memoizedState,
				memoizedCycleState
			);
		}
	}

	Component component{ graph.vertexComponentMapping[vertex] };
	int newState{
		stateBehaviors[component](graph, vertex, inputSignal, currentState, t)
	};

	memoizedState[vertex] = { .signal = newState, .t = t };
	if (memoizedCycleState.contains(vertex)) {
		memoizedCycleState[vertex].pushFront({ .signal = newState, .t = t });
	}

	return newState;
}

int getVertexState(
	const int vertex,
	const int t,
	const std::vector<int>& initialState,
	const Graph& graph,
	std::vector<VertexState>&& memoizedState = {},
	std::unordered_map<int, SlidingBuffer<VertexState>>&&
		memoizedCycleState = {}
) {
	return getVertexState(
		vertex, t, initialState, graph, memoizedState, memoizedCycleState
	);
}

int getVertexOutput(
	const int vertex,
	const int t,
	const std::vector<int>& initialState,
	const Graph& graph,
	std::vector<VertexState>& memoizedState,
	std::unordered_map<int, SlidingBuffer<VertexState>>& memoizedCycleState
) {
	int state{ getVertexState(
		vertex, t, initialState, graph, memoizedState, memoizedCycleState
	) };
	Component component{ graph.vertexComponentMapping[vertex] };
	int output{ outputBehaviors[component](state) };
	return output;
}

int getLengthOfLongestPathToVertex(const Graph& graph, int vertex) {
	if (graph.vertexComponentMapping.size() == 0) {
		return 0;
	}
	std::set<int> verticesVisited;
	std::set<int> verticesToCheck{ vertex };
	int diameter{};
	while (verticesToCheck.size() != 0) {
		std::set<int> newVerticesToCheck;
		for (const auto& vertexToCheck : verticesToCheck) {
			std::vector<int> vertices{ propegateBack(vertexToCheck, graph) };
			for (const auto& vertex : vertices) {
				bool vertexFound{ verticesVisited.contains(vertex) };
				if (!vertexFound) {
					newVerticesToCheck.insert(vertex);
				}
			}
		}
		verticesToCheck = newVerticesToCheck;
		diameter++;
	};
	return diameter - 1;
}

std::vector<int> initializeAcyclicGraph(
	const Graph& graph,
	const int outputVertex,
	const std::vector<std::pair<int, int>>& initialStateOfLevers = {}
	// vertex, signal
) {
	std::vector<int> initialState(graph.vertexComponentMapping.size());
	for (const auto& lever : initialStateOfLevers) {
		initialState[lever.first] = lever.second;
	}

	int ticksToSimulate{ getLengthOfLongestPathToVertex(graph, outputVertex) };

	std::vector<VertexState> memoizedState(graph.vertexComponentMapping.size());
	std::unordered_map<int, SlidingBuffer<VertexState>> memoizedCycleState{};
	for (int i = 0; i < ticksToSimulate; i++) {
		getVertexState(
			outputVertex,
			i,
			initialState,
			graph,
			memoizedState,
			memoizedCycleState
		);
	}
	for (int i{}; i < initialState.size(); i++) {
		initialState[i] = memoizedState[i].signal;
	}

	return initialState;
}

void depthFirstSearch(
	const Graph& graph,
	int tailVertex,
	int headVertex,
	std::vector<int>& colors,
	std::vector<int>& parents,
	std::vector<std::vector<int>>& cycles
) {
	if (colors[tailVertex] == 2) {
		return;
	}
	if (colors[tailVertex] == 1) {
		std::vector<int> cycle;
		int backtrackedVertex{ headVertex };
		while (backtrackedVertex != tailVertex) {
			cycle.emplace_back(backtrackedVertex);
			backtrackedVertex = parents[backtrackedVertex];
		}
		cycle.emplace_back(backtrackedVertex);
		cycles.emplace_back(cycle);
		return;
	}
	parents[tailVertex] = headVertex;

	colors[tailVertex] = 1;

	std::vector<int> inputVertices{ propegateBack(tailVertex, graph) };
	for (const auto& vertex : inputVertices) {
		if (vertex == parents[tailVertex]) {
			continue;
		}

		depthFirstSearch(graph, vertex, tailVertex, colors, parents, cycles);
	}

	colors[tailVertex] = 2;
}

std::vector<Cycle> depthFirstSearch(const Graph& graph, int headVertex) {
	std::vector<int> colors(graph.vertexComponentMapping.size());
	std::vector<int> parents(graph.vertexComponentMapping.size());
	std::vector<std::vector<int>> cycles;

	std::vector<int> inputVertices{ propegateBack(headVertex, graph) };
	for (const auto& tailVertex : inputVertices) {
		depthFirstSearch(
			graph, tailVertex, headVertex, colors, parents, cycles
		);
	}

	std::vector<Cycle> reducedCycles;
	for (const auto& cycle : cycles) {
		Cycle reducedCycle{ .firstVertex = cycle[0],
							.lastVertex = cycle[cycle.size() - 1],
							.length = (int)cycle.size() };
		reducedCycles.emplace_back(reducedCycle);
	}
	return reducedCycles;
}

int main() {
	Graph clockGraph{};
	for (int i{}; i < 3; i++) {
		clockGraph.vertexComponentMapping.emplace_back(RedstoneTorch);
		clockGraph.dropoffs.emplace_back(Dropoff{ 0, 15 });
		int endpoint{ i + 1 };
		if (endpoint != 3) {
			clockGraph.edges.emplace_back(Edge{ i, endpoint });
		}
	}
	clockGraph.edges.emplace_back(Edge{ 2, 0 });
	clockGraph.dropoffs.emplace_back(Dropoff{});

	size_t outputVertex{ clockGraph.vertexComponentMapping.size() - 1 };
	std::vector<Cycle> cycles{ depthFirstSearch(clockGraph, outputVertex) };

	Graph acyclicClockGraph{
		.vertexComponentMapping = clockGraph.vertexComponentMapping,
		.dropoffs = clockGraph.dropoffs,
	};
	for (const auto& edge : clockGraph.edges) {
		bool isCyclic{};
		for (const auto& cycle : cycles) {
			if (edge.head == cycle.firstVertex &&
				edge.tail == cycle.lastVertex) {
				isCyclic = true;
				break;
			}
		}
		if (!isCyclic) {
			acyclicClockGraph.edges.emplace_back(edge);
		}
	}

	std::vector<int> clockGraphInitialState{
		initializeAcyclicGraph(acyclicClockGraph, outputVertex)
	};

	std::vector<VertexState> memoizedState(
		clockGraph.vertexComponentMapping.size()
	);
	std::unordered_map<int, SlidingBuffer<VertexState>> memoizedCycleState{};

	for (const auto& cycle : cycles) {
		memoizedCycleState[cycle.firstVertex] =
			SlidingBuffer<VertexState>(cycle.length);
	}

	for (int i{}; i < 1000000; i++) {
		int output{ getVertexOutput(
			outputVertex,
			i,
			clockGraphInitialState,
			clockGraph,
			memoizedState,
			memoizedCycleState
		) };
		std::cout << i << " : " << output << '\n';
	}
	std::cout << std::endl;
}
