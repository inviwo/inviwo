/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

class IsoTFProperty;

class IVW_CORE_API TFPropertyObserver : public Observer {
public:
    virtual void onMaskChange(const dvec2& mask);
    virtual void onZoomHChange(const dvec2& zoomH);
    virtual void onZoomVChange(const dvec2& zoomV);
    virtual void onHistogramModeChange(HistogramMode mode);
    virtual void onHistogramSelectionChange(HistogramSelection selection);
};
class IVW_CORE_API TFPropertyObservable : public Observable<TFPropertyObserver> {
protected:
    void notifyMaskChange(const dvec2& mask);
    void notifyZoomHChange(const dvec2& zoomH);
    void notifyZoomVChange(const dvec2& zoomV);
    void notifyHistogramModeChange(HistogramMode mode);
    void notifyHistogramSelectionChange(HistogramSelection selection);
};

class IVW_CORE_API TFData {
public:
    struct Base {
        virtual ~Base() = default;
        virtual std::unique_ptr<Base> clone() const = 0;

        virtual const DataMapper& getDataMap() const = 0;

        virtual DispatcherHandle<HistogramCache::Callback> calculateHistograms(
            std::function<void(const std::vector<Histogram1D>&)> whenDone) const = 0;
    };

    template <typename T>
    struct Implementation : Base {
        Implementation(T toWrap) : wrapped{toWrap} {}

        virtual std::unique_ptr<Base> clone() const override {
            return std::make_unique<Implementation<T>>(*this);
        }

        virtual const DataMapper& getDataMap() const override {
            static const DataMapper default_;
            if (auto data = wrapped->getData()) {
                return data->dataMap;
            }
            return default_;
        }

        virtual DispatcherHandle<HistogramCache::Callback> calculateHistograms(
            std::function<void(const std::vector<Histogram1D>&)> whenDone) const override {
            if (auto data = wrapped->getData()) {
                return data->calculateHistograms(whenDone);
            }
        }

        T wrapped;
    };

    template <typename T>
    TFData(T data) : base_{std::make_unique<Implementation<T>>(data)} {}

    TFData(const TFData& rhs) : base_{rhs.base_->clone()} {}
    TFData(TFData&& rhs) : base_{std::exchange(rhs.base_, nullptr)} {}
    TFData& operator=(const TFData& that) {
        if (this != &that) {
            base_ = that.base_->clone();
        }
        return *this;
    }
    TFData& operator=(TFData&& that) {
        if (this != &that) {
            base_ = std::exchange(that.base_, nullptr);
        }
        return *this;
    }

    const DataMapper& getDataMap() const { return base_->getDataMap(); }

    DispatcherHandle<HistogramCache::Callback> calculateHistograms(
        std::function<void(const std::vector<Histogram1D>&)> whenDone) const {
        return base_->calculateHistograms(whenDone);
    }

private:
    std::unique_ptr<Base> base_;
};

/**
 * \ingroup properties
 * A property holding a TransferFunction data structure
 */
class IVW_CORE_API TransferFunctionProperty : public Property,
                                              public TFPrimitiveSetObserver,
                                              public TFPropertyObservable {

public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    TransferFunctionProperty(
        std::string_view identifier, std::string_view displayName, Document help,
        const TransferFunction& value = TransferFunction({{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                          {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
        VolumeInport* volumeInport = nullptr,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);

    TransferFunctionProperty(
        std::string_view identifier, std::string_view displayName,
        const TransferFunction& value = TransferFunction({{0.0, vec4(0.0f, 0.0f, 0.0f, 0.0f)},
                                                          {1.0, vec4(1.0f, 1.0f, 1.0f, 1.0f)}}),
        VolumeInport* volumeInport = nullptr,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);

    TransferFunctionProperty(std::string_view identifier, std::string_view displayName,
                             VolumeInport* volumeInport,
                             InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                             PropertySemantics semantics = PropertySemantics::Default);

    TransferFunctionProperty(const TransferFunctionProperty& rhs);
    virtual TransferFunctionProperty* clone() const override;
    virtual ~TransferFunctionProperty();

    TransferFunction& get();
    const TransferFunction& get() const;
    TransferFunctionProperty& set(const TransferFunction& tf);

    const TransferFunction& operator*() const;
    TransferFunction& operator*();
    const TransferFunction* operator->() const;
    TransferFunction* operator->();

    TransferFunctionProperty& setMask(double maskMin, double maskMax);
    const dvec2 getMask() const;
    TransferFunctionProperty& clearMask();

    TransferFunctionProperty& setZoomH(double zoomHMin, double zoomHMax);
    const dvec2& getZoomH() const;

    TransferFunctionProperty& setZoomV(double zoomVMin, double zoomVMax);
    const dvec2& getZoomV() const;

    /**
     * Set the HistogramMode to control how to scale the histogram
     * The options are:
     *
     * * Off Don't show any histograms
     * * All Make sure all the bars are fully visible
     * * P99 Make sure 99% of the bars are fully visible
     * * P95 Make sure 95% of the bars are fully visible
     * * P90 Make sure 90% of the bars are fully visible
     * * Log Apply logarithmic scaling to each bar.
     *
     */
    TransferFunctionProperty& setHistogramMode(HistogramMode type);
    HistogramMode getHistogramMode() const;

    /**
     * Set the HistogramSelection. The selection determine which of the histograms from the volume
     * in the optional volume port to show. The selection is a bitset, up to 32 histograms are
     * supported. By default all available histograms will be shown.
     */
    TransferFunctionProperty& setHistogramSelection(HistogramSelection selection);
    HistogramSelection getHistogramSelection() const;

    VolumeInport* getVolumeInport();

    virtual TransferFunctionProperty& setCurrentStateAsDefault() override;
    TransferFunctionProperty& setDefault(const TransferFunction& tf);
    virtual TransferFunctionProperty& resetToDefaultState() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void set(const Property* property) override;
    void set(const TransferFunctionProperty* p);
    void set(const IsoTFProperty* p);

    // Override TFPrimitiveSetObserver
    virtual void onTFPrimitiveAdded(TFPrimitive& p) override;
    virtual void onTFPrimitiveRemoved(TFPrimitive& p) override;
    virtual void onTFPrimitiveChanged(const TFPrimitive& p) override;
    virtual void onTFTypeChanged(const TFPrimitiveSet& primitiveSet) override;

private:
    ValueWrapper<TransferFunction> tf_;
    ValueWrapper<dvec2> zoomH_;
    ValueWrapper<dvec2> zoomV_;
    ValueWrapper<HistogramMode> histogramMode_;
    ValueWrapper<HistogramSelection> histogramSelection_;

    VolumeInport* volumeInport_;
};

}  // namespace inviwo
