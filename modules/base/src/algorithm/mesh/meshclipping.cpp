/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/base/algorithm/mesh/meshclipping.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/geometry/edge.h>
#include <inviwo/core/datastructures/geometry/polygon.h>

namespace inviwo {

namespace meshutil {

namespace detail {

// Check point equality with threshold
inline bool equal(vec3 v1, vec3 v2, float eps) {
    return (std::fabs(v1.x - v2.x) < eps && std::fabs(v1.y - v2.y) < eps &&
            std::fabs(v1.z - v2.z) < eps);
}

// Compute barycentric coordinates/weights for
// point p (which is inside the polygon) with respect to polygons of vertices (v)
// Based on Mean Value Coordinates by Hormann/Floater
inline std::vector<float> barycentricInsidePolygon2D(vec2 p, const std::vector<vec2>& v) {

    const size_t numV = v.size();

    // Use float precision for result
    std::vector<float> baryW(numV, 0.0f);

    // Use double precision for intermediate values
    std::vector<dvec2> s(numV);
    std::vector<double> ri(numV);

    for (size_t i = 0; i < numV; ++i) {
        s[i] = v[i] - p;
        ri[i] = std::sqrt(glm::dot(s[i], s[i]));
    }

    std::vector<double> A(numV);
    std::vector<double> tanA(numV);

    for (size_t i = 0; i < numV; ++i) {
        size_t ip = (i + 1) % numV;
        A[i] = s[i].x * s[ip].y - s[ip].x * s[i].y;
        if (A[i] == 0.0) {
            if (util::almostEqual(p, v[i])) {
                baryW[i] = 1.f;
                return baryW;
            } else if (util::almostEqual(p, v[ip])) {
                baryW[ip] = 1.f;
                return baryW;
            } else {
                double l = ri[i] + ri[ip];
                baryW[i] = static_cast<float>(ri[ip] / l);
                baryW[ip] = static_cast<float>(ri[i] / l);
                return baryW;
            }
        }
        tanA[i] = (ri[i] * ri[ip] - glm::dot(s[i], s[ip])) / A[i];
    }

    double wsum = 0.0;

    for (size_t i = 0; i < numV; ++i) {
        size_t ip = (numV - 1 + i) % numV;
        double wi = 2.0 * (tanA[i] + tanA[ip]) / ri[i];
        baryW[i] = static_cast<float>(wi);
        wsum += wi;
    }

    if (std::abs(wsum) > 0.f) {
        for (size_t i = 0; i < numV; ++i) {
            double wnorm = static_cast<double>(baryW[i]) / wsum;
            baryW[i] = static_cast<float>(wnorm);
        }
    }

    for (size_t i = 0; i < numV; ++i) {
        if (std::isnan(baryW[i])) throw Exception("Mean Value Coordinate computation yield NaN");
    }

    return baryW;
}

// Compute barycentric coordinates/weights for
// point p with respect to triangle (a, b, c)
inline vec3 barycentricTriangle(vec3 p, vec3 a, vec3 b, vec3 c) {
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
    float invDenom = 1.f / (d00 * d11 - d01 * d01);
    vec3 bary;
    bary.y = (d11 * d20 - d01 * d21) * invDenom;
    bary.z = (d00 * d21 - d01 * d20) * invDenom;
    bary.x = 1.f - bary.y - bary.z;
    return bary;
}

std::vector<Edge3D> findUniqueEdges(const std::vector<Edge3D>& edges, const float EPSILON) {
    std::vector<Edge3D> uniqueEdges;
    for (const auto& edge : edges) {
        if (!equal(edge.v1, edge.v2, EPSILON)) {
            bool found = false;
            for (const auto& existingEdge : uniqueEdges) {
                if (equal(edge.v1, existingEdge.v1, EPSILON) &&
                    equal(edge.v2, existingEdge.v2, EPSILON)) {
                    found = true;
                    break;
                }
            }
            if (!found) uniqueEdges.push_back(edge);
        }
    }
    return uniqueEdges;
}

std::vector<Polygon<Edge3D>> findLoops(const std::vector<Edge3D>& edges, const float EPSILON) {
    std::vector<Polygon<Edge3D>> polygons;
    std::vector<Edge3D> connectedEdges;
    std::vector<Edge3D> unconnectEdges(edges);

    // Start with one edge, check which other edge is connected to it
    while (!unconnectEdges.empty()) {
        Edge3D currentEdge = unconnectEdges.front();
        connectedEdges.push_back(currentEdge);
        unconnectEdges.erase(unconnectEdges.begin());
        bool createdPolygon = false;

        // Search all edges for a connection
        for (size_t i = 0; i < edges.size(); ++i) {
            if (equal(edges[i].v1, currentEdge.v2, EPSILON) && edges[i].v2 != currentEdge.v1) {
                connectedEdges.push_back(Edge3D(currentEdge.v2, edges[i].v2));
                std::vector<Edge3D>::iterator it =
                    std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                currentEdge = edges[i];
                i = 0;
            } else if (equal(edges[i].v2, currentEdge.v2, EPSILON) &&
                       edges[i].v1 != currentEdge.v1) {
                connectedEdges.push_back(Edge3D(currentEdge.v2, edges[i].v1));
                std::vector<Edge3D>::iterator it =
                    std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                currentEdge = Edge3D(edges[i].v2, edges[i].v1);
                i = 0;
            }

            // Last edge connect to first edge, close the loop and make a polygon
            if (equal(connectedEdges[0].v1, currentEdge.v2, EPSILON)) {
                if (currentEdge.v1 != currentEdge.v2) {
                    connectedEdges.push_back(Edge3D(currentEdge.v2, connectedEdges[0].v1));
                    std::vector<Edge3D>::iterator it =
                        std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                    if (it != unconnectEdges.end()) unconnectEdges.erase(it);
                }

                Polygon<Edge3D> newPoly(connectedEdges.size());

                for (size_t j = 0; j < connectedEdges.size(); ++j) {
                    newPoly.at(j) = connectedEdges.at(j);
                }

                polygons.push_back(newPoly);
                connectedEdges.clear();
                createdPolygon = true;
                break;
            }

            if (i == edges.size() - 1) {
                throw Exception(
                    "Found edge, that is not connected to any other edge. This could mean, the "
                    "clipped mesh was not manifold.");
            }
        }

        if (!createdPolygon) {
            throw Exception("Could not connect edges to manifold polygon");
        }
    }

    return polygons;
}

std::vector<Polygon<Edge3D>> simplifyPolygons(const std::vector<Polygon<Edge3D>>& polygons,
                                              const float EPSILON) {
    std::vector<Polygon<Edge3D>> simplifiedPolygons;

    for (const auto& poly : polygons) {
        if (poly.size() == 0) continue;

        std::vector<Edge3D> simplifiedEdges{poly.get(0)};
        for (size_t i = 1; i < poly.size(); ++i) {
            const auto pivotEdge = &simplifiedEdges.back();
            const auto testEdge = poly.get(i);
            const auto pivotDir = glm::normalize(pivotEdge->v2 - pivotEdge->v1);
            const auto testDir = glm::normalize(testEdge.v2 - testEdge.v1);
            if (glm::abs(glm::dot(pivotDir, testDir) - 1.0f) < EPSILON) {
                pivotEdge->v2 = testEdge.v2;
            } else {
                simplifiedEdges.push_back(testEdge);
            }
        }

        Polygon<Edge3D> newPoly(simplifiedEdges.size());
        for (size_t i = 0; i < simplifiedEdges.size(); ++i) {
            newPoly.at(i) = simplifiedEdges[i];
        }
        simplifiedPolygons.push_back(newPoly);
    }

    std::vector<Polygon<Edge3D>> notDegenerated;
    for (const auto& poly : simplifiedPolygons) {
        if (poly.size() >= 3) notDegenerated.push_back(poly);
    }

    return notDegenerated;
}

}  // namespace detail

std::shared_ptr<Mesh> clipMeshAgainstPlane(const Mesh& mesh, const Plane& worldSpacePlane,
                                           bool capClippedHoles) {

    using namespace detail;

    const float EPSILON = 0.00001f;

    // Perform clipping in data space

    // Transform plane:

    auto worldToData = mesh.getCoordinateTransformer().getWorldToDataMatrix();
    auto worldToDataNormal = glm::transpose(glm::inverse(worldToData));
    auto dataSpacePos = vec3(worldToData * vec4(worldSpacePlane.getPoint(), 1.0));
    auto dataSpaceNormal =
        glm::normalize(vec3(worldToDataNormal * vec4(worldSpacePlane.getNormal(), 0.0)));

    Plane plane(dataSpacePos, dataSpaceNormal);

    // Extract vertex data from mesh:

    DrawType drawType = DrawType::Triangles;
    ConnectivityType connectivityType = ConnectivityType::Strip;
    const std::vector<vec3>* vertexList = nullptr;
    const std::vector<vec3>* texcoordlist = nullptr;
    const std::vector<vec4>* colorList = nullptr;
    const std::vector<unsigned int>* triangleList = nullptr;

    if (auto simple = dynamic_cast<const SimpleMesh*>(&mesh)) {
        vertexList = &simple->getVertexList()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &simple->getTexCoordList()->getRAMRepresentation()->getDataContainer();
        colorList = &simple->getColorList()->getRAMRepresentation()->getDataContainer();
        if (simple->getNumberOfIndicies() > 0) {
            triangleList = &simple->getIndexList()->getRAMRepresentation()->getDataContainer();
            drawType = simple->getIndexMeshInfo(0).dt;
            connectivityType = simple->getIndexMeshInfo(0).ct;
        }
    } else if (auto basic = dynamic_cast<const BasicMesh*>(&mesh)) {
        // TODO do clipping in all the index list now we only consider the first one
        vertexList = &basic->getVertices()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &basic->getTexCoords()->getRAMRepresentation()->getDataContainer();
        colorList = &basic->getColors()->getRAMRepresentation()->getDataContainer();
        if (basic->getNumberOfIndicies() > 0) {
            triangleList =
                &basic->getIndexBuffers()[0].second->getRAMRepresentation()->getDataContainer();
            drawType = basic->getIndexBuffers()[0].first.dt;
            connectivityType = basic->getIndexBuffers()[0].first.ct;
        }
    } else {
        throw Exception("Unsupported mesh type, only simple and basic meshes are supported");
    }

    if (drawType != DrawType::Triangles) {
        throw Exception("Cannot clip, need triangle mesh");
    }

    auto outputMesh = std::make_shared<SimpleMesh>(DrawType::Triangles, ConnectivityType::None);

    if (vertexList->empty()) {
        return outputMesh;  // nothing to do
    }

    // Check if we are using indices
    if (triangleList == nullptr) {
        throw Exception("Cannot clip, need mesh with indices");
    }

    if (triangleList->size() == 0) {
        return outputMesh;  // nothing to do
    }

    /* Sutherland-Hodgman Clipping
            1) Traverse each edge of each triangle
            2) For each edge with vertices [v1, v2]
                    Case 1: If v1 and v2 is inside, add v2
                    Case 2: If v1 inside and v2 outside, add intersection
                    Case 3: If v1 outside and v2 inside, add intersection and then add v2
                    Case 4: If v1 and v2 is outside, add nothing
            Observation: A clipped triangle can either contain
                    3 points (if only case 1 and 4 occurred) or
                    4 points (if case 2 and 3 occurred) or
                    0 points (if only case 4 occurred, thus no points)
            3) If 4 points, make two triangles, 0 1 2 and 0 3 2, total 6 points.
    */

    std::vector<Edge3D> intersectionsEdges;
    std::vector<std::pair<vec3, vec3>> intersectionTex;
    std::vector<std::pair<vec3, vec4>> intersectionCol;

    // Sutherland-Hodgman on one triangle
    const auto sutherlandHodgman = [&](unsigned int t, bool clockwise) {
        const auto indices = *triangleList;

        unsigned int tri[3];

        tri[0] = indices[t];

        if (clockwise) {
            tri[1] = indices[t + 2];
            tri[2] = indices[t + 1];
        } else {
            tri[1] = indices[t + 1];
            tri[2] = indices[t + 2];
        }

        std::vector<vec3> newVertices;
        std::vector<vec3> newTexCoords;
        std::vector<vec4> newColors;
        bool intersectionAdded = false;

        // Handle the 4 cases for each of the 3 edges
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;

            if (plane.isInside(vertexList->at(tri[i]))) {
                if (plane.isInside(vertexList->at(tri[j]))) {
                    // Case 1
                    // Add v2
                    newVertices.push_back(vertexList->at(tri[j]));
                    newTexCoords.push_back(texcoordlist->at(tri[j]));
                    newColors.push_back(colorList->at(tri[j]));
                } else {
                    // Case 2
                    // Add Intersection
                    const auto inter =
                        plane.getIntersection(vertexList->at(tri[i]), vertexList->at(tri[j]));
                    if (!inter.intersects_) throw Exception("Edge must intersect plane");
                    vec3 intersection = inter.intersection_;
                    newVertices.push_back(intersection);
                    vec3 interBC =
                        barycentricTriangle(intersection, vertexList->at(tri[0]),
                                            vertexList->at(tri[1]), vertexList->at(tri[2]));
                    vec3 interTex = (texcoordlist->at(tri[0]) * interBC.x) +
                                    (texcoordlist->at(tri[1]) * interBC.y) +
                                    (texcoordlist->at(tri[2]) * interBC.z);
                    newTexCoords.push_back(interTex);
                    vec4 interCol = (colorList->at(tri[0]) * interBC.x) +
                                    (colorList->at(tri[1]) * interBC.y) +
                                    (colorList->at(tri[2]) * interBC.z);
                    newColors.push_back(interCol);
                    intersectionTex.push_back(std::make_pair(intersection, interTex));
                    intersectionCol.push_back(std::make_pair(intersection, interCol));

                    // We save the intersection as part of edge on the clipping plane
                    if (intersectionAdded)
                        intersectionsEdges.back().v1 = intersection;
                    else {
                        intersectionsEdges.push_back(Edge3D(intersection));
                        intersectionAdded = true;
                    }
                }
            } else {
                if (plane.isInside(vertexList->at(tri[j]))) {
                    // Case 3
                    // Add Intersection
                    const auto inter =
                        plane.getIntersection(vertexList->at(tri[i]), vertexList->at(tri[j]));
                    if (!inter.intersects_) throw Exception("Edge must intersect plane");
                    vec3 intersection = inter.intersection_;
                    newVertices.push_back(intersection);
                    vec3 interBC =
                        barycentricTriangle(intersection, vertexList->at(tri[0]),
                                            vertexList->at(tri[1]), vertexList->at(tri[2]));
                    vec3 interTex = (texcoordlist->at(tri[0]) * interBC.x) +
                                    (texcoordlist->at(tri[1]) * interBC.y) +
                                    (texcoordlist->at(tri[2]) * interBC.z);
                    newTexCoords.push_back(interTex);
                    vec4 interCol = (colorList->at(tri[0]) * interBC.x) +
                                    (colorList->at(tri[1]) * interBC.y) +
                                    (colorList->at(tri[2]) * interBC.z);
                    newColors.push_back(interCol);
                    intersectionTex.push_back(std::make_pair(intersection, interTex));
                    intersectionCol.push_back(std::make_pair(intersection, interCol));

                    // We save the intersection as part of edge on the clipping plane
                    if (intersectionAdded)
                        intersectionsEdges.back().v2 = intersection;
                    else {
                        intersectionsEdges.push_back(Edge3D(intersection));
                        intersectionAdded = true;
                    }

                    // Add v2
                    newVertices.push_back(vertexList->at(tri[j]));
                    newTexCoords.push_back(texcoordlist->at(tri[j]));
                    newColors.push_back(colorList->at(tri[j]));
                }

                // Case 4
            }
        }

        // Handle more then 3 vertices
        if (newVertices.size() > 3) {
            if (newVertices.size() >= 5)
                throw Exception("Can't handle " + std::to_string(newVertices.size()) +
                                " vertices after clipping");
            vec3 lastVert = newVertices.at(3);
            vec3 lastTexc = newTexCoords.at(3);
            vec4 lastColor = newColors.at(3);
            newVertices.pop_back();
            newTexCoords.pop_back();
            newColors.pop_back();
            newVertices.push_back(newVertices.at(0));
            newTexCoords.push_back(newTexCoords.at(0));
            newColors.push_back(newColors.at(0));
            newVertices.push_back(newVertices.at(2));
            newTexCoords.push_back(newTexCoords.at(2));
            newColors.push_back(newColors.at(2));
            newVertices.push_back(lastVert);
            newTexCoords.push_back(lastTexc);
            newColors.push_back(lastColor);
        }

        // Add vertices to mesh
        for (size_t i = 0; i < newVertices.size(); ++i) {
            outputMesh->addIndex(
                outputMesh->addVertex(newVertices.at(i), newTexCoords.at(i), newColors.at(i)));
        }
    };

