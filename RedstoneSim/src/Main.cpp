#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <set>
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

VertexState vState(
	int vertexIndex,
	int t,
	const std::vector<VertexState>& initialState,
	const Graph& graph
) {
	if (t <= 0) {
		return initialState[vertexIndex];
	}

	std::set<int> inputVertices{ propegateBack(vertexIndex, graph) };
	std::set<int> inputSignals{};
	for (const auto& inVertex : inputVertices) {
		Component inComponent{ graph.vertices[inVertex] };
		VertexState inVertexState{
			vState(vertexIndex, t - 1, initialState, graph)
		};
		int inSignal{ outputBehaviors[inComponent](inVertexState) };

		inputSignals.insert(inSignal);
	}

	VertexState state{ vState(vertexIndex, t - 1, initialState, graph) };
	Component component{ graph.vertices[vertexIndex] };
	int newSaturation{ stateBehaviors[component](inputSignals, state) };

	int newDuration{};
	if (state.saturation > 0 && newSaturation > 0 ||
		state.saturation == 0 && newSaturation == 0) {
		newDuration = state.duration + 1;
	}

	VertexState newState{ .saturation = newSaturation,
						  .duration = newDuration };
	return newState;
}

int main() {
	// throwaway MVP code

	Graph graph{
		.vertices = { Lever, RedstoneTorch, RedstoneTorch, RedstoneTorch },
		.edges = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 1 } }
	};

	std::vector<VertexState> initialState;
	initialState.reserve(graph.vertices.size());
	for (auto& vertex : graph.vertices) {
		initialState.emplace_back(VertexState{ 0, 4 });
	}

	initialState[3] = { 0, 4 };

	std::vector<int> inVertexIndices;

	for (int i{}; i < graph.vertices.size(); i++) {
		if (graph.vertices[i] == Lever) {
			inVertexIndices.emplace_back(i);
		}
	}

	int nTicks{ 10 };
	for (int i{}; i < inVertexIndices.size(); i++) {
		for (int j{}; j < 2; j++) {
			int inVertexIndex{ inVertexIndices[i] };
			int inSaturation{ ((j + 1) % 2) * 15 };
			initialState[0] = { inSaturation, 4 };

			std::cout << inSaturation;
			for (int t{}; t < nTicks; t++) {
				VertexState state{ vState(3, t, initialState, graph) };
				int signal{ outputBehaviors[graph.vertices[3]](state) };
				std::cout << "\t" << signal;
			}
			initialState[inVertexIndex] = { 0, 4 };
			std::cout << std::endl;
		}
	}
}
