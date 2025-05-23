/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/eigenutils/processors/eigennormalize.h>

#include <inviwo/core/processors/processor.h>       // for Processor
#include <inviwo/core/processors/processorstate.h>  // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>   // for Tags
#include <inviwo/core/properties/optionproperty.h>  // for OptionPropertyOption, OptionProperty
#include <modules/eigenutils/eigenports.h>          // for EigenMatrixOutport, EigenMatrixInport

#include <memory>       // for make_shared, shared_ptr, shared_ptr<>...
#include <type_traits>  // for remove_extent_t

#include <Eigen/Dense>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo EigenNormalize::processorInfo_{
    "org.inviwo.EigenNormalize",  // Class identifier
    "Matrix Normalization",       // Display name
    "Eigen",                      // Category
    CodeState::Experimental,      // Code state
    "Eigen",                      // Tags
    R"(Normalizes an Eigen::MatrixXf and supports the following methods:
    - `MaxElement`: Divide in element in the matrix by the value of the largest element
    - `MniMaxElement`: Normalize each element based on the min and max value of the matrix
    - `Normalize`: Uses the Eigens provided normalization method)"_unindentHelp,
};
const ProcessorInfo& EigenNormalize::getProcessorInfo() const { return processorInfo_; }

EigenNormalize::EigenNormalize()
    : Processor()
    , in_("in", "Unnormalized matrix"_help)
    , out_("out", "Normalized matrix"_help)

    , method_("method", "Method", "Select which method to use (see above)"_help,
              {{"maxelement", "Max Element", Method::MaxElement},
               {"minmaxelement", "Min/Max Element", Method::MinMaxElement},
               {"normalize", "Normalize", Method::Normalize}}) {

    addPort(in_);
    addPort(out_);

    addProperty(method_);
}

void EigenNormalize::process() {
    auto m = in_.getData();
    switch (method_.get()) {
        case Method::MaxElement: {
            auto maxV = m->maxCoeff();
            out_.setData(std::make_shared<Eigen::MatrixXf>((*m) / maxV));
            break;
        }
        case Method::MinMaxElement: {
            auto minV = m->minCoeff();
            auto maxV = m->maxCoeff();
            auto m2 = std::make_shared<Eigen::MatrixXf>(*m);
            m2->array() -= minV;
            m2->array() /= maxV - minV;
            out_.setData(m2);
            break;
        }
        case Method::Normalize:
            out_.setData(std::make_shared<Eigen::MatrixXf>(m->normalized()));
            break;
    }
}

}  // namespace inviwo
