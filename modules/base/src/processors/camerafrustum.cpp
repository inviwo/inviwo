/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/base/processors/camerafrustum.h>

#include <inviwo/core/algorithm/boundingbox.h>       // for boundingBox
#include <inviwo/core/algorithm/markdown.h>          // for operator""_help, operator""_unindent...
#include <inviwo/core/ports/meshport.h>              // for MeshOutport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>    // for Tags, Tags::CPU
#include <inviwo/core/properties/cameraproperty.h>   // for CameraProperty
#include <inviwo/core/properties/ordinalproperty.h>  // for ordinalColor, FloatVec4Property, Ord...
#include <inviwo/core/util/glmvec.h>                 // for vec4
#include <modules/base/algorithm/meshutils.h>        // for cameraFrustum

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CameraFrustum::processorInfo_{
    "org.inviwo.CameraFrustum",  // Class identifier
    "Camera Frustum",            // Display name
    "Information",               // Category
    CodeState::Stable,           // Code state
    Tags::CPU,                   // Tags
    R"(Creates a line mesh of a frustum for a given camera.
    
    Example Workspace:
    [base/camera_frustum.inv](file:~modulePath~/data/workspaces/camera_frustum.inv)
    )"_unindentHelp};

const ProcessorInfo& CameraFrustum::getProcessorInfo() const { return processorInfo_; }

CameraFrustum::CameraFrustum()
    : Processor()
    , mesh_("mesh", "Line mesh (GL_LINES)"_help)
    , camera_("camera", "Camera", "Color of the lines"_help, util::boundingBox(mesh_))
    , color_("color", "Color",
             util::ordinalColor(1.0f, 1.0f, 1.0f)
                 .set("Camera for which to create the frustum."_help)) {

    addPort(mesh_);
    addProperties(color_, camera_);
}

void CameraFrustum::process() { mesh_.setData(meshutil::cameraFrustum(camera_, color_)); }

}  // namespace inviwo
