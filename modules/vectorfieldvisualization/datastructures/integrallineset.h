/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#ifndef IVW_INTEGRALLINESET_H
#define IVW_INTEGRALLINESET_H

#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/datastructures/integralline.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/port.h>

namespace inviwo {

/**
 * \class IntegralLineSet
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineSet {
public:
    IntegralLineSet(mat4 modelMatrix);
    virtual ~IntegralLineSet();

    mat4 getModelMatrix() const;

    std::vector<IntegralLine>::const_iterator begin() const;
    std::vector<IntegralLine>::const_iterator end() const;

    std::vector<IntegralLine>::iterator begin();
    std::vector<IntegralLine>::iterator end();

    size_t size() const;

    IntegralLine& operator[](size_t idx);
    const IntegralLine& operator[](size_t idx) const;

    IntegralLine& at(size_t idx);
    const IntegralLine& at(size_t idx) const;
    void push_back(IntegralLine &line);
    void push_back(IntegralLine &line,size_t idx);

private:
    std::vector<IntegralLine> lines_;
    mat4 modelMatrix_;
};

using IntegralLineSetInport = DataInport<IntegralLineSet>;
using IntegralLineSetOutport = DataOutport<IntegralLineSet>;

template <>
struct port_traits<IntegralLineSet> {
    static std::string class_identifier() { return "IntegralLineSet"; }
    static uvec3 color_code() { return uvec3(255, 150, 0); }
    static std::string data_info(const IntegralLineSet* data) {
        std::ostringstream oss;
        oss << "Integral Line Set with " << data->size() << " lines";
        return oss.str();
    }
};

}  // namespace

#endif  // IVW_INTEGRALLINESET_H
