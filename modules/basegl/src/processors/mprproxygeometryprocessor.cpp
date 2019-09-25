/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <modules/basegl/processors/mprproxygeometryprocessor.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/volumeutils.h>
#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>
#include <modules/base/algorithm/cubeproxygeometry.h>
#include <modules/opengl/openglutils.h>

namespace inviwo {

const ProcessorInfo MPRProxyGeometry::processorInfo_{
    "org.inviwo.MPRProxyGeometry",  // Class identifier
    "MPR Proxy Geometry",           // Display name
    "Mesh Creation",                // Category
    CodeState::Experimental,        // Code state
    Tags::CPU,                      // Tags
};
const ProcessorInfo MPRProxyGeometry::getProcessorInfo() const { return processorInfo_; }

MPRProxyGeometry::MPRProxyGeometry()
    : Processor()
    , inport_("volume")
    , outport_("proxyGeometry")
    , recenterP_("recenterP", "Re-center")
    , mprP_("mprP_", "Center", vec3(0.0f), vec3(-100.0f), vec3(100.0f), vec3(1.0f),
            InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , mprBasisN_("mprBasisN", "Normal", vec3(0.0f, 0.0f, 1.0f), vec3(-1.0f), vec3(1.0f), vec3(.1f),
                 InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , mprBasisR_("mprBasisR", "Right", vec3(1.0f, 0.0f, 0.0f), vec3(-1.0f), vec3(1.0f), vec3(.1f),
                 InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , mprBasisU_("mprBasisU", "Up", vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f), vec3(1.0f), vec3(.1f),
                 InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox)
    , slabThickness_("slabThickness", "Thickness (mm)", 1.0f, 0.0f, 100.0f, 1.0f)
    , depth_("depth", "Depth", 0.0f, -100.0f, 100.0f)
    , zoom_("zoom", "Zoom", 100.0f, 0.0f, 1000.0f, 0.01f)
    , adjustCam_("adjustCam", "Adjust Cam")
    , enableStaticCam_("enableStaticCam", "Static Cam", true)
    , cam_("cam", "Camera")
    , cursor_("cursor", "Cursor", vec2(0.5f), vec2(0.0f), vec2(1.0f), vec2(0.1f),
              InvalidationLevel::InvalidOutput, PropertySemantics::SpinBox) {

    addPort(inport_);
    addPort(outport_);

    addProperties(recenterP_, mprP_, mprBasisN_, mprBasisR_, mprBasisU_, slabThickness_, depth_,
                  zoom_, adjustCam_, enableStaticCam_, cam_, cursor_);

    mprP_.setReadOnly(true);
    mprBasisN_.setReadOnly(true);
    mprBasisU_.setReadOnly(true);
    mprBasisR_.setReadOnly(true);
    cursor_.setReadOnly(true);

    const auto calcPRange = [this]() {
        // ...
    };

    const auto calcCenter = [this]() {
        const auto vol = inport_.getData();
        const auto basis = vol->getBasis();
        const auto diag = basis[0] + basis[1] + basis[2];
        mprP_.set(vol->getOffset() + diag * 0.5f);
    };

    const auto calcPFromDepth = [this]() {
        const auto N = glm::normalize(mprBasisN_.get());
        mprP_.set(mprP_.get() + depth_.get() * N);

        // TODO slide through volume (find available length along normal)
        // TODO how to calculate other planes when P is not center anymore?
    };

    const auto calcDepthRange = [this]() {
        // ...
    };

    // TODO prevent volume value clamping

    const auto calcSlabRange = [this]() {
        const auto vol = inport_.getData();
        // TODO update slabOffset range in new model coords
    };

    const auto calcCamera = [this]() {
        // TODO set camera orthographic
        cam_.setLookFrom(mprP_.get() + mprBasisN_.get() * zoom_.get());
        cam_.setLookTo(mprP_.get());
        cam_.setLookUp(mprBasisU_.get());
        cam_.setNearFarPlaneDist(0.1f, zoom_.get() + slabThickness_.get() + 1.0f);
    };

    const auto intersectPlane = [](vec3 pos, vec3 dir, vec3 middle, vec3 normal) {
        const float a = glm::dot(dir, normal);
        const float b = glm::dot(middle - pos, normal);
        return b / a;
    };

    const auto calcPFromCursor = [this, intersectPlane, calcCamera]() {
        const auto pt = cursor_.get();
        auto camPos = cam_.getLookFrom();
        auto nearPlanePt =
            cam_.getWorldPosFromNormalizedDeviceCoords(vec3(pt * 2.f - 1.f, -1.f));
        auto viewDir = glm::normalize(nearPlanePt - camPos);
        auto sign = glm::dot(viewDir, mprBasisN_.get()) >= 0.0f ? -1.0f : 1.0f;
        float t = intersectPlane(nearPlanePt, viewDir, mprP_, sign * mprBasisN_.get());
        if (t < 0.0f) {
            throw Exception("No hit point");
        }
        mprP_.set(camPos + viewDir * t);
    };

    const auto projectP = [this]() {
        const auto view = cam_.get().getViewMatrix();
        const auto proj = cam_.get().getProjectionMatrix();
        const auto p = proj * view * vec4(mprP_.get(), 1.0f);
        return vec2(p / p.w + 1.0f) * 0.5f;
    };

    // Reset on new data
    inport_.onChange([this, calcCenter, calcSlabRange, calcPRange, calcCamera]() {
        calcPRange();
        calcCenter();
        calcSlabRange();
        calcCamera();
    });

    mprBasisN_.onChange([this, calcSlabRange, calcCamera]() {
        calcSlabRange();
        if (enableStaticCam_.get()) calcCamera();
    });

    // MPR point interaction
    mprP_.onChange([this, calcSlabRange, projectP]() {
        calcSlabRange();
        //cursor_.set(projectP()); //!!!! endless loop!!!!
    });
    recenterP_.onChange(calcCenter);
    depth_.onChange(calcPFromDepth);
    cursor_.onChange(calcPFromCursor);

    // Camera interaction
    zoom_.onChange(calcCamera);
    adjustCam_.onChange(calcCamera);
    cam_.setReadOnly(enableStaticCam_.get());
    enableStaticCam_.onChange([this]() { cam_.setReadOnly(enableStaticCam_.get()); });
}

MPRProxyGeometry::~MPRProxyGeometry() {}

void MPRProxyGeometry::process() {
    const auto vol = inport_.getData();
    const auto basis = vol->getBasis();

    // const auto maxLength_model = glm::max(glm::length(basis[0]), glm::max(glm::length(basis[1]),
    // glm::length(basis[2])));
    const auto length_model = glm::length(basis[0] + basis[1] + basis[2]) * 2.0f;

    const auto thickness_model = slabThickness_.get();

    const auto P = mprP_.get();
    const auto N = glm::normalize(mprBasisN_.get()) * thickness_model;
    const auto R = glm::normalize(mprBasisR_.get()) * length_model;
    const auto U = glm::normalize(mprBasisU_.get()) * length_model;

    // (P,N) defines one of the three MPR planes
    // (R,U) define orientation of image -- replace by camera?

    // vertices
    glm::vec3 origin(P - 0.5f * (R + U + N));
    glm::vec3 e1(R);
    glm::vec3 e2(U);
    glm::vec3 e3(N);

    // colors
    glm::vec4 colOrigin(mprBasisN_.get(), 1.0f);
    glm::vec4 c1(mprBasisN_.get(), 0.0f);
    glm::vec4 c2(mprBasisN_.get(), 0.0f);
    glm::vec4 c3(mprBasisN_.get(), 0.0f);

    std::shared_ptr<Mesh> mesh = SimpleMeshCreator::parallelepiped(origin, e1, e2, e3, origin, e1,
                                                                   e2, e3, colOrigin, c1, c2, c3);

    outport_.setData(mesh);
}

}  // namespace inviwo
