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

std::vector<int> propegateBack(int vertex, const Graph& graph);

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

	int repeater(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t,
		const int n
	) {
		std::vector<int> inputSignals{ propegateBack(vertex, graph) };
		assert(n > 0 && n <= 4);
		if (signalInput > 0 || max(inputSignals) > 0) {
			if (signalInput > 0 && (state >= 2 * n || state == 0)) {
				return 1;
			}
			if (state > 0 && state < n) {
				return state + 1;
			}
			if (signalInput == 0 && n <= state && state < 2 * n) {
				return state + 1;
			}
			if (signalInput >= 0 && state == (2 * n) - 1) {
				return n;
			}
			return 0;
		} else {
			if (n <= state && state < 2 * n) {
				return n;
			} else {
				return 0;
			}
		}
	}
	int repeater1(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		return repeater(graph, vertex, signalInput, state, t, 1);
	}
	int repeater2(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		return repeater(graph, vertex, signalInput, state, t, 2);
	}
	int repeater3(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		return repeater(graph, vertex, signalInput, state, t, 3);
	}
	int repeater4(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		return repeater(graph, vertex, signalInput, state, t, 4);
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
	int output(
		const Graph& graph,
		const int vertex,
		const int signalInput,
		const int state,
		const int t
	) {
		return signalInput;
	}

}  // namespace SBF

// output behavior function
namespace OBF {
	int torch(const int state) {
		if (state == MIN_SIGNAL_VALUE) {
			return MAX_SIGNAL_VALUE;
		}
		return MIN_SIGNAL_VALUE;
	}

	int repeater(const int state, const int n) {
		if (n <= state && state < 2 * n) {
			return 15;
		}
		return 0;
	}

	int repeater1(const int state) {
		return repeater(state, 1);
	}
	int repeater2(const int state) {
		return repeater(state, 2);
	}
	int repeater3(const int state) {
		return repeater(state, 3);
	}
	int repeater4(const int state) {
		return repeater(state, 4);
	}

	int lever(const int state) {
		return state;
	}

	int output(const int state) {
		return state;
	}
}  // namespace OBF

namespace {
	std::function<
		int(const Graph&, const int vertex, const int, const int, const int)>
		stateBehaviors[]{ SBF::torch,	  SBF::repeater1, SBF::repeater2,
						  SBF::repeater3, SBF::repeater4, SBF::lever,
						  SBF::output };

	std::function<int(const int)> outputBehaviors[]{
		OBF::torch,		OBF::repeater1, OBF::repeater2, OBF::repeater3,
		OBF::repeater4, OBF::lever,		OBF::output
	};
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

	int currentState{ getVertexState(
		vertex, t - 1, initialState, graph, memoizedState, memoizedCycleState
	) };

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

int getDepth(Graph graph, int vertex) {
	if (graph.vertexComponentMapping.size() == 0) {
		return 0;
	}
	std::set<int> verticesToVisit{ vertex };
	int diameter{};
	while (verticesToVisit.size() != 0) {
		std::set<int> newVerticesToVisit;
		for (const auto& vertexToCheck : verticesToVisit) {
			std::vector<int> vertices{ propegateBack(vertexToCheck, graph) };
			for (const auto& vertex : vertices) {
				for (Edge& edge : graph.edges) {
					if (edge.head == vertexToCheck && edge.tail == vertex) {
						edge = Edge{ -1, -1 };
					}
				}
				newVerticesToVisit.insert(vertex);
			}
		}
		verticesToVisit = newVerticesToVisit;
		diameter++;
	};
	return diameter - 1;
}

std::vector<int> initializeGraph(
	const Graph& graph,
	const int outputVertex,
	const int ticksToSimulate,
	const std::vector<std::pair<int, int>>& customState = {}  // vertex, signal
) {
	std::vector<int> initialState(graph.vertexComponentMapping.size());
	for (const auto& state : customState) {
		initialState[state.first] = state.second;
	}

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
		VertexState memState{ memoizedState[i] };
		if (memState.t != 0) {
			initialState[i] = memState.signal;
		}
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
			colors[vertex] = 1;
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
		int lastVertex{};
		if (cycle.size() > 1) {
			lastVertex = cycle[1];
		} else {
			lastVertex = cycle[0];
		}

		Cycle reducedCycle{ .firstVertex = cycle[0],
							.lastVertex = lastVertex,
							.length = (int)cycle.size() };
		reducedCycles.emplace_back(reducedCycle);
	}
	return reducedCycles;
}

int main() {
	Graph clockGraph{ .vertexComponentMapping = { RedstoneTorch,
												  RedstoneTorch,
												  RedstoneTorch },
					  .edges = { { 0, 1 }, { 1, 2 }, { 2, 0 } },
					  .dropoffs = { { 0, 15 },
									{ 0, 15 },
									{ 0, 15 },
									{ 0, 15 },
									{ 0, 15 },
									{ 0, 15 } } };

	size_t outputVertex{ 2 };
	std::vector<Cycle> cycles{ depthFirstSearch(clockGraph, outputVertex) };

	Graph acyclicClockGraph{
		.vertexComponentMapping = clockGraph.vertexComponentMapping,
		.dropoffs = clockGraph.dropoffs,
	};
	for (const auto& edge : clockGraph.edges) {
		bool isCyclic{};
		for (const auto& cycle : cycles) {
			if (edge.head == cycle.lastVertex &&
				edge.tail == cycle.firstVertex) {
				isCyclic = true;
				break;
			}
		}
		if (!isCyclic) {
			acyclicClockGraph.edges.emplace_back(edge);
		}
	}

	int ticksToSimulate{ getDepth(clockGraph, outputVertex) };
	std::cout << ticksToSimulate << std::endl;

	std::vector<int> clockGraphInitialState{
		initializeGraph(clockGraph, outputVertex, ticksToSimulate, {})
	};
	std::cout << "initial state: ";
	for (int i{}; i < clockGraphInitialState.size(); i++) {
		std::cout << "v" << i << " = " << clockGraphInitialState[i] << ", ";
	}
	std::cout << std::endl;

	std::vector<VertexState> memoizedState(
		clockGraph.vertexComponentMapping.size()
	);
	std::unordered_map<int, SlidingBuffer<VertexState>> memoizedCycleState{};

	for (const auto& cycle : cycles) {
		memoizedCycleState[cycle.firstVertex] =
			SlidingBuffer<VertexState>(cycle.length);
	}

	std::cout << std::endl;
	std::cout << "------" << std::endl;
	std::cout << "t : v" << outputVertex << std::endl;
	std::cout << "------" << std::endl;
	for (int i{}; i < 10; i++) {
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
