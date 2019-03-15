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

#pragma once

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/interaction/cameratrackball.h>

#include <modules/basegl/algorithm/entryexitpoints.h>

namespace inviwo {

/** \docpage{org.inviwo.GeometryEntryExitPoints, Geometry Entry Exit Points}
 * ![](org.inviwo.GeometryEntryExitPoints.png?classIdentifier=org.inviwo.GeometryEntryExitPoints)
 * Computes entry and exit points of a given mesh based on the current camera parameters.
 * The positions of the input geometry are mapped to Data space, i.e. texture coordinates, of the
 * input volume. The output color will be zero if no intersection is found, otherwise.
 *
 * ### Inports
 *   * __volume__     Input volume used to map geometry positions to Data space
 *   * __geometry__   Input mesh used for determining entry and exit points
 *
 * ### Outports
 *   * __entry__ The first hit point in texture coordinates [0,1]
 *   * __exit__  The last hit point in texture coordinates [0,1]
 *
 * ### Properties
 *   * __Camera__   Scene camera
 */

class IVW_MODULE_BASEGL_API GeometryEntryExitPoints : public Processor {
public:
    GeometryEntryExitPoints();
    virtual ~GeometryEntryExitPoints() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volumeInport_;
    MeshInport meshInport_;
    ImageOutport entryPort_;
    ImageOutport exitPort_;

    CameraProperty camera_;
    BoolProperty capNearClipping_;
    CameraTrackball trackball_;

    algorithm::EntryExitPointsHelper entryExitHelper_;
};

}  // namespace inviwo
