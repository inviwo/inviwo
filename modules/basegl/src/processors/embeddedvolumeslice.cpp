/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/basegl/processors/embeddedvolumeslice.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/plane.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/algorithm/cubeplaneintersection.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <limits>
#include <numeric>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo EmbeddedVolumeSlice::processorInfo_{
    "org.inviwo.EmbeddedVolumeSlice",  // Class identifier
    "Embedded Volume Slice",           // Display name
    "Volume Operation",                // Category
    CodeState::Stable,                 // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo EmbeddedVolumeSlice::getProcessorInfo() const { return processorInfo_; }

EmbeddedVolumeSlice::EmbeddedVolumeSlice()
    : Processor()
    , inport_{"volume"}
    , backgroundPort_{"background"}
    , outport_{"outport"}
    , shader_{"embeddedvolumeslice.vert", "embeddedvolumeslice.frag", false}
    , planeNormal_{"planeNormal",          "Plane Normal",      vec3(1.f, 0.f, 0.f),
                   vec3(-1.f, -1.f, -1.f), vec3(1.f, 1.f, 1.f), vec3(0.01f, 0.01f, 0.01f)}
    , planePosition_{"planePosition", "Plane Position", vec3(0.5f), vec3(0.0f), vec3(1.0f)}
    , transferFunction_{"transferFunction", "Transfer Function", &inport_}
    , camera_{"camera", "Camera", util::boundingBox(inport_)}
    , trackball_{&camera_}
    , picking_{this, 1, [this](PickingEvent* e) { handlePicking(e); }} {

    addPort(inport_);
    addPort(backgroundPort_).setOptional(true);
    addPort(outport_);

    addProperties(planeNormal_, planePosition_, transferFunction_, camera_, trackball_);

    planePosition_.onChange([this]() { planeSettingsChanged(); });
    planeNormal_.onChange([this]() { planeSettingsChanged(); });
    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void EmbeddedVolumeSlice::initializeResources() {
    auto vso = shader_.getVertexShaderObject();
    vso->clearInDeclarations();
    vso->addInDeclaration("in_Vertex", buffertraits::PositionsBuffer::bi().location, "vec3");
    shader_.build();
    planeSettingsChanged();
}

void EmbeddedVolumeSlice::planeSettingsChanged() {
    if (!inport_.hasData()) return;

    // In texture space
    const Plane plane{*planePosition_, glm::normalize(*planeNormal_)};

    auto& intersections =
        embeddedMesh_.getEditableVertices()->getEditableRAMRepresentation()->getDataContainer();
    intersections.clear();

    if (embeddedMesh_.getNumberOfIndicies() == 0) {
        embeddedMesh_.addIndexBuffer(DrawType::Triangles, ConnectivityType::None);
    }

    auto& inds = embeddedMesh_.getIndices(0)->getEditableRAMRepresentation()->getDataContainer();
    inds.clear();

    util::cubePlaneIntersectionAppend(plane, intersections, inds);

    embeddedMesh_.setModelMatrix(inport_.getData()->getModelMatrix());
    embeddedMesh_.setWorldMatrix(inport_.getData()->getWorldMatrix());
}

void EmbeddedVolumeSlice::handlePicking(PickingEvent* p) {
    if (inport_.hasData() && p->getHoverState() == PickingHoverState::Move &&
        p->getPressItems().empty()) {
        const auto data = inport_.getData();

        const Plane plane{*planePosition_, glm::normalize(*planeNormal_)};
        const auto& ct = data->getCoordinateTransformer(camera_);

        const vec3 ndc1{p->getNDC().x, p->getNDC().y, -1.0};
        const auto worldPos1 = camera_.getWorldPosFromNormalizedDeviceCoords(ndc1);
        const auto dataPos1 = vec3{ct.getWorldToDataMatrix() * vec4{worldPos1, 1.0f}};

        const vec3 ndc2{p->getNDC().x, p->getNDC().y, 1.0};
        const auto worldPos2 = camera_.getWorldPosFromNormalizedDeviceCoords(ndc2);
        const auto dataPos2 = vec3{ct.getWorldToDataMatrix() * vec4{worldPos2, 1.0f}};

        if (const auto dataPoint = plane.getIntersection(dataPos1, dataPos2); dataPoint) {
            const auto index =
                static_cast<size3_t>(vec3{ct.getDataToIndexMatrix() * vec4{*dataPoint, 1.0f}});

            const auto cind = glm::clamp(index, size3_t{0}, data->getDimensions() - size3_t{1});
            const auto value = data->getRepresentation<VolumeRAM>()->getAsDVec4(cind);

            const auto worldPos = vec3{ct.getDataToWorldMatrix() * vec4{*dataPoint, 1.0f}};

            p->setToolTip(fmt::format("<div>{}\n<pre>Index: {}\nWorld: {}\nValue: {}</pre></div>",
                                      getDisplayName(), index, worldPos, value));
            p->markAsUsed();
        }
    } else if (p->getHoverState() == PickingHoverState::Exit) {
        p->setToolTip("");
    }
}

void EmbeddedVolumeSlice::process() {
    if (backgroundPort_.hasData() &&
        !shader_.getFragmentShaderObject()->hasShaderDefine("BACKGROUND_AVAILABLE")) {
        shader_.getFragmentShaderObject()->addShaderDefine("BACKGROUND_AVAILABLE");
        shader_.build();
    } else if (!backgroundPort_.hasData() &&
               shader_.getFragmentShaderObject()->hasShaderDefine("BACKGROUND_AVAILABLE")) {
        shader_.getFragmentShaderObject()->removeShaderDefine("BACKGROUND_AVAILABLE");
        shader_.build();
    }

    utilgl::activateTargetAndClearOrCopySource(outport_, backgroundPort_);
    shader_.activate();

    TextureUnitContainer units;
    utilgl::bindAndSetUniforms(shader_, units, inport_);
    utilgl::bindAndSetUniforms(shader_, units, transferFunction_);
    if (backgroundPort_.hasData()) {
        utilgl::bindAndSetUniforms(shader_, units, backgroundPort_, ImageType::ColorDepthPicking);
    }

    utilgl::setShaderUniforms(shader_, embeddedMesh_, "geometry");
    shader_.setUniform("pickColor", picking_.getColor(0));
    utilgl::setUniforms(shader_, camera_);

    MeshDrawerGL::DrawObject drawer(embeddedMesh_.getRepresentation<MeshGL>(),
                                    embeddedMesh_.getDefaultMeshInfo());
    drawer.draw();

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
