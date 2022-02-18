#include "ChunkNode.hpp"

ChunkNode::ChunkNode(glm::vec3 position) {
	node_position_ = position;
	chunk.set_chunk_position(position);
    node_state_ = ChunkState::UNINITIALIZED;
    geometry_ = new ChunkMesh();
}

void ChunkNode::create_neighbors(uint32_t distance, std::vector<ChunkNode*> *nodes) {
    //nodes will be created by going in circles around the root, every two corners the step distance is increased by one
    //the direction is given by the enum, every corner is plus one in enum until 4 is reached

    std::vector<ChunkNode*>().swap(*nodes);
    nodes->push_back(this);
    ChunkNode* crnt_node = this;

    fmt::print("Starting Node Generation from {} {}\n", crnt_node->node_position_.x, crnt_node->node_position_.z);

    //static array to hold new node position offset
    glm::vec3 new_pos_offset[4] = {{0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {-1, 0, 0}};

    std::vector<uint32_t> side_lengths;

    uint32_t corners = distance * 4;
    uint32_t current_direction = 0, current_side_length = 0, total_steps = 0;
    for(uint32_t co = 0; co < corners; ++co) {
        //for every corner add one to direction and for every second corner add one in side length
        current_direction = (current_direction + 1) % 4;
        if(co % 2 == 0) { current_side_length += 1; }
        side_lengths.push_back(current_side_length);

        fmt::print("New Corner. New dir: {}, side length {}\n", current_direction, current_side_length);

        for(uint32_t step = 0; step < current_side_length; ++step) {
            if(crnt_node->neighbors_[current_direction] == nullptr) {
                //create new node, connect with last node, update current node
                glm::vec3 new_pos = crnt_node->node_position_ + new_pos_offset[current_direction];
                fmt::print("  New node with pos {} {} {}\n", new_pos.x, new_pos.y, new_pos.x);

                crnt_node->neighbors_[current_direction] = new ChunkNode(new_pos);
                uint32_t opposite_direction = (current_direction + 2) % 4;
                crnt_node->neighbors_[current_direction]->neighbors_[opposite_direction] = crnt_node;
                crnt_node = crnt_node->neighbors_[current_direction];
                nodes->push_back(crnt_node);

                ++total_steps;
                //if node could have an inner neighbor, try to connect
                if(current_side_length > 1 && step < current_side_length - 1) {
                    //node could have inner neighbor, calculate distance to step back in array
                    ChunkNode* neighboring_node;
                    if(co < 4) {
                        fmt::print("    Step {}, neighboring node availible but 0\n", step);
                        neighboring_node = nodes->at(0);
                    } else {
                        uint32_t step_back = (side_lengths.at(co)) + (side_lengths.at(co - 1)) + (side_lengths.at(co - 2)) + (side_lengths.at(co - 3)) + 1;
                        uint32_t index = nodes->size() - 1 - step_back;
                        if(index < 0) { index = 0; }
                        neighboring_node = nodes->at(index);
                        fmt::print("    Step {}, neighboring node at {}\n", step, nodes->size() - 1 - step_back);
                    }

                    //determine direction for creating neighbor pointer
                    uint32_t neighbor_direction = (current_direction + 1) % 4;
                    uint32_t opposite_neighbor_direction = (neighbor_direction + 2) % 4;

                    crnt_node->neighbors_[neighbor_direction] = neighboring_node;
                    neighboring_node->neighbors_[opposite_neighbor_direction] = crnt_node;
                }
            }
        }
    }
}

void ChunkNode::create_node_neighbors_recursively(int distance, std::vector<ChunkNode*> *nodes) {

    //update_neighbor_pointers(nodes);

    //X - 1 WEST
	if (neighbors_[ChunkNode::WEST] == nullptr) {
		neighbors_[ChunkNode::WEST] = new ChunkNode({node_position_.x - 1, node_position_.y, node_position_.z});
        nodes->push_back(neighbors_[ChunkNode::WEST]);
		neighbors_[ChunkNode::WEST]->neighbors_[ChunkNode::EAST] = this;
	}
	//X + 1 EAST
	if (neighbors_[ChunkNode::EAST] == nullptr) {
		neighbors_[ChunkNode::EAST] = new ChunkNode({node_position_.x + 1, node_position_.y, node_position_.z});
        nodes->push_back(neighbors_[ChunkNode::EAST]);
		neighbors_[ChunkNode::EAST]->neighbors_[ChunkNode::WEST] = this;
	}
	//Z + 1 NORTH
	if (neighbors_[ChunkNode::NORTH] == nullptr) {
		neighbors_[ChunkNode::NORTH] = new ChunkNode({node_position_.x, node_position_.y, node_position_.z + 1});
        nodes->push_back(neighbors_[ChunkNode::NORTH]);
		neighbors_[ChunkNode::NORTH]->neighbors_[ChunkNode::SOUTH] = this;
	}
	//Z - 1 SOUTH
	if (neighbors_[ChunkNode::SOUTH] == nullptr) {
		neighbors_[ChunkNode::SOUTH] = new ChunkNode({node_position_.x, node_position_.y, node_position_.z - 1});
        nodes->push_back(neighbors_[ChunkNode::SOUTH]);
		neighbors_[ChunkNode::SOUTH]->neighbors_[ChunkNode::NORTH] = this;
	}

	//Y + 1 Down
	/*if (neighbors_[ChunkNode::UP] == nullptr) {
		neighbors_[ChunkNode::UP] = NEW ChunkNode({node_position_.x, node_position_.y + 1, node_position_.z});
		neighbors_[ChunkNode::UP]->neighbors_[ChunkNode::DOWN] = this;
	}
	//Y - 1 Up
	if (neighbors_[ChunkNode::DOWN] == nullptr) {
		neighbors_[ChunkNode::DOWN] = NEW ChunkNode({node_position_.x, node_position_.y - 1, node_position_.z});
		neighbors_[ChunkNode::DOWN]->neighbors_[ChunkNode::UP] = this;
	}*/

	if (distance > 0) {
		for (unsigned int i = 0; i < 4; i++) {
			neighbors_[i]->create_node_neighbors_recursively(distance - 1, nodes);
		}
	}
}

void ChunkNode::update_neighbor_pointers(std::vector<ChunkNode*> *nodes) {
	if (neighbors_[ChunkNode::WEST] == nullptr) {
		ChunkNode* east = search_node({node_position_.x - 1, node_position_.y, node_position_.z}, nodes);
		if (east != nullptr) {
			neighbors_[ChunkNode::WEST] = east;
		}
	}

	if (neighbors_[ChunkNode::EAST] == nullptr) {
		ChunkNode* west = search_node({ node_position_.x + 1, node_position_.y, node_position_.z }, nodes);
		if (west != nullptr) {
			neighbors_[ChunkNode::EAST] = west;
		}
	}

	if (neighbors_[ChunkNode::SOUTH] == nullptr) {
		ChunkNode* north = search_node({ node_position_.x, node_position_.y, node_position_.z - 1}, nodes);
		if (north != nullptr) {
			neighbors_[ChunkNode::SOUTH] = north;
		}
	}
	if (neighbors_[ChunkNode::NORTH] == nullptr) {
		ChunkNode* south = search_node({ node_position_.x, node_position_.y, node_position_.z + 1}, nodes);
		if (south != nullptr) {
			neighbors_[ChunkNode::NORTH] = south;
		}
	}
}

ChunkNode* ChunkNode::search_node(glm::vec3 pos, std::vector<ChunkNode *> *nodes) {
    for(uint32_t i = nodes->size(); i > 0; --i) {
        if(pos == nodes->at(i-1)->node_position_) {
            return nodes->at(i-1);
        }
    }

    return nullptr;
}

void ChunkNode::get_nodes_recursive(std::vector<ChunkNode*> *nodes, int distance) {
	//for every NEW node, search existing array if it exists, only add if not
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

	if (neighbors_[ChunkNode::WEST] != nullptr) {
		neighbors_[ChunkNode::WEST]->chunk.generate_chunk_data();
	}
	if (neighbors_[ChunkNode::EAST] != nullptr) {
		neighbors_[ChunkNode::EAST]->chunk.generate_chunk_data();
	}
	if (neighbors_[ChunkNode::NORTH] != nullptr) {
		neighbors_[ChunkNode::NORTH]->chunk.generate_chunk_data();
	}
	if (neighbors_[ChunkNode::SOUTH] != nullptr) {
		neighbors_[ChunkNode::SOUTH]->chunk.generate_chunk_data();
	}
}

Mesh* ChunkNode::getGeometry() {
    if (node_state_ < ChunkState::MESH_DATA_GENERATED) {
		generateGeometry();
	}

	return geometry_;
}

void ChunkNode::generateGeometry() {
    if (node_state_ < ChunkState::CHUNK_DATA_GENERATED){
		generateData();
	}

    if(node_state_ < SURROUND_DATA_GATHERED) {
        gather_surrounding_chunk_data();
    }

	geometry_->generate(chunk);
	node_state_ = ChunkState::MESH_DATA_GENERATED;
}

void ChunkNode::gather_surrounding_chunk_data() {
    std::cout << "Generating surrounding chunk data" << std::endl;

    //for every side of the chunk, gather corresponding data
    for(uint32_t chunk_face = 0; chunk_face < 4; ++chunk_face) {
        if(neighbors_[chunk_face] != nullptr) {
            //Caution, only made for 2d chunk world!!!
            int relevant_chunk_face = (chunk_face + 2) % 4;
            int static_chunk_var = chunk_face > 1 ? 15 : 0;

            //North or South, z is static
            if(chunk_face % 2 == 0) {
                for(int x = 0; x < Chunk::SIZE; ++x) {
                    for(int y = 0; y < Chunk::SIZE; ++y) {
                        geometry_->surround_data_[relevant_chunk_face][x][y] = neighbors_[chunk_face]->chunk.chunk_data_[x][y][static_chunk_var];
                    }
                }
            }

            //West or East, x is static
            if(chunk_face % 2 == 1) {
                for(int z = 0; z < Chunk::SIZE; ++z) {
                    for(int y = 0; y < Chunk::SIZE; ++y) {
                        geometry_->surround_data_[relevant_chunk_face][z][y] = neighbors_[chunk_face]->chunk.chunk_data_[static_chunk_var][y][z];
                    }
                }
            }
        } else {
            fmt::print("Fatal Error retrieving chunk border data for side {}", chunk_face);
        }
    }
}
