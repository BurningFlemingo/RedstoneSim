#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <set>
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

int max(const std::set<int>& signalInputs) {
	int max{};
	for (const auto signal : signalInputs) {
		if (signal > max) {
			max = signal;
		}
	}

	return max;
}

// output behavior function
namespace OBF {
	int torch(const VertexState& state) {
		if (state.saturation == 0) {
			return 15;
		}
		return 0;
	}
	int lever(const VertexState& state) {
		return state.saturation;
	}
}  // namespace OBF

// state behavior function
namespace SBF {
	int torch(const std::set<int>& signalInputs, const VertexState& state) {
		if (max(signalInputs) == 0) {
			return 0;
		}
		return 15;
	}

	int lever(const std::set<int>& signalInputs, const VertexState& state) {
		return state.saturation;
	}

}  // namespace SBF

namespace {
	std::function<int(const std::set<int>&, const VertexState&)>
		stateBehaviors[]{ SBF::torch, SBF::lever };

	std::function<int(const VertexState&)> outputBehaviors[]{ OBF::torch,
															  OBF::lever };
}  // namespace

std::set<int> propegateBack(int vertex, const Graph& graph) {
	std::set<int> verticesFound{};
	for (const auto& edge : graph.edges) {
		if (edge.second == vertex) {
			verticesFound.insert(edge.first);
		}
	}
	return verticesFound;
}

VertexState getVertexState(
	int vertexIndex,
	int t,
	const std::vector<VertexState>& initialState,
	const Graph& graph
) {
	static std::vector<std::pair<int, VertexState>> memoizedVertexState(
		graph.vertexComponentMapping.size()
	);

	if (t <= 0) {
		return initialState[vertexIndex];
	}

	std::set<int> inputVertices{ propegateBack(vertexIndex, graph) };

	std::set<int> inputSignals{};
	for (const auto& inVertex : inputVertices) {
		Component inComponent{ graph.vertexComponentMapping[inVertex] };
		VertexState inVertexState{
			getVertexState(inVertex, t - 1, initialState, graph)
		};
		int inSignal{ outputBehaviors[inComponent](inVertexState) };

		inputSignals.insert(inSignal);
	}

	VertexState state{};
	std::pair<int, VertexState> memoizedState{
		memoizedVertexState[vertexIndex]
	};
	if (memoizedState.first == t - 1) {
		state = memoizedState.second;
	} else {
		state = getVertexState(vertexIndex, t - 1, initialState, graph);
	}

	Component component{ graph.vertexComponentMapping[vertexIndex] };
	int newSaturation{ stateBehaviors[component](inputSignals, state) };

	int newDuration{};
	if (!(state.saturation > 0 ^ newSaturation > 0)) {
		newDuration = state.duration + 1;
	}

	VertexState newState{ .saturation = newSaturation,
						  .duration = newDuration };
	memoizedVertexState[vertexIndex] = { t, newState };

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
