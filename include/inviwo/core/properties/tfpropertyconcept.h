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

#ifndef IVW_TFPROPERTYCONCEPT_H
#define IVW_TFPROPERTYCONCEPT_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

namespace inviwo {

class TFPropertyObserver;

namespace util {

/**
 * \class TFPropertyConcept
 * \brief property interface used by the TF dialog to support different TF properties
 */
struct IVW_CORE_API TFPropertyConcept {
    virtual ~TFPropertyConcept() = default;
    virtual Property* getProperty() const = 0;

    virtual bool hasTF() const = 0;
    virtual bool hasIsovalues() const = 0;

    virtual TransferFunctionProperty* getTFProperty() const = 0;

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
    virtual HistogramMode getHistogramMode() = 0;
    virtual VolumeInport* getVolumeInport() = 0;
    virtual void addObserver(TFPropertyObserver* observer) = 0;
    virtual void removeObserver(TFPropertyObserver* observer) = 0;
};

template <typename U>
class TFPropertyModel : public TFPropertyConcept {
public:
    TFPropertyModel(U data) : data_(data) {}

    virtual U getProperty() const override { return data_; }

    virtual bool hasTF() const override { return hasTFInternal(); }
    virtual bool hasIsovalues() const override { return hasIsovaluesInternal(); }

    virtual TransferFunctionProperty* getTFProperty() const override {
        return getTFPropertyInternal();
    }

    virtual TFPrimitiveSet* getTransferFunction() const override { return getTFInternal(); }
    virtual TFPrimitiveSet* getIsovalues() const override { return getIsovaluesInternal(); }

    virtual bool supportsMask() const override { return supportsMaskInternal(); }
    virtual void setMask(double maskMin, double maskMax) override {
        setMaskInternal(maskMin, maskMax);
    }
    virtual const dvec2 getMask() const override { return getMaskInternal(); }
    virtual void clearMask() override { clearMaskInternal(); }

    virtual void setZoomH(double zoomHMin, double zoomHMax) override {
        data_->setZoomH(zoomHMin, zoomHMax);
    }
    virtual const dvec2& getZoomH() const override { return data_->getZoomH(); }

    virtual void setZoomV(double zoomVMin, double zoomVMax) override {
        data_->setZoomV(zoomVMin, zoomVMax);
    }
    virtual const dvec2& getZoomV() const override { return data_->getZoomV(); }

    virtual void setHistogramMode(HistogramMode type) override { data_->setHistogramMode(type); }
    virtual HistogramMode getHistogramMode() override { return data_->getHistogramMode(); }
    virtual VolumeInport* getVolumeInport() override { return data_->getVolumeInport(); }

    virtual void addObserver(TFPropertyObserver* observer) override {
        data_->TFPropertyObservable::addObserver(observer);
    }
    virtual void removeObserver(TFPropertyObserver* observer) override {
        data_->TFPropertyObservable::removeObserver(observer);
    }

private:
    TFPrimitiveSet* getTFInternal() const { return nullptr; }
    TFPrimitiveSet* getIsovaluesInternal() const { return nullptr; }
    bool hasTFInternal() const { return false; }
    bool hasIsovaluesInternal() const { return false; }
    TransferFunctionProperty* getTFPropertyInternal() const { return nullptr; }

    bool supportsMaskInternal() const { return true; }
    void setMaskInternal(double maskMin, double maskMax) { data_->setMask(maskMin, maskMax); }
    const dvec2 getMaskInternal() const { return data_->getMask(); }
    void clearMaskInternal() { data_->clearMask(); }

    U data_;
};

// TransferFunctionProperty
template <>
IVW_CORE_API TFPrimitiveSet* TFPropertyModel<TransferFunctionProperty*>::getTFInternal() const;
template <>
IVW_CORE_API bool TFPropertyModel<TransferFunctionProperty*>::hasTFInternal() const;
template <>
IVW_CORE_API TransferFunctionProperty*
TFPropertyModel<TransferFunctionProperty*>::getTFPropertyInternal() const;

extern template class IVW_CORE_TMPL_EXP TFPropertyModel<TransferFunctionProperty*>;

// IsoValueProperty
template <>
IVW_CORE_API TFPrimitiveSet* TFPropertyModel<IsoValueProperty*>::getIsovaluesInternal() const;
template <>
IVW_CORE_API bool TFPropertyModel<IsoValueProperty*>::hasIsovaluesInternal() const;
template <>
IVW_CORE_API bool TFPropertyModel<IsoValueProperty*>::supportsMaskInternal() const;
template <>
IVW_CORE_API void TFPropertyModel<IsoValueProperty*>::setMaskInternal(double, double);
template <>
IVW_CORE_API const dvec2 TFPropertyModel<IsoValueProperty*>::getMaskInternal() const;
template <>
IVW_CORE_API void TFPropertyModel<IsoValueProperty*>::clearMaskInternal();

extern template class IVW_CORE_TMPL_EXP TFPropertyModel<IsoValueProperty*>;

// IsoTFProperty
template <>
IVW_CORE_API TFPrimitiveSet* TFPropertyModel<IsoTFProperty*>::getTFInternal() const;
template <>
IVW_CORE_API TFPrimitiveSet* TFPropertyModel<IsoTFProperty*>::getIsovaluesInternal() const;
template <>
IVW_CORE_API bool TFPropertyModel<IsoTFProperty*>::hasTFInternal() const;
template <>
IVW_CORE_API bool TFPropertyModel<IsoTFProperty*>::hasIsovaluesInternal() const;
template <>
IVW_CORE_API TransferFunctionProperty* TFPropertyModel<IsoTFProperty*>::getTFPropertyInternal()
    const;

extern template class IVW_CORE_TMPL_EXP TFPropertyModel<IsoTFProperty*>;

}  // namespace util

}  // namespace inviwo

#endif  // IVW_TFPROPERTYCONCEPT_H
