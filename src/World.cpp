#include "World.hpp"

World::World() {
    root_ = new ChunkNode({0, 0, 0});
    current = root_;
}

Mesh* World::getWorldMesh(glm::vec3 position, int distance) {
    //get chunk position
    glm::vec3 chunk_position = get_current_node_index_of_position(position);

    //get current node from position
    current = get_current_node_from_position(position);

    //create neighbor object for all nodes in range
    if(current != nullptr) {
        current->create_node_neighbors_recursively(distance + 1);
    }
    
    //get all neighbors
    std::vector<ChunkNode*> nodes;
    nodes.push_back(current);
    current->get_nodes_recursive(&nodes, distance);

    //get all neighbors meshes
    std::vector<Mesh*> meshes;
    meshes.push_back(current->getGeometry(this));

    for(std::vector<ChunkNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        //meshes.push_back((*it).getGeometry(this));
    }

    //merge meshes

    return root_->getGeometry(this);
}

glm::vec3 World::get_current_node_index_of_position(glm::vec3 position) {
    int x = position.x >= 0 ? (position.x / Chunk::SIZE) : (position.x / Chunk::SIZE - 1);
	int y = position.y <= 0 ? (position.y / Chunk::SIZE) : (position.y / Chunk::SIZE - 1);
	int z = position.z <= 0 ? (position.z / Chunk::SIZE) : (position.z / Chunk::SIZE - 1);

	return { x, y, z };
}

ChunkNode* World::get_current_node_from_position(glm::vec3 position) {
    glm::ivec3 offset = current->node_position_ - position;

    // offset x direction
    if (offset.x > 0) {
		for (int x = 0; x < offset.x; ++x) {
			current = current->neighbors_[ChunkNode::WEST];
		}
	} else if (offset.x < 0) {
		for (int x = offset.x; x < 0; ++x) {
			current = current->neighbors_[ChunkNode::EAST];
		}
	}

    //offset z direction
    if (offset.z > 0) {
		for (int z = 0; z < offset.z; ++z) {
			current = current->neighbors_[ChunkNode::NORTH];
		}
	} else if (offset.z < 0) {
		for (int z = offset.z; z < 0; ++z) {
			current = current->neighbors_[ChunkNode::SOUTH];
		}
	}

    return current;
}

int World::getBlock(glm::ivec3 position) {
    ChunkNode *node = get_current_node_from_position(get_current_node_index_of_position(position));

	return node->chunk.chunk_data_[position.x % Chunk::SIZE][position.y % Chunk::SIZE][position.z % Chunk::SIZE];
}