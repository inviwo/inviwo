/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/temporalvolume.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/**
 * @brief Resolves the current time of a TemporalVolume into a single Volume.
 *
 * The player picks the frame for the requested time, optionally linearly interpolating (per voxel
 * on the CPU) between the two bracketing frames. It also schedules a background prefetch of the
 * upcoming frames so that interactive playback stays smooth.
 *
 * For GPU-side interpolation, see the TemporalVolumeRaycaster which consumes the TemporalVolume
 * directly and blends two frames in the shader.
 *
 * @see TemporalVolume
 */
class IVW_MODULE_BASE_API TemporalVolumePlayer : public Processor {
public:
    TemporalVolumePlayer();
    TemporalVolumePlayer(const TemporalVolumePlayer&) = delete;
    TemporalVolumePlayer(TemporalVolumePlayer&&) = delete;
    TemporalVolumePlayer& operator=(const TemporalVolumePlayer&) = delete;
    TemporalVolumePlayer& operator=(TemporalVolumePlayer&&) = delete;
    virtual ~TemporalVolumePlayer() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    enum class Interpolation : std::uint8_t { Nearest, Linear };

    TemporalVolumeInport inport_;
    VolumeOutport outport_;

    DoubleProperty time_;
    OptionProperty<Interpolation> interpolation_;
    BoolProperty prefetch_;
    IntSizeTProperty prefetchAhead_;
};

}  // namespace inviwo
