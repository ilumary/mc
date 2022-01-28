#ifndef CHUNKNODE_HPP
#define CHUNKNODE_HPP

#include "Chunk.hpp"
#include "ChunkMesh.hpp"

class ChunkNode {

public:

    //represents all possible states of one chunk
    enum ChunkState {
        UNINITIALIZED = 0,
        CHUNK_DATA_GENERATED = 1,
        MESH_DATA_GENERATED = 2,
    };

    //represents current chunk state
    int node_state_;

    //holds chunk data of node
    Chunk chunk;

    //holds chunk gemotry of node
    ChunkMesh* geometry_;

    //initialise new chunk node
    ChunkNode();

    //generate data for this node
    void generateData();

    //get mesh of this node
    Mesh* getGeometry(World* world);

    //generate Mesh of this node
    void generateGeometry(World *world);
};

#endif // CHUNKNODE_HPP