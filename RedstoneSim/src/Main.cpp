#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <span>
#include <unordered_map>
#include <tuple>
#include <set>
#include <sstream>

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

	std::vector<VertexState> initialState;

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "RedSim");
	GuiLoadStyle("./Assets/style_dark.rgs");

	Vector2 center = { (GetScreenWidth() - 300) / 2.0f,
					   GetScreenHeight() / 2.0f };

	SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	Color darkestGray{ GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)) };
	Color darkGray{ GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)) };
	Color medGray{ GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)) };
	Color lightGray{ GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_DISABLED)) };

	float vertexRadius{ 20.f };
	Color vertexColor{ RAYWHITE };
	Color edgeColor{ lightGray };
	Color vertexTextColor{ BLACK };

	bool selected{};
	int selectedVertex{ 0 };

	Vector2 mousePosition{};
	std::vector<Vector2> vertexLocations;
	float zoom{ 0.5 };

	float menuLeftSize{ 20 };
	float menuRightSize{ 200 };
	float menuTopSize{ 50 };
	float menuBottomSize{ 10 };

	float menuBorderSize{ 2.f };

	int tick{};

	int simulatedVertex{};
	int simulatedVertexOutput{};
	bool runSim{};

	Component selectedComponent{ NullComponent };

	bool torch{};
	bool lever{};

	int outputVertex{};
	while (!WindowShouldClose()) {
		mousePosition = GetMousePosition();
		bool inMenu{ mousePosition.x >= screenWidth - menuRightSize };

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
		if (closestVertexDistance <= vertexRadius * 2.f * zoom) {
			overlapping = true;
		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !inMenu) {
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
			} else if (selectedComponent != NullComponent) {
				vertexLocations.emplace_back(mousePosition);
				graph.vertexComponentMapping.emplace_back(selectedComponent);
			}
		}

		if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (selected && overlapping) {
				graph.edges.emplace_back(std::pair<int, int>{ selectedVertex,
															  closestVertex });
				selected = false;
			} else if (selected) {
				selected = false;
			}
		}

		if (IsKeyDown(KEY_SPACE) && graph.vertexComponentMapping.size() > 0) {
			runSim = true;
			tick++;
		} else {
			runSim = false;
		}

		BeginDrawing();
		ClearBackground(darkestGray);

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			if (overlapping && selected) {
				DrawLineEx(
					vertexLocations[selectedVertex],
					vertexLocations[closestVertex],
					3 * zoom,
					edgeColor
				);
			} else if (selected) {
				DrawLineEx(
					vertexLocations[selectedVertex],
					mousePosition,
					3 * zoom,
					edgeColor
				);
			}
		}
		for (const auto& edge : graph.edges) {
			DrawLineEx(
				vertexLocations[edge.first],
				vertexLocations[edge.second],
				3 * zoom,
				edgeColor
			);
		}

		if (!inMenu) {
			if (!overlapping && !selected) {
				DrawCircleV(
					mousePosition, vertexRadius * zoom, Fade(vertexColor, 0.5f)
				);
			}
		}
		for (int i{}; i < vertexLocations.size(); i++) {
			const Vector2 vertex{ vertexLocations[i] };

			if (Vector2Distance(mousePosition, vertex) <= vertexRadius) {
				DrawCircleLines(vertex.x, vertex.y, vertexRadius * zoom, PINK);
			}
			DrawCircle(vertex.x, vertex.y, vertexRadius * zoom, vertexColor);
		}

		// right
		DrawRectangle(
			screenWidth - menuRightSize,
			0,
			menuRightSize,
			screenHeight,
			darkGray
		);
		// bottom
		DrawRectangle(
			0,
			screenHeight - menuBottomSize,
			screenWidth,
			menuBottomSize,
			darkGray
		);

		// left
		DrawRectangle(0, 0, menuLeftSize, screenHeight, darkGray);
		// top
		DrawRectangle(0, 0, screenWidth, menuTopSize, darkGray);

		// left
		DrawLineEx(
			{ menuLeftSize - menuBorderSize, menuTopSize - menuBorderSize },
			{ menuLeftSize - menuBorderSize,
			  screenHeight - (menuBottomSize - menuBorderSize + 1) },
			menuBorderSize,
			darkestGray
		);
		// top
		DrawLineEx(
			{ menuLeftSize - menuBorderSize, menuTopSize - menuBorderSize },
			{ (screenWidth - menuRightSize), menuTopSize - menuBorderSize },
			menuBorderSize,
			darkestGray
		);

		// bottom
		DrawLineEx(
			{ menuLeftSize - (menuBorderSize + 1),
			  screenHeight - menuBottomSize },
			{ (screenWidth - menuRightSize), screenHeight - menuBottomSize },
			menuBorderSize,
			lightGray
		);
		// right
		DrawLineEx(
			{ screenWidth - menuRightSize, menuTopSize - (menuBorderSize + 1) },
			{ screenWidth - menuRightSize, screenHeight - menuBottomSize },
			menuBorderSize,
			lightGray
		);

		DrawRectangleLines(
			600, 80, 120, 40, GetColor(GuiGetStyle(TEXTBOX, LINE_COLOR))
		);

		GuiCheckBox({ screenWidth - 150, 100, 50, 20 }, "torch", &torch);
		GuiCheckBox({ screenWidth - 150, 120, 50, 20 }, "lever", &lever);

		if (torch && lever) {
			selectedComponent = NullComponent;
		} else if (torch) {
			selectedComponent = RedstoneTorch;
		} else if (lever) {
			selectedComponent = Lever;
		}

		if (runSim) {
			VertexState vertexState{
				getVertexState(outputVertex, tick, initialState, graph)
			};
			simulatedVertexOutput =
				outputBehaviors[graph.vertexComponentMapping[outputVertex]](
					vertexState
				);
		}

		std::stringstream outputString{
			TextFormat("%i = %i", tick, simulatedVertexOutput)
		};
		GuiDrawText(
			outputString.str().data(),
			{ screenWidth - 150, 60, 50, 20 },
			TEXT_ALIGN_MIDDLE,
			WHITE
		);

		for (int i{}; i < graph.vertexComponentMapping.size(); i++) {
			std::stringstream vertexString{ TextFormat("%i", i) };

			GuiDrawText(
				vertexString.str().data(),
				Rectangle{ vertexLocations[i].x - vertexRadius,
						   vertexLocations[i].y,
						   vertexRadius * 2,
						   0 },
				TEXT_ALIGN_CENTER,
				vertexTextColor
			);
		}

		EndDrawing();
	}

	CloseWindow();
}
