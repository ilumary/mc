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
    std::vector<ChunkNode*> nodes;

    if(current != nullptr) {
        current->create_node_neighbors_recursively(distance + 1, &nodes);
    }

    //clear node vector
    nodes.clear();

    //get all neighbors
    current->get_nodes_recursive(&nodes, distance);

    fmt::print("Got {} nodes: [\n", nodes.size());
    for(int i = 0; i < nodes.size(); i++) {
        fmt::print("    Node No {} with pos {} {} {}\n", i, nodes[i]->node_position_.x, nodes[i]->node_position_.y, nodes[i]->node_position_.z);
    }
    fmt::print("]\n");

    //get all neighbors meshes
    std::vector<Mesh*> meshes;
    for(std::size_t i = 0; i < nodes.size(); ++i) {
        meshes.push_back(nodes[i]->getGeometry());
        fmt::print("Pushing back {} vertices to node with pos {} {} {}\n", nodes[i]->geometry_->vertices.size(), nodes[i]->node_position_.x, nodes[i]->node_position_.y, nodes[i]->node_position_.z);
    }
    fmt::print("Merging {} meshes\n", meshes.size());

    //merge meshes
    for(std::size_t i = 0; i < meshes.size(); ++i) {
        mesh_.merge(meshes.at(i));
    }

    fmt::print("Returning {} vertices and {} indices to draw\n", mesh_.vertices.size(), mesh_.indices.size());

    return &mesh_;
}

glm::vec3 World::get_current_node_index_of_position(glm::vec3 position) {
    int x = position.x >= 0 ? (position.x / Chunk::SIZE) : (position.x / Chunk::SIZE - 1);
	int y = position.y <= 0 ? (position.y / Chunk::SIZE) : (position.y / Chunk::SIZE);
	int z = position.z >= 0 ? (position.z / Chunk::SIZE) : (position.z / Chunk::SIZE - 1);

 	return { x, y, z };
}

ChunkNode* World::get_current_node_from_position(glm::vec3 position) {
    glm::ivec3 offset = current->node_position_ - position;

    // offset x direction
    if (offset.x > 0) {
		for (int x = 0; x < offset.x; ++x) {
            if(current->neighbors_[ChunkNode::WEST] != nullptr) {
                current = current->neighbors_[ChunkNode::WEST];
            }
		}
	} else if (offset.x < 0) {
		for (int x = offset.x; x < 0; ++x) {
            if(current->neighbors_[ChunkNode::EAST] != nullptr) {
                current = current->neighbors_[ChunkNode::EAST];
            }
		}
	}

    //offset z direction
    if (offset.z > 0) {
		for (int z = 0; z < offset.z; ++z) {
            if(current->neighbors_[ChunkNode::SOUTH] != nullptr) {
                current = current->neighbors_[ChunkNode::SOUTH];
            }

		}
	} else if (offset.z < 0) {
		for (int z = offset.z; z < 0; ++z) {
            if(current->neighbors_[ChunkNode::NORTH] != nullptr) {
                current = current->neighbors_[ChunkNode::NORTH];
            }

		}
	}

    return current;
}
