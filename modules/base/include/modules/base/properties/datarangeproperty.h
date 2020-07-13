/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {

/**
 * \ingroup properties
 * A CompositeProperty holding data and value ranges for volumes and optional custom range overrides
 */
class IVW_MODULE_BASE_API DataRangeProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;
    DataRangeProperty(std::string identifier, std::string displayName, bool customRanges = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    DataRangeProperty(std::string identifier, std::string displayName, VolumeInport& port,
                      bool customRanges = true,
                      InvalidationLevel invalidationLevel = InvalidationLevel::InvalidResources,
                      PropertySemantics semantics = PropertySemantics::Default);
    DataRangeProperty(const DataRangeProperty& rhs);
    virtual DataRangeProperty* clone() const override;
    virtual ~DataRangeProperty() = default;

    /**
     * \brief update both value and data ranges using \p volume
     */
    void updateFromVolume(std::shared_ptr<Volume> volume);
    /**
     * \brief update the data range
     */
    void setDataRange(const dvec2& range);
    /**
     * \brief update the value range
     */
    void setValueRange(const dvec2& range);
    /**
     * \brief returns the currently selected data range
     *
     * @return custom data range if enabled, otherwise input data range
     */
    dvec2 getDataRange() const;
    dvec2 getCustomDataRange() const;
    /**
     * \brief returns the currently selected value range
     *
     * @return custom value range if enabled, otherwise input value range
     */
    dvec2 getValueRange() const;
    dvec2 getCustomValueRange() const;

    bool getCustomRangeEnabled() const;

private:
    const bool customRanges_;

    DoubleMinMaxProperty dataRange_;
    DoubleMinMaxProperty valueRange_;
    BoolCompositeProperty useCustomRange_;
    DoubleMinMaxProperty customDataRange_;
    DoubleMinMaxProperty customValueRange_;
    ButtonProperty copyFromInput_;
};

}  // namespace inviwo
