#include "Chunk.hpp"

void Chunk::set_chunk_position(glm::vec3 new_position) {
    chunk_position_ = new_position;
}

void Chunk::generate_chunk_data() {
    for(int x = 0; x < SIZE; ++x) {
        for(int z = 0; z < SIZE; ++z) {
            //chunk_data_[x][0][z] = Chunk::BlockType::STONE;
            //int surface_height = get_surface_height({x, z});
            int surface_height = 10;

            if(surface_height > 15) { surface_height = 15; }

            for(int y = 0; y < surface_height; ++y) {
                chunk_data_[x][y][z] = get_block_type({x, y, z}, surface_height);
                //chunk_data_[x][y][z] = Chunk::BlockType::CLEAR;
            }
            for(int y = surface_height; y < SIZE; ++y) {
                chunk_data_[x][y][z] = Chunk::BlockType::CLEAR;
            }
        }
    }
}

int Chunk::get_surface_height(glm::vec2 position) {
    double n = 15 * pn.noise(position.x / SIZE, position.y / SIZE, 0.9);

    return n + (SIZE / 4);
}

Chunk::BlockType Chunk::get_block_type(glm::vec3 position, uint32_t terrain_height) {
    double n = 3 * pn.noise(position.x / SIZE, position.y / SIZE, 0.99);
    int grass_depth = terrain_height - 3 - n;
    if(position.y - grass_depth < 0) {
        return Chunk::BlockType::STONE;
    } else if(position.y == terrain_height - 1) {
        return Chunk::BlockType::GRASS;
    } else {
        return Chunk::BlockType::DIRT;
    }
}
