#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <span>
#include <unordered_map>
#include <tuple>

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
	int torch(const VertexState& state) {
		if (state.saturation == MIN_SIGNAL_VALUE) {
			return MAX_SIGNAL_VALUE;
		}
		return MIN_SIGNAL_VALUE;
	}
	int lever(const VertexState& state) {
		return state.saturation;
	}
}  // namespace OBF

// state behavior function
namespace SBF {
	int torch(std::span<int> signalInputs, const VertexState& state) {
		assert(isSignalValid(max(signalInputs)));

		if (max(signalInputs) == 0) {
			return MIN_SIGNAL_VALUE;
		}
		return MAX_SIGNAL_VALUE;
	}

	int lever(const std::span<int>& signalInputs, const VertexState& state) {
		assert(isSignalValid(state.saturation));
		return state.saturation;
	}

}  // namespace SBF

namespace {
	std::function<int(std::span<int>, const VertexState&)> stateBehaviors[]{
		SBF::torch, SBF::lever
	};

	std::function<int(const VertexState&)> outputBehaviors[]{ OBF::torch,
															  OBF::lever };
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

VertexState getVertexState(
	int vertex,
	int t,
	const std::vector<VertexState>& initialState,
	const Graph& graph
) {
	assert(vertex < graph.vertexComponentMapping.size());
	assert(initialState.size() == graph.vertexComponentMapping.size());

	static std::vector<std::pair<int, VertexState>> memoizedVertexState(
		graph.vertexComponentMapping.size()
	);

	if (t <= 0) {
		return initialState[vertex];
	}

	std::vector<int> inputVertices{ propegateBack(vertex, graph) };

	std::vector<int> inputSignals{};
	for (const auto& inVertex : inputVertices) {
		Component inComponent{ graph.vertexComponentMapping[inVertex] };
		VertexState inVertexState{
			getVertexState(inVertex, t - 1, initialState, graph)
		};
		int inSignal{ outputBehaviors[inComponent](inVertexState) };

		inputSignals.emplace_back(inSignal);
	}

	VertexState state{};
	std::pair<int, VertexState> memoizedState{ memoizedVertexState[vertex] };
	if (memoizedState.first == t - 1) {
		state = memoizedState.second;
	} else {
		state = getVertexState(vertex, t - 1, initialState, graph);
	}

	Component component{ graph.vertexComponentMapping[vertex] };
	int newSaturation{ stateBehaviors[component](inputSignals, state) };

	int newDuration{};
	if (!(state.saturation > 0 ^ newSaturation > 0)) {
		newDuration = state.duration + 1;
	}

	VertexState newState{ .saturation = newSaturation,
						  .duration = newDuration };
	memoizedVertexState[vertex] = { t, newState };

	return newState;
}

int main() {
	// throwaway MVP code

	Graph graph{ .vertexComponentMapping = { RedstoneTorch,
											 RedstoneTorch,
											 RedstoneTorch },
				 .edges = { { 0, 1 }, { 1, 2 }, { 2, 0 } } };

	std::vector<VertexState> initialState(
		graph.vertexComponentMapping.size(), VertexState{ 0, 4 }
	);

	initialState[0] = { 15, 4 };

	int outputVertex{ 2 };
	for (int i{}; i < 100; i++) {
		VertexState state{
			getVertexState(outputVertex, i, initialState, graph)
		};
		int signal{
			outputBehaviors[graph.vertexComponentMapping[outputVertex]](state)
		};
		std::cout << signal << std::endl;
	}
}
