#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <span>

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
		if (edge.second == vertex) {
			verticesFound.emplace_back(edge.first);
		}
	}
	return verticesFound;
}

int getVertexState(
	int vertex,
	int t,
	const std::vector<int>& initialState,
	const Graph& graph,
	std::vector<std::pair<int, int>>& history
) {
	assert(vertex < graph.vertexComponentMapping.size());
	assert(initialState.size() == graph.vertexComponentMapping.size());

	if (t <= 0) {
		return initialState[vertex];
	}

	std::vector<int> inputVertices{ propegateBack(vertex, graph) };

	int inputSignal{};

	for (const auto& inVertex : inputVertices) {
		Component inComponent{ graph.vertexComponentMapping[inVertex] };
		int inVertexState{
			getVertexState(inVertex, t - 1, initialState, graph, history)
		};
		int inSignal{ outputBehaviors[inComponent](inVertexState) -
					  graph.dropoffs[inVertex].backInputDropoff };

		if (inSignal > inputSignal) {
			inputSignal = inSignal;
		}
	}

	int currentState{};
	std::pair<int, int> memoizedState{ history[vertex] };
	if (memoizedState.first == t - 1) {
		currentState = memoizedState.second;
	} else {
		currentState =
			getVertexState(vertex, t - 1, initialState, graph, history);
	}

	Component component{ graph.vertexComponentMapping[vertex] };
	int newState{
		stateBehaviors[component](graph, vertex, inputSignal, currentState, t)
	};

	history[vertex] = { t, newState };

	return newState;
}

int getVertexOutput(
	int vertex,
	int t,
	const std::vector<int>& initialState,
	const Graph& graph,
	std::vector<std::pair<int, int>>& history

) {
	int state{ getVertexState(vertex, t, initialState, graph, history) };
	Component component{ graph.vertexComponentMapping[vertex] };
	int output{ outputBehaviors[component](state) };
	return output;
}

int main() {
	Graph clockGraph{};
	for (int i{}; i < 21; i++) {
		clockGraph.vertexComponentMapping.emplace_back(RedstoneTorch);
		clockGraph.dropoffs.emplace_back(Dropoff{ 0, 15 });
		int endpoint{ i + 1 };
		if (endpoint != 21) {
			clockGraph.edges.emplace_back(std::pair{ i, endpoint });
		}
	}

	std::vector<int> clockGraphInitialState(
		clockGraph.vertexComponentMapping.size(), 0
	);
	size_t outputVertex{ clockGraphInitialState.size() - 1 };

	std::vector<std::pair<int, int>> history(
		clockGraph.vertexComponentMapping.size()
	);
	for (int i = 0; i < 21; i++) {
		int state = getVertexState(
			outputVertex, i, clockGraphInitialState, clockGraph, history
		);
		std::cout << i << " : " << state << std::endl;
	}
	for (int i{}; i < 21; i++) {
		clockGraphInitialState[i] = history[i].second;
	}

	clockGraph.edges.emplace_back(std::pair{ 20, 0 });
	clockGraph.dropoffs.emplace_back(Dropoff{ 0, 15 });

	for (int i{}; i < 100000; i++) {
		int output{ getVertexOutput(
			outputVertex, i, clockGraphInitialState, clockGraph, history
		) };
		std::cout << i << " : " << output << std::endl;
	}
}
