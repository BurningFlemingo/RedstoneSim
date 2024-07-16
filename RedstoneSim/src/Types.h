#pragma once
#include <vector>
#include <tuple>

enum Component : int { RedstoneTorch = 0, Lever };

struct VertexState {
	int saturation;
	int duration;
};

using Edges = std::vector<std::pair<int, int>>;
using Vertices = std::vector<Component>;

struct Graph {
	Vertices vertices;
	Edges edges;
};
