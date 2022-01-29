#include "Chunk.hpp"

void Chunk::set_chunk_position(glm::vec3 new_position) {
    chunk_position_ = new_position;
}

void Chunk::generate_chunk_data() {
    for(int x = 0; x < SIZE; ++x) {
        for(int z = 0; z < SIZE; ++z) {

            int surface_height = get_surface_height({x, z});

            if(surface_height >= 16) { surface_height = 15; }

            for(int y = 1; y < surface_height; ++y) {
                chunk_data_[x][y][z] = BlockType::STONE;
            }
        }
    }
}

int Chunk::get_surface_height(glm::vec2 position) {
    double n = 15 * pn.noise(position.x / SIZE, position.y / SIZE, 0.9);

    return n + (SIZE / 4);
}