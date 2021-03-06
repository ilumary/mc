#ifndef CHUNKNODE_HPP
#define CHUNKNODE_HPP

#include "World.hpp"
#include "Chunk.hpp"
#include "ChunkMesh.hpp"

class ChunkMesh;
class World;

class ChunkNode {

public:

    //defines chunk neighbor positions in neighbors_ vector
    enum RelativeChunkPosition {
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3,
        UP = 4,
        DOWN = 5,
    };

    //represents all possible states of one chunk
    enum ChunkState {
        UNINITIALIZED = 0,
        CHUNK_DATA_GENERATED = 1,
        MESH_DATA_GENERATED = 2,
        SURROUND_DATA_GATHERED = 3,
    };

    //represents current chunk state
    int node_state_;

    //holds node position
    glm::vec3 node_position_;

    //holds chunk data of node
    Chunk chunk;

    //holds chunk gemotry of node
    ChunkMesh* geometry_;

    //holds pointers to all neighboring chunk nodes
    ChunkNode* neighbors_[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    //initialise new chunk node
    ChunkNode(glm::vec3 position);

    //creates chunk data recursively
    void create_node_neighbors_recursively(int distance, std::vector<ChunkNode*> *nodes);
    void create_neighbors(uint32_t distance, std::vector<ChunkNode*> *nodes);

    //searches for already existing neighbors that haven't been linked to the neighbors node array
    void update_neighbor_pointers(std::vector<ChunkNode*> *nodes);

    //searches node by position in given vector of nodes
    ChunkNode* search_node(glm::vec3 pos, std::vector<ChunkNode*> *nodes);

    //traverses recursively through the nodes, each checking if it already exists within vector
    void get_nodes_recursive(std::vector<ChunkNode*> *nodes, int distance);

    //generate data for this node
    void generateData();

    //get mesh of this node
    Mesh* getGeometry();

    //generate Mesh of this node
    void generateGeometry();

    //gathers the border data of the surrounding chunk for mesh creation
    void gather_surrounding_chunk_data();
};

#endif // CHUNKNODE_HPP
