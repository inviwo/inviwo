/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#ifndef IVW_ISOVALUE_H
#define IVW_ISOVALUE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/observer.h>

namespace inviwo {

class IsoValue;

class IVW_CORE_API IsoValueObserver : public Observer {
public:
    virtual void onIsoValueChange(const IsoValue& v);
};

struct IVW_CORE_API IsoValueData {
    float isovalue;
    vec4 color;
};

/**
 * \ingroup datastructures
 * \class IsoValue
 * \brief basic data structure representing a scalar isovalue and associated color
 */
class IVW_CORE_API IsoValue : public Observable<IsoValueObserver>, public Serializable {
public:
    IsoValue(float value = 0.0f, const vec4& color = vec4(0.0f));
    IsoValue(const IsoValue& rhs);
    IsoValue(IsoValue&& rhs) = default;
    IsoValue& operator=(const IsoValue& rhs);
    virtual ~IsoValue() = default;


    void set(float value, const vec4& color);
    void set(const IsoValueData& value);
    const IsoValueData& get() const;

    void setIsoValue(const float& value);
    const float& getIsoValue() const;

    void setColor(const vec4& color);
    const vec4& getColor() const;

    void notifyIsoValueObservers();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    IsoValueData data_;
};

inline const IsoValueData& IsoValue::get() const { return data_; }

inline const float& IsoValue::getIsoValue() const { return data_.isovalue; }

inline const vec4& IsoValue::getColor() const { return data_.color; }


IVW_CORE_API bool operator==(const IsoValue& lhs, const IsoValue& rhs);
IVW_CORE_API bool operator!=(const IsoValue& lhs, const IsoValue& rhs);
IVW_CORE_API bool operator<(const IsoValue& lhs, const IsoValue& rhs);
IVW_CORE_API bool operator>(const IsoValue& lhs, const IsoValue& rhs);
IVW_CORE_API bool operator<=(const IsoValue& lhs, const IsoValue& rhs);
IVW_CORE_API bool operator>=(const IsoValue& lhs, const IsoValue& rhs);

IVW_CORE_API bool operator==(const IsoValueData& lhs, const IsoValueData& rhs);
IVW_CORE_API bool operator!=(const IsoValueData& lhs, const IsoValueData& rhs);
IVW_CORE_API bool operator<(const IsoValueData& lhs, const IsoValueData& rhs);
IVW_CORE_API bool operator>(const IsoValueData& lhs, const IsoValueData& rhs);
IVW_CORE_API bool operator<=(const IsoValueData& lhs, const IsoValueData& rhs);
IVW_CORE_API bool operator>=(const IsoValueData& lhs, const IsoValueData& rhs);

}  // namespace inviwo

#endif  // IVW_ISOVALUE_H
