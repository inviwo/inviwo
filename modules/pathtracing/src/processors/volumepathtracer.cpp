/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023 Inviwo Foundation
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

#include <inviwo/pathtracing/processors/volumepathtracer.h>
#include <modules/opengl/openglmodule.h>
#include <modules/opengl/shader/shadermanager.h>

#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/image/imagegl.h>

#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/shader/shaderutils.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumePathTracer::processorInfo_{
    "org.inviwo.VolumePathTracer",  // Class identifier
    "VolumePathTracer",        // Display name
    "Volume",                   // Category
    CodeState::Experimental,       // Code state
    Tags::GL,                    // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo VolumePathTracer::getProcessorInfo() const { return processorInfo_; }

VolumePathTracer::VolumePathTracer()
    : Processor()
    , volumePort_("Volume")
    , entryPort_("EntryPoints")
    , exitPort_("ExitPoints")
    , lights_("LightSources")
    //, minMaxOpacity_{"VolumeMinMaxOpacity"}
    , outport_("Outport")
    , shader_({{ShaderType::Compute, "bidirectionalvolumepathtracer.comp"}}) {
    addPort(volumePort_, "VolumePortGroup");
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addPort(outport_, "ImagePortGroup1");
    addPort(lights_, "LightPortGroup");
    //addPort(minMaxOpacity_);
    //minMaxOpacity_.setOptional(true);
    /*
    volumePort_.onChange([this]() { invalidateProgressiveRendering(); });
    entryPort_.onChange([this]() { invalidateProgressiveRendering(); });
    exitPort_.onChange([this]() { invalidateProgressiveRendering(); });
    lights_.onChange([this]() { updateLightSources(); });
    */
    /*
    minMaxOpacity_.onConnect([this]() { partitionedTransmittance_.setVisible(true); });
    minMaxOpacity_.onDisconnect([this]() {
        partitionedTransmittance_.set(false);
        partitionedTransmittance_.setVisible(false);
    }
    */

   // What can we set up before process
   shader_.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); } );

   //exitPort_.setOptional(true);
   lights_.setOptional(true);
   volumePort_.setOptional(true);
}


void VolumePathTracer::process() {

    // gets a pointer to outport_ data, and entry writes to that;
    Image* outImage = outport_.getEditableData().get();
    ImageGL* outImageGL = outImage->getEditableRepresentation<ImageGL>();
    
    /*
        i got a strange vector related error when doing
        auto entry = entryPort_.getData()->getEditableRepresentation<ImageGL>->clone();
        LayerGL* entryGL = entry->getColorLayerGL();

        it went away when i went back to
        auto entry = entryPort_.getData()->clone();
        auto layerGL = imgInternal->getColorLayer()->getEditableRepresentation<LayerGL>();
    */

    glActiveTexture(GL_TEXTURE1);
    auto exit = exitPort_.getData()->clone();
    LayerGL* exitGL = exit->getColorLayer()->getEditableRepresentation<LayerGL>();
    auto exitTexHandle = exitGL->getTexture()->getID();
    glBindImageTexture(1, exitTexHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    exitGL->setSwizzleMask(swizzlemasks::luminance);

    glActiveTexture(GL_TEXTURE0);
    auto entry = entryPort_.getData()->clone();
    //auto layerGL = imgInternal->getColorLayer()->getEditableRepresentation<LayerGL>();
    LayerGL* entryGL = entry->getColorLayer()->getEditableRepresentation<LayerGL>();
    auto entryTexHandle = entryGL->getTexture()->getID();
    glBindImageTexture(0, entryTexHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    entryGL->setSwizzleMask(swizzlemasks::luminance); // sets the data format (sort of) of the layer.

    
    

    /*
    glActiveTexture(GL_TEXTURE2);
    auto output = outport_.getEditableData().get();
    LayerGL* outputGL = output->getEditableRepresentation<ImageGL>()->getColorLayerGL();
    auto outputTexHandle = exitGL->getTexture()->getID();
    glBindImageTexture(1, exitTexHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    outputGL->setSwizzleMask(swizzlemasks::rgba);
    */

    dispatchPathTracerComputeShader(entryGL, exitGL, entryGL);

    //entry->getRepresentation<ImageGL>()->copyRepresentationsTo(outImageGL);
    outport_.setData(entry);
}

void VolumePathTracer::dispatchPathTracerComputeShader(LayerGL* entryGL, LayerGL* exitGL, LayerGL* outportGL) {
    shader_.activate();

    entryGL->getTexture()->bind();
    shader_.setUniform("entry", 0);

    exitGL->getTexture()->bind();
    shader_.setUniform("exit", 1);

    glDispatchCompute(512/16, 512/16, 1);

    shader_.deactivate();
}

}  // namespace inviwo
