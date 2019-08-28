/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <modules/abuffergl/processors/abuffergeometryglprocessor.h>
#include <modules/opengl/volume/volumegl.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/volume/volumeutils.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <modules/opengl/image/imagegl.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/openglutils.h>
#include <inviwo/core/algorithm/boundingbox.h>

#include <fstream>

namespace inviwo {

const ProcessorInfo ABufferGeometryGLProcessor::processorInfo_{
    "org.inviwo.ABufferGeometryGLProcessor",  // Class identifier
    "ABufferGeometryGLProcessor",             // Display name
    "ABufferDemo",                            // Category
    CodeState::Experimental,                  // Code state
    Tags::None,                               // Tags
};
const ProcessorInfo ABufferGeometryGLProcessor::getProcessorInfo() const { return processorInfo_; }

ABufferGeometryGLProcessor::ABufferGeometryGLProcessor()
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

    , abuffer_()
    , transparency_("transparency", "Global Transparency", 0.5f, 0.0f, 1.0f, 0.001f)
    , verboseLogging_("logging", "Verbose Log", false)
    , shader_("geometryrendering.vert", "abuffergeometrygl.frag", false) {

    addPort(inport_);
    addPort(imageInport_);
    addPort(outport_);

    imageInport_.setOptional(true);

    addProperty(camera_);

    inport_.onChange([this]() { updateDrawers(); });

    geomProperties_.addProperty(cullFace_);
    geomProperties_.addProperty(enableDepthTest_);
    geomProperties_.addProperty(overrideColorBuffer_);
    geomProperties_.addProperty(overrideColor_);
    overrideColor_.setSemantics(PropertySemantics::Color);
    overrideColor_.setVisible(false);
    overrideColorBuffer_.onChange(
        [this]() { overrideColor_.setVisible(overrideColorBuffer_.get()); });

    addProperty(geomProperties_);
    addProperty(lightingProperty_);
    addProperty(trackball_);

    addProperty(abuffer_.settings_);
    addProperty(transparency_);
    addProperty(verboseLogging_);

    abuffer_.settings_.onChange([this]() { onAbufferSettingChanged(); });
    transparency_.onChange([this]() { onAbufferTransparencyChanged(); });

    shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

ABufferGeometryGLProcessor::~ABufferGeometryGLProcessor() = default;

void ABufferGeometryGLProcessor::initializeResources() {
    // shading defines
    utilgl::addShaderDefines(shader_, lightingProperty_);

    shader_.build();
}

void ABufferGeometryGLProcessor::process() {
    if (verboseLogging_.isModified()) {
        abuffer_.setLogStatus(verboseLogging_.get());
    }

    ABUFFER_PROFILE("Total-Time");

    // tempImage_ = new Image(outport_.getDimensions());
    auto tempImage_ = outport_.getEditableData();
    ImageGL* tempImageGL = tempImage_->getEditableRepresentation<ImageGL>();
    ivec2 dim = outport_.getDimensions();

    bool abufferEnabled = abuffer_.settings_.abufferEnable_.get();

    // abuffer initialization
    if (abufferEnabled) {
        ABUFFER_PROFILE("Initialize-abuffer");
        abuffer_.abuffer_initABuffer(dim, updateRequried_);

        LGL_ERROR;
        abuffer_.abuffer_addShaderDefinesAndBuild(&abuffer_.resolveABufferShader_);

        LGL_ERROR;
        ABUFFER_PROFILE("Reset-abuffer");
        abuffer_.abuffer_addShaderDefinesAndBuild(&abuffer_.resetABufferShader_);
        abuffer_.aBuffer_resetLinkList(tempImageGL, true);

        updateRequried_ = false;
    }

    abuffer_.abuffer_addShaderDefinesAndBuild(&shader_);

    // Rendering
    {
        ABUFFER_PROFILE("Rendering");

        tempImageGL = tempImage_->getEditableRepresentation<ImageGL>();

        if (imageInport_.isConnected())
            utilgl::activateTargetAndCopySource(outport_, imageInport_);
        else
            utilgl::activateAndClearTarget(outport_);

        // initialize shader
        shader_.activate();

        utilgl::setUniforms(shader_, camera_, lightingProperty_);

        if (abufferEnabled) {
            abuffer_.abuffer_addUniforms(&shader_);
            shader_.setUniform("globalTransparency_", transparency_.get());
        }

        utilgl::GlBoolState depthTest(GL_DEPTH_TEST, true);
        utilgl::CullFaceState culling(cullFace_.get());

        for (auto& drawer : drawers_) {
            utilgl::setShaderUniforms(shader_, *(drawer.second->getMesh()), "geometry");
            drawer.second->draw();
        }

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();

        glMemoryBarrierEXT(GL_ALL_BARRIER_BITS_EXT);
    }

    // abuffer resolve
    if (abufferEnabled) {
        ABUFFER_PROFILE("Resolving-abuffer");
        utilgl::activateAndClearTarget(outport_);
        abuffer_.aBuffer_resolveLinkList(tempImageGL, imageInport_.getData().get());
        utilgl::deactivateCurrentTarget();
    }
}

void ABufferGeometryGLProcessor::onAbufferSettingChanged() { updateRequried_ = true; }

void ABufferGeometryGLProcessor::onAbufferTransparencyChanged() { /*updateRequried_ = true;*/
}

void ABufferGeometryGLProcessor::updateDrawers() {
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
