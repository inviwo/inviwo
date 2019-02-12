/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#ifndef IVW_IMAGESNAPSHOT_H
#define IVW_IMAGESNAPSHOT_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>

namespace inviwo {

/** \docpage{org.inviwo.ImageSnapshot, Image Snapshot}
 * ![](org.inviwo.ImageSnapshot.png?classIdentifier=org.inviwo.ImageSnapshot)
 * Save snapshot of images that can be viewed later. Useful for comparisons.
 *
 * ### Inports
 *   * __inport__ Input image
 *
 * ### Outports
 *   * __outport1__ Outputs the input image or a saved image
 *   * __outport2__ Outputs the input image or a saved image
 *
 * ### Properties
 *   * __Image 1 index__ The image to output on outport1, -1 means input pass through.
 *   * __Image 2 index__ The image to output on outport2, -1 means input pass through.
 */
class IVW_MODULE_BASE_API ImageSnapshot : public Processor {
public:
    ImageSnapshot();
    virtual ~ImageSnapshot() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void deserialize(Deserializer& d) override;

private:
    ImageInport inport_;
    ImageOutport outport1_;
    ImageOutport outport2_;
    IntProperty outport1ImageIndex_;
    IntProperty outport2ImageIndex_;
    ButtonProperty snapshot_;
    ButtonProperty clear_;

    std::vector<std::shared_ptr<Image>> snapshots_;
};

}  // namespace inviwo

#endif  // IVW_IMAGESNAPSHOT_H
