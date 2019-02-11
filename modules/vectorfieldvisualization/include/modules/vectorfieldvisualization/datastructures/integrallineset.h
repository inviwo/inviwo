/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <inviwo/core/datastructures/datatraits.h>

namespace inviwo {

/**
 * \class IntegralLineSet
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLineSet {
public:
    enum class SetIndex { Yes, No };

    using value_type = IntegralLine;
    IntegralLineSet(mat4 modelMatrix, mat4 worldMatrix = mat4(1));
    virtual ~IntegralLineSet();

    mat4 getModelMatrix() const;
    mat4 getWorldMatrix() const;

    std::vector<IntegralLine>::const_iterator begin() const;
    std::vector<IntegralLine>::const_iterator end() const;

    std::vector<IntegralLine>::iterator begin();
    std::vector<IntegralLine>::iterator end();

    const IntegralLine& back() const { return lines_.back(); }
    IntegralLine& back() { return lines_.back(); }

    const IntegralLine& front() const { return lines_.front(); }
    IntegralLine& front() { return lines_.front(); }

    size_t size() const;

    IntegralLine& operator[](size_t idx);
    const IntegralLine& operator[](size_t idx) const;

    IntegralLine& at(size_t idx);
    const IntegralLine& at(size_t idx) const;

    void push_back(const IntegralLine& line, SetIndex updateIndex);
    void push_back(const IntegralLine& line, size_t idx);

    void push_back(IntegralLine&& line, SetIndex updateIndex);
    void push_back(IntegralLine&& line, size_t idx);

    std::vector<IntegralLine>& getVector() { return lines_; }
    const std::vector<IntegralLine>& getVector() const { return lines_; }

private:
    std::vector<IntegralLine> lines_;
    mat4 modelMatrix_;
    mat4 worldMatrix_;
};

using IntegralLineSetInport = DataInport<IntegralLineSet>;
using IntegralLineSetOutport = DataOutport<IntegralLineSet>;

template <>
struct DataTraits<IntegralLineSet> {
    static std::string classIdentifier() { return "org.inviwo.IntegralLineSet"; }
    static std::string dataName() { return "IntegralLineSet"; }
    static uvec3 colorCode() { return uvec3(255, 150, 0); }
    static Document info(const IntegralLineSet& data) {
        std::ostringstream oss;
        oss << "Integral Line Set with " << data.size() << " lines";
        Document doc;
        doc.append("p", oss.str());
        return doc;
    }
};

}  // namespace inviwo

#endif  // IVW_INTEGRALLINESET_H
