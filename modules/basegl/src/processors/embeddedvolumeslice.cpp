/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>                          // for boundingBox
#include <inviwo/core/algorithm/cubeplaneintersection.h>                // for cubePlaneIntersec...
#include <inviwo/core/datastructures/buffer/buffer.h>                   // for IndexBuffer
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAMPrecision
#include <inviwo/core/datastructures/coordinatetransformer.h>           // for StructuredCameraC...
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for ConnectivityType
#include <inviwo/core/datastructures/geometry/mesh.h>                   // for Mesh::BufferInfo
#include <inviwo/core/datastructures/geometry/plane.h>                  // for Plane
#include <inviwo/core/datastructures/geometry/typedmesh.h>              // for TypedMesh, Positi...
#include <inviwo/core/datastructures/image/imagetypes.h>                // for ImageType, ImageT...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/datastructures/volume/volumeram.h>                // for VolumeRAM
#include <inviwo/core/interaction/events/pickingevent.h>                // for PickingEvent
#include <inviwo/core/interaction/pickingmapper.h>                      // for PickingMapper
#include <inviwo/core/interaction/pickingstate.h>                       // for PickingHoverState
#include <inviwo/core/ports/imageport.h>                                // for ImageInport, Base...
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::GL
#include <inviwo/core/properties/cameraproperty.h>                      // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>                   // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatVec3Property
#include <inviwo/core/util/glmvec.h>                                    // for vec3, size3_t, dvec3
#include <modules/opengl/geometry/meshgl.h>                             // for MeshGL
#include <modules/opengl/rendering/meshdrawergl.h>                      // for MeshDrawerGL::Dra...
#include <modules/opengl/shader/shader.h>                               // for Shader, Shader::B...
#include <modules/opengl/shader/shaderobject.h>                         // for ShaderObject
#include <modules/opengl/shader/shaderutils.h>                          // for setShaderUniforms
#include <modules/opengl/texture/textureunit.h>                         // for TextureUnitContainer
#include <modules/opengl/texture/textureutils.h>                        // for bindAndSetUniforms
#include <modules/opengl/volume/volumeutils.h>                          // for bindAndSetUniforms

#include <functional>     // for __base, function
#include <memory>         // for shared_ptr, share...
#include <optional>       // for optional
#include <string>         // for string
#include <string_view>    // for string_view
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <fmt/core.h>         // for basic_string_view
#include <glm/common.hpp>     // for clamp
#include <glm/geometric.hpp>  // for normalize
#include <glm/gtx/io.hpp>     // for operator<<
#include <glm/mat4x4.hpp>     // for operator*, mat
#include <glm/vec3.hpp>       // for operator*, vec<>:...
#include <glm/vec4.hpp>       // for operator*, operator+

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo EmbeddedVolumeSlice::processorInfo_{
    "org.inviwo.EmbeddedVolumeSlice",  // Class identifier
    "Embedded Volume Slice",           // Display name
    "Volume Operation",                // Category
    CodeState::Stable,                 // Code state
    Tags::GL,                          // Tags
};
const ProcessorInfo& EmbeddedVolumeSlice::getProcessorInfo() const { return processorInfo_; }

EmbeddedVolumeSlice::EmbeddedVolumeSlice()
    : Processor()
    , inport_{"volume"}
    , backgroundPort_{"background"}
    , outport_{"outport"}
    , shader_{"embeddedvolumeslice.vert", "embeddedvolumeslice.frag", Shader::Build::No}
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

    embeddedMesh_.setModelMatrix(inport_.getData()->getModelMatrix());
    embeddedMesh_.setWorldMatrix(inport_.getData()->getWorldMatrix());
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
