#include "Chunk.hpp"

void Chunk::set_chunk_position(glm::vec3 new_position) {
    chunk_position_ = new_position;
}

void Chunk::generate_chunk_data() {
    for(int x = 0; x < SIZE; ++x) {
        for(int z = 0; z < SIZE; ++z) {
            chunk_data_[x][0][z] = BlockType::DIRT;
            for(int y = 1; y < SIZE; ++y) {
                chunk_data_[x][y][z] = BlockType::CLEAR;
            }
        }
    }
}