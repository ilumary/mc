#include "ChunkNode.hpp"

ChunkNode::ChunkNode() {
    node_state_ = ChunkState::UNINITIALIZED;
    geometry_ = new ChunkMesh();
}

void ChunkNode::generateData() {
    chunk.generate_chunk_data();
    node_state_ = ChunkState::CHUNK_DATA_GENERATED;
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

	node_state_ = ChunkState::MESH_DATA_GENERATED;
	geometry_->generate(&chunk, world);
}