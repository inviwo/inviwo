/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMESOURCE_H
#define IVW_VOLUMESOURCE_H

#include <modules/base/basemoduledefine.h>
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <modules/base/properties/sequencetimerproperty.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeSource, Volume Source}
 * ![](org.inviwo.VolumeSource.png?classIdentifier=org.inviwo.VolumeSource)
 *
 * Loads a Volume
 *
 * ### Outports
 *   * __Outport__ The loaded volume
 *
 * ### Properties
 *   * __File name__ File to load.
 */
class IVW_MODULE_BASE_API VolumeSource : public Processor {
public:
    using VolumeSequence = std::vector<std::shared_ptr<Volume>>;
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    VolumeSource();
    virtual ~VolumeSource() = default;

    virtual void deserialize(Deserializer& d) override;
    virtual void process() override;

private:
    void load(bool deserialize = false);
    void addFileNameFilters();

    virtual bool isSink() const override;

    std::shared_ptr<VolumeSequence> volumes_;

    VolumeOutport outport_;
    FileProperty file_;
    ButtonProperty reload_;

    BasisProperty basis_;
    VolumeInformationProperty information_;
    SequenceTimerProperty volumeSequence_;

    bool deserialized_ = false;
};

}  // namespace

#endif  // IVW_VOLUMESOURCE_H
