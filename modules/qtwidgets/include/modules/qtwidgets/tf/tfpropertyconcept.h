/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/histogram.h>  // for HistogramMode, HistogramSelection
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/ports/volumeport.h>                     // for VolumeInport
#include <inviwo/core/properties/transferfunctionproperty.h>  // IWYU pragma: keep
#include <inviwo/core/properties/isovalueproperty.h>          // IWYU pragma: keep
#include <inviwo/core/util/glmvec.h>                          // for dvec2
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/detected.h>
#include <inviwo/core/network/networklock.h>  // for NetworkLock
#include <modules/qtwidgets/tf/tfutils.h>     // for exportToFile, importFro...

#include <utility>  // for declval
#include <span>
#include <array>

namespace inviwo {
/**
 * \class TFPropertyConcept
 * \brief property interface used by the TF dialog to support different TF properties
 */
class IVW_MODULE_QTWIDGETS_API TFPropertyConcept {
public:
    virtual ~TFPropertyConcept() = default;
    virtual Property* getProperty() const = 0;

    virtual bool hasTF() const = 0;
    virtual bool hasIsovalues() const = 0;

    virtual TransferFunctionProperty* getTFProperty() const = 0;
    virtual IsoValueProperty* getIsoValueProperty() const = 0;

    virtual TransferFunction* getTransferFunction() const = 0;
    virtual IsoValueCollection* getIsovalues() const = 0;

    virtual bool supportsMask() const = 0;
    virtual void setMask(double maskMin, double maskMax) = 0;
    virtual const dvec2 getMask() const = 0;
    virtual void clearMask() = 0;

    virtual void setZoomH(double zoomHMin, double zoomHMax) = 0;
    virtual const dvec2& getZoomH() const = 0;

    virtual void setZoomV(double zoomVMin, double zoomVMax) = 0;
    virtual const dvec2& getZoomV() const = 0;

    virtual void setHistogramMode(HistogramMode type) = 0;
    virtual HistogramMode getHistogramMode() const = 0;

    virtual void setHistogramSelection(HistogramSelection selection) = 0;
    virtual HistogramSelection getHistogramSelection() const = 0;

    virtual const DataMapper* getDataMap() const = 0;

    virtual void addObserver(TFPropertyObserver* observer) = 0;
    virtual void removeObserver(TFPropertyObserver* observer) = 0;

    virtual void showExportDialog() const = 0;
    virtual void showImportDialog() = 0;

    virtual std::span<TFPrimitiveSet*> sets() = 0;

    [[nodiscard]] virtual DispatcherHandle<void()> onDataChange(std::function<void()>) = 0;

    enum class HistogramChange { NoData, Requested, NewData };
    using HistogramCallback = void(HistogramChange, const std::vector<Histogram1D>&);
    [[nodiscard]] virtual DispatcherHandle<HistogramCallback> onHistogramChange(
        std::function<HistogramCallback>) = 0;
};

template <typename U>
class TFPropertyModel : public TFPropertyConcept {
public:
    explicit TFPropertyModel(U* data)
        : tfLike_{data}
        , dataOnChangeHandle_(tfLike_->data().onChange([this]() {
            onDataChangeDispatcher_.invoke();
            setUpHistogramCallback();
        }))
        , sets_{[&]() {
            if constexpr (std::is_same_v<TransferFunctionProperty, U>) {
                return std::array<TFPrimitiveSet*, 2>{&tfLike_->get(), nullptr};
            } else if constexpr (std::is_same_v<IsoValueProperty, U>) {
                return std::array<TFPrimitiveSet*, 2>{&tfLike_->get(), nullptr};
            } else if constexpr (std::is_same_v<IsoTFProperty, U>) {
                return std::array<TFPrimitiveSet*, 2>{&tfLike_->tf_.get(),
                                                      &tfLike_->isovalues_.get()};
            } else {
                static_assert(util::alwaysFalse<U>(), "Type not supported");
            }
        }()} {}

    virtual Property* getProperty() const override { return tfLike_; }

    virtual std::span<TFPrimitiveSet*> sets() override {
        if (sets_[1] != nullptr) {
            return std::span<TFPrimitiveSet*>{sets_.data(), 2};
        } else {
            return std::span<TFPrimitiveSet*>{sets_.data(), 1};
        }
    }

    virtual bool hasTF() const override { return getTFProperty() != nullptr; }
    virtual bool hasIsovalues() const override { return getIsoValueProperty() != nullptr; }

