#ifndef CHUNKMESH_HPP
#define CHUNKMESH_HPP

#include "Mesh.hpp"
#include "Chunk.hpp"

#include <iostream>

class World;

class ChunkMesh : public Mesh {

public:

    //holds the texture uv's for all block types
    const glm::vec4 block_uvs[2] = {
        calculate_uv(2, 0, 16), // DIRT
        calculate_uv(1, 0, 16) // STONE
    };

    //calculates uv cords of given texture size
    static glm::vec4 calculate_uv(int x, int y, int size);

    //generate geometry data for given chunk
    void generate(Chunk* chunk, World* world);

private: 
    //function streamlines index pushing for better understanding
    void add_block_side_indices(uint32_t push_indices[6], uint32_t size);
};

#endif // CHUNKMESH_HPP