#pragma once
#include <vector>
#include <tuple>

enum Component : int { RedstoneTorch = 0, Lever, NullComponent };

using Edge = std::pair<int, int>;
struct Dropoff {
	int backInputDropoff;
	int sideInputDropoff{ 15 };
};

// vertices are indices.
struct Graph {
	std::vector<Component> vertexComponentMapping;
	std::vector<Edge> edges;
	std::vector<Dropoff> dropoffs;
};
