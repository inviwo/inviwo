/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/ports/volumeport.h>           // for VolumeOutport
#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorinfo.h>   // for ProcessorInfo
#include <inviwo/core/properties/buttonproperty.h>  // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>    // for FileProperty
#include <inviwo/core/properties/optionproperty.h>  // for OptionProperty
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/util/fileextension.h>                     // for FileExtension, operator==
#include <modules/base/properties/basisproperty.h>              // for BasisProperty
#include <modules/base/properties/sequencetimerproperty.h>      // for SequenceTimerProperty
#include <modules/base/properties/volumeinformationproperty.h>  // for VolumeInformationProperty

#include <functional>   // for __base
#include <memory>       // for shared_ptr
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, opera...

namespace inviwo {

class Deserializer;
class InviwoApplication;

class IVW_MODULE_BASE_API VolumeSource : public Processor {
public:
    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    VolumeSource(InviwoApplication* app, const std::filesystem::path& filePath = {});
    virtual ~VolumeSource() = default;

    virtual void deserialize(Deserializer& d) override;
    virtual void process() override;

private:
    void load(bool deserialize = false);

    InviwoApplication* app_;
    std::shared_ptr<VolumeSequence> volumes_;

    VolumeOutport outport_;
    FileProperty file_;
    OptionProperty<FileExtension> reader_;
    ButtonProperty reload_;

    BasisProperty basis_;
    VolumeInformationProperty information_;
    SequenceTimerProperty volumeSequence_;

    bool deserialized_ = false;
    std::string error_;
};

}  // namespace inviwo
