/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <inviwo/dataframe/properties/colormapproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \brief Property for selecting which column to apply colormapping to.
 * Allows the user to select a column and options for the color map.
 * A ColormapProperty for each column will be added to this property, but only the
 * one corresponding to the selected axis will be visible.
 *
 * It is possible to override the ColormapProperty
 *
 */
class IVW_MODULE_DATAFRAME_API DataFrameColormapProperty : public CompositeProperty {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    DataFrameColormapProperty(
        std::string identifier, std::string displayName, DataInport<DataFrame>& port,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);
    virtual ~DataFrameColormapProperty() = default;

    void createOrUpdateProperties(std::shared_ptr<const DataFrame> dataframe);

    DataFrameColumnProperty selectedColorAxis;
    BoolProperty overrideColormap;
    TransferFunctionProperty tf;

protected:
    std::vector<ColormapProperty*> colormaps_;
    std::shared_ptr<const DataFrame> dataframe_;
    std::shared_ptr<std::function<void()>> colormapChanged_;
};

}  // namespace inviwo
