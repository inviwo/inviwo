#pragma once

#include "vec2_indexed.h"
#include "edge.h"
#include "triangle.h"

#include <vector>
#include <algorithm>
#include <limits>

class Delaunay
{
    public:
        const std::vector<Triangle>& triangulate(const std::vector<Vec2Indexed>& vertices)
        {
            // Store the vertices locally
            _vertices = vertices;

            // Determinate the super triangle
            auto minX = vertices[0].x;
            auto minY = vertices[0].y;
            auto maxX = minX;
            auto maxY = minY;

            for(std::size_t i = 0; i < vertices.size(); ++i)
            {
                if (vertices[i].x < minX) minX = vertices[i].x;
                if (vertices[i].y < minY) minY = vertices[i].y;
                if (vertices[i].x > maxX) maxX = vertices[i].x;
                if (vertices[i].y > maxY) maxY = vertices[i].y;
            }

            const auto dx = maxX - minX;
            const auto dy = maxY - minY;
            const auto deltaMax = std::max(dx, dy);
            const auto midx = 0.5f * (minX + maxX);
            const auto midy = 0.5f * (minY + maxY);

            const Vec2Indexed p1(midx - 20.0f * deltaMax, midy - deltaMax, std::numeric_limits<size_t>::max());
            const Vec2Indexed p2(midx, midy + 20.0f * deltaMax, std::numeric_limits<size_t>::max());
            const Vec2Indexed p3(midx + 20.0f * deltaMax, midy - deltaMax, std::numeric_limits<size_t>::max());

            //std::cout << "Super triangle " << std::endl << Triangle(p1, p2, p3) << std::endl;

            // Create a list of triangles, and add the supertriangle in it
            _triangles.push_back(Triangle(p1, p2, p3));

            for(auto p = begin(vertices); p != end(vertices); p++)
            {
                //std::cout << "Traitement du point " << *p << std::endl;
                //std::cout << "_triangles contains " << _triangles.size() << " elements" << std::endl;

                std::vector<Edge> polygon;

                for(auto& t : _triangles)
                {
                    //std::cout << "Processing " << std::endl << *t << std::endl;

                    if(t.circumCircleContains(*p))
                    {
                        //std::cout << "Pushing bad triangle " << *t << std::endl;
                        t.isBad = true;
                        polygon.push_back(t.e1);
                        polygon.push_back(t.e2);
                        polygon.push_back(t.e3);
                    }
                    else
                    {
                        //std::cout << " does not contains " << *p << " in his circum center" << std::endl;
                    }
                }

                _triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [](Triangle &t){
                    return t.isBad;
                }), end(_triangles));

                for(auto e1 = begin(polygon); e1 != end(polygon); ++e1)
                {
                    for(auto e2 = e1 + 1; e2 != end(polygon); ++e2)
                    {
                        if(almost_equal(*e1, *e2))
                        {
                            e1->isBad = true;
                            e2->isBad = true;
                        }
                    }
                }

                polygon.erase(std::remove_if(begin(polygon), end(polygon), [](Edge &e){
                    return e.isBad;
                }), end(polygon));

                for(const auto e : polygon) {
                    _triangles.push_back(Triangle(e.v1, e.v2, *p));
                }
            }

            _triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [p1, p2, p3](Triangle &t){
                return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
            }), end(_triangles));

            for(const auto t : _triangles)
            {
                _edges.push_back(t.e1);
                _edges.push_back(t.e2);
                _edges.push_back(t.e3);
            }

            return _triangles;
        }

        const std::vector<Triangle>& getTriangles() const { return _triangles; };
        const std::vector<Edge>& getEdges() const { return _edges; };
        const std::vector<Vec2Indexed>& getVertices() const { return _vertices; };

    private:
        std::vector<Triangle> _triangles;
        std::vector<Edge> _edges;
        std::vector<Vec2Indexed> _vertices;
};
