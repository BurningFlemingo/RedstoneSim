#pragma once
#include <vector>

enum Component : int {
	RedstoneTorch = 0,
	Repeater1,
	Repeater2,
	Repeater3,
	Repeater4,
	Lever,
	Output,

	NullComponent
};

struct Edge {
	int tail;
	int head;
};
struct Dropoff {
	int backInputDropoff;
	int sideInputDropoff{ 15 };
};

// vertices are indices
struct Graph {
	std::vector<Component> vertexComponentMapping;
	std::vector<Edge> edges;
	std::vector<Dropoff> dropoffs;
};

struct Cycle {
	int firstVertex;
	int lastVertex;
	int length;
};

struct VertexState {
	int signal;
	int t;
};

template<typename T>
class SlidingBuffer {
   public:
	SlidingBuffer(int size = 0) : m_Size(size), m_Data(size) {}

	void pushFront(const T& element) {
		m_Index = (m_Index + 1) % m_Size;
		m_Data[m_Index] = element;
	}
	T& operator[](size_t index) { return m_Data[index]; }
	std::vector<T>& data() { return m_Data; }

   private:
	int m_Size{};
	int m_Index{};
	std::vector<T> m_Data;
};