    // Check if it is a Triangle Strip
    if (connectivityType == ConnectivityType::Strip) {
        for (unsigned int t = 0; t < triangleList->size() - 2; ++t) {
            sutherlandHodgman(t, t & 1);
        }
    } else if (connectivityType == ConnectivityType::None) {
        for (unsigned int t = 0; t < triangleList->size() - 2; t += 3) {
            sutherlandHodgman(t, false);
        }
    } else {
        throw Exception("Cannot clip, need triangle connectivity Strip or None");
    }

    if (!capClippedHoles) {
        return outputMesh;  // mesh with hole, i.e. outside vertices replaced by intersections
    }

    // =======================================================================================

    // Find unique edges that surround the hole(s):

    // Triangulation can get very bad very quickly, when clipping a mesh successively,
    // i.e. we get very thin triangles. Depending on the value of EPSILON,
    // too many edges are discarded, because they are found to be the same (result: missing faces!),
    // or we cannot find closed edge loops anymore. When EPSILON is really small, we find loops with
    // less than 3 edges, i.e. not polygons anymore.

    // Possible solution: Find unique edges with low epsilon, find loops with high epsilon!

    const auto uniqueintersectionsEdges = findUniqueEdges(intersectionsEdges, EPSILON);

    if (uniqueintersectionsEdges.empty()) {
        return outputMesh;
    }

