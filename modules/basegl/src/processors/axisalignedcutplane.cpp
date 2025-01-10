/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/basegl/processors/axisalignedcutplane.h>

#include <inviwo/core/algorithm/boundingbox.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/network/networklock.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

namespace inviwo {

namespace {

const std::vector<OptionPropertyIntOption> channelsList = util::enumeratedOptions("Channel", 4);

void normalizedPropertyUpdate(IntProperty& prop, int maxValue) {
    if (prop.getMaxValue() == maxValue) {
        return;
    }
    const double t = static_cast<double>(prop.get()) / static_cast<double>(prop.getMaxValue());
    prop.setMaxValue(maxValue);
    prop.set(static_cast<int>(t * maxValue));
}

}  // namespace

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo AxisAlignedCutPlane::processorInfo_{
    "org.inviwo.AxisAlignedCutPlane",  // Class identifier
    "Axis Aligned Cut Plane",          // Display name
    "Volume Operation",                // Category
    CodeState::Experimental,           // Code state
    Tags::GL | Tag{"Volume"},          // Tags
    R"(Render up to three volume slices aligned with the principal axes of a volume.
    )"_unindentHelp,
};
const ProcessorInfo& AxisAlignedCutPlane::getProcessorInfo() const { return processorInfo_; }

AxisAlignedCutPlane::AxisAlignedCutPlane()
    : Processor()
    , volume_{"volume", "Input Volume"_help}
    , imageInport_{"imageInport", "Optional background image"_help}
    , outport_{"outport", "Output image  containing the volume slices"_help}

    , channel_{"channel", "Channel", channelsList, 0}
    , axes_{{{"xAxis", "X Axis", true}, {"yAxis", "Y Axis", true}, {"zAxis", "Z Axis", true}}}
    , slices_{{{"sliceX", "Slice",
                util::ordinalCount(50).setMin(1).set(PropertySemantics::Default)},
               {"sliceY", "Slice",
                util::ordinalCount(50).setMin(1).set(PropertySemantics::Default)},
               {"sliceZ", "Slice",
                util::ordinalCount(50).setMin(1).set(PropertySemantics::Default)}}}

    , applyTF_{"applyTF", "Apply TF", true}
    , tf_{"transferfunction", "Transfer function",
          TransferFunction{{{0.0, vec4{0.0f, 0.0f, 0.0f, 1.0f}}, {1.0, vec4{1.0f}}}}, &volume_}
    , camera_{"camera", "Camera", util::boundingBox(volume_)}
    , trackball_{&camera_}
    , sliceShader_{"axisalignedcutplaneslice.vert", "axisalignedcutplaneslice.frag"} {

    addPorts(volume_, imageInport_, outport_);
    imageInport_.setOptional(true);

    for (size_t i = 0; i < axes_.size(); ++i) {
        axes_[i].addProperty(slices_[i]);
        slices_[i].setSerializationMode(PropertySerializationMode::All);
    }
    addProperties(channel_, axes_[0], axes_[1], axes_[2], applyTF_, tf_, camera_, trackball_);

    sliceShader_.onReload([&]() { this->invalidate(InvalidationLevel::InvalidResources); });

    volume_.onChange([&]() {
        if (!volume_.hasData()) return;
        const auto volume = volume_.getData();

        NetworkLock const lock(this);

        const ivec3 dims{volume->getDimensions()};
        for (size_t i = 0; i < 3; ++i) {
            normalizedPropertyUpdate(slices_[i], dims[i]);
            slices_[i].setCurrentStateAsDefault();
        }
        const auto channels = volume->getDataFormat()->getComponents();
        if (channels != channel_.size()) {
            channel_.replaceOptions(std::vector<OptionPropertyIntOption>{
                channelsList.begin(), channelsList.begin() + channels});
            channel_.setCurrentStateAsDefault();
        }
    });

    auto verticesBuffer =
        util::makeBuffer<vec2>({{0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});
    auto indices_ = util::makeIndexBuffer({0, 1, 2, 3});

    mesh_.addBuffer(BufferType::PositionAttrib, verticesBuffer);
    mesh_.addBuffer(BufferType::TexCoordAttrib, verticesBuffer);
    mesh_.addIndices(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices_);
}

void AxisAlignedCutPlane::process() {
    if (imageInport_.isReady()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_, ImageType::ColorDepth);
    } else {
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepth);
    }

    auto volume = volume_.getData();
    const size3_t volDims = volume_.getData()->getDimensions();
    const vec3 volDimsInv{1.0f / vec3{volDims}};
    const vec3 halfVoxel = vec3{0.5f} * volDimsInv;

    auto sliceTrafos = [&]() -> std::array<mat4, 3> {
        const vec3 offset =
            (vec3{slices_[0], slices_[1], slices_[2]} - 1.0f) * volDimsInv + halfVoxel;

        std::array<mat4, 3> trafos{{
            {glm::rotate(-glm::half_pi<float>(), vec3(0.0f, 1.0f, 0.0f))},
            {glm::rotate(glm::half_pi<float>(), vec3(1.0f, 0.0f, 0.0f))},
            mat4{1.0f},
        }};
        // consider 0.5 voxel offset per slice
        trafos[0][3] = vec4{offset[0], 0.0f, 0.0f, 1.0f};
        trafos[1][3] = vec4{0.0f, offset[1], 0.0f, 1.0f};
        trafos[2][3] = vec4{0.0f, 0.0f, offset[2], 1.0f};

        return trafos;
    }();

    const utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
    const utilgl::BlendModeState blend(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    TextureUnitContainer cont;

    sliceShader_.activate();

    utilgl::setUniforms(sliceShader_, camera_);
    utilgl::bindAndSetUniforms(sliceShader_, cont, volume_);
    utilgl::bindAndSetUniforms(sliceShader_, cont, tf_);
    sliceShader_.setUniform("useTF", applyTF_);

    auto drawer = MeshDrawerGL::DrawObject{mesh_};
    for (size_t i = 0; i < 3; ++i) {
        if (!axes_[i].isChecked()) continue;

        sliceShader_.setUniform("sliceTrafo", sliceTrafos[i]);
        drawer.draw();
    }

    sliceShader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace inviwo
