#ifndef CHUNKMESH_HPP
#define CHUNKMESH_HPP

#include "Mesh.hpp"
#include "Chunk.hpp"
#include "World.hpp"

#include <iostream>

class World;

class ChunkMesh : public Mesh {

public:

    //holds the texture uv's for all block types
    const glm::vec4 block_uvs[3] = {
        calculate_uv(2, 0, 16), // DIRT
        calculate_uv(0, 0, 16), // GRASS
        calculate_uv(1, 0, 16) // STONE
    };

    //holds border data of surrounding chunks
    int surround_data_[6][Chunk::SIZE][Chunk::SIZE];

    //calculates uv cords of given texture size
    static glm::vec4 calculate_uv(int x, int y, int size);

    //generate geometry data for given chunk
    void generate(Chunk& chunk);

private:

    //function streamlines index pushing for better understanding
    void add_block_side_indices(uint32_t push_indices[6], uint32_t size);
};

#endif // CHUNKMESH_HPP
