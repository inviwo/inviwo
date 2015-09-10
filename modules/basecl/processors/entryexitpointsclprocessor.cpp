/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "entryexitpointsclprocessor.h"
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <modules/opencl/image/imagecl.h>
#include <modules/opencl/image/imageclgl.h>
#include <modules/opencl/settings/openclsettings.h>
#include <modules/opencl/syncclgl.h>

namespace inviwo {

ProcessorClassIdentifier(EntryExitPointsCLProcessor, "org.inviwo.EntryExitPointsCL");
ProcessorDisplayName(EntryExitPointsCLProcessor, "Entry Exit Points");
ProcessorTags(EntryExitPointsCLProcessor, Tags::CL);
ProcessorCategory(EntryExitPointsCLProcessor, "Geometry Rendering");
ProcessorCodeState(EntryExitPointsCLProcessor, CODE_STATE_STABLE);

EntryExitPointsCLProcessor::EntryExitPointsCLProcessor()
    : Processor()
    , KernelObserver()
    , geometryPort_("geometry")
    , entryPort_(
          "entry-points",
          DataVec4FLOAT32::get())  // Using 8-bits will create artifacts when entering the volume
    , exitPort_("exit-points", DataVec4FLOAT32::get())
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f), &geometryPort_)
    , workGroupSize_("wgsize", "Work group size", ivec2(8, 8), ivec2(0), ivec2(256))
    , useGLSharing_("glsharing", "Use OpenGL sharing", true)
    , trackball_(&camera_) {
    addPort(geometryPort_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperty(camera_);
    addProperty(workGroupSize_);
    addProperty(useGLSharing_);
    addProperty(trackball_);
    entryPort_.addResizeEventListener(&camera_);
    // Will enable the processor to invalidate when the kernel has recompiled
    entryExitPoints_.addObserver(this);
}

EntryExitPointsCLProcessor::~EntryExitPointsCLProcessor() {}

void EntryExitPointsCLProcessor::initialize() {
    Processor::initialize();

    if (!InviwoApplication::getPtr()->getSettingsByType<OpenCLSettings>()->isSharingEnabled()) {
        useGLSharing_.setReadOnly(true);
        useGLSharing_.set(false);
    }
}

void EntryExitPointsCLProcessor::deinitialize() {}

void EntryExitPointsCLProcessor::process() {
    auto mesh = geometryPort_.getData();
    if (!mesh) return;

    if (mesh->getNumberOfIndicies() == 0) {
        LogInfo("Unable to compute entry exit points for a mesh without indices");
        return;
    }
    // Computes entry exit points in texture coordinates
    entryExitPoints_.computeEntryExitPoints(
        mesh.get(), camera_.viewMatrix(), camera_.projectionMatrix(),
        entryPort_.getEditableData()->getColorLayer(), exitPort_.getEditableData()->getColorLayer(),
        useGLSharing_.get());
}

}  // namespace
