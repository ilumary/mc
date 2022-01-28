#ifndef WORLD_HPP
#define WORLD_HPP

#include "ChunkNode.hpp"
#include "Mesh.hpp"

class World {
    
public:
    //root chunk
    ChunkNode* root;

    //visible mesh
    Mesh* mesh;

    World();

    Mesh* getMeshes();
    
};

#endif // CHUNK_HPP