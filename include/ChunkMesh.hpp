#ifndef CHUNKMESH_HPP
#define CHUNKMESH_HPP

#include "Mesh.hpp"
#include "Chunk.hpp"

class World;

namespace gen {

class ChunkMesh : public Mesh {

public:

    //holds the texture uv's for all block types
    const glm::vec4 block_uvs[1] = {
        calculate_uv(0, 1, 1) // DIRT
    };

    //calculates uv cords of given texture size
    static glm::vec4 calculate_uv(int x, int y, int size);

    //generate geometry data for given chunk
    void generate(Chunk* chunk, World* world);
};

}

#endif // CHUNKMESH_HPP