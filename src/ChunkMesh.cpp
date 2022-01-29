#include "ChunkMesh.hpp"

glm::vec4 ChunkMesh::calculate_uv(int x, int y, int size) {
    float step = 1.0 / size;
	return glm::vec4(step * x, step * y, step * x + step, step * y + step);
}

void ChunkMesh::generate(Chunk* chunk, World* world) {
    vertices.clear();
	indices.clear();

	int size = 0;

	glm::ivec3 start = { chunk->chunk_position_.x * Chunk::SIZE, chunk->chunk_position_.y * Chunk::SIZE, chunk->chunk_position_.z * Chunk::SIZE };

	for (int x = 0; x < Chunk::SIZE; ++x) {
		for (int z = 0; z < Chunk::SIZE; ++z) {
			for (int y = 0; y < Chunk::SIZE; ++y) {
				int value = chunk->chunk_data_[x][y][z];

				if (value != Chunk::BlockType::CLEAR) {
					float ix = x + start.x;
					float iy = (y * -1) - start.y;
					float iz = (z * -1) + start.z;
						
					float px = 1.0f + ix;
					float py = iy - 1.0f;
					float pz = 1.0f + iz;
						
					float nx = ix;
					float ny = iy;
					float nz = iz;
					
					//Back Face
					if (z == Chunk::SIZE - 1 || chunk->chunk_data_[x][y][z + 1] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);
						
						vertices.push_back({ { nx, ny, nz },{ 0, 0, -1 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { nx, py, nz },{ 0, 0, -1 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { px, py, nz },{ 0, 0, -1 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { px, ny, nz },{ 0, 0, -1 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}

					//Front Face
					if (z == 0 || chunk->chunk_data_[x][y][z - 1] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);

						vertices.push_back({ { px, ny, pz },{ 0, 0, 1 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { px, py, pz },{ 0, 0, 1 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { nx, py, pz },{ 0, 0, 1 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { nx, ny, pz },{ 0, 0, 1 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}

					//Top Face
					if (y == (Chunk::SIZE * -1) + 1 || chunk->chunk_data_[x][y + 1][z] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);

						vertices.push_back({ { nx, py, nz },{ 0, -1, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { nx, py, pz },{ 0, -1, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { px, py, pz },{ 0, -1, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { px, py, nz },{ 0, -1, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}

					//Bottom face
					if (y == 0 || chunk->chunk_data_[x][y - 1][z] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);

						vertices.push_back({ { nx, ny, pz },{ 0, 1, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { nx, ny, nz },{ 0, 1, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { px, ny, nz },{ 0, 1, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { px, ny, pz },{ 0, 1, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}

					//Right face
					if (x == Chunk::SIZE - 1 || chunk->chunk_data_[x + 1][y][z] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);

						vertices.push_back({ { px, ny, nz },{ 1, 0, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { px, py, nz },{ 1, 0, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { px, py, pz },{ 1, 0, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { px, ny, pz },{ 1, 0, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}

					//Left face
					if (x == 0 || chunk->chunk_data_[x - 1][y][z] == Chunk::BlockType::CLEAR) {
						indices.push_back(size);
						indices.push_back(size + 3);
						indices.push_back(size + 2);
						indices.push_back(size + 2);
						indices.push_back(size + 1);
						indices.push_back(size);

						vertices.push_back({ { nx, ny, pz },{ -1, 0, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].w } });
						vertices.push_back({ { nx, py, pz },{ -1, 0, 0 },{ block_uvs[value - 1].x, block_uvs[value - 1].y } });
						vertices.push_back({ { nx, py, nz },{ -1, 0, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].y } });
						vertices.push_back({ { nx, ny, nz },{ -1, 0, 0 },{ block_uvs[value - 1].z, block_uvs[value - 1].w } });
						size += 4;
					}
				}
			}
		}
	}
}