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

#include <modules/eigenutils/processors/eigenmix.h>

#include <inviwo/core/processors/processor.h>        // for Processor
#include <inviwo/core/processors/processorstate.h>   // for CodeState, CodeState::Experimental
#include <inviwo/core/processors/processortags.h>    // for Tags
#include <inviwo/core/properties/ordinalproperty.h>  // for FloatProperty
#include <modules/eigenutils/eigenports.h>           // for EigenMatrixInport, EigenMatrixOutport

#include <memory>       // for make_shared, shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

#include <Eigen/Core>  // for MatrixXf, Matrix

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo EigenMix::processorInfo_{
    "org.inviwo.EigenMix",    // Class identifier
    "Matrix Mix",             // Display name
    "Eigen",                  // Category
    CodeState::Experimental,  // Code state
    "Eigen",                  // Tags
};
const ProcessorInfo& EigenMix::getProcessorInfo() const { return processorInfo_; }

EigenMix::EigenMix()
    : Processor(), a_("a"), b_("b"), res_("res"), w_("w", "Mix factor", 0.5f, 0.f, 1.f, 0.1f) {

    addPort(a_);
    addPort(b_);
    addPort(res_);
    addProperty(w_);
}

void EigenMix::process() {
    auto A = a_.getData();
    auto B = b_.getData();
    res_.setData(std::make_shared<Eigen::MatrixXf>((*A) + w_.get() * ((*B) - (*A))));
}

}  // namespace inviwo