    // Triangulate hole:

    // With convex input we could assume only one hole, that is a convex, non-self-intersecting,
    // closed polygon, but generally we have to find edge groups that form loops.

    const auto loops = findLoops(uniqueintersectionsEdges, EPSILON);

    // Simplify polygons, e.g. merging successive straight edges,
    // to prevent triangulation becoming too bad, espescially when clipping a mesh multiple times.

    const auto polygons = simplifyPolygons(loops, EPSILON);

    // Calculate uv basis.
    const auto u = vec3(polygons[0].get(0).v2 - polygons[0].get(0).v1);
    const auto v = glm::cross(plane.getNormal(), u);

    // Add new polygons as triangles to the mesh.
    // Interpolating tex coords and color using barycentric coordinates.
    for (size_t p = 0; p < polygons.size(); ++p) {
        const size_t pSize = polygons[p].size();

        std::vector<vec2> uv;
        std::vector<vec3> tex;
        std::vector<vec4> col;

        for (size_t i = 0; i < pSize; ++i) {
            // Calculate u-v plane coordinates of the vertex on the polygon
            uv.push_back(
                vec2(glm::dot(u, polygons[p].get(i).v1), glm::dot(v, polygons[p].get(i).v1)));

            // Lookup texcoord and colors for the vertex of the polygon
            for (size_t t = 0; t < intersectionTex.size(); ++t) {
                if (intersectionTex.at(t).first == polygons[p].get(i).v1) {
                    tex.push_back(intersectionTex.at(t).second);
                    col.push_back(intersectionCol.at(t).second);
                    break;
                }
            }
        }

        uv.pop_back();

        std::vector<vec3> newVertices;
        std::vector<vec3> newTexCoords;
        std::vector<vec4> newColors;

        // For polygons with less than 5 unique points, we could do better triangulation without the
        // centroid, but the following works in any case.

        // Calculate fake centroid
        // (this will not give us the most optimal triangles,
        // but it is sufficient to close the hole)
        vec3 fakeCentroid(0.0f);
        {
            vec3 p1 = polygons[p].get(0).v1;
            vec3 p2 = p1;
            for (size_t i = 1; i < polygons[p].size(); i++) {
                const auto edge = polygons[p].get(i);
                if (glm::distance(p1, edge.v1) > glm::distance(p1, p2)) p2 = edge.v1;
                if (glm::distance(p1, edge.v2) > glm::distance(p1, p2)) p2 = edge.v2;
            }
            fakeCentroid = p1 + 0.5f * (p2 - p1);
        }

        // Calculate barycentric coordinates (weights) for all the vertices based on
        // centroid.
        vec2 uvC(glm::dot(u, fakeCentroid), glm::dot(v, fakeCentroid));

        // Barycentrics: uvC should not be too similar to any other coordinate in uv,
        // otherwise there could be problems in the interpolated colors and texcoords
        std::vector<float> baryW;
        barycentricInsidePolygon2D(uvC, uv, baryW);

        vec3 texC(0.f);
        vec4 colC(0.f);
        for (size_t i = 0; i < pSize - 1; ++i) {
            texC += tex[i] * baryW[i];
            colC += col[i] * baryW[i];
        }

        for (size_t i = 0; i < pSize; ++i) {
            size_t ip = (i + 1) % pSize;
            newVertices.push_back(fakeCentroid);
            newVertices.push_back(polygons[p].get(i).v2);
            newVertices.push_back(polygons[p].get(i).v1);

            newTexCoords.push_back(texC);
            newTexCoords.push_back(tex[ip]);
            newTexCoords.push_back(tex[i]);

            newColors.push_back(colC);
            newColors.push_back(col[ip]);
            newColors.push_back(col[i]);
        }

        // Add triangles to the mesh
        for (size_t i = 0; i < newVertices.size(); ++i) {
            outputMesh->addIndex(
                outputMesh->addVertex(newVertices[i], newTexCoords[i], newColors[i]));
        }
    }

    return outputMesh;
}

}  // namespace meshutil

}  // namespace inviwo
