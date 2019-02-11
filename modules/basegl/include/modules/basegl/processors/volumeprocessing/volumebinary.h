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

#ifndef IVW_VOLUMEBINARY_H
#define IVW_VOLUMEBINARY_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/basegl/processors/volumeprocessing/volumeglprocessor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.VolumeBinary, Volume Binary}
 * ![](org.inviwo.VolumeBinary.png?classIdentifier=org.inviwo.VolumeBinary)
 * Computes a binary volume of the input volume using a threshold. The output
 * will contain "0" for all values below the threshold and "1" otherwise.
 *
 * ### Inports
 *   * __inputVolume__ Input volume
 *
 * ### Outports
 *   * __outputVolume__ Binary output volume
 *
 * ### Properties
 *   * __Threshold__ Threshold used for the binarization of the input volume
 */

/**
 * \class VolumeBinary
 *
 * \brief computes a binary volume of the input volume using a threshold.
 */
class IVW_MODULE_BASEGL_API VolumeBinary : public VolumeGLProcessor {
public:
    enum class Operator {
        GreaterThen,
        GreaterThenOrEqual,
        LessThen,
        LessThenOrEqual,
        Equal,
        NotEqual
    };

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    VolumeBinary();
    virtual ~VolumeBinary() {}

protected:
    virtual void preProcess(TextureUnitContainer &cont) override;
    virtual void postProcess() override;

    virtual void initializeResources() override;

    FloatProperty threshold_;
    TemplateOptionProperty<Operator> op_;
};

}  // namespace inviwo

#endif  // IVW_VOLUMEBINARY_H
