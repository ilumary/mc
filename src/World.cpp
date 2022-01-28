#include "World.hpp"

World::World() {
    root = new ChunkNode();
}

Mesh* World::getMeshes() {
    return root->getGeometry(this);
}