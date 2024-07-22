#pragma once
#include <vector>
#include <tuple>

enum Component : int { RedstoneTorch = 0, Lever, NullComponent };

struct VertexState {
	int saturation;
	int duration;
};

using Edges = std::vector<std::pair<int, int>>;
using Components = std::vector<Component>;

// vertices are indices.
struct Graph {
	Components vertexComponentMapping;
	Edges edges;
};
