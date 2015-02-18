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

#ifndef IVW_TRANSFERFUNCTIONDATAPOINT_H
#define IVW_TRANSFERFUNCTIONDATAPOINT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class TransferFunctionDataPoint;

class IVW_CORE_API TransferFunctionPointObserver : public Observer {
public:
    TransferFunctionPointObserver() : Observer() {};
    virtual void onTransferFunctionPointChange(const TransferFunctionDataPoint* p) {};
};

class IVW_CORE_API TransferFunctionPointObservable
    : public Observable<TransferFunctionPointObserver> {
public:
    TransferFunctionPointObservable() : Observable<TransferFunctionPointObserver>() {};
    void notifyTransferFunctionPointObservers() const;
};

class IVW_CORE_API TransferFunctionDataPoint : public TransferFunctionPointObservable,
                                               public IvwSerializable {
public:
    TransferFunctionDataPoint(const vec2& pos = vec2(0), const vec4& rgba = vec4(0));
    TransferFunctionDataPoint(const TransferFunctionDataPoint& rhs);
    virtual ~TransferFunctionDataPoint();
    TransferFunctionDataPoint& operator=(const TransferFunctionDataPoint& that);

    inline const vec2& getPos() const { return pos_; };
    inline const vec4& getRGBA() const { return rgba_; }

    void setPos(const vec2& pos);
    void setRGBA(const vec4& rgba);
    void setRGB(const vec3& rgb);
    void setA(float alpha);
    void setPosA(const vec2& pos, float alpha);

    void setNotificationsEnabled(bool enabled) { notify_ = enabled; }
    void notifyTransferFunctionPointObservers() const;

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    friend IVW_CORE_API bool operator==(const TransferFunctionDataPoint& lhs,
                           const TransferFunctionDataPoint& rhs);

    // Compare points by their "x" value
    friend IVW_CORE_API bool operator<(const TransferFunctionDataPoint& lhs,
                          const TransferFunctionDataPoint& rhs);

private:
    vec2 pos_;
    vec4 rgba_;
    bool notify_;
};

IVW_CORE_API bool operator==(const TransferFunctionDataPoint& lhs,
                             const TransferFunctionDataPoint& rhs);
IVW_CORE_API bool operator!=(const TransferFunctionDataPoint& lhs,
                             const TransferFunctionDataPoint& rhs);
IVW_CORE_API bool operator<(const TransferFunctionDataPoint& lhs,
                            const TransferFunctionDataPoint& rhs);
IVW_CORE_API bool operator>(const TransferFunctionDataPoint& lhs,
                            const TransferFunctionDataPoint& rhs);
IVW_CORE_API bool operator<=(const TransferFunctionDataPoint& lhs,
                             const TransferFunctionDataPoint& rhs);
IVW_CORE_API bool operator>=(const TransferFunctionDataPoint& lhs,
                             const TransferFunctionDataPoint& rhs);
}  // namespace

#endif  // IVW_TRANSFERFUNCTIONDATAPOINT_H
