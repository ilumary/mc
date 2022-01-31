#ifndef WORLD_HPP
#define WORLD_HPP

#include "ChunkNode.hpp"
#include "Mesh.hpp"

class World {
    
public:
    //root chunk
    ChunkNode* root_;

    //combined mesh of all terrain vertices
    //Mesh* mesh_;

    //constructor creates root ChunkNode Object
    World();

    //returns world mesh in [distance] chunks generated from position
    Mesh* getWorldMesh(glm::vec3 position, int distance);

    //returns position in chunk index
    glm::vec3 get_current_node_index_of_position(glm::vec3 position);

    //
    ChunkNode* get_current_node_from_position(glm::vec3 position);

    //returns Chunk block at given position
    int getBlock(glm::ivec3 position);

private:

    ChunkNode* current;
    
};

#endif // CHUNK_HPP