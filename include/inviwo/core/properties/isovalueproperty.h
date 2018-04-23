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

#ifndef IVW_ISOVALUEPROPERTY_H
#define IVW_ISOVALUEPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/properties/templateproperty.h>

#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

class IVW_CORE_API IsoValuePropertyObserver : public Observer {
public:
    virtual void onEnabledChange(bool enabled);
    virtual void onZoomHChange(const vec2& zoomH);
    virtual void onZoomVChange(const vec2& zoomV);
    virtual void onHistogramModeChange(HistogramMode mode);
};

class IVW_CORE_API IsoValuePropertyObservable : public Observable<IsoValuePropertyObserver> {
protected:
    virtual void notifyEnabledChange(bool enabled);
    virtual void notifyZoomHChange(const vec2& zoomH);
    virtual void notifyZoomVChange(const vec2& zoomV);
    virtual void notifyHistogramModeChange(HistogramMode mode);
};

/**
 * \ingroup properties
 * \class IsoValueProperty
 * \brief property managing a collection of isovalues
 */
class IVW_CORE_API IsoValueProperty : public TemplateProperty<IsoValueCollection>,
                                      public TFPrimitiveSetObserver,
                                      public IsoValuePropertyObservable {
public:
    InviwoPropertyInfo();

    IsoValueProperty(const std::string& identifier, const std::string& displayName,
                     const IsoValueCollection& value =
                         IsoValueCollection({{0.5f, vec4(1.0f, 1.0f, 1.0f, 0.5f)}}),
                     VolumeInport* volumeInport = nullptr,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                     PropertySemantics semantics = PropertySemantics::Default);

    IsoValueProperty(const std::string& identifier, const std::string& displayName,
                     VolumeInport* volumeInport,
                     InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                     PropertySemantics semantics = PropertySemantics::Default);

    IsoValueProperty(const IsoValueProperty& rhs);
    virtual ~IsoValueProperty();

    IsoValueProperty& operator=(const IsoValueProperty& rhs);
    virtual IsoValueProperty* clone() const override;

    bool getEnabled() const;
    void setEnabled(bool enable);

    const vec2& getZoomH() const;
    void setZoomH(float zoomHMin, float zoomHMax);

    const vec2& getZoomV() const;
    void setZoomV(float zoomVMin, float zoomVMax);

    void setHistogramMode(HistogramMode mode);
    HistogramMode getHistogramMode();

    VolumeInport* getVolumeInport();

    virtual void setCurrentStateAsDefault() override;
    virtual void resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    // Overrides
    virtual void set(const IsoValueCollection& c) override;
    virtual void set(const Property* property) override;
    virtual void onTFPrimitiveAdded(TFPrimitive* p) override;
    virtual void onTFPrimitiveRemoved(TFPrimitive* p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitive* p) override;

private:
    ValueWrapper<bool> enabled_;
    ValueWrapper<vec2> zoomH_;
    ValueWrapper<vec2> zoomV_;
    ValueWrapper<HistogramMode> histogramMode_;

    VolumeInport* volumeInport_;
};

}  // namespace inviwo

#endif  // IVW_ISOVALUEPROPERTY_H
