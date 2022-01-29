#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "PerlinNoise.hpp"

#include <glm/glm.hpp>

#include <iostream>

class Chunk {

public:
    //defines Chunk size in x, y and z dimension
    static const int SIZE = 16;

    //represents all possible block types
    enum BlockType {
        CLEAR = 0,
        DIRT = 1,
        STONE = 2,
    };

    //represents one chunk
    int chunk_data_[SIZE][SIZE][SIZE];

    PerlinNoise pn = PerlinNoise(123456);

    //chunk position relative to other chunks
    glm::vec3 chunk_position_{};

    //setter for chunk position. +1 equals +SIZE
    void set_chunk_position(glm::vec3 new_position);

    //generate chunk data
    void generate_chunk_data();

    //returns height of position in chunk
    int get_surface_height(glm::vec2 position);
};

#endif // CHUNK_HPP