/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMEBASISTRANSFORMER_H
#define IVW_VOLUMEBASISTRANSFORMER_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/basemoduledefine.h>

namespace inviwo {

template <typename InportType, typename OutportType>
class IVW_MODULE_BASE_API BasisTransform : public Processor {

public:
    BasisTransform();
    ~BasisTransform();

    InviwoProcessorInfo();

protected:
    virtual void process();

private:
    InportType inport_;
    OutportType outport_;

    FloatVec3Property lengths_;
    FloatVec3Property angels_;
    FloatVec3Property offset_;

    mat4 orgBasisAndOffset_;
};

//ProcessorClassIdentifier(VolumeBasisTransformer, "org.inviwo.BasisTransformer");
//ProcessorDisplayName(VolumeBasisTransformer, "Basis Transformer");
template <typename InportType, typename OutportType> 
const Tags BasisTransform<InportType, OutportType>::TAGS = Tags::CPU;
template <typename InportType, typename OutportType> 
const std::string BasisTransform<InportType, OutportType>::CATEGORY = "Coordinate Transforms";
template <typename InportType, typename OutportType> 
const CodeState BasisTransform<InportType, OutportType>::CODE_STATE = CODE_STATE_EXPERIMENTAL;

template <typename InportType, typename OutportType>
inviwo::BasisTransform<InportType, OutportType>::BasisTransform()
    : Processor()
    , inport_("inport")
    , outport_("outport")
    , lengths_("length_", "Lengths", vec3(1.0f), vec3(0.0f), vec3(10.0f))
    , angels_("angles_", "Angles", vec3(90.0f), vec3(0.0f), vec3(180.0f), vec3(1.0f))
    , offset_("offset_", "Offset", vec3(0.0f), vec3(-10.0f), vec3(10.0f))
    , orgBasisAndOffset_(0.0f) {
    addPort(inport_);
    addPort(outport_);
    addProperty(lengths_);
    addProperty(angels_);
    addProperty(offset_);
}

template <typename InportType, typename OutportType>
inviwo::BasisTransform<InportType, OutportType>::~BasisTransform() {}

template <typename InportType, typename OutportType>
void inviwo::BasisTransform<InportType, OutportType>::process() {
    if (inport_.hasData()) {
        if (orgBasisAndOffset_ != inport_.getData()->getModelMatrix()) {
            orgBasisAndOffset_ = inport_.getData()->getModelMatrix();
            //TODO: Can't set these values as deserialization might already have done that.
            //Also not that glm::angle always return radians from now on.
            /*vec3 a(orgBasisAndOffset_[0]);
            vec3 b(orgBasisAndOffset_[1]);
            vec3 c(orgBasisAndOffset_[2]);
            vec3 offset(orgBasisAndOffset_[3]);

            float alpha = glm::angle(b,c);
            float beta = glm::angle(c,a);
            float gamma = glm::angle(a,b);

            lengths_.setMaxValue(vec3(2.0f * (glm::length(a) + glm::length(b) + glm::length(c))));
            offset_.setMaxValue(vec3(5.0*glm::length(offset)));
            offset_.setMinValue(vec3(-5.0*glm::length(offset)));

            lengths_.set(vec3(glm::length(a),glm::length(b),glm::length(c)));
            angels_.set(vec3(alpha, beta, gamma));
            offset_.set(offset);*/
        }

        // TODO: This should be changed to make some kind of shallow copy of the representations
        // and update if the inport changes the representations
        if (!outport_.hasData() || inport_.isChanged()) {
            outport_.setData(inport_.getData()->clone());
        }

        float a = lengths_.get()[0];
        float b = lengths_.get()[1];
        float c = lengths_.get()[2];
        vec3 offset = offset_.get();
        float alpha = glm::radians(angels_.get()[0]);
        float beta =  glm::radians(angels_.get()[1]);
        float gamma = glm::radians(angels_.get()[2]);
        float v = std::sqrt(1 - std::cos(alpha)*std::cos(alpha) - std::cos(beta)*std::cos(beta) - std::cos(gamma)*std::cos(gamma)
                            - 2*std::cos(alpha)*std::cos(beta)*std::cos(gamma));
        mat4 newBasisAndOffset(
            a,    b*std::cos(gamma), c*std::cos(beta),                                                   offset[0],
            0.0f, b*std::sin(gamma), c*(std::cos(alpha)-std::cos(beta)*std::cos(gamma))/std::sin(gamma), offset[1],
            0.0f, 0.0f,              c*v/std::sin(gamma),                                                offset[2],
            0.0f, 0.0f,              0.0f,                                                               1.0f
        );

       outport_.getData()->setModelMatrix(glm::transpose(newBasisAndOffset));
    }
}

} // namespace

#endif