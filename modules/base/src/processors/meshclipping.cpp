/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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
#include <modules/base/algorithm/mesh/meshclipping.h>
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

MeshClipping::MeshClipping()
    : Processor()
    , inport_("inputMesh")
    , outport_("clippedMesh")
    , clippingPlane_("clippingPlane")
    , clippingEnabled_("clippingEnabled", "Enable clipping", true)
    , movePointAlongNormal_("movePointAlongNormal", "Move Plane Point Along Normal", false,
                            InvalidationLevel::Valid)
    , moveCameraAlongNormal_("moveCameraAlongNormal", "Move Camera Along Normal", true,
                             InvalidationLevel::Valid)
    , pointPlaneMove_("pointPlaneMove", "Plane Point Along Normal Move", 0.f, 0.f, 2.f, 0.01f)
    , capClippedHoles_("capClippedHoles", "Cap clipped holes", true)
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

    addProperty(capClippedHoles_);

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
        if (auto clippedPlaneGeom =
                meshutil::clipMeshAgainstPlane(*inport_.getData(), *plane, capClippedHoles_)) {
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

}  // namespace inviwo
