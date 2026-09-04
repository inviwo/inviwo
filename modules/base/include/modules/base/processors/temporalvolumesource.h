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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/fileextension.h>

#include <memory>

namespace inviwo {

class InviwoApplication;

/**
 * @brief Loads a sequence of volume files from a folder as a lazily-loaded TemporalVolume.
 *
 * Unlike VolumeSequenceSource, which eagerly loads every frame into memory, TemporalVolumeSource
 * only reads metadata (a prototype volume) up front. Individual frames are read on demand and kept
 * in a bounded LRU cache. This makes it suitable for large time-varying data sets where only a
 * sliding window of frames fits in memory.
 *
 * @see TemporalVolume, FileSequenceLoader
 */
class IVW_MODULE_BASE_API TemporalVolumeSource : public Processor {
public:
    explicit TemporalVolumeSource(InviwoApplication* app);
    TemporalVolumeSource(const TemporalVolumeSource&) = delete;
    TemporalVolumeSource(TemporalVolumeSource&&) = delete;
    TemporalVolumeSource& operator=(const TemporalVolumeSource&) = delete;
    TemporalVolumeSource& operator=(TemporalVolumeSource&&) = delete;
    virtual ~TemporalVolumeSource() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;

private:
    /// Time values assigned to each frame.
    enum class TimeMode : std::uint8_t {
        Index,       //!< 0, 1, 2, … (frame index)
        FileNumber,  //!< the number extracted from each file name
    };

    void load();

    InviwoApplication* app_;

    TemporalVolumeOutport outport_;

    DirectoryProperty folder_;
    StringProperty filter_;
    OptionProperty<FileExtension> reader_;
    OptionProperty<TimeMode> timeMode_;
    IntSizeTProperty cacheSize_;
    ButtonProperty reload_;

    bool dirty_ = true;
};

}  // namespace inviwo
