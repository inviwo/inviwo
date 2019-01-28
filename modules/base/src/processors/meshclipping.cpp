/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#include <modules/base/processors/meshclipping.h>
#include <inviwo/core/datastructures/geometry/basicmesh.h>
#include <inviwo/core/datastructures/geometry/edge.h>
#include <inviwo/core/datastructures/geometry/polygon.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <vector>

namespace inviwo {

const ProcessorInfo MeshClipping::processorInfo_{
    "org.inviwo.MeshClipping",  // Class identifier
    "Mesh Clipping",            // Display name
    "Mesh Creation",            // Category
    CodeState::Experimental,    // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo MeshClipping::getProcessorInfo() const { return processorInfo_; }

const float MeshClipping::EPSILON = 0.00001f;

MeshClipping::MeshClipping()
    : Processor()
    , inport_("inputMesh")
    , outport_("clippedMesh")
    , clippingPlane_("clippingPlane")
    , clippingEnabled_("clippingEnabled", "Enable clipping", false)
    , movePointAlongNormal_("movePointAlongNormal", "Move Plane Point Along Normal", false,
                            InvalidationLevel::Valid)
    , moveCameraAlongNormal_("moveCameraAlongNormal", "Move Camera Along Normal", true,
                             InvalidationLevel::Valid)
    , pointPlaneMove_("pointPlaneMove", "Plane Point Along Normal Move", 0.f, 0.f, 2.f, 0.01f)
    , planePoint_("planePoint", "Plane Point", vec3(0.0f), vec3(-10000.0f), vec3(10000.0f),
                  vec3(0.1f))
    , planeNormal_("planeNormal", "Plane Normal", vec3(0.0f, 0.0f, -1.0f), vec3(-1.0f), vec3(1.0f),
                   vec3(0.1f))
    , alignPlaneNormalToCameraNormal_("alignPlaneNormalToCameraNormal",
                                      "Align Plane Normal To Camera Normal",
                                      InvalidationLevel::Valid)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid) {
    addPort(inport_);
    addPort(outport_);
    addPort(clippingPlane_);
    addProperty(clippingEnabled_);
    addProperty(movePointAlongNormal_);
    addProperty(moveCameraAlongNormal_);
    addProperty(pointPlaneMove_);

    addProperty(planePoint_);
    addProperty(planeNormal_);

    addProperty(alignPlaneNormalToCameraNormal_);
    alignPlaneNormalToCameraNormal_.onChange(
        [this]() { onAlignPlaneNormalToCameraNormalPressed(); });

    addProperty(camera_);

    auto onMovePointAlongNormalToggled = [this]() {
        planePoint_.setReadOnly(movePointAlongNormal_.get());
        pointPlaneMove_.set(0.f);
        previousPointPlaneMove_ = 0.f;
        pointPlaneMove_.setVisible(movePointAlongNormal_.get());
    };

    movePointAlongNormal_.onChange(onMovePointAlongNormalToggled);
    onMovePointAlongNormalToggled();
}

MeshClipping::~MeshClipping() = default;

void MeshClipping::process() {
    /** Process overview
     *   - Take axis-aligned bounding box (AABB) mesh as input.
     *   - Call clipGeometryAgainstPlane(...) with input and plane_ as arguments
     *   - Extract and store an edge list from the input mesh's triangle list
     *   - Start with an empty outputList and empty outputEdgeList. Iterate over the edge list
     *     extracted from the triangle list and check each edge against the clip plane. Store verts
     *     and edges in outputList and outputEdgeList as you go.
     *   - Use outputEdgeList and, indirectly, outputList to rebuild a correctly sorted
     *     triangle strip list.
     *   - Build new mesh from the triangle strip list and return it.
     */
    auto plane = std::make_shared<Plane>(planePoint_.get(), planeNormal_.get());

    if (clippingEnabled_.get()) {

        if (movePointAlongNormal_.get()) {
            // Set new plane position based on offset
            vec3 offsetPlaneDiff = plane->getNormal() * pointPlaneMove_.get();
            plane->setPoint(plane->getPoint() + offsetPlaneDiff);
            // Move camera along the offset as well
            if (moveCameraAlongNormal_.get()) {

                float planeMoveDiff = pointPlaneMove_.get() - previousPointPlaneMove_;
                // Move camera half of the plane movement distance.
                // Ensures that lookAt position is centered in the mesh,
                // assuming that initial lookAt position is in the center of the mesh.
                vec3 lookOffset = plane->getNormal() * planeMoveDiff * 0.5f;
                camera_.setLook(camera_.getLookFrom() + lookOffset,
                                camera_.getLookTo() + lookOffset, camera_.getLookUp());
            }
            previousPointPlaneMove_ = pointPlaneMove_.get();
        }
        if (auto clippedPlaneGeom = clipGeometryAgainstPlane(inport_.getData().get(), *plane)) {
            clippedPlaneGeom->setModelMatrix(inport_.getData()->getModelMatrix());
            clippedPlaneGeom->setWorldMatrix(inport_.getData()->getWorldMatrix());
            outport_.setData(clippedPlaneGeom);
        } else {
            outport_.setData(inport_.getData());
        }
    } else {
        outport_.setData(inport_.getData());
    }
    clippingPlane_.setData(plane);
}

void MeshClipping::onAlignPlaneNormalToCameraNormalPressed() {
    planeNormal_.set(glm::normalize(camera_.getLookTo() - camera_.getLookFrom()));

    // Calculate new plane point by finding the closest geometry point to the camera
    auto geom = inport_.getData();

    auto it = util::find_if(geom->getBuffers(), [](const auto& buf) {
        return buf.first.type == BufferType::PositionAttrib;
    });
    if (it == geom->getBuffers().end()) {
        LogError("Unsupported mesh, no buffers with the Position Attribute found");
        return;
    }

    auto& camera = camera_.get();
    auto direction = glm::normalize(camera.getDirection());
    auto nearPos = camera.getLookFrom() + camera.getNearPlaneDist() * direction;
    // Transform coordinates to data space
    auto worldToData = geom->getCoordinateTransformer().getWorldToDataMatrix();
    auto worldToDataNormal = glm::transpose(glm::inverse(worldToData));
    auto dataSpacePos = vec3(worldToData * vec4(nearPos, 1.0));
    auto dataSpaceNormal = glm::normalize(vec3(worldToDataNormal * vec4(direction, 0.0)));
    // Plane start/end position based on distance to camera near plane
    Plane nearPlane(dataSpacePos, dataSpaceNormal);

    // Align clipping plane to camera and make sure it starts and ends on the mesh boundaries.
    // Start point will be on the camera near plane if it is inside the mesh.
    const auto ram = it->second->getRepresentation<BufferRAM>();
    if (ram && ram->getDataFormat()->getComponents() == 3) {
        ram->dispatch<void, dispatching::filter::Float3s>([&](auto pb) -> void {
            const auto& vertexList = pb->getDataContainer();
            // Get closest and furthest vertex with respect to the camera near plane
            auto minMaxVertices =
                std::minmax_element(std::begin(vertexList), std::end(vertexList),
                                    [&nearPlane](const auto& a, const auto& b) {
                                        // Use max(0, dist) to make sure we do not consider vertices
                                        // behind plane
                                        return std::max(0.f, nearPlane.distance(a)) <
                                               std::max(0.f, nearPlane.distance(b));
                                    });
            auto minDist = nearPlane.distance(*minMaxVertices.first);
            auto maxDist = nearPlane.distance(*minMaxVertices.second);

            auto closestVertex = minDist * nearPlane.getNormal() + nearPlane.getPoint();
            auto farVertex = maxDist * nearPlane.getNormal() + nearPlane.getPoint();
            auto closestWorldSpacePos = vec3(
                geom->getCoordinateTransformer().getDataToWorldMatrix() * vec4(closestVertex, 1.f));
            auto farWorldSpacePos = vec3(geom->getCoordinateTransformer().getDataToWorldMatrix() *
                                         vec4(farVertex, 1.f));
            auto range = glm::abs((farWorldSpacePos - closestWorldSpacePos));
            auto minVal = glm::min(closestWorldSpacePos, farWorldSpacePos);
            auto maxVal = glm::max(closestWorldSpacePos, farWorldSpacePos);
            planePoint_.set(closestWorldSpacePos, minVal, maxVal, range * 0.1f);
            pointPlaneMove_.setMaxValue(glm::distance(farWorldSpacePos, closestWorldSpacePos));
        });

    } else {
        LogError("Unsupported mesh, only 3D meshes supported");
    }
}

// Check point equality with threshold
inline bool equal(vec3 v1, vec3 v2, float eps) {
    return (std::fabs(v1.x - v2.x) < eps && std::fabs(v1.y - v2.y) < eps &&
            std::fabs(v1.z - v2.z) < eps);
}

// Compute barycentric coordinates/weights for
// point p (which is inside the polygon) with respect to polygons of vertices (v)
// Based on Mean Value Coordinates by Hormann/Floater
inline void barycentricInsidePolygon2D(vec2 p, const std::vector<vec2>& v,
                                       std::vector<float>& baryW) {
    size_t numV = v.size();
    std::vector<vec2> s(numV);
    std::vector<float> ri(numV);

    for (size_t i = 0; i < numV; ++i) {
        s[i] = v[i] - p;
        ri[i] = std::sqrt(glm::dot(s[i], s[i]));
    }

    size_t ip;
    float Ai;
    std::vector<float> tanA(numV);

    for (size_t i = 0; i < numV; ++i) {
        ip = (i + 1) % numV;
        Ai = (s[i].x * s[ip].y - s[ip].x * s[i].y);
        tanA[i] = (ri[i] * ri[ip] - glm::dot(s[i], s[ip])) / Ai;
    }

    baryW.resize(numV);

    for (size_t i = 0; i < numV; ++i) {
        baryW[i] = 0.f;
    }

    float wi;
    float wsum = 0.f;

    for (size_t i = 0; i < numV; ++i) {
        ip = (numV - 1 + i) % numV;
        wi = 2.f * (tanA[i] + tanA[ip]) / ri[i];
        wsum += wi;
        baryW[i] = wi;
    }

    if (std::fabs(wsum) > 0.f) {
        for (size_t i = 0; i < numV; ++i) {
            baryW[i] /= wsum;
        }
    }
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

std::shared_ptr<Mesh> MeshClipping::clipGeometryAgainstPlane(const Mesh* in,
                                                             const Plane& worldSpacePlane) {
    // Perform clipping in data space
    auto worldToData = in->getCoordinateTransformer().getWorldToDataMatrix();
    auto worldToDataNormal = glm::transpose(glm::inverse(worldToData));
    auto dataSpacePos = vec3(worldToData * vec4(worldSpacePlane.getPoint(), 1.0));
    auto dataSpaceNormal =
        glm::normalize(vec3(worldToDataNormal * vec4(worldSpacePlane.getNormal(), 0.0)));
    Plane plane(dataSpacePos, dataSpaceNormal);

    ConnectivityType indexAttrInfo;
    const std::vector<vec3>* vertexList;
    const std::vector<vec3>* texcoordlist;
    const std::vector<vec4>* colorList;
    const std::vector<unsigned int>* triangleList;

    if (auto simple = dynamic_cast<const SimpleMesh*>(in)) {
        vertexList = &simple->getVertexList()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &simple->getTexCoordList()->getRAMRepresentation()->getDataContainer();
        colorList = &simple->getColorList()->getRAMRepresentation()->getDataContainer();
        triangleList = &simple->getIndexList()->getRAMRepresentation()->getDataContainer();
        indexAttrInfo = simple->getIndexMeshInfo(0).ct;
    } else if (auto basic = dynamic_cast<const BasicMesh*>(in)) {
        // TODO do clipping in all the index list now we only consider the first one
        vertexList = &basic->getVertices()->getRAMRepresentation()->getDataContainer();
        texcoordlist = &basic->getTexCoords()->getRAMRepresentation()->getDataContainer();
        colorList = &basic->getColors()->getRAMRepresentation()->getDataContainer();
        triangleList =
            &basic->getIndexBuffers()[0].second->getRAMRepresentation()->getDataContainer();
        indexAttrInfo = basic->getIndexBuffers()[0].first.ct;
    } else {
        throw Exception("Unsupported mesh type, only simple and basic meshes are supported");
    }

    auto outputMesh = std::make_shared<SimpleMesh>(DrawType::Triangles);

    // Check if we are using indices
    if (triangleList->size() > 0) {
        // Check if it is a Triangle Strip
        if (indexAttrInfo == ConnectivityType::Strip) {
            // Iterate over edges by edge
            unsigned int idx[3];
            std::vector<vec3> newVertices;
            std::vector<vec3> newTexCoords;
            std::vector<vec4> newColors;
            std::vector<Edge3D> intersectionsEdges;
            std::vector<std::pair<vec3, vec3>> intersectionTex;
            std::vector<std::pair<vec3, vec4>> intersectionCol;

            for (unsigned int t = 0; t < triangleList->size() - 2; ++t) {
                idx[0] = triangleList->at(t);

                // Clockwise
                if (t & 1) {
                    idx[1] = triangleList->at(t + 2);
                    idx[2] = triangleList->at(t + 1);
                } else {
                    idx[1] = triangleList->at(t + 1);
                    idx[2] = triangleList->at(t + 2);
                }

                /* Sutherland-Hodgman Clipping
                   1) Traverse each edge by edge, such as successive vertex pairs make up edges
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
                newVertices.clear();
                newTexCoords.clear();
                newColors.clear();
                bool intersectionAdded = false;

                for (size_t i = 0; i < 3; ++i) {
                    size_t j = (i == 2 ? 0 : i + 1);

                    if (plane.isInside(vertexList->at(idx[i]))) {
                        if (plane.isInside(vertexList->at(idx[j]))) {  // Case 1
                            // Add v2
                            newVertices.push_back(vertexList->at(idx[j]));
                            newTexCoords.push_back(texcoordlist->at(idx[j]));
                            newColors.push_back(colorList->at(idx[j]));
                        } else {  // Case 2
                            // Add Intersection
                            vec3 intersection =
                                plane
                                    .getIntersection(vertexList->at(idx[i]), vertexList->at(idx[j]))
                                    .intersection_;
                            newVertices.push_back(intersection);
                            vec3 interBC =
                                barycentricTriangle(intersection, vertexList->at(idx[0]),
                                                    vertexList->at(idx[1]), vertexList->at(idx[2]));
                            vec3 interTex = (texcoordlist->at(idx[0]) * interBC.x) +
                                            (texcoordlist->at(idx[1]) * interBC.y) +
                                            (texcoordlist->at(idx[2]) * interBC.z);
                            newTexCoords.push_back(interTex);
                            vec4 interCol = (colorList->at(idx[0]) * interBC.x) +
                                            (colorList->at(idx[1]) * interBC.y) +
                                            (colorList->at(idx[2]) * interBC.z);
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
                        if (plane.isInside(vertexList->at(idx[j]))) {  // Case 3
                            // Add Intersection
                            vec3 intersection =
                                plane
                                    .getIntersection(vertexList->at(idx[i]), vertexList->at(idx[j]))
                                    .intersection_;
                            newVertices.push_back(intersection);
                            vec3 interBC =
                                barycentricTriangle(intersection, vertexList->at(idx[0]),
                                                    vertexList->at(idx[1]), vertexList->at(idx[2]));
                            vec3 interTex = (texcoordlist->at(idx[0]) * interBC.x) +
                                            (texcoordlist->at(idx[1]) * interBC.y) +
                                            (texcoordlist->at(idx[2]) * interBC.z);
                            newTexCoords.push_back(interTex);
                            vec4 interCol = (colorList->at(idx[0]) * interBC.x) +
                                            (colorList->at(idx[1]) * interBC.y) +
                                            (colorList->at(idx[2]) * interBC.z);
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
                            newVertices.push_back(vertexList->at(idx[j]));
                            newTexCoords.push_back(texcoordlist->at(idx[j]));
                            newColors.push_back(colorList->at(idx[j]));
                        }

                        // Case 4
                    }
                }

                // Handle more then 3 vertices
                if (newVertices.size() > 3) {
                    ivwAssert(newVertices.size() < 5,
                              "Can't handle " << newVertices.size() << " vertices after clipping");
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
                    outputMesh->addVertex(newVertices.at(i), newVertices.at(i), newColors.at(i));
                }
            }

            // Based on intersection edges, create triangles where plane intersect mesh.
            // http://en.wikipedia.org/wiki/Point_set_triangulation
            // http://en.wikipedia.org/wiki/Delaunay_triangulation
            // https://code.google.com/p/poly2tri/
            // Simpler Approach? :
            // One point on a clipped triangle is replaced with intersection edge,
            // Connect all edges to form one or more polygons
            // Then calculate centroids of these polygons
            // Then create one triangle per edge
            std::vector<Edge3D> uniqueintersectionsEdges;

            for (auto& intersectionsEdge : intersectionsEdges) {
                if (!equal(intersectionsEdge.v1, intersectionsEdge.v2, EPSILON)) {
                    if (uniqueintersectionsEdges.empty() ||
                        std::find(uniqueintersectionsEdges.begin(), uniqueintersectionsEdges.end(),
                                  intersectionsEdge) == uniqueintersectionsEdges.end()) {
                        uniqueintersectionsEdges.push_back(intersectionsEdge);
                    }
                }
            }

            // The points are not at same point, introduce threshold
            if (!uniqueintersectionsEdges.empty()) {
                // Create closed polygons based on edges
                std::vector<Polygon<Edge3D>> polygons;
                std::vector<Edge3D> connectedEdges;
                std::vector<Edge3D> unconnectEdges = uniqueintersectionsEdges;

                // Start with one edge, check which other edge it's connect to it
                while (!unconnectEdges.empty()) {
                    Edge3D currentEdge = unconnectEdges.front();
                    connectedEdges.push_back(currentEdge);
                    unconnectEdges.erase(unconnectEdges.begin());
                    bool createdPolygon = false;

                    // Search all edges for a connection
                    for (size_t i = 0; i < uniqueintersectionsEdges.size(); ++i) {
                        if (equal(uniqueintersectionsEdges[i].v1, currentEdge.v2, EPSILON) &&
                            uniqueintersectionsEdges[i].v2 != currentEdge.v1) {
                            connectedEdges.push_back(
                                Edge3D(currentEdge.v2, uniqueintersectionsEdges[i].v2));
                            std::vector<Edge3D>::iterator it = std::find(
                                unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                            if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                            currentEdge = uniqueintersectionsEdges[i];
                            i = 0;
                        } else if (equal(uniqueintersectionsEdges[i].v2, currentEdge.v2, EPSILON) &&
                                   uniqueintersectionsEdges[i].v1 != currentEdge.v1) {
                            connectedEdges.push_back(
                                Edge3D(currentEdge.v2, uniqueintersectionsEdges[i].v1));
                            std::vector<Edge3D>::iterator it = std::find(
                                unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                            if (it != unconnectEdges.end()) unconnectEdges.erase(it);

                            currentEdge = Edge3D(uniqueintersectionsEdges[i].v2,
                                                 uniqueintersectionsEdges[i].v1);
                            i = 0;
                        }

                        // Last edge connect to first edge, close the loop and make a polygon
                        if (equal(connectedEdges[0].v1, currentEdge.v2, EPSILON)) {
                            if (currentEdge.v1 != currentEdge.v2) {
                                connectedEdges.push_back(
                                    Edge3D(currentEdge.v2, connectedEdges[0].v1));
                                std::vector<Edge3D>::iterator it = std::find(
                                    unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

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

                        if (i == uniqueintersectionsEdges.size() - 1) {
                            throw Exception("Could not connect find intersecting edges",
                                            IvwContext);
                        }
                    }

                    if (!createdPolygon) {
                        throw Exception("Could not connect clipped edges to manifold polygon",
                                        IvwContext);
                    }
                }

                // Calculate centroids per polygon
                // First in the x-y plane and then in the x-z plane.
                std::vector<vec3> polygonCentroids;
                vec3 u{0};
                vec3 n = plane.getNormal();

                for (auto& polygon : polygons) {
                    // Skip x-y plane if current plane is parallel to x-y plane
                    // or skip x-z plane if current plane is parallel to x-z plane
                    // or skip y-z if both skipXY and skip XZ are false
                    vec3 centroid = plane.getPoint();

                    // X-Y Plane
                    if (!plane.perpendicularToPlane(vec3(0.f, 0.f, 1.f))) {
                        u = vec3(n.y, -n.z, 0.f);
                        centroid.x = 0.f;
                        centroid.y = 0.f;
                        float signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            const auto p0 = polygon.get(i).v1;
                            const auto p1 = polygon.get(i).v2;
                            const auto a = p0.x * p1.y - p1.x * p0.y;
                            signedArea += a;
                            centroid.x += (p0.x + p1.x) * a;
                            centroid.y += (p0.y + p1.y) * a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.x = 0.f;
                            centroid.y = 0.f;
                        } else {
                            centroid.x /= (6.f * signedArea);
                            centroid.y /= (6.f * signedArea);
                        }
                    }

                    // X-Z Plane
                    if (!plane.perpendicularToPlane(vec3(0.f, 1.f, 0.f))) {
                        u = vec3(n.x, -n.y, 0.f);
                        centroid.x = 0.f;
                        centroid.z = 0.f;
                        float signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            const auto p0 = polygon.get(i).v1;
                            const auto p1 = polygon.get(i).v2;
                            const auto a = p0.x * p1.z - p1.x * p0.z;
                            signedArea += a;
                            centroid.x += (p0.x + p1.x) * a;
                            centroid.z += (p0.z + p1.z) * a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.x = 0.f;
                            centroid.z = 0.f;
                        } else {
                            centroid.x /= (6.f * signedArea);
                            centroid.z /= (6.f * signedArea);
                        }
                    }

                    // Y-Z Plane
                    if (!plane.perpendicularToPlane(vec3(1.f, 0.f, 0.f))) {
                        u = vec3(n.y, -n.z, 0.f);
                        centroid.y = 0.f;
                        centroid.z = 0.f;
                        float signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            const auto p0 = polygon.get(i).v1;
                            const auto p1 = polygon.get(i).v2;
                            const auto a = p0.y * p1.z - p1.y * p0.z;
                            signedArea += a;
                            centroid.y += (p0.y + p1.y) * a;
                            centroid.z += (p0.z + p1.z) * a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.y = 0.f;
                            centroid.z = 0.f;
                        } else {
                            centroid.y /= (6.f * signedArea);
                            centroid.z /= (6.f * signedArea);
                        }
                    }

                    polygonCentroids.push_back(centroid);
                }

                const auto v = glm::cross(plane.getNormal(), u);
                // Add new polygons as triangles to the mesh
                std::vector<vec2> uv;
                std::vector<float> baryW;
                std::vector<vec3> tex;
                std::vector<vec4> col;
                vec2 uvC;
                vec3 texC;
                vec4 colC;

                for (size_t p = 0; p < polygons.size(); ++p) {
                    size_t pSize = polygons[p].size();
                    uv.clear();
                    tex.clear();
                    col.clear();

                    for (size_t i = 0; i < pSize; ++i) {
                        // Calculate u-v plane coordinates of the vertex on the polygon
                        uv.push_back(vec2(glm::dot(u, polygons[p].get(i).v1),
                                          glm::dot(v, polygons[p].get(i).v1)));

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
                    // Calculate barycentric coordinates (weights) for all the vertices based on
                    // centroid.
                    uvC = vec2(glm::dot(u, polygonCentroids[p]), glm::dot(v, polygonCentroids[p]));
                    barycentricInsidePolygon2D(uvC, uv, baryW);
                    texC = vec3(0.f);
                    colC = vec4(0.f);

                    for (size_t i = 0; i < pSize - 1; ++i) {
                        texC += tex[i] * baryW[i];
                        colC += col[i] * baryW[i];
                    }

                    // Add triangles to the mesh
                    size_t ip;

                    for (size_t i = 0; i < pSize; ++i) {
                        ip = (i + 1) % pSize;
                        outputMesh->addVertex(polygonCentroids[p], texC, colC);
                        outputMesh->addVertex(polygons[p].get(i).v2, tex[ip], col[ip]);
                        outputMesh->addVertex(polygons[p].get(i).v1, tex[i], col[i]);
                    }
                }
            }
        }
    }

    return outputMesh;
}

}  // namespace inviwo
