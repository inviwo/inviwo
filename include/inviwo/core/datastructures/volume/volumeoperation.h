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

#ifndef IVW_VOLUMEOPERATION_H
#define IVW_VOLUMEOPERATION_H

#include <inviwo/core/datastructures/dataoperation.h>
#include <inviwo/core/util/formats.h>

namespace inviwo {

class VolumeRepresentation;

class IVW_CORE_API VolumeOperation : public DataOperation {
public:
    VolumeOperation(const VolumeRepresentation* in) : DataOperation(), in_(in) {}
    virtual ~VolumeOperation() {}

    const VolumeRepresentation* getInputVolume() { return in_; }

    template <typename T>
    void evaluate(){}

    template<typename VO>
    void evaluateFor(){
        VO* t = dynamic_cast<VO*>(this);
        if(!t) return; //todo maybe print error= 

        DataFormatEnums::Id id_ = getInputVolume()->getDataFormat()->getId();
        CallFunctionWithTemplateArgsForType(t->template evaluate, id_);
    }


private:
    const VolumeRepresentation* in_;
};



} // namespace

#endif // IVW_VOLUMEOPERATION_H
