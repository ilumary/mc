#include "MarchingCubes.hpp"

/*
 * Given a grid cell and an isolevel, calculate the triangular
 * facets required to represent the isosurface through the cell.
 * Return the number of triangular facets, the array "triangles"
 * will be loaded up with the vertices at most 5 triangular facets.
 * 0 will be returned if the grid cell is either totally above
 * of totally below the isolevel.
*/
std::vector<Triangle> polygonise(Gridcell grid, double isolevel) {
    int cubeindex = 0;
    glm::vec3 vertlist[12];

    /*
    * Determine the index into the edge table which
    * tells us which vertices are inside of the surface
    */
    if (grid.val[0] < isolevel) cubeindex |= 1;
    if (grid.val[1] < isolevel) cubeindex |= 2;
    if (grid.val[2] < isolevel) cubeindex |= 4;
    if (grid.val[3] < isolevel) cubeindex |= 8;
    if (grid.val[4] < isolevel) cubeindex |= 16;
    if (grid.val[5] < isolevel) cubeindex |= 32;
    if (grid.val[6] < isolevel) cubeindex |= 64;
    if (grid.val[7] < isolevel) cubeindex |= 128;

    /* Cube is entirely in/out of the surface */
    //if (edge_table[cubeindex] == 0)
        //return std::vector<Triangle>;

   /* Find the vertices where the surface intersects the cube */
    if (edge_table[cubeindex] & 1)
        vertlist[0] = VertexInterp(isolevel, grid.p[0], grid.p[1],grid.val[0], grid.val[1]);
    if (edge_table[cubeindex] & 2)
        vertlist[1] = VertexInterp(isolevel, grid.p[1], grid.p[2],grid.val[1], grid.val[2]);
    if (edge_table[cubeindex] & 4)
        vertlist[2] = VertexInterp(isolevel, grid.p[2], grid.p[3],grid.val[2], grid.val[3]);
    if (edge_table[cubeindex] & 8)
        vertlist[3] = VertexInterp(isolevel, grid.p[3], grid.p[0],grid.val[3], grid.val[0]);
    if (edge_table[cubeindex] & 16)
        vertlist[4] = VertexInterp(isolevel, grid.p[4], grid.p[5],grid.val[4], grid.val[5]);
    if (edge_table[cubeindex] & 32)
        vertlist[5] = VertexInterp(isolevel, grid.p[5], grid.p[6],grid.val[5], grid.val[6]);
    if (edge_table[cubeindex] & 64)
        vertlist[6] = VertexInterp(isolevel, grid.p[6], grid.p[7],grid.val[6], grid.val[7]);
    if (edge_table[cubeindex] & 128)
        vertlist[7] = VertexInterp(isolevel, grid.p[7], grid.p[4],grid.val[7], grid.val[4]);
    if (edge_table[cubeindex] & 256)
        vertlist[8] = VertexInterp(isolevel, grid.p[0], grid.p[4],grid.val[0], grid.val[4]);
    if (edge_table[cubeindex] & 512)
        vertlist[9] = VertexInterp(isolevel, grid.p[1], grid.p[5],grid.val[1], grid.val[5]);
    if (edge_table[cubeindex] & 1024)
        vertlist[10] = VertexInterp(isolevel, grid.p[2], grid.p[6],grid.val[2], grid.val[6]);
    if (edge_table[cubeindex] & 2048)
        vertlist[11] = VertexInterp(isolevel, grid.p[3], grid.p[7],grid.val[3], grid.val[7]);

   /* Create the triangle */
    std::vector<Triangle> triangles;
    for (int i = 0; tri_table[cubeindex][i] != -1; i += 3) {
        triangles.push_back(Triangle{.p_v = {
            Vertex{.position = vertlist[tri_table[cubeindex][i]], .normal = {0.f, 0.f, 1.f}, .color = {0.f, 1.f, 0.f}},
            Vertex{.position = vertlist[tri_table[cubeindex][i + 1]], .normal = {0.f, 0.f, 1.f}, .color = {1.f, 0.f, 0.f}},
            Vertex{.position = vertlist[tri_table[cubeindex][i + 2]], .normal = {0.f, 0.f, 1.f}, .color = {0.f, 0.f, 1.f}},
        }});

        //triangles[ntriang].p[0] = vertlist[tri_table[cubeindex][i]];
        //triangles[ntriang].p[1] = vertlist[tri_table[cubeindex][i + 1]];
        //triangles[ntriang].p[2] = vertlist[tri_table[cubeindex][i + 2]];
    }
    return triangles;
}

/*
   Linearly interpolate the position where an isosurface cuts
   an edge between two vertices, each with their own scalar value
*/
glm::vec3 VertexInterp(double isolevel, glm::vec3 p1, glm::vec3 p2, double valp1, double valp2) {
    glm::vec3 p;

    if (std::abs(isolevel - valp1) < 0.00001f)
        return p1;
    if (std::abs(isolevel - valp2) < 0.00001f)
        return p2;
    if (std::abs(valp1 - valp2) < 0.00001f)
        return p1;

    double mu = (isolevel - valp1) / (valp2 - valp1);

    p.x = p1.x + mu * (p2.x - p1.x);
    p.y = p1.y + mu * (p2.y - p1.y);
    p.z = p1.z + mu * (p2.z - p1.z);

    return p;
}

std::vector<Vertex> generate_terrain() {
    const glm::vec3 corners[] = {
        {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}, 
    };

    Gridcell cell;
    for(int i = 0; i < 8; ++i) {
        cell.p[i] = corners[i];
        cell.val[i] = std::sin(static_cast<float>(i));
    }

    auto triangles = polygonise(cell, 0);

    std::vector<Vertex> results;
    for(const auto& triangle : triangles) {
        results.push_back(triangle.p_v[0]);
        results.push_back(triangle.p_v[1]);
        results.push_back(triangle.p_v[2]);
    }
    return results;
}

VkVertexInputBindingDescription Vertex::binding_description() {
    return VkVertexInputBindingDescription {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::vector<VkVertexInputAttributeDescription> Vertex::attributes_description() {
    std::vector<VkVertexInputAttributeDescription> attributes;
    
	VkVertexInputAttributeDescription positionAttribute = {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position),
    };

	VkVertexInputAttributeDescription normalAttribute = {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, normal),
    };

	VkVertexInputAttributeDescription colorAttribute = {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color),
    };

	attributes.push_back(positionAttribute);
	attributes.push_back(normalAttribute);
	attributes.push_back(colorAttribute);
	return attributes;
}