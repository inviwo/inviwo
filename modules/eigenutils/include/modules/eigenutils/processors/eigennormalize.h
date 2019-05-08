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

#ifndef IVW_EIGENNORMALIZE_H
#define IVW_EIGENNORMALIZE_H

#include <modules/eigenutils/eigenutilsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/eigenutils/eigenports.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.EigenNormalize, Matrix Normalization}
 * ![](org.inviwo.Normalize.png?classIdentifier=org.inviwo.EigenNormalize)
 *
 * A processor to normalize an Eigen::MatrixXf, supports following methods:
 * * MaxElement: Divide in element in the matrix by the value of the largest element
 * * MniMaxElement: Normalize each element based on the min and max value of the matrix
 * * Normalize: Uses the Eigens provided normalization method
 *
 *
 * ### Inports
 *   * __in__ Unnormalized matrix
 *
 * ### Outports
 *   * __out__ Normalized matrix
 *
 * ### Properties
 *   * __Method__ Select which method to use (see above)
 *
 */

/**
 * \class EigenNormalize
 * \brief A processor to normalize an Eigen::MatrixXf
 * A processor to normalize an Eigen::MatrixXf, supports following methods:
 * * MaxElement: Divide in element in the matrix by the value of the largest element
 * * MinMaxElement: Normalize each element based on the min and max value of the matrix
 * * Normalize: Uses the Eigens provided normalization method
 */
class IVW_MODULE_EIGENUTILS_API EigenNormalize : public Processor {
public:
    enum class Method { MaxElement, MinMaxElement, Normalize };

    EigenNormalize();
    virtual ~EigenNormalize() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    EigenMatrixInport in_;
    EigenMatrixOutport out_;

    TemplateOptionProperty<Method> method_;
};

}  // namespace inviwo

#endif  // IVW_EIGENNORMALIZE_H
