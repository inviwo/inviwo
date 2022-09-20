/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2022 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>           // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/meshport.h>              // for MeshOutport
#include <inviwo/core/ports/volumeport.h>            // for VolumeInport
#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorinfo.h>    // for ProcessorInfo
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatVec4Property

namespace inviwo {

/** \docpage{org.inviwo.VolumeBoundingBox, Volume Bounding Box}
 * ![](org.inviwo.VolumeBoundingBox.png?classIdentifier=org.inviwo.VolumeBoundingBox)
 *
 * Creates a mesh containing the bounding box of the volumes (Lines with Adjacency Information).
 *
 *
 * ### Inports
 *   * __volume__ The volume.
 *
 * ### Outports
 *   * __mesh__ The bounding mesh.
 *
 * ### Properties
 *   * __Color__ The color of the lines in the mesh.
 *
 */

class IVW_MODULE_BASE_API VolumeBoundingBox : public Processor {
public:
    VolumeBoundingBox();
    virtual ~VolumeBoundingBox() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volume_;
    MeshOutport mesh_;
    FloatVec4Property color_;
};

}  // namespace inviwo
