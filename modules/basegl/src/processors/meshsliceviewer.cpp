/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>
#include <modules/base/algorithm/mesh/meshclipping.h>
#include <modules/basegl/processors/meshsliceviewer.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>


namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeshSliceViewer::processorInfo_{
    "org.inviwo.MeshSliceViewer",      // Class identifier
    "Mesh Slice Viewer",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo MeshSliceViewer::getProcessorInfo() const { return processorInfo_; }

MeshSliceViewer::MeshSliceViewer()
    : Processor() 
    , volumeIn_("volumeIn")
    , inport_("inport")
    , meshOut_("meshOut")
    , position_("position", "Plane Position", vec3(0.0f), vec3(-100.0f), vec3(100.0f)) 
    , normal_("normal", "Plane Normal", vec3(1.0f, 0, 0), vec3(-1.0f), vec3(1.0f)) 
    , width_("sliceWidth", "Slice Width", 512, 0, 2000)
    , sliceResolution_("sliceResolution", "Slice Resolution", vec2(256,256), vec2(0), vec2(65536)) 
    , camera_("camera", "Camera") {

    addPort(volumeIn_);
    addPort(inport_);
    addPort(meshOut_);
    addProperty(position_);
    addProperty(normal_);
    addProperty(width_);
    addProperty(sliceResolution_);
    addProperty(camera_);

    position_.onChange([this]() { planeSettingsChanged(); });
    normal_.onChange([this]() { planeSettingsChanged(); });
}

void MeshSliceViewer::process() {
    if (inport_.hasData()) {
        const mat4 texToWorld(mat3(volumeIn_.getData()->getCoordinateTransformer().getTextureToWorldMatrix()));
        const vec3 worldPos = vec3(texToWorld * vec4(position_.get() - 0.5f, 1.0f));
        const vec3 normal = glm::normalize(normal_.get());
        const auto aabb1 = meshutil::axisAlignedBoundingBox(*inport_.getData());
        auto clippedPlaneGeom = meshutil::clipMeshAgainstPlane(
            *meshutil::clipMeshAgainstPlane(*inport_.getData(), Plane(worldPos, -normal), true),  // Cut Slicing plane
                                                                Plane(worldPos - 1.0f * normal, normal), true); // Cut backside
        if(clippedPlaneGeom->getIndexBuffers().size() == 0){
            clippedPlaneGeom->addIndexBuffer(DrawType::Lines, ConnectivityType::None);
        }
        clippedPlaneGeom->setModelMatrix(inport_.getData()->getModelMatrix());
        clippedPlaneGeom->setWorldMatrix(inport_.getData()->getWorldMatrix());
        meshOut_.setData(clippedPlaneGeom);
    } else {
        meshOut_.setData(inport_.getData());
    }
}

void MeshSliceViewer::planeSettingsChanged() {
    if (!volumeIn_.hasData()) return;

    // Make sure we keep the aspect of the input data.

    // In texture space
    const vec3 normal = glm::normalize(normal_.get());
    const Plane plane(position_.get(), normal);

    // In worldSpace, ignoring translation because it should not affect rotation (fixes issue #875)
    const mat4 texToWorld(
        mat3(volumeIn_.getData()->getCoordinateTransformer().getTextureToWorldMatrix()));

    const float W = texToWorld[0][0];
    const float H = texToWorld[1][1];
    const float D = texToWorld[2][2];

    const vec3 worldNormal(
        glm::normalize(vec3(glm::inverseTranspose(texToWorld) * vec4(normal, 0.0f))));
    const mat4 boxrotation(glm::toMat4(glm::rotation(worldNormal, vec3(0.0f, 0.0f, 1.0f))));

    // Construct the edges of a unit box and intersect with the plane.
    std::vector<std::optional<vec3>> points;
    points.reserve(12);

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 1.0f)));

    points.push_back(plane.getIntersection(vec3(0.0f, 0.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 0.0f, 1.0f), vec3(1.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 1.0f)));
    points.push_back(plane.getIntersection(vec3(0.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, 1.0f)));

    // Calculate the aspect of the intersected plane in world space.
    vec2 xrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    vec2 yrange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    for (auto& point : points) {
        if (point) {
            vec4 corner = vec4(*point, 1.0f);
            corner = boxrotation * texToWorld * corner;

            xrange[0] = std::min(xrange[0], corner.x);
            xrange[1] = std::max(xrange[1], corner.x);
            yrange[0] = std::min(yrange[0], corner.y);
            yrange[1] = std::max(yrange[1], corner.y);
        }
    }
    width_.set(glm::max(xrange[1] - xrange[0], yrange[1] - yrange[0]));
    const float sourceRatio = glm::abs((xrange[1] - xrange[0]) / (yrange[1] - yrange[0]));

    // Goal: define a transformation that maps the view 2D texture coordinates into
    // 3D texture coordinates at at some plane in the volume.
    // const mat4 flipMatX(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1);
    // const mat4 flipMatY(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);

    const vec2 targetDim(sliceResolution_.get());
    const float targetRatio = targetDim.x / targetDim.y;

    const vec3 scaleSource(1.0f / (sourceRatio > 1.0f ? 1.0f : sourceRatio),
                           1.0f * (sourceRatio > 1.0f ? sourceRatio : 1.0f), 1.0f);

    const vec3 scaleTarget(1.0f * (targetRatio < 1.0f ? 1.0f : targetRatio),
                           1.0f / (targetRatio < 1.0f ? targetRatio : 1.0f), 1.0f);

    mat4 rotation(
        glm::translate(vec3(0.5f)) * glm::toMat4(glm::rotation(vec3(0.0f, 0.0f, 1.0f), normal)) *
        glm::scale(scaleSource) * //glm::rotate(imageRotation_.get(), vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(scaleTarget) * glm::translate(vec3(-0.5f)));

    // if (flipHorizontal_) rotation *= flipMatX;
    // if (flipVertical_) rotation *= flipMatY;

    // Save the inverse rotation.
    sliceRotation_ = rotation;
    inverseSliceRotation_ = glm::inverse(rotation);

    // Compute up vector of the slice in world
    vec4 p1(0.5,0.5,0.0,1.0);
    vec4 p2(0.5,1.0,0.0,1.0);
    vec3 up = glm::normalize(vec3(sliceRotation_ * p2 - sliceRotation_ * p1));

    camera_.setLookFrom(glm::normalize(normal_.get()));
    camera_.setLookUp(up);
    camera_.setFarPlaneDist(glm::length(vec3(W,H,D)) + 1.0f);
    camera_.setNearPlaneDist(-glm::length(vec3(W,H,D)));

    return;
}

}  // namespace inviwo
