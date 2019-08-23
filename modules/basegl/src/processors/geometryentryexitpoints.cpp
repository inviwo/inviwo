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

#include <modules/basegl/processors/geometryentryexitpoints.h>
#include <inviwo/core/algorithm/boundingbox.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GeometryEntryExitPoints::processorInfo_{
    "org.inviwo.GeometryEntryExitPoints",  // Class identifier
    "Geometry Entry Exit Points",          // Display name
    "Mesh Rendering",                      // Category
    CodeState::Stable,                     // Code state
    Tags::GL,                              // Tags
};
const ProcessorInfo GeometryEntryExitPoints::getProcessorInfo() const { return processorInfo_; }

GeometryEntryExitPoints::GeometryEntryExitPoints()
    : Processor()
    , volumeInport_("volume")
    , meshInport_("geometry")
    , entryPort_("entry", DataVec4UInt16::get())
    , exitPort_("exit", DataVec4UInt16::get())
    , camera_("camera", "Camera", util::boundingBox(meshInport_))
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_) {
    addPort(volumeInport_);
    addPort(meshInport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(capNearClipping_);
    addProperty(camera_);
    addProperty(trackball_);

    for (auto& shader : entryExitHelper_.getShaders()) {
        shader.get().onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
    }
}

void GeometryEntryExitPoints::process() {
    entryExitHelper_(*entryPort_.getEditableData().get(), *exitPort_.getEditableData().get(),
                     camera_.get(), *volumeInport_.getData().get(), *meshInport_.getData().get(),
                     capNearClipping_.get());
}

}  // namespace inviwo
