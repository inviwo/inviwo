/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2017 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class TransferFunctionDataPoint;

class IVW_CORE_API TransferFunctionPointObserver : public Observer {
public:
    virtual void onTransferFunctionPointChange(const TransferFunctionDataPoint* p) {};
};

class IVW_CORE_API TransferFunctionDataPoint : public Observable<TransferFunctionPointObserver>,
                                               public Serializable {
public:
    struct Point {
        float pos;
        vec4 color;
    };

    TransferFunctionDataPoint(const Point& point);
    TransferFunctionDataPoint(const float& pos = 0.0, const vec4& rgba = vec4(0));
    TransferFunctionDataPoint(const TransferFunctionDataPoint& rhs) = default;
    TransferFunctionDataPoint& operator=(const TransferFunctionDataPoint& that);
    virtual ~TransferFunctionDataPoint() = default;

    inline const float& getPos() const { return point_.pos; }
    inline const vec4& getRGBA() const { return point_.color; }
    inline const Point& getPoint() const { return point_; }

    void setPoint(const Point& point);
    void setPos(const float& pos);
    void setRGBA(const vec4& rgba);
    void setRGB(const vec3& rgb);
    void setA(float alpha);
    void setPosA(const float& pos, float alpha);

    void notifyTransferFunctionPointObservers();

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

private:
    Point point_;
};

IVW_CORE_API bool operator==(const TransferFunctionDataPoint::Point& lhs,
                             const TransferFunctionDataPoint::Point& rhs);
IVW_CORE_API bool operator!=(const TransferFunctionDataPoint::Point& lhs,
                             const TransferFunctionDataPoint::Point& rhs);
IVW_CORE_API bool operator<(const TransferFunctionDataPoint::Point& lhs,
    const TransferFunctionDataPoint::Point& rhs);
IVW_CORE_API bool operator>(const TransferFunctionDataPoint::Point& lhs,
                            const TransferFunctionDataPoint::Point& rhs);
IVW_CORE_API bool operator<=(const TransferFunctionDataPoint::Point& lhs,
                             const TransferFunctionDataPoint::Point& rhs);
IVW_CORE_API bool operator>=(const TransferFunctionDataPoint::Point& lhs,
                             const TransferFunctionDataPoint::Point& rhs);



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
