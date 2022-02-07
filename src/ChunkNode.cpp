#include "ChunkNode.hpp"

ChunkNode::ChunkNode(glm::vec3 position) {
	node_position_ = position;
	chunk.set_chunk_position(position);
    node_state_ = ChunkState::UNINITIALIZED;
    geometry_ = new ChunkMesh();
}

void ChunkNode::create_node_neighbors_recursively(int distance) {
	//X - 1 WEST
	if (neighbors_[ChunkNode::WEST] == nullptr) {
		neighbors_[ChunkNode::WEST] = new ChunkNode({node_position_.x - 1, node_position_.y, node_position_.z});
		neighbors_[ChunkNode::WEST]->neighbors_[ChunkNode::EAST] = this;
	}
	//X + 1 EAST
	if (neighbors_[ChunkNode::EAST] == nullptr) {
		neighbors_[ChunkNode::EAST] = new ChunkNode({node_position_.x + 1, node_position_.y, node_position_.z});
		neighbors_[ChunkNode::EAST]->neighbors_[ChunkNode::WEST] = this;
	}
	//Z + 1 NORTH
	if (neighbors_[ChunkNode::NORTH] == nullptr) {
		neighbors_[ChunkNode::NORTH] = new ChunkNode({node_position_.x, node_position_.y, node_position_.z + 1});
		neighbors_[ChunkNode::NORTH]->neighbors_[ChunkNode::SOUTH] = this;
	}
	//Z - 1 SOUTH
	if (neighbors_[ChunkNode::SOUTH] == nullptr) {
		neighbors_[ChunkNode::SOUTH] = new ChunkNode({node_position_.x, node_position_.y, node_position_.z - 1});
		neighbors_[ChunkNode::SOUTH]->neighbors_[ChunkNode::NORTH] = this;
	}

	//Y + 1 Down
	/*if (neighbors_[ChunkNode::UP] == nullptr) {
		neighbors_[ChunkNode::UP] = new ChunkNode({node_position_.x, node_position_.y + 1, node_position_.z});
		neighbors_[ChunkNode::UP]->neighbors_[ChunkNode::DOWN] = this;
	}
	//Y - 1 Up
	if (neighbors_[ChunkNode::DOWN] == nullptr) {
		neighbors_[ChunkNode::DOWN] = new ChunkNode({node_position_.x, node_position_.y - 1, node_position_.z});
		neighbors_[ChunkNode::DOWN]->neighbors_[ChunkNode::UP] = this;
	}*/

	if (distance > 0) {
		for (unsigned int i = 0; i < 4; i++) {
			neighbors_[i]->create_node_neighbors_recursively(distance - 1);
		}
	}
}

void ChunkNode::get_nodes_recursive(std::vector<ChunkNode*> *nodes, int distance) {
	//for every new node, search existing array if it exists, only add if not
	bool found = false;
	for (unsigned int i = 0; i < nodes->size(); i++) {
		if ((*nodes)[i]->node_position_ == node_position_) {
			found = true;
			break;
		}
	}
	
	if (found == false) {
		nodes->push_back(this);
	}

	if (distance > 0) {
		for (unsigned int i = 0; i < 4; i++) {
			if(neighbors_[i] != nullptr) {
				neighbors_[i]->get_nodes_recursive(nodes, distance - 1);
			}
		}
	}
}

//add chunk gen for direkt neighbors
void ChunkNode::generateData() {
    chunk.generate_chunk_data();
	node_state_ = ChunkState::CHUNK_DATA_GENERATED;

	//X - 1 WEST
	if (neighbors_[ChunkNode::WEST] != nullptr) {
		neighbors_[ChunkNode::WEST]->chunk.generate_chunk_data();
	}
	//X + 1 EAST
	if (neighbors_[ChunkNode::EAST] != nullptr) {
		neighbors_[ChunkNode::EAST]->chunk.generate_chunk_data();
	}
	//Z + 1 NORTH
	if (neighbors_[ChunkNode::NORTH] != nullptr) {
		neighbors_[ChunkNode::NORTH]->chunk.generate_chunk_data();
	}
	//Z - 1 SOUTH
	if (neighbors_[ChunkNode::SOUTH] != nullptr) {
		neighbors_[ChunkNode::SOUTH]->chunk.generate_chunk_data();
	}
}

Mesh* ChunkNode::getGeometry(World* world) {
    if (node_state_ < ChunkState::MESH_DATA_GENERATED) {
		generateGeometry(world);
	}

	return geometry_;
}

void ChunkNode::generateGeometry(World* world) {
    if (node_state_ < ChunkState::CHUNK_DATA_GENERATED){
		generateData();
	}

	geometry_->generate(chunk, world);
	node_state_ = ChunkState::MESH_DATA_GENERATED;
}