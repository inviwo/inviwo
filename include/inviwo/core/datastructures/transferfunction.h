/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

/** \class for holding transfer function data
 *
 *  This class holds transfer function data, currently one parameter in the variable data_.
 */

#ifndef IVW_TRANSFERFUNCTION_H
#define IVW_TRANSFERFUNCTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/transferfunctiondatapoint.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class Layer;

class IVW_CORE_API TransferFunctionObserver: public Observer {
public:
    TransferFunctionObserver(): Observer() {};

    virtual void onControlPointAdded(TransferFunctionDataPoint* p) {};
    virtual void onControlPointRemoved(TransferFunctionDataPoint* p) {};
    virtual void onControlPointChanged(const TransferFunctionDataPoint* p) {};
};
class IVW_CORE_API TransferFunctionObservable: public Observable<TransferFunctionObserver> {
public:
    TransferFunctionObservable(): Observable<TransferFunctionObserver>() {};
    void notifyControlPointAdded(TransferFunctionDataPoint* p) const;
    void notifyControlPointRemoved(TransferFunctionDataPoint* p) const;
    void notifyControlPointChanged(const TransferFunctionDataPoint* p) const;
};

class IVW_CORE_API TransferFunction
    : public IvwSerializable
    , public TransferFunctionObservable
    , public TransferFunctionPointObserver {
    
public:

    enum InterpolationType {
        InterpolationLinear = 0,
        InterpolationCubic
    };

    TransferFunction(int textureSize=1024);
    TransferFunction(const TransferFunction& rhs);
    TransferFunction& operator=(const TransferFunction& rhs);

    virtual ~TransferFunction();

    const Layer* getData() const;
    int getNumPoints() const;
    int getTextureSize();

    TransferFunctionDataPoint* getPoint(int i) const;

    void addPoint(const vec2& pos, const vec4& color);
    void addPoint(TransferFunctionDataPoint* dataPoint);
    void addPoint(const vec2& pos);
    void removePoint(TransferFunctionDataPoint* dataPoint);

    void clearPoints();

    float getMaskMin() const;
    void setMaskMin(float maskMin);
    float getMaskMax() const;
    void setMaskMax(float maskMax);
    void setInterpolationType(InterpolationType interpolationType);
    InterpolationType getInterpolationType() const;

    /** 
     * Notify that the layer data (texture) needs to be updated next time it is requested.
     */
    void invalidate();
    
    virtual void onTransferFunctionPointChange(const TransferFunctionDataPoint* p);
    
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    typedef std::vector<TransferFunctionDataPoint*> TFPoints;
    friend bool operator==(const TransferFunction& lhs, const TransferFunction& rhs);
protected:
    void calcTransferValues();

private:
    float maskMin_;
    float maskMax_;
    TFPoints points_;
    InterpolationType interpolationType_;

    int textureSize_;
    bool invalidData_;
    Layer* data_;
};

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs);
bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs);

} // namespace
#endif // IVW_TRANSFERFUNCTION_H
