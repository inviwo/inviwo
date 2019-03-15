/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_PROPERTYSEMANTICS_H
#define IVW_PROPERTYSEMANTICS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <string>
#include <ostream>

namespace inviwo {

class IVW_CORE_API PropertySemantics : public Serializable {
public:
    PropertySemantics();
    PropertySemantics(std::string semantic);
    PropertySemantics(const PropertySemantics& rhs) = default;
    PropertySemantics(PropertySemantics&& rhs) noexcept = default;
    PropertySemantics& operator=(const PropertySemantics& that) = default;
    PropertySemantics& operator=(PropertySemantics&& that) noexcept = default;
    virtual ~PropertySemantics() noexcept = default;

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    const std::string& getString() const;

    static const PropertySemantics Default;
    static const PropertySemantics Text;
    static const PropertySemantics SpinBox;
    static const PropertySemantics Color;
    static const PropertySemantics LightPosition;
    static const PropertySemantics TextEditor;
    static const PropertySemantics Multiline;
    static const PropertySemantics ImageEditor;
    static const PropertySemantics ShaderEditor;
    static const PropertySemantics PythonEditor;

private:
    std::string semantic_;
};

template <class Elem, class Traits>
std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& ss,
                                             const PropertySemantics& obj) {
    ss << obj.getString();
    return ss;
}

inline bool operator==(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return lhs.getString() == rhs.getString();
}
inline bool operator<(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return lhs.getString() < rhs.getString();
}
inline bool operator!=(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return !operator==(lhs, rhs);
}
inline bool operator>(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return operator<(rhs, lhs);
}
inline bool operator<=(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return !operator>(lhs, rhs);
}
inline bool operator>=(const PropertySemantics& lhs, const PropertySemantics& rhs) {
    return !operator<(lhs, rhs);
}

}  // namespace inviwo

#endif  // IVW_PROPERTYSEMANTICS_H
