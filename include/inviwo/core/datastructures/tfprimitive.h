/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TFPRIMITIVE_H
#define IVW_TFPRIMITIVE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class TFPrimitive;

class IVW_CORE_API TFPrimitiveObserver : public Observer {
public:
    virtual void onTFPrimitiveChange(const TFPrimitive& p);
};

struct IVW_CORE_API TFPrimitiveData {
    double pos;
    vec4 color;
};

/**
 * \class TFPrimitive
 * \brief Base class for a variety of primitives as used by a transfer function
 */
class IVW_CORE_API TFPrimitive : public Observable<TFPrimitiveObserver>, public Serializable {
public:
    TFPrimitive(double pos = 0.0, const vec4& color = vec4(0.0f));
    // Cannot use default constructors and assignment operator for TFPrimitive!
    //
    // Default constructors would call the base class constructor of Observable and thereby
    // copy all observers. This must be avoided since TFPrimitives are a part of a property
    // and when setting/assigning a property, no observers must be copied!
    TFPrimitive(const TFPrimitiveData& data);
    TFPrimitive(const TFPrimitive& rhs);
    TFPrimitive(TFPrimitive&& rhs);
    TFPrimitive& operator=(const TFPrimitive& that);
    virtual ~TFPrimitive() = default;

    void setData(const TFPrimitiveData& data);
    inline const TFPrimitiveData& getData() const;

    void setPosition(double pos);
    inline double getPosition() const;

    void setAlpha(float alpha);
    inline float getAlpha() const;

    void setPositionAlpha(double pos, float alpha);
    void setPositionAlpha(const dvec2& p);

    void setColor(const vec3& color);
    void setColor(const vec4& color);
    inline const vec4& getColor() const;

    void notifyTFPrimitiveObservers();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    TFPrimitiveData data_;
};

inline const TFPrimitiveData& TFPrimitive::getData() const { return data_; }
inline double TFPrimitive::getPosition() const { return data_.pos; }
inline float TFPrimitive::getAlpha() const { return data_.color.a; }
inline const vec4& TFPrimitive::getColor() const { return data_.color; }

IVW_CORE_API bool operator==(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);
IVW_CORE_API bool operator!=(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);
IVW_CORE_API bool operator<(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);
IVW_CORE_API bool operator>(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);
IVW_CORE_API bool operator<=(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);
IVW_CORE_API bool operator>=(const TFPrimitiveData& lhs, const TFPrimitiveData& rhs);

IVW_CORE_API bool operator==(const TFPrimitive& lhs, const TFPrimitive& rhs);
IVW_CORE_API bool operator!=(const TFPrimitive& lhs, const TFPrimitive& rhs);
IVW_CORE_API bool operator<(const TFPrimitive& lhs, const TFPrimitive& rhs);
IVW_CORE_API bool operator>(const TFPrimitive& lhs, const TFPrimitive& rhs);
IVW_CORE_API bool operator<=(const TFPrimitive& lhs, const TFPrimitive& rhs);
IVW_CORE_API bool operator>=(const TFPrimitive& lhs, const TFPrimitive& rhs);

}  // namespace inviwo

#endif  // IVW_TFPRIMITIVE_H
