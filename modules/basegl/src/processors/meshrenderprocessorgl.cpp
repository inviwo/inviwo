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

#include <modules/basegl/processors/meshrenderprocessorgl.h>
#include <modules/opengl/geometry/meshgl.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/interaction/trackball.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/base/algorithm/mesh/axisalignedboundingbox.h>
#include <modules/base/algorithm/mesh/meshcameraalgorithms.h>
#include <modules/opengl/rendering/meshdrawergl.h>
#include <inviwo/core/processors/processor.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>

#include <modules/base/algorithm/dataminmax.h>
#include <inviwo/core/algorithm/boundingbox.h>

#include <limits>

namespace inviwo {

const ProcessorInfo MeshRenderProcessorGL::processorInfo_{
    "org.inviwo.GeometryRenderGL",  // Class identifier
    "Mesh Renderer",                // Display name
    "Mesh Rendering",               // Category
    CodeState::Stable,              // Code state
    Tags::GL,                       // Tags
};
const ProcessorInfo MeshRenderProcessorGL::getProcessorInfo() const { return processorInfo_; }

MeshRenderProcessorGL::MeshRenderProcessorGL()
    : Processor()
    , inport_("geometry")
    , imageInport_("imageInport")
    , outport_("image")
    , camera_("camera", "Camera", util::boundingBox(inport_))
    , trackball_(&camera_)
    , overrideColorBuffer_("overrideColorBuffer", "Override Color Buffer", false,
                           InvalidationLevel::InvalidResources)
    , overrideColor_("overrideColor", "Override Color", vec4(0.75f, 0.75f, 0.75f, 1.0f), vec4(0.0f),
                     vec4(1.0f))
    , geomProperties_("geometry", "Geometry Rendering Properties")
    , cullFace_("cullFace", "Cull Face",
                {{"culldisable", "Disable", GL_NONE},
                 {"cullfront", "Front", GL_FRONT},
                 {"cullback", "Back", GL_BACK},
                 {"cullfrontback", "Front & Back", GL_FRONT_AND_BACK}},
                0)
    , enableDepthTest_("enableDepthTest_", "Enable Depth Test", true)
    , lightingProperty_("lighting", "Lighting", &camera_)
    , layers_("layers", "Output Layers")
    , colorLayer_("colorLayer", "Color", true, InvalidationLevel::InvalidResources)
    , texCoordLayer_("texCoordLayer", "Texture Coordinates", false,
                     InvalidationLevel::InvalidResources)
    , normalsLayer_("normalsLayer", "Normals (World Space)", false,
                    InvalidationLevel::InvalidResources)
    , viewNormalsLayer_("viewNormalsLayer", "Normals (View space)", false,
                        InvalidationLevel::InvalidResources)
    , shader_("meshrendering.vert", "meshrendering.frag", false) {

    addPort(inport_).onChange([this]() { updateDrawers(); });
    addPort(imageInport_).setOptional(true);
    addPort(outport_);

    addProperties(camera_, geomProperties_, lightingProperty_, trackball_, layers_);

    geomProperties_.addProperties(cullFace_, enableDepthTest_, overrideColorBuffer_,
                                  overrideColor_);

    overrideColor_.setSemantics(PropertySemantics::Color)
        .visibilityDependsOn(overrideColorBuffer_, [](const BoolProperty p) { return p.get(); });

    layers_.addProperties(colorLayer_, texCoordLayer_, normalsLayer_, viewNormalsLayer_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MeshRenderProcessorGL::~MeshRenderProcessorGL() = default;

void MeshRenderProcessorGL::initializeResources() {
    utilgl::addShaderDefines(shader_, lightingProperty_);

    auto vert = shader_.getVertexShaderObject();
    auto frag = shader_.getFragmentShaderObject();

    vert->setShaderDefine("OVERRIDE_COLOR_BUFFER", overrideColorBuffer_);
    frag->setShaderDefine("COLOR_LAYER", colorLayer_);

    // first two layers (color and picking) are reserved
    int layerID = 2;
    frag->setShaderDefine("TEXCOORD_LAYER", texCoordLayer_);
    if (texCoordLayer_) frag->addOutDeclaration("tex_coord_out", layerID++);

    frag->setShaderDefine("NORMALS_LAYER", normalsLayer_);
    if (normalsLayer_) frag->addOutDeclaration("normals_out", layerID++);

    frag->setShaderDefine("VIEW_NORMALS_LAYER", viewNormalsLayer_);
    if (viewNormalsLayer_) frag->addOutDeclaration("view_normals_out", layerID++);

    // get a hold of the current output data
    auto prevData = outport_.getData();
    auto numLayers = static_cast<std::size_t>(layerID - 1);  // Don't count picking
    if (prevData->getNumberOfColorLayers() != numLayers) {
        // create new image with matching number of layers
        auto image = std::make_shared<Image>(prevData->getDimensions(), prevData->getDataFormat());
        // update number of layers
        for (auto i = image->getNumberOfColorLayers(); i < numLayers; ++i) {
            image->addColorLayer(std::shared_ptr<Layer>(image->getColorLayer(0)->clone()));
        }
        outport_.setData(image);
    }

    shader_.build();
}

void MeshRenderProcessorGL::process() {
    utilgl::activateTargetAndClearOrCopySource(outport_, imageInport_);
    shader_.activate();

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, enableDepthTest_);
    utilgl::CullFaceState culling(cullFace_);
    utilgl::BlendModeState blendModeStateGL(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    utilgl::setUniforms(shader_, camera_, lightingProperty_, overrideColor_);
    for (auto& drawer : drawers_) {
        utilgl::setShaderUniforms(shader_, *(drawer.second->getMesh()), "geometry");
        shader_.setUniform("pickingEnabled", meshutil::hasPickIDBuffer(drawer.second->getMesh()));
        drawer.second->draw();
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void MeshRenderProcessorGL::updateDrawers() {
    auto changed = inport_.getChangedOutports();
    DrawerMap temp;
    std::swap(temp, drawers_);

    std::map<const Outport*, std::vector<std::shared_ptr<const Mesh>>> data;
    for (auto& elem : inport_.getSourceVectorData()) {
        data[elem.first].push_back(elem.second);
    }

    for (auto elem : data) {
        auto ibegin = temp.lower_bound(elem.first);
        auto iend = temp.upper_bound(elem.first);

        if (util::contains(changed, elem.first) || ibegin == temp.end() ||
            static_cast<long>(elem.second.size()) !=
                std::distance(ibegin, iend)) {  // data is changed or new.

            for (auto geo : elem.second) {
                auto factory = getNetwork()->getApplication()->getMeshDrawerFactory();
                if (auto renderer = factory->create(geo.get())) {
                    drawers_.emplace(std::make_pair(elem.first, std::move(renderer)));
                }
            }
        } else {  // reuse the old data.
            drawers_.insert(std::make_move_iterator(ibegin), std::make_move_iterator(iend));
        }
    }
}

}  // namespace inviwo
