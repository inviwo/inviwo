/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_MESHMAPPING_H
#define IVW_MESHMAPPING_H

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {

/** \docpage{org.inviwo.MeshMapping, Map Buffer To Mesh Color}
 * ![](org.inviwo.MeshMapping.png?classIdentifier=org.inviwo.MeshMapping)
 * Maps the contents of a buffer component to colors of a mesh via a transfer function.
 *
 * ### Inports
 *   * __Mesh__  Input mesh
 *
 * ### Outports
 *   * __Output__   Mesh identical to input mesh but with the color mapped to a specific buffer
 *
 * ### Properties
 *   * __Buffer__     buffer used as source for the color mapping
 *   * __Component__  selected buffer component, i.e. x, y, z, or w (if available) used for mapping
 *   * __Use Custom Range__   if enabled, the custom range is used for mapping instead of the
 *                    range of the input buffer
 */

/**
 * \class MeshMapping
 * \brief Maps the contents of a buffer to colors of a mesh via a transfer function.
 */
class IVW_MODULE_BASE_API MeshMapping : public Processor {
public:
    MeshMapping();
    virtual ~MeshMapping() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    MeshInport meshInport_;
    MeshOutport outport_;

    BoolProperty enabled_;
    TransferFunctionProperty tf_;

    OptionPropertyInt buffer_;
    OptionPropertyInt component_;

    BoolProperty useCustomDataRange_;
    DoubleMinMaxProperty customDataRange_;
    DoubleMinMaxProperty dataRange_;
};

}  // namespace inviwo

#endif  // IVW_MESHMAPPING_H
