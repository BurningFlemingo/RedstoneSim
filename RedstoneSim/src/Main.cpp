#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <span>
#include <unordered_map>
#include <tuple>
#include <set>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define RAYMATH_IMPLEMENTATION
#include "raymath.h"

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
	SetConfigFlags(FLAG_MSAA_4X_HINT);

	Graph graph{};

	// std::vector<VertexState> initialState(
	// 	graph.vertexComponentMapping.size(), VertexState{ 0, 4 }
	// );

	// initialState[0] = { 15, 4 };

	int outputVertex{ 0 };

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "RedSim");
	GuiLoadStyle("./Assets/style_dark.rgs");

	Vector2 center = { (GetScreenWidth() - 300) / 2.0f,
					   GetScreenHeight() / 2.0f };

	SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// VertexState state{
	// 	getVertexState(outputVertex, tick, initialState, graph)
	// };
	// int signal{
	// 	outputBehaviors[graph.vertexComponentMapping[outputVertex]](state)
	// };

	float vertexRadius{ 20.f };
	Color vertexColor{ WHITE };

	bool selected{};
	int selectedVertex{ 0 };

	Vector2 mousePosition{};
	std::vector<Vector2> vertexLocations;
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

		for (const auto& edge : graph.edges) {
			Vector2 second{};
			DrawLineEx(
				vertexLocations[edge.first],
				vertexLocations[edge.second],
				3,
				BLACK
			);
		}
		mousePosition = GetMousePosition();

		float closestVertexDistance{ 100000000000.f };
		int closestVertex{ -1 };
		for (int i{}; i < vertexLocations.size(); i++) {
			float distance{
				Vector2Distance(vertexLocations[i], mousePosition)
			};
			if (distance < closestVertexDistance) {
				closestVertexDistance = distance;
				closestVertex = i;
			}
		}
		bool overlapping{};
		if (closestVertexDistance <= vertexRadius * 2.f) {
			overlapping = true;
		}
		if (overlapping && selected) {
			DrawLineEx(
				vertexLocations[selectedVertex],
				vertexLocations[closestVertex],
				3,
				BLACK
			);
		} else if (selected) {
			DrawLineEx(
				vertexLocations[selectedVertex], mousePosition, 3, BLACK
			);
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if (overlapping) {
				if (selected) {
					selected = false;
					graph.edges.emplace_back(std::pair{ selectedVertex,
														closestVertex });
				} else {
					selected = true;
					selectedVertex = closestVertex;
				}
			} else if (selected) {
				selected = false;
			} else {
				vertexLocations.emplace_back(mousePosition);
				graph.vertexComponentMapping.emplace_back(RedstoneTorch);
			}
		}
		if (!overlapping && !selected) {
			DrawCircleV(mousePosition, vertexRadius, Fade(vertexColor, 0.5f));
		}

		DrawLine(500, 0, 500, GetScreenHeight(), Fade(LIGHTGRAY, 0.6f));
		DrawRectangle(
			500,
			0,
			GetScreenWidth() - 500,
			GetScreenHeight(),
			Fade(LIGHTGRAY, 0.3f)
		);

		bool checked{};
		GuiCheckBox(Rectangle{ 600, 320, 20, 20 }, "box", &checked);

		for (int i{}; i < vertexLocations.size(); i++) {
			const Vector2 vertex{ vertexLocations[i] };

			if (Vector2Distance(mousePosition, vertex) <= vertexRadius) {
				DrawCircleLines(vertex.x, vertex.y, vertexRadius, PINK);
			}
			DrawCircle(vertex.x, vertex.y, vertexRadius, vertexColor);
		}

		if (selected) {
			DrawCircleLinesV(
				vertexLocations[selectedVertex], vertexRadius, YELLOW
			);
		}

		DrawLine(
			500,
			0,
			500,
			GetScreenHeight(),
			GetColor(GuiGetStyle(COMBOBOX, LINE_COLOR))
		);
		DrawRectangle(
			500,
			0,
			GetScreenWidth() - 500,
			GetScreenHeight(),
			GetColor(GuiGetStyle(STATUSBAR, BACKGROUND_COLOR))
		);

		float startAngle{};
		GuiSliderBar(
			{ 600, 40, 120, 20 }, "StartAngle", NULL, &startAngle, -450, 450
		);

		DrawFPS(10, 10);

		DrawRectangleLines(
			600, 80, 120, 40, GetColor(GuiGetStyle(TEXTBOX, LINE_COLOR))
		);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	CloseWindow();
}
