/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.atlasboundary, atlasboundary}
 * ![](org.inviwo.atlasboundary.png?classIdentifier=org.inviwo.atlasboundary)
 * Creates an atlas volume representing the current brushing and linking state.
 * - 0 = none
 * - 1 = selected
 * - 2 = filtered
 * - 3 = highlighted
 * 
 * Note: Rendering of selections can be made within Atlas Volume Raycaster processor.
 *
 * ### Inports
 *   * __volume__ Atlas volume.
 *   * __brushing__ Brushing and linking.
 *
 * ### Outports
 *   * __outport__ Segmented volume in range [0,3].
 */
class IVW_MODULE_VOLUME_API AtlasBoundary : public Processor {
public:
    AtlasBoundary();
    virtual ~AtlasBoundary() override = default;
    virtual void process() override;
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    VolumeInport volumeInport_;
    BrushingAndLinkingInport brushing_;
    VolumeOutport outport_;
    std::shared_ptr<Volume> volume_;
};

}  // namespace inviwo
