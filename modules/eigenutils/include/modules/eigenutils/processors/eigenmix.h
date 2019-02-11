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

#ifndef IVW_EIGENMIX_H
#define IVW_EIGENMIX_H

#include <modules/eigenutils/eigenutilsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/eigenutils/eigenports.h>

namespace inviwo {

/** \docpage{org.inviwo.EigenMix, EigenMix}
 * ![](org.inviwo.EigenMix.png?classIdentifier=org.inviwo.EigenMix)
 *
 * Creates a linear mix of matrix A and B such that Cij = Aij + w (Bij-Aij)
 *
 *
 * ### Inports
 *   * __a__ Matrix A
 *   * __b__ Matrix B
 *
 * ### Outports
 *   * __res__ Lineart mix of Matrix A and B
 *
 * ### Properties
 *   * __Mix factor__ Weighting factor, a low value favors A and high value favors B
 *
 */

/**
 * \class EigenMix
 * \brief Creates a linear mix of matrix A and B such that Cij = Aij + w (Bij-Aij)
 */
class IVW_MODULE_EIGENUTILS_API EigenMix : public Processor {
public:
    EigenMix();
    virtual ~EigenMix() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    EigenMatrixInport a_;
    EigenMatrixInport b_;
    EigenMatrixOutport res_;

    FloatProperty w_;
};

}  // namespace inviwo

#endif  // IVW_MIX_H
