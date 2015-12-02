/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include "meshclipping.h"
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
    "Geometry Creation",        // Category
    CodeState::Experimental,    // Code state
    Tags::CPU,                  // Tags
};
const ProcessorInfo MeshClipping::getProcessorInfo() const {
    return processorInfo_;
}

const float MeshClipping::EPSILON = 0.00001f;

MeshClipping::MeshClipping()
    : Processor()
    , inport_("geometry.input")
    , outport_("geometry.output")
    , clippingEnabled_("clippingEnabled", "Enable clipping", false)
    , movePointAlongNormal_("movePointAlongNormal", "Move Plane Point Along Normal", false, InvalidationLevel::Valid)
    , moveCameraAlongNormal_("moveCameraAlongNormal", "Move Camera Along Normal", true, InvalidationLevel::Valid)
    , pointPlaneMove_("pointPlaneMove", "Plane Point Along Normal Move", 0.f, -2.f, 2.f, 0.01f)
    , planePoint_("planePoint", "Plane Point", vec3(0.0f), vec3(-10.0f), vec3(10.0f), vec3(0.1f))
    , planeNormal_("planeNormal", "Plane Normal", vec3(0.0f, 0.0f, -1.0f), vec3(-1.0f), vec3(1.0f), vec3(0.1f))
    , alignPlaneNormalToCameraNormal_("alignPlaneNormalToCameraNormal", "Align Plane Normal To Camera Normal", InvalidationLevel::Valid)
    , renderAsPoints_("renderAsPoints", "Render As Points by Default", false)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f), nullptr, InvalidationLevel::Valid){
    addPort(inport_);
    addPort(outport_);
    addProperty(clippingEnabled_);
    addProperty(movePointAlongNormal_);
    movePointAlongNormal_.onChange(this, &MeshClipping::onMovePointAlongNormalToggled);
    addProperty(moveCameraAlongNormal_);
    addProperty(pointPlaneMove_);

    addProperty(planePoint_);
    addProperty(planeNormal_);

    addProperty(alignPlaneNormalToCameraNormal_);
    alignPlaneNormalToCameraNormal_.onChange(this, &MeshClipping::onAlignPlaneNormalToCameraNormalPressed);

    addProperty(renderAsPoints_);

    addProperty(camera_);

    onMovePointAlongNormalToggled();
}

MeshClipping::~MeshClipping() {}

void MeshClipping::process() {
    /* Processor overview
        - Take axis-aligned boudning box (AABB) mesh as input.
        - Call clipGeometryAgainstPlane(...) with input and plane_ as arguments
        - Extract and store an edge list from the input mesh's triangle list
        - Start with an empty outputList and empty outputEdgeList. Iterate over the edge list
          extracted from the triangle list and check each edge against the clip plane. Store verts
          and edges in outputList and outputEdgeList as you go.
        - Use outputEdgeList and, indirectly, outputList to rebuild a correctly sorted
          triangle strip list.
        - Build new mesh from the triangle strip list and return it.
    */
    if (clippingEnabled_.get()) {
        auto geom = inport_.getData();

        vec3 point = planePoint_.get();
        vec3 normal = planeNormal_.get();
        if (movePointAlongNormal_.get()){
            // Set new plane position based on offset
            vec3 offsetPlaneDiff = normal * pointPlaneMove_.get();
            point += offsetPlaneDiff;
            // Move camera along the offset as well
            if (moveCameraAlongNormal_.get()){
                vec3 offsetGeometryABS = glm::abs(geom->getOffset());
                float planeMoveDiff = pointPlaneMove_.get() - previousPointPlaneMove_;
                vec3 offsetDiff = normal * planeMoveDiff;
                vec3 lookOffset = offsetDiff*offsetGeometryABS*0.5f;
                //camera_.setLookTo(camera_.getLookTo() + lookOffset);
                camera_.setLook(camera_.getLookFrom() + lookOffset,
                    camera_.getLookTo() + lookOffset, camera_.getLookUp());
            }
            previousPointPlaneMove_ = pointPlaneMove_.get();

            // If pointPlaneMove_ zero i.e. no clipping, so output same mesh
            if (pointPlaneMove_ <= 0.f + EPSILON){
                outport_.setData(inport_.getData());
                return;
            }
        }

        // LogInfo("Calling clipping method.");
        Mesh* clippedPlaneGeom =
            clipGeometryAgainstPlaneRevised(geom.get(), Plane(point, normal));
        if (clippedPlaneGeom) {
            clippedPlaneGeom->setModelMatrix(inport_.getData()->getModelMatrix());
            clippedPlaneGeom->setWorldMatrix(inport_.getData()->getWorldMatrix());
            // LogInfo("Setting new mesh as outport data.");
            outport_.setData(clippedPlaneGeom);
        }else{
            outport_.setData(inport_.getData());
        }
        // LogInfo("Done.");
    } else {
        outport_.setData(inport_.getData());
    }
}

