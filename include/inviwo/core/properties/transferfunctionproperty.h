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

#ifndef IVW_TRANSFERFUNCTIONPROPERTY_H
#define IVW_TRANSFERFUNCTIONPROPERTY_H

#include <inviwo/core/properties/templateproperty.h>

#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

class IVW_CORE_API TransferFunctionPropertyObserver : public Observer {
public:
    virtual void onMaskChange(const vec2& mask) {};
    virtual void onZoomHChange(const vec2& zoomH) {};
    virtual void onZoomVChange(const vec2& zoomV) {};
    virtual void onHistogramModeChange(HistogramMode mode) {};
};
class IVW_CORE_API TransferFunctionPropertyObservable : public Observable<TransferFunctionPropertyObserver> {
protected:
    virtual void notifyMaskChange(const vec2& mask);
    virtual void notifyZoomHChange(const vec2& zoomH);
    virtual void notifyZoomVChange(const vec2& zoomV);
    virtual void notifyHistogramModeChange(HistogramMode mode);
};



/**
 * \ingroup properties
 * A property holding a TransferFunction data structure
 */
class IVW_CORE_API TransferFunctionProperty 
    : public TemplateProperty<TransferFunction>
    , public TransferFunctionObserver
    , public TransferFunctionPropertyObservable {

public:
    InviwoPropertyInfo();

    TransferFunctionProperty(
        const std::string& identifier, const std::string& displayName,
        const TransferFunction& value = TransferFunction({{0.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                          {1.0f, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
        VolumeInport* volumeInport = nullptr,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);

    TransferFunctionProperty(const TransferFunctionProperty& rhs);
    TransferFunctionProperty& operator=(const TransferFunctionProperty& that);  
    virtual TransferFunctionProperty* clone() const override;
    virtual ~TransferFunctionProperty();

    const vec2 getMask() const;
    void setMask(float maskMin, float maskMax);

    const vec2& getZoomH() const;
    void setZoomH(float zoomHMin, float zoomHMax);

    const vec2& getZoomV() const;
    void setZoomV(float zoomVMin, float zoomVMax);

    void setHistogramMode(HistogramMode type);
    HistogramMode getHistogramMode();
    VolumeInport* getVolumeInport();

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // Override
    virtual void set(const TransferFunction& property) override;
    virtual void set(const Property *property) override;
    virtual void onControlPointAdded(TransferFunctionDataPoint* p) override;
    virtual void onControlPointRemoved(TransferFunctionDataPoint* p) override;
    virtual void onControlPointChanged(const TransferFunctionDataPoint* p) override;

private:
    ValueWrapper<vec2> zoomH_;
    ValueWrapper<vec2> zoomV_;
    ValueWrapper<HistogramMode> histogramMode_;

    VolumeInport* volumeInport_;
};

} // namespace inviwo

#endif // IVW_TRANSFERFUNCTIONPROPERTY_H