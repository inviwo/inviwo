/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <inviwo/core/util/detected.h>

namespace inviwo {

class TFPropertyObserver;

namespace util {

/**
 * \class TFPropertyConcept
 * \brief property interface used by the TF dialog to support different TF properties
 */
struct IVW_MODULE_QTWIDGETS_API TFPropertyConcept {
    virtual ~TFPropertyConcept() = default;
    virtual Property* getProperty() const = 0;

    virtual bool hasTF() const = 0;
    virtual bool hasIsovalues() const = 0;

    virtual TransferFunctionProperty* getTFProperty() const = 0;
    virtual IsoValueProperty* getIsoValueProperty() const = 0;

    virtual TFPrimitiveSet* getTransferFunction() const = 0;
    virtual TFPrimitiveSet* getIsovalues() const = 0;

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

    virtual VolumeInport* getVolumeInport() = 0;
    virtual void addObserver(TFPropertyObserver* observer) = 0;
    virtual void removeObserver(TFPropertyObserver* observer) = 0;
};

template <typename U>
class TFPropertyModel : public TFPropertyConcept {
public:
    template <typename T>
    using hasTFProp = decltype(std::declval<T>().tf_);

    template <typename T>
    using hasISOProp = decltype(std::declval<T>().isovalues_);

    TFPropertyModel(U* data) : data_(data) {}

    virtual Property* getProperty() const override { return data_; }

    virtual bool hasTF() const override { return getTFProperty() != nullptr; }
    virtual bool hasIsovalues() const override { return getIsoValueProperty() != nullptr; }

    virtual TransferFunctionProperty* getTFProperty() const override {
        if constexpr (std::is_same_v<TransferFunctionProperty, U>) {
            return data_;
        } else if constexpr (util::is_detected_exact_v<TransferFunctionProperty, hasTFProp, U>) {
            return &data_->tf_;
        } else {
            return nullptr;
        }
    }
    virtual IsoValueProperty* getIsoValueProperty() const override {
        if constexpr (std::is_same_v<IsoValueProperty, U>) {
            return data_;
        } else if constexpr (util::is_detected_exact_v<IsoValueProperty, hasISOProp, U>) {
            return &data_->isovalues_;
        } else {
            return nullptr;
        }
    }
    virtual TFPrimitiveSet* getTransferFunction() const override {
        if (auto tf = getTFProperty()) {
            return &tf->get();
        } else {
            return nullptr;
        }
    }
    virtual TFPrimitiveSet* getIsovalues() const override {
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
        data_->setZoomH(zoomHMin, zoomHMax);
    }
    virtual const dvec2& getZoomH() const override { return data_->getZoomH(); }

    virtual void setZoomV(double zoomVMin, double zoomVMax) override {
        data_->setZoomV(zoomVMin, zoomVMax);
    }
    virtual const dvec2& getZoomV() const override { return data_->getZoomV(); }

    virtual void setHistogramMode(HistogramMode type) override { data_->setHistogramMode(type); }
    virtual HistogramMode getHistogramMode() const override { return data_->getHistogramMode(); }

    virtual void setHistogramSelection(HistogramSelection selection) override {
        data_->setHistogramSelection(selection);
    }
    virtual HistogramSelection getHistogramSelection() const override {
        return data_->getHistogramSelection();
    }

    virtual VolumeInport* getVolumeInport() override { return data_->getVolumeInport(); }

    virtual void addObserver(TFPropertyObserver* observer) override {
        data_->TFPropertyObservable::addObserver(observer);
    }
    virtual void removeObserver(TFPropertyObserver* observer) override {
        data_->TFPropertyObservable::removeObserver(observer);
    }

private:
    U* data_;
};

}  // namespace util

}  // namespace inviwo