    virtual TransferFunctionProperty* getTFProperty() const override {
        if constexpr (std::is_same_v<TransferFunctionProperty, U>) {
            return tfLike_;
        } else if constexpr (std::is_same_v<IsoTFProperty, U>) {
            return &tfLike_->tf_;
        } else {
            return nullptr;
        }
    }
    virtual IsoValueProperty* getIsoValueProperty() const override {
        if constexpr (std::is_same_v<IsoValueProperty, U>) {
            return tfLike_;
        } else if constexpr (std::is_same_v<IsoTFProperty, U>) {
            return &tfLike_->isovalues_;
        } else {
            return nullptr;
        }
    }
    virtual TransferFunction* getTransferFunction() const override {
        if (auto tf = getTFProperty()) {
            return &tf->get();
        } else {
            return nullptr;
        }
    }
    virtual IsoValueCollection* getIsovalues() const override {
        if (auto iso = getIsoValueProperty()) {
            return &iso->get();
        } else {
            return nullptr;
        }
    }

    virtual bool supportsMask() const override { return hasTF(); }
    virtual void setMask(double maskMin, double maskMax) override {
        if (auto tf = getTFProperty()) {
            tf->setMask(maskMin, maskMax);
        }
    }
    virtual const dvec2 getMask() const override {
        if (auto tf = getTFProperty()) {
            return tf->getMask();
        } else {
            return {};
        }
    }
    virtual void clearMask() override {
        if (auto tf = getTFProperty()) {
            tf->clearMask();
        }
    }

    virtual void setZoomH(double zoomHMin, double zoomHMax) override {
        tfLike_->setZoomH(zoomHMin, zoomHMax);
    }
    virtual const dvec2& getZoomH() const override { return tfLike_->getZoomH(); }

    virtual void setZoomV(double zoomVMin, double zoomVMax) override {
        tfLike_->setZoomV(zoomVMin, zoomVMax);
    }
    virtual const dvec2& getZoomV() const override { return tfLike_->getZoomV(); }

    virtual void setHistogramMode(HistogramMode type) override { tfLike_->setHistogramMode(type); }
    virtual HistogramMode getHistogramMode() const override { return tfLike_->getHistogramMode(); }

    virtual void setHistogramSelection(HistogramSelection selection) override {
        tfLike_->setHistogramSelection(selection);
    }
    virtual HistogramSelection getHistogramSelection() const override {
        return tfLike_->getHistogramSelection();
    }

    virtual void addObserver(TFPropertyObserver* observer) override {
        tfLike_->TFPropertyObservable::addObserver(observer);
    }
    virtual void removeObserver(TFPropertyObserver* observer) override {
        tfLike_->TFPropertyObservable::removeObserver(observer);
    }

    virtual void showExportDialog() const override {
        if (auto* tf = getTransferFunction()) {
            util::exportTransferFunctionDialog(*tf);
        } else if (auto* iso = getIsovalues()) {
            util::exportIsoValueCollectionDialog(*iso);
        }
    }

    virtual void showImportDialog() override {
        if (auto* tf = getTransferFunction()) {
            if (auto newTf = util::importTransferFunctionDialog()) {
                NetworkLock lock{tfLike_};
                *tf = *newTf;
            }
        } else if (auto* iso = getIsovalues()) {
            if (auto newIso = util::importIsoValueCollectionDialog()) {
                NetworkLock lock{tfLike_};
                *iso = *newIso;
            }
        }
    }

    virtual const DataMapper* getDataMap() const override { return tfLike_->data().getDataMap(); }

    virtual DispatcherHandle<void()> onDataChange(std::function<void()> callback) override {
        return onDataChangeDispatcher_.add(callback);
    }

    virtual DispatcherHandle<HistogramCallback> onHistogramChange(
        std::function<HistogramCallback> callback) override {
        auto handle = histogramChangeCallbacks_.add(std::move(callback));
        setUpHistogramCallback();
        return handle;
    }

private:
    void setUpHistogramCallback() {
        histogramChangeCallbacks_.invoke(HistogramChange::Requested, std::vector<Histogram1D>{});
        histogramResult_ = tfLike_->data().calculateHistograms(
            [this](const std::vector<Histogram1D>& histograms) -> void {
                histogramChangeCallbacks_.invoke(HistogramChange::NewData, histograms);
            });

        if (histogramResult_.progress == HistogramCache::Progress::NoData) {
            histogramChangeCallbacks_.invoke(HistogramChange::NoData, std::vector<Histogram1D>{});
        } else if (histogramResult_.progress == HistogramCache::Progress::Calculating) {
            histogramChangeCallbacks_.invoke(HistogramChange::Requested,
                                             std::vector<Histogram1D>{});
        }
    }

    U* tfLike_;
    Dispatcher<void()> onDataChangeDispatcher_;
    Dispatcher<HistogramCallback> histogramChangeCallbacks_;
    HistogramCache::Result histogramResult_;
    TFData::OnChangeHandle dataOnChangeHandle_;
    std::array<TFPrimitiveSet*, 2> sets_;
};

}  // namespace inviwo