void MeshClipping::onMovePointAlongNormalToggled(){
    planePoint_.setReadOnly(movePointAlongNormal_.get());
    pointPlaneMove_.set(0.f);
    previousPointPlaneMove_ = 0.f;
    pointPlaneMove_.setVisible(movePointAlongNormal_.get());
}

void MeshClipping::onAlignPlaneNormalToCameraNormalPressed(){
    planeNormal_.set(glm::normalize(camera_.getLookTo() - camera_.getLookFrom()));

    // Calculate new plane point by finding the closest geometry point to the camera
    auto geom = inport_.getData();
    const std::vector<vec3>* vertexList;
    const SimpleMesh* simpleInputMesh = dynamic_cast<const SimpleMesh*>(geom.get());
    if (simpleInputMesh) {
        vertexList = simpleInputMesh->getVertexList()->getRAMRepresentation()->getDataContainer();
    }
    else {
        const BasicMesh* basicInputMesh = dynamic_cast<const BasicMesh*>(geom.get());
        if (basicInputMesh) {
            vertexList = basicInputMesh->getVertices()->getRAMRepresentation()->getDataContainer();
        }
        else {
            LogError("Unsupported mesh type, only simple and basic meshes are supported");
            return;
        }
    }

    float minDist = glm::distance(camera_.getLookFrom(), vertexList->at(0));
    vec3 closestVertex = vertexList->at(0);
    for (unsigned int t = 1; t < vertexList->size(); ++t) {
        // Calculate distance to camera
        float dist = glm::distance(camera_.getLookFrom(), vertexList->at(t));
        if (dist < minDist){
            minDist = dist;
            closestVertex = vertexList->at(t);
        }
    }

    planePoint_.set(closestVertex);
}

// Convert degrees to radians
float MeshClipping::degreeToRad(float degree) {
    return degree * (glm::pi<float>() / 180.f);
}

//Check point equality with threshold
inline bool equal(vec3 v1, vec3 v2, float eps) {
    return (std::fabs(v1.x-v2.x)<eps && std::fabs(v1.y-v2.y)<eps && std::fabs(v1.z-v2.z)<eps);
}

