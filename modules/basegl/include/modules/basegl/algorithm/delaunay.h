#pragma once

#include <modules/basegl/algorithm/vec2_indexed.h>
#include <modules/basegl/algorithm/edge.h>
#include <modules/basegl/algorithm/triangle.h>

#include <vector>

class Delaunay
{
    public:
        const std::vector<Triangle>& triangulate(const std::vector<Vec2Indexed>& vertices);

        const std::vector<Triangle>& getTriangles() const;
        const std::vector<Edge>& getEdges() const;
        const std::vector<Vec2Indexed>& getVertices() const;

    private:
        std::vector<Triangle> _triangles;
        std::vector<Edge> _edges;
        std::vector<Vec2Indexed> _vertices;
};
