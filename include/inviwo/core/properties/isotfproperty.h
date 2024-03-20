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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

/**
 * \ingroup properties
 * \class IsoTFProperty
 * \brief composite property combining transfer function and isovalue properties
 */
class IVW_CORE_API IsoTFProperty : public CompositeProperty,
                                   public PropertyObserver,
                                   public TFPropertyObservable,
                                   public TFPropertyObserver {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    IsoTFProperty(std::string_view identifier, std::string_view displayName, Document help,
                  const IsoValueCollection& isovalues = {},
                  const TransferFunction& tf = TransferFunction({{0.0f, vec4(0.0f)},
                                                                 {1.0f, vec4(1.0f)}}),
                  TFData port = {},
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    IsoTFProperty(std::string_view identifier, std::string_view displayName,
                  const IsoValueCollection& isovalues = {},
                  const TransferFunction& tf = TransferFunction({{0.0f, vec4(0.0f)},
                                                                 {1.0f, vec4(1.0f)}}),
                  TFData port = {},
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    IsoTFProperty(std::string_view identifier, std::string_view displayName, Document help,
                  TFData port,
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    IsoTFProperty(std::string_view identifier, std::string_view displayName, TFData port,
                  InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                  PropertySemantics semantics = PropertySemantics::Default);

    IsoTFProperty(const IsoTFProperty& rhs);
    virtual ~IsoTFProperty() = default;

    virtual IsoTFProperty* clone() const override;

    virtual std::string getClassIdentifierForWidget() const override;

    virtual void set(const Property* property) override;
    /**
     * \brief sets only the isovalue property to \p p. The transfer function property remains
     * unchanged.
     */
    void set(const IsoValueProperty* p);
    /**
     * \brief sets only the transfer function property to \p p. The isovalue property remains
     * unchanged.
     */
    void set(const TransferFunctionProperty* p);

    void setMask(double maskMin, double maskMax);
    const dvec2 getMask() const;
    void clearMask();

    void setZoomH(double zoomHMin, double zoomHMax);
    const dvec2& getZoomH() const;

    void setZoomV(double zoomVMin, double zoomVMax);
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
    IsoTFProperty& setHistogramMode(HistogramMode type);
    HistogramMode getHistogramMode() const;

    /**
     * Set the HistogramSelection. The selection determine which of the histograms from the volume
     * in the optional volume port to show. The selection is a bitset, up to 32 histograms are
     * supported. By default all available histograms will be shown.
     */
    IsoTFProperty& setHistogramSelection(HistogramSelection selection);
    HistogramSelection getHistogramSelection() const;

    const TFData& data() const { return tf_.data(); }

    IsoValueProperty isovalues_;
    TransferFunctionProperty tf_;

protected:
    virtual void onZoomHChange(const dvec2& zoomH) override;
    virtual void onZoomVChange(const dvec2& zoomV) override;
    virtual void onHistogramModeChange(HistogramMode mode) override;
    virtual void onHistogramSelectionChange(HistogramSelection selection) override;
};

}  // namespace inviwo