// Compute barycentric coordinates/weights for
// point p (which is inside the polygon) with respect to polygons of vertices (v)
// Based on Mean Value Coordinates by Hormann/Floater
inline void barycentricInsidePolygon2D(vec2 p, const std::vector<vec2>& v, std::vector<float>& baryW) {
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
        ip = (i+1)%numV;
        Ai = (s[i].x*s[ip].y - s[ip].x*s[i].y);
        tanA[i] = (ri[i]*ri[ip] - glm::dot(s[i], s[ip]))/Ai;
    }

    baryW.resize(numV);

    for (size_t i = 0; i < numV; ++i) {
        baryW[i] = 0.f;
    }

    float wi;
    float wsum = 0.f;

    for (size_t i = 0; i < numV; ++i) {
        ip = (numV-1+i)%numV;
        wi = 2.f*(tanA[i] + tanA[ip])/ri[i];
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

std::vector<unsigned int> edgeListtoTriangleList(std::vector<EdgeIndex>& edges) {
    // Traverse edge list and construct correctly sorted triangle strip list.
    return std::vector<unsigned int>();
}

// Extract edges from triangle strip list
std::vector<EdgeIndex> triangleListtoEdgeList(const std::vector<unsigned int>* triList) {
    std::vector<EdgeIndex> result;
    LogInfoCustom("MeshClipping", "Size of tri list: " << triList->size() - 1);

    for (size_t i = 0; i < triList->size(); ++i) {
        EdgeIndex e1;

        if (i == 0 || i % 3 == 0) {
            if (i + 1 < triList->size()) {
                e1.v1 = triList->at(i);
                e1.v2 = triList->at(i + 1);

                if (result.empty() || std::find(result.begin(), result.end(), e1) == result.end()) {
                    result.push_back(e1);
                }
            }

            if (i + 2 < triList->size()) {
                e1.v1 = triList->at(i);
                e1.v2 = triList->at(i + 2);

                if (result.empty() || std::find(result.begin(), result.end(), e1) == result.end()) {
                    result.push_back(e1);
                }
            }
        } else if ((i - 1) % 3 == 0) {
            if (i + 2 < triList->size()) {
                e1.v1 = triList->at(i);
                e1.v2 = triList->at(i + 2);

                if (result.empty() || std::find(result.begin(), result.end(), e1) == result.end()) {
                    result.push_back(e1);
                }
            }
        } else if ((i + 1) % 3 == 0) {
            if (i + 1 < triList->size()) {
                e1.v1 = triList->at(i);
                e1.v2 = triList->at(i + 1);

                if (result.empty() || std::find(result.begin(), result.end(), e1) == result.end()) {
                    result.push_back(e1);
                }
            }
        }
    }

    LogInfoCustom("MeshClipping", "Size of edge list: " << result.size());

    for (size_t i = 0; i < result.size(); ++i) {
        LogInfoCustom("MeshClipping", "Edge, " << i << " = " << result.at(i).v1 << "->"
                                               << result.at(i).v2);
    }

    return result;
}

Mesh* MeshClipping::clipGeometryAgainstPlaneRevised(const Mesh* in, Plane plane) {
    ConnectivityType indexAttrInfo;
    const std::vector<vec3>* vertexList;
    const std::vector<vec3>* texcoordlist;
    const std::vector<vec4>* colorList;
    const std::vector<unsigned int>* triangleList;
    
    const SimpleMesh* simpleInputMesh = dynamic_cast<const SimpleMesh*>(in);
    if (simpleInputMesh) {
        vertexList = simpleInputMesh->getVertexList()->getRAMRepresentation()->getDataContainer();
        texcoordlist = simpleInputMesh->getTexCoordList()->getRAMRepresentation()->getDataContainer();
        colorList = simpleInputMesh->getColorList()->getRAMRepresentation()->getDataContainer();
        triangleList = simpleInputMesh->getIndexList()->getRAMRepresentation()->getDataContainer();
        indexAttrInfo = simpleInputMesh->getIndexMeshInfo(0).ct;
    } else {
        // TODO do clipping in all the index list now we only consider the first one 
        const BasicMesh* basicInputMesh = dynamic_cast<const BasicMesh*>(in);
        if(basicInputMesh) {
            vertexList = basicInputMesh->getVertices()->getRAMRepresentation()->getDataContainer();
            texcoordlist = basicInputMesh->getTexCoords()->getRAMRepresentation()->getDataContainer();
            colorList = basicInputMesh->getColors()->getRAMRepresentation()->getDataContainer();
            triangleList = basicInputMesh->getIndexBuffers()[0].second->getRAMRepresentation()->getDataContainer();
            indexAttrInfo = basicInputMesh->getIndexBuffers()[0].first.ct;
        } else {
            LogError("Unsupported mesh type, only simeple and basic meshes are supported");
            return nullptr;
        }
    }


    SimpleMesh* outputMesh = new SimpleMesh(DrawType::Triangles);

    //Check if we are using indicies
    if (triangleList->size() > 0) {
        //Check if it is a Triangle Strip
        if (indexAttrInfo == ConnectivityType::Strip) {
            // Iterate over edges by edge
            unsigned int idx[3];
            std::vector<vec3> newVertices;
            std::vector<vec3> newTexCoords;
            std::vector<vec4> newColors;
            std::vector<Edge3D> intersectionsEdges;
            std::vector<std::pair<vec3, vec3> > intersectionTex;
            std::vector<std::pair<vec3, vec4> > intersectionCol;

            for (unsigned int t=0; t<triangleList->size()-2; ++t) {
                idx[0] = triangleList->at(t);

                //Clockwise
                if (t & 1) {
                    idx[1] = triangleList->at(t+2);
                    idx[2] = triangleList->at(t+1);
                }
                else {
                    idx[1] = triangleList->at(t+1);
                    idx[2] = triangleList->at(t+2);
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

                for (size_t i=0; i<3; ++i) {
                    size_t j = (i==2 ? 0 : i+1);

                    if (plane.isInside(vertexList->at(idx[i]))) {
                        if (plane.isInside(vertexList->at(idx[j]))) { // Case 1
                            //Add v2
                            newVertices.push_back(vertexList->at(idx[j]));
                            newTexCoords.push_back(texcoordlist->at(idx[j]));
                            newColors.push_back(colorList->at(idx[j]));
                        }
                        else { //Case 2
                            //Add Intersection
                            vec3 intersection = plane.getIntersection(vertexList->at(idx[i]), vertexList->at(idx[j]));
                            newVertices.push_back(intersection);
                            vec3 interBC = barycentricTriangle(intersection, vertexList->at(idx[0]), vertexList->at(idx[1]), vertexList->at(idx[2]));
                            vec3 interTex = (texcoordlist->at(idx[0])*interBC.x) + (texcoordlist->at(idx[1])*interBC.y) + (texcoordlist->at(idx[2])*interBC.z);
                            newTexCoords.push_back(interTex);
                            vec4 interCol = (colorList->at(idx[0])*interBC.x) + (colorList->at(idx[1])*interBC.y) + (colorList->at(idx[2])*interBC.z);
                            newColors.push_back(interCol);
                            intersectionTex.push_back(std::make_pair(intersection, interTex));
                            intersectionCol.push_back(std::make_pair(intersection, interCol));

                            //We save the intersection as part of edge on the clipping plane
                            if (intersectionAdded)
                                intersectionsEdges.back().v1 = intersection;
                            else {
                                intersectionsEdges.push_back(Edge3D(intersection));
                                intersectionAdded = true;
                            }
                        }
                    }
                    else {
                        if (plane.isInside(vertexList->at(idx[j]))) { // Case 3
                            //Add Intersection
                            vec3 intersection = plane.getIntersection(vertexList->at(idx[i]), vertexList->at(idx[j]));
                            newVertices.push_back(intersection);
                            vec3 interBC = barycentricTriangle(intersection, vertexList->at(idx[0]), vertexList->at(idx[1]), vertexList->at(idx[2]));
                            vec3 interTex = (texcoordlist->at(idx[0])*interBC.x) + (texcoordlist->at(idx[1])*interBC.y) + (texcoordlist->at(idx[2])*interBC.z);
                            newTexCoords.push_back(interTex);
                            vec4 interCol = (colorList->at(idx[0])*interBC.x) + (colorList->at(idx[1])*interBC.y) + (colorList->at(idx[2])*interBC.z);
                            newColors.push_back(interCol);
                            intersectionTex.push_back(std::make_pair(intersection, interTex));
                            intersectionCol.push_back(std::make_pair(intersection, interCol));

                            //We save the intersection as part of edge on the clipping plane
                            if (intersectionAdded)
                                intersectionsEdges.back().v2 = intersection;
                            else {
                                intersectionsEdges.push_back(Edge3D(intersection));
                                intersectionAdded = true;
                            }

                            //Add v2
                            newVertices.push_back(vertexList->at(idx[j]));
                            newTexCoords.push_back(texcoordlist->at(idx[j]));
                            newColors.push_back(colorList->at(idx[j]));
                        }

                        // Case 4
                    }
                }

                //Handle more then 3 vertices
                if (newVertices.size() > 3) {
                    ivwAssert(newVertices.size() < 5, "Can't handle " << newVertices.size() << " vertices after clipping");
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

                //Add vertices to mesh
                for (size_t i=0; i<newVertices.size(); ++i) {
                    outputMesh->addVertex(newVertices.at(i), newVertices.at(i), newColors.at(i));
                }
            }

            //Based on intersection edges, create triangles where plane intersect mesh.
            //http://en.wikipedia.org/wiki/Point_set_triangulation
            //http://en.wikipedia.org/wiki/Delaunay_triangulation
            //https://code.google.com/p/poly2tri/
            //Simpler Approach? :
            //One point on a clipped triangle is replaced with intersection edge,
            //Connect all edges to form one or more polygons
            //Then calculate centroids of these polygons
            //Then create one triangle per edge
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

            //The points are not at same point, introduce threshold
            if (!uniqueintersectionsEdges.empty()) {
                //Create closed polygons based on edges
                std::vector<Polygon<Edge3D> > polygons;
                std::vector<Edge3D> connectedEdges;
                std::vector<Edge3D> unconnectEdges = uniqueintersectionsEdges;

                //Start with one edge, check which other edge it's connect to it
                while (!unconnectEdges.empty()) {
                    Edge3D currentEdge = unconnectEdges.front();
                    connectedEdges.push_back(currentEdge);
                    unconnectEdges.erase(unconnectEdges.begin());
                    bool createdPolygon = false;

                    //Search all edges for a connection
                    for (size_t i=0; i < uniqueintersectionsEdges.size(); ++i) {
                        if (equal(uniqueintersectionsEdges[i].v1, currentEdge.v2, EPSILON) && uniqueintersectionsEdges[i].v2 != currentEdge.v1) {
                            connectedEdges.push_back(Edge3D(currentEdge.v2, uniqueintersectionsEdges[i].v2));
                            std::vector<Edge3D>::iterator it = std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                            if (it != unconnectEdges.end())
                                unconnectEdges.erase(it);

                            currentEdge = uniqueintersectionsEdges[i];
                            i = 0;
                        }
                        else if (equal(uniqueintersectionsEdges[i].v2, currentEdge.v2, EPSILON) && uniqueintersectionsEdges[i].v1 != currentEdge.v1) {
                            connectedEdges.push_back(Edge3D(currentEdge.v2, uniqueintersectionsEdges[i].v1));
                            std::vector<Edge3D>::iterator it = std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                            if (it != unconnectEdges.end())
                                unconnectEdges.erase(it);

                            currentEdge = Edge3D(uniqueintersectionsEdges[i].v2, uniqueintersectionsEdges[i].v1);
                            i = 0;
                        }

                        //Last edge connect to first edge, close the loop and make a polygon
                        if (equal(connectedEdges[0].v1, currentEdge.v2, EPSILON)) {
                            if (currentEdge.v1 != currentEdge.v2) {
                                connectedEdges.push_back(Edge3D(currentEdge.v2, connectedEdges[0].v1));
                                std::vector<Edge3D>::iterator it = std::find(unconnectEdges.begin(), unconnectEdges.end(), currentEdge);

                                if (it != unconnectEdges.end())
                                    unconnectEdges.erase(it);
                            }

                            Polygon<Edge3D> newPoly(connectedEdges.size());

                            for (size_t j=0; j < connectedEdges.size(); ++j) {
                                newPoly.at(j) = connectedEdges.at(j);
                            }

                            polygons.push_back(newPoly);
                            connectedEdges.clear();
                            createdPolygon = true;
                            break;
                        }

                        ivwAssert(i != uniqueintersectionsEdges.size()-1, "Could not connect find intersecting edges");
                    }

                    ivwAssert(createdPolygon, "Could not connect clipped edges to manifold polygon");
                }

                //Calculate centroids per polygon
                //First in the x-y plane and then in the x-z plane.
                std::vector<vec3> polygonCentroids;
                vec3 u, v;
                vec3 n = plane.getNormal();

                for (auto& polygon : polygons) {
                    //Skip x-y plane if current plane is parallel to x-y plane
                    //or skip x-z plane if current plane is parallel to x-z plane
                    //or skip y-z if both skipXY and skip XZ are false
                    vec3 p0, p1;
                    vec3 centroid = plane.getPoint();
                    float signedArea = 0.f;
                    float a = 0.f;

                    //X-Y Plane
                    if (!plane.perpendicularToPlane(vec3(0.f, 0.f, 1.f))) {
                        u = vec3(n.y, -n.z, 0.f);
                        centroid.x = 0.f;
                        centroid.y = 0.f;
                        signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            p0 = polygon.get(i).v1;
                            p1 = polygon.get(i).v2;
                            a = p0.x*p1.y - p1.x*p0.y;
                            signedArea += a;
                            centroid.x += (p0.x + p1.x)*a;
                            centroid.y += (p0.y + p1.y)*a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.x = 0.f;
                            centroid.y = 0.f;
                        }
                        else {
                            centroid.x /= (6.f*signedArea);
                            centroid.y /= (6.f*signedArea);
                        }
                    }

                    //X-Z Plane
                    if (!plane.perpendicularToPlane(vec3(0.f, 1.f, 0.f))) {
                        u = vec3(n.x, -n.y, 0.f);
                        centroid.x = 0.f;
                        centroid.z = 0.f;
                        signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            p0 = polygon.get(i).v1;
                            p1 = polygon.get(i).v2;
                            a = p0.x*p1.z - p1.x*p0.z;
                            signedArea += a;
                            centroid.x += (p0.x + p1.x)*a;
                            centroid.z += (p0.z + p1.z)*a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.x = 0.f;
                            centroid.z = 0.f;
                        }
                        else {
                            centroid.x /= (6.f*signedArea);
                            centroid.z /= (6.f*signedArea);
                        }
                    }

                    //Y-Z Plane
                    if (!plane.perpendicularToPlane(vec3(1.f, 0.f, 0.f))) {
                        u = vec3(n.y, -n.z, 0.f);
                        centroid.y = 0.f;
                        centroid.z = 0.f;
                        signedArea = 0.f;

                        for (size_t i = 0; i < polygon.size(); ++i) {
                            p0 = polygon.get(i).v1;
                            p1 = polygon.get(i).v2;
                            a = p0.y*p1.z - p1.y*p0.z;
                            signedArea += a;
                            centroid.y += (p0.y + p1.y)*a;
                            centroid.z += (p0.z + p1.z)*a;
                        }

                        signedArea *= 0.5f;

                        if (std::fabs(signedArea) < EPSILON) {
                            centroid.y = 0.f;
                            centroid.z = 0.f;
                        }
                        else {
                            centroid.y /= (6.f*signedArea);
                            centroid.z /= (6.f*signedArea);
                        }
                    }

                    polygonCentroids.push_back(centroid);
                }

                v = glm::cross(plane.getNormal(), u);
                //Add new polygons as triangles to the mesh
                std::vector<vec2> uv;
                std::vector<float> baryW;
                std::vector<vec3> tex;
                std::vector<vec4> col;
                vec2 uvC;
                vec3 texC;
                vec4 colC;

                for (size_t p=0; p < polygons.size(); ++p) {
                    size_t pSize = polygons[p].size();
                    uv.clear();
                    tex.clear();
                    col.clear();

                    for (size_t i=0; i < pSize; ++i) {
                        //Calculate u-v plane coordinates of the vertex on the polygon
                        uv.push_back(vec2(glm::dot(u, polygons[p].get(i).v1), glm::dot(v, polygons[p].get(i).v1)));

                        //Lookup texcoord and colors for the vertex of the polygon
                        for (size_t t=0; t<intersectionTex.size(); ++t) {
                            if (intersectionTex.at(t).first == polygons[p].get(i).v1) {
                                tex.push_back(intersectionTex.at(t).second);
                                col.push_back(intersectionCol.at(t).second);
                                break;
                            }
                        }
                    }

                    uv.pop_back();
                    //Calculate barycentric coordinates (weights) for all the vertices based on centroid.
                    uvC = vec2(glm::dot(u, polygonCentroids[p]), glm::dot(v, polygonCentroids[p]));
                    barycentricInsidePolygon2D(uvC, uv, baryW);
                    texC = vec3(0.f);
                    colC = vec4(0.f);

                    for (size_t i=0; i < pSize-1; ++i) {
                        texC += tex[i]*baryW[i];
                        colC += col[i]*baryW[i];
                    }

                    //Add triangles to the mesh
                    size_t ip;

                    for (size_t i=0; i < pSize; ++i) {
                        ip = (i+1)%pSize;
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

Mesh* MeshClipping::clipGeometryAgainstPlane(const Mesh* in, Plane plane) {
    //LogInfo("Entered clipGeometryAgainstPlane(...).");
    const SimpleMesh* inputMesh = dynamic_cast<const SimpleMesh*>(in);

    if (!inputMesh) {
        LogError("Can only clip a SimpleMesh*");
        return nullptr;
    }

    /* ---TODO / bugs
        -   Create correct outputEdgeList while running clipping algorithm, currently edges between clipped verts
            sometimes end up in incorrect order.
        -   Use correct outputEdgeList to create a correctly sorted triangle strip list
    */
    //LogInfo("Fetching vertex- and triangle lists.");
    const std::vector<vec3>* inputList = inputMesh->getVertexList()->getRAMRepresentation()->getDataContainer();
    const std::vector<unsigned int>* triangleList = inputMesh->getIndexList()->getRAMRepresentation()->getDataContainer();
    std::vector<EdgeIndex> edgeList = triangleListtoEdgeList(triangleList);
    std::vector<unsigned int> clippedVertInd;
    // For each clip plane, do:
    std::vector<glm::vec3> outputList;
    std::vector<unsigned int> outputIndexList; // vertex index list
    std::vector<EdgeIndex> outputEdgeList; // output edge list

    // Iterate over edges extracted from triangle strip list, and perform clipping against plane
    for (auto& elem : edgeList) {
        //LogInfo("i = "<<i);
        unsigned int Sind = elem.v1;
        unsigned int Eind = elem.v2;
        glm::vec3 S = inputList->at(Sind);
        glm::vec3 E = inputList->at(Eind);
        EdgeIndex edge;
        int duplicate = -1;

        // For each clip plane
        if (plane.isInside(E)) {
            if (!plane.isInside(S)) { // Going in
                //LogInfo("Going in!");
                // Must put in all the vert to edges in the right order not to mess up the vert-ids
                glm::vec3 clippedVert = plane.getIntersection(S,E);
                outputList.push_back(clippedVert);
                outputIndexList.push_back(static_cast<unsigned int>(outputList.size()-1)); // Ny vertex, uppdatera edge-listan
                clippedVertInd.push_back(static_cast<unsigned int>(outputList.size()-1));
                edge.v1=static_cast<unsigned int>(outputList.size()-1);

                for (unsigned int j=0; j<outputList.size(); ++j) {
                    if (std::fabs(E.x-outputList.at(j).x)<EPSILON && std::fabs(E.y-outputList.at(j).y)<EPSILON && std::fabs(E.z-outputList.at(j).z)<EPSILON) {
                        duplicate = j;
                    }
                }

                if (duplicate != -1) { // Duplicate found
                    edge.v2 = duplicate;
                } else { // No duplicate end vertex found
                    outputList.push_back(E);
                    outputIndexList.push_back(static_cast<unsigned int>(outputList.size()-1));
                    edge.v2 = static_cast<unsigned int>(outputList.size()-1);
                }

                if (std::find(outputEdgeList.begin(), outputEdgeList.end(),edge) == outputEdgeList.end()) {
                    //LogInfo("Going in, Before: "<<Sind<<"->"<<Eind<<", after: "<<edge.v1<<"->"<<edge.v2);
                    outputEdgeList.push_back(edge);
                }
            } else { // S and E both inside
                //LogInfo("Both inside! S = "<<glm::to_string(S));
                for (unsigned int j=0; j<outputList.size(); ++j) {
                    if (std::fabs(S.x-outputList.at(j).x)<EPSILON && std::fabs(S.y-outputList.at(j).y)<EPSILON && std::fabs(S.z-outputList.at(j).z)<EPSILON) {
                        duplicate = j;
                    }
                }

                if (duplicate != -1) {
                    //LogInfo("Duplicate found at index "<<std::distance(outputList.begin(),it)<<", position "<<glm::to_string(*it));
                    edge.v1 = duplicate;
                    duplicate = -1;
                } else { // No duplicate found
                    outputList.push_back(S);
                    outputIndexList.push_back(static_cast<unsigned int>(outputList.size()-1));
                    edge.v1 = static_cast<unsigned int>(outputList.size()-1);
                }

                for (int j=0; j<static_cast<int>(outputList.size()); ++j) {
                    if (std::fabs(E.x-outputList.at(j).x)<EPSILON && std::fabs(E.y-outputList.at(j).y)<EPSILON && std::fabs(E.z-outputList.at(j).z)<EPSILON) {
                        duplicate = j;
                    }
                }

                if (duplicate != -1) {
                    //LogInfo("Duplicate found at index "<<std::distance(outputList.begin(),it));
                    edge.v2 = duplicate;
                } else { // Duplicate found
                    outputList.push_back(E);
                    outputIndexList.push_back(static_cast<unsigned int>(outputList.size()-1));
                    edge.v2 = static_cast<unsigned int>(outputList.size()-1);
                }

                if (std::find(outputEdgeList.begin(), outputEdgeList.end(),edge) == outputEdgeList.end()) {
                    //LogInfo("Both inside, Before: "<<Sind<<"->"<<Eind<<", after: "<<edge.v1<<"->"<<edge.v2);
                    outputEdgeList.push_back(edge);
                }
            }
        } else if (plane.isInside(S)) { // Going out (S inside, E outside) ( fungerar ej atm, skapar ingen S),
            //LogInfo("Going out!");
            // Check if S aldready in outputList, otherwise add it. Add clippedVert between S->E
            for (unsigned int j=0; j<outputList.size(); ++j) {
                if (std::fabs(S.x-outputList.at(j).x)<EPSILON && std::fabs(S.y-outputList.at(j).y)<EPSILON && std::fabs(S.z-outputList.at(j).z)<EPSILON) {
                    duplicate = j;
                }
            }

            if (duplicate != -1) {
                //LogInfo("Duplicate found at index "<<std::distance(outputList.begin(),it));
                edge.v1 = duplicate;
            } else { // No duplicate found
                outputList.push_back(S);
                outputIndexList.push_back(static_cast<unsigned int>(outputList.size()-1));
                edge.v1 = static_cast<unsigned int>(outputList.size()-1);
            }

            glm::vec3 clippedVert = plane.getIntersection(S,E);
            outputList.push_back(clippedVert);
            outputIndexList.push_back(static_cast<unsigned int>(outputList.size()));
            clippedVertInd.push_back(static_cast<unsigned int>(outputList.size()-1));
            edge.v2 = static_cast<unsigned int>(outputList.size()-1);

            if (std::find(outputEdgeList.begin(), outputEdgeList.end(),edge) == outputEdgeList.end()) {
                //LogInfo("Going out, Before: "<<Sind<<"->"<<Eind<<", after: "<<edge.v1<<"->"<<edge.v2);
                outputEdgeList.push_back(edge);
            }
        } else {
            // Nothing - Entire edge outside clip plane
        }
    }

    // End, for each clip plane
    LogInfo("outputList.size() = "<<outputList.size()<<", std::distance = "<<std::distance(outputList.begin(), outputList.end()));

    for (auto& elem : outputIndexList) LogInfo("Vertex indices: " << elem);

    for (size_t i=0; i<outputList.size(); ++i)
        LogInfo("Output verts, " << i << ": ("+glm::to_string(outputList.at(i)[0])+", "+glm::to_string(outputList.at(i)[1])+", "+glm::to_string(
                    outputList.at(i)[2])+")");

    LogInfo("Size of clipped verts vector:" << clippedVertInd.size());

    // Create edges between new (clipped) vertices
    for (unsigned int i=0; i<clippedVertInd.size(); ++i) {
        EdgeIndex edge;
        unsigned int idx1,idx2;
        idx1 = clippedVertInd.at(i % clippedVertInd.size());
        idx2 = clippedVertInd.at((i+1) % clippedVertInd.size());
        edge.v1 = idx1;
        edge.v2 = idx2;

        if (std::find(outputEdgeList.begin(), outputEdgeList.end(), edge) == outputEdgeList.end()) {
            outputEdgeList.push_back(edge);
        }
    }

    /*LogInfo("Size of clipped edge list: " << outputEdgeList.size());
    for(size_t i=0; i<outputEdgeList.size();++i) {
        LogInfo("Edge, " << i << " = " << outputEdgeList.at(i).v1 << "->" << outputEdgeList.at(i).v2);
    }*/
    // Build a new SimpleMesh here form outputList-vektor
    //LogInfo("Buildning new mesh from clipped vertices.");
    SimpleMesh* outputMesh = new SimpleMesh();

    for (unsigned int i=0; i<outputList.size(); ++i) {
        outputMesh->addVertex(outputList.at(i), glm::vec3(1.f), glm::vec4(1.,i/(float)outputList.size(),0.,1.0f));
    }

    //LogInfo("Number of verts in output mesh: " <<
    //  outputList.size());
    if (renderAsPoints_.get())
        outputMesh->setIndicesInfo(DrawType::Points, ConnectivityType::None);
    else
        outputMesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::Strip);

    for (unsigned int i=0; i<outputList.size(); ++i) {
        outputMesh->addIndex(i);
        //LogInfo("Adding to index list, vertex no.: " << i);
    }

    //LogInfo("Returning new mesh.");
    return outputMesh;
}

} // namespace
