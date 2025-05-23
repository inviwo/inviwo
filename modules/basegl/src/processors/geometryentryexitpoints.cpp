/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/boundingbox.h>         // for boundingBox
#include <inviwo/core/interaction/cameratrackball.h>   // for CameraTrackball
#include <inviwo/core/ports/imageport.h>               // for ImageOutport
#include <inviwo/core/ports/meshport.h>                // for MeshInport
#include <inviwo/core/ports/volumeport.h>              // for VolumeInport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeState::Stable
#include <inviwo/core/processors/processortags.h>      // for Tags, Tags::GL
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/cameraproperty.h>     // for CameraProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel, InvalidationLev...
#include <inviwo/core/util/formats.h>                  // for DataFormat, DataVec4UInt16
#include <modules/basegl/algorithm/entryexitpoints.h>  // for CapNearClip, EntryExitPointsHelper

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GeometryEntryExitPoints::processorInfo_{
    "org.inviwo.GeometryEntryExitPoints",  // Class identifier
    "Geometry Entry Exit Points",          // Display name
    "Mesh Rendering",                      // Category
    CodeState::Stable,                     // Code state
    Tags::GL,                              // Tags
    R"(Computes entry and exit points of a given mesh based on the current camera parameters.
    The positions of the input geometry are mapped to Data space, i.e. texture coordinates, of the
    input volume. The output color will be zero if no intersection is found, otherwise.
    )"_unindentHelp,
};
const ProcessorInfo& GeometryEntryExitPoints::getProcessorInfo() const { return processorInfo_; }

GeometryEntryExitPoints::GeometryEntryExitPoints()
    : Processor()
    , volumeInport_("volume", "Input volume used to map geometry positions to Data space"_help)
    , meshInport_("geometry", "Input mesh used for determining entry and exit points"_help)
    , entryPort_("entry", "The first hit point in texture coordinates [0,1]"_help,
                 DataVec4UInt16::get())
    , exitPort_("exit", "The last hit point in texture coordinates [0,1]"_help,
                DataVec4UInt16::get())
    , camera_("camera", "Camera", util::boundingBox(meshInport_))
    , capNearClipping_("capNearClipping", "Cap near plane clipping", true)
    , trackball_(&camera_) {
    addPort(volumeInport_);
    addPort(meshInport_);
    addPort(entryPort_, "ImagePortGroup1");
    addPort(exitPort_, "ImagePortGroup1");
    addProperties(capNearClipping_, camera_, trackball_);

    onReloadCallback_ =
        entryExitHelper_.onReload([this]() { invalidate(InvalidationLevel::InvalidResources); });
}

void GeometryEntryExitPoints::process() {
    entryExitHelper_(*entryPort_.getEditableData().get(), *exitPort_.getEditableData().get(),
                     camera_.get(), *volumeInport_.getData().get(), *meshInport_.getData().get(),
                     capNearClipping_ ? algorithm::CapNearClip::Yes : algorithm::CapNearClip::No,
                     algorithm::IncludeNormals::Yes);
}

}  // namespace inviwo
