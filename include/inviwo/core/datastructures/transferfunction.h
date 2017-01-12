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

#ifndef IVW_TRANSFERFUNCTION_H
#define IVW_TRANSFERFUNCTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/transferfunctiondatapoint.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/util/fileextension.h>


namespace inviwo {

class Layer;

template <typename T>
class LayerRAMPrecision;

class IVW_CORE_API TransferFunctionObserver : public Observer {
public:
    virtual void onControlPointAdded(TransferFunctionDataPoint* p){};
    virtual void onControlPointRemoved(TransferFunctionDataPoint* p){};
    virtual void onControlPointChanged(const TransferFunctionDataPoint* p){};
};
class IVW_CORE_API TransferFunctionObservable : public Observable<TransferFunctionObserver> {
protected:
    void notifyControlPointAdded(TransferFunctionDataPoint* p);
    void notifyControlPointRemoved(TransferFunctionDataPoint* p);
    void notifyControlPointChanged(const TransferFunctionDataPoint* p);
};

/** 
 * \ingroup datastructures 
 * \brief for holding transfer function data.
 *  This class holds transfer function data, currently one parameter in the variable data_.
 */
class IVW_CORE_API TransferFunction : public Serializable,
                                      public TransferFunctionObservable,
                                      public TransferFunctionPointObserver {

public:
    using TFPoints = std::vector<std::unique_ptr<TransferFunctionDataPoint>>;

    TransferFunction(int textureSize = 1024);
    TransferFunction(const TransferFunction& rhs);
    TransferFunction& operator=(const TransferFunction& rhs);

    virtual ~TransferFunction();
    
    const Layer* getData() const;
    int getNumPoints() const;
    int getTextureSize();

    TransferFunctionDataPoint* getPoint(size_t i);
    const TransferFunctionDataPoint* getPoint(size_t i) const;

    void addPoint(const vec2& pos, const vec4& color);
    void addPoint(const vec2& pos);
    void removePoint(TransferFunctionDataPoint* dataPoint);

    void clearPoints();

    float getMaskMin() const;
    void setMaskMin(float maskMin);
    float getMaskMax() const;
    void setMaskMax(float maskMax);

    /**
     * Notify that the layer data (texture) needs to be updated next time it is requested.
     */
    void invalidate();

    virtual void onTransferFunctionPointChange(const TransferFunctionDataPoint* p);

    virtual void serialize(Serializer& s) const;
    virtual void deserialize(Deserializer& d);

    vec4 sample(double v) const;
    vec4 sample(float v) const;

    friend bool operator==(const TransferFunction& lhs, const TransferFunction& rhs);

    void save(const std::string& filename, const FileExtension& ext = FileExtension()) const;
    void load(const std::string& filename, const FileExtension& ext = FileExtension());

protected:

    void addPoint(std::unique_ptr<TransferFunctionDataPoint> dataPoint);

    void calcTransferValues() const;

private:
    float maskMin_;
    float maskMax_;
    TFPoints points_;

    mutable bool invalidData_;
    std::shared_ptr<LayerRAMPrecision<vec4>> dataRepr_;
    std::unique_ptr<Layer> data_;
};

bool operator==(const TransferFunction& lhs, const TransferFunction& rhs);
bool operator!=(const TransferFunction& lhs, const TransferFunction& rhs);

} // namespace
#endif // IVW_TRANSFERFUNCTION_H
