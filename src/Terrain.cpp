#include "Terrain.hpp"

Terrain::Terrain() {
	vertices = generate_terrain();
	indices = generate_indices();
}

Terrain::~Terrain() {}

std::vector<Vertex> Terrain::generate_terrain() {
    std::vector<Vertex> vertices =
	{
		//Front face
		{ { -1, -1, 1 },{ 0, 0, 1 }}, //0
		{ { -1, 1, 1 },{ 0, 0, 1 }}, //3
		{ { 1, -1, 1 },{ 0, 0, 1 }}, //1
		{ { 1, 1, 1 },{ 0, 0, 1 }}, //2

		//Back face
		{ { 1, -1, -1 },{ 0, 0, -1 }}, //7
		{ { 1, 1, -1 },{ 0, 0, -1 }}, //6
		{ { -1, -1, -1 },{ 0, 0, -1 }}, //4
		{ { -1, 1, -1 },{ 0, 0, -1 }}, //5
			
		//Top face
		{ { -1, 1, 1 },{ 0, 1, 0 }}, //9
		{ { -1, 1, -1 },{ 0, 1, 0 }}, //8
		{ { 1, 1, 1 },{ 0, 1, 0 }}, //10
		{ { 1, 1, -1 },{ 0, 1, 0 }}, //11

		//Bottom face
		{ { 1, -1, 1 },{ 0, -1, 0 }}, //14
		{ { 1, -1, -1 },{ 0, -1, 0 }}, //13
		{ { -1, -1, 1 },{ 0, -1, 0 }}, //15
		{ { -1, -1, -1 },{ 0, -1, 0 }}, //12

		//Right face
		{ { 1, -1, 1 },{ 1, 0, 0 }}, //19
		{ { 1, 1, 1 },{ 1, 0, 0 }}, //18
		{ { 1, -1, -1 },{ 1, 0, 0 }}, //16
		{ { 1, 1, -1 },{ 1, 0, 0 }}, //17

		//Left face
		{ { -1, -1, -1 },{ -1, 0, 0 }}, //20
		{ { -1, 1, -1 },{ -1, 0, 0 }}, //23
		{ { -1, -1, 1 },{ -1, 0, 0 }}, //21
		{ { -1, 1, 1 },{ -1, 0, 0 }} //22
	};
    return vertices;
}

std::vector<std::uint32_t> Terrain::generate_indices() {
    std::vector<std::uint32_t> indices =
	{
		//Front face
		0, 2, 3, 0, 3, 1,

		//Back face
		6, 7, 5, 6, 5, 4,

		//Top face
		9, 8, 10, 9, 10, 11,

		//Bottom face
		15, 13, 12, 15, 12, 14,

		//Right face
		18, 19, 17, 18, 17, 16,

		//Left face
		20, 22, 23, 20, 23, 21
	};

    return indices;
}