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
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &inport_)
    , centerViewOnGeometry_("centerView", "Center view on geometry")
    , setNearFarPlane_("setNearFarPlane", "Calculate Near and Far Plane")
    , resetViewParams_("resetView", "Reset Camera")
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
    , shader_("geometryrendering.vert", "geometryrendering.frag", false) {

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);

    imageInport_.setOptional(true);

    addProperty(camera_);

    centerViewOnGeometry_.onChange([&]() {
        if (!inport_.hasData()) return;
        meshutil::centerViewOnMeshes(inport_.getVectorData(), camera_);
    });
    addProperty(centerViewOnGeometry_);
    setNearFarPlane_.onChange([&]() {
        if (!inport_.hasData()) return;
        auto nearFar = meshutil::computeNearFarPlanes(
            meshutil::axisAlignedBoundingBox(inport_.getVectorData()), camera_);
        camera_.setNearFarPlaneDist(nearFar.first, nearFar.second);
    });

    addProperty(setNearFarPlane_);
    resetViewParams_.onChange([this]() { camera_.resetCamera(); });
    addProperty(resetViewParams_);
    outport_.addResizeEventListener(&camera_);
    inport_.onChange([this]() { updateDrawers(); });

    geomProperties_.addProperty(cullFace_);
    geomProperties_.addProperty(enableDepthTest_);
    geomProperties_.addProperty(overrideColorBuffer_);
    geomProperties_.addProperty(overrideColor_);
    overrideColor_.setSemantics(PropertySemantics::Color);
    overrideColor_.setVisible(false);
    overrideColorBuffer_.onChange([&]() { overrideColor_.setVisible(overrideColorBuffer_.get()); });

    addProperty(geomProperties_);
    addProperty(lightingProperty_);
    addProperty(trackball_);

    addProperty(layers_);
    layers_.addProperty(colorLayer_);
    layers_.addProperty(texCoordLayer_);
    layers_.addProperty(normalsLayer_);
    layers_.addProperty(viewNormalsLayer_);

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

MeshRenderProcessorGL::~MeshRenderProcessorGL() = default;

void MeshRenderProcessorGL::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);

    if (overrideColorBuffer_.get()) {
        shader_.getVertexShaderObject()->addShaderDefine("OVERRIDE_COLOR_BUFFER");
    } else {
        shader_.getVertexShaderObject()->removeShaderDefine("OVERRIDE_COLOR_BUFFER");
    }

    if (colorLayer_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("COLOR_LAYER");
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("COLOR_LAYER");
    }

    // first two layers (color and picking) are reserved
    int layerID = 2;
    if (texCoordLayer_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("TEXCOORD_LAYER");
        shader_.getFragmentShaderObject()->addOutDeclaration("tex_coord_out", layerID);
        ++layerID;
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("TEXCOORD_LAYER");
    }

    if (normalsLayer_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("NORMALS_LAYER");
        shader_.getFragmentShaderObject()->addOutDeclaration("normals_out", layerID);
        ++layerID;
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("NORMALS_LAYER");
    }

    if (viewNormalsLayer_.get()) {
        shader_.getFragmentShaderObject()->addShaderDefine("VIEW_NORMALS_LAYER");
        shader_.getFragmentShaderObject()->addOutDeclaration("view_normals_out", layerID);
        ++layerID;
    } else {
        shader_.getFragmentShaderObject()->removeShaderDefine("VIEW_NORMALS_LAYER");
    }

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
    if (imageInport_.isConnected()) {
        utilgl::activateTargetAndCopySource(outport_, imageInport_);
    } else {
        utilgl::activateAndClearTarget(outport_);
    }

    shader_.activate();

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, enableDepthTest_.get());
    utilgl::CullFaceState culling(cullFace_.get());
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
