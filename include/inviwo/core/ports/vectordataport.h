/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VECTORDATA_PORT_H
#define IVW_VECTORDATA_PORT_H

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <vector>
#include <typeinfo>

namespace inviwo {

template <typename T>
class VectorData : public BaseData {
public:
    VectorData() {}
    VectorData(const VectorData& rhs) : BaseData(rhs), vector(rhs.vector) {}
    VectorData& operator=(const VectorData& rhs) {
        if (this != &rhs) {
            this->vector = rhs.vector;
        }
        return *this;
    }
    virtual VectorData* clone() const { return new VectorData(*this); }
    virtual ~VectorData() {}

    std::vector<T> vector;

    std::string getDataInfo() const {
        std::ostringstream stream;
        stream << "<table border='0' cellspacing='0' cellpadding='0' "
                  "style='border-color:white;white-space:pre;'>\n"
               << "<tr><td "
                  "style='color:#bbb;padding-right:8px;'>Type:</td><td><nobr>vector</nobr></td></"
                  "tr>\n"
               << "<tr><td style='color:#bbb;padding-right:8px;'>Format:</td><td><nobr>"
               << typeid(T).name() << "</nobr></td></tr>\n"
               << "<tr><td style='color:#bbb;padding-right:8px;'>Length:</td><td><nobr>"
               << vector.size() << "</nobr></td></tr>\n"
               << "</tr></table>\n";
        return stream.str();
    }
};

template <typename Type>
class VectorDataInport : public DataInport<VectorData<Type>> {
public:
    VectorDataInport(std::string identifier) : DataInport<VectorData<Type>>(identifier) {}
    virtual ~VectorDataInport() {}

    uvec3 getColorCode() const { return uvec3(30, 30, 30) + port_traits<Type>::color_code(); }
    virtual std::string getClassIdentifier() const {
        return port_traits<Type>::class_identifier() + "VectorInport";
    }
};

template <typename Type>
class VectorDataOutport : public DataOutport<VectorData<Type>> {
public:
    VectorDataOutport(std::string identifier) : DataOutport<VectorData<Type>>(identifier) {
        this->setData(new VectorData<Type>, true);
    }
    virtual ~VectorDataOutport() {}

    uvec3 getColorCode() const { return uvec3(30, 30, 30) + port_traits<Type>::color_code(); }
    virtual std::string getClassIdentifier() const {
        return port_traits<Type>::class_identifier() + "VectorOutport";
    }
};

}  // namespace

#endif  // IVW_VECTORDATA_PORT_H
