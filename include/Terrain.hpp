#ifndef MARCHING_CUBES_HPP
#define MARCHING_CUBES_HPP

#include "Vertex.hpp"

    std::vector<Vertex> generate_terrain();

    std::vector<std::uint32_t> generate_indices();

#endif // MARCHING_CUBES_HPP