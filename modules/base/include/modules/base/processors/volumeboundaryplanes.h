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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <inviwo/core/properties/boolproperty.h>

#include <inviwo/core/datastructures/geometry/plane.h>

#include <vector>

namespace inviwo {

/** \docpage{org.inviwo.VolumeBoundaryPlanes, Volume Boundary Planes}
 * ![](org.inviwo.VolumeBoundaryPlanes.png?classIdentifier=org.inviwo.VolumeBoundaryPlanes)
 * Outputs the six planes that enclose the input volume in world space.
 * Order of planes: -X, -Y, -Z, +X, +Y, +Z (sides of volume in model coordinates).
 * Planes face outward by default, but can be flipped.
 *
 * ### Inports
 *   * __volumeInport__ Input volume.
 *
 * ### Outports
 *   * __planeOutport__ The six boundary planes.
 *
 * ### Properties
 *   * __Flip planes__ Switch plane normals between inward and outward.
 */

class IVW_MODULE_BASE_API VolumeBoundaryPlanes : public Processor {
public:
    VolumeBoundaryPlanes();
    virtual ~VolumeBoundaryPlanes() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    DataOutport<std::vector<Plane>> planes_;

    BoolProperty flipPlanes_;
};

}  // namespace inviwo
