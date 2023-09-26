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

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumePathTracer::processorInfo_{
    "org.inviwo.VolumePathTracer",  // Class identifier
    "VolumePathTracer",        // Display name
    "Undefined",                   // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
    R"(<Explanation of how to use the processor.>)"_unindentHelp};

const ProcessorInfo VolumePathTracer::getProcessorInfo() const { return processorInfo_; }

VolumePathTracer::VolumePathTracer()
    : Processor{}
    , volumePort_{"Volume"}
    , entryPort_{"EntryPoints"}
    , exitPort_{"ExitPoints"}
    , lights_{"LightSources"}
    //, minMaxOpacity_{"VolumeMinMaxOpacity"}
    , outport_{"Outport"} {
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
}

void VolumePathTracer::process() {
    outport_.setData(entryPort_.getData());
}

}  // namespace inviwo
