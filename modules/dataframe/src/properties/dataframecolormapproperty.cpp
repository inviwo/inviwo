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

#include <inviwo/dataframe/properties/dataframecolormapproperty.h>
#include <inviwo/core/util/utilities.h>

namespace inviwo {

const std::string DataFrameColormapProperty::classIdentifier =
    "org.inviwo.DataFrameColormapProperty";
std::string DataFrameColormapProperty::getClassIdentifier() const { return classIdentifier; }

DataFrameColormapProperty::DataFrameColormapProperty(std::string identifier,
                                                     std::string displayName,
                                                     DataInport<DataFrame>& port,
                                                     InvalidationLevel invalidationLevel,
                                                     PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , selectedColorAxis{"selectedColorAxis", "Column", port, false, 1}
    , overrideColormap("overrideColormap", "Override", false)
    , tf{"tf", "Colormap"}
    , dataframe_(port.getData()) {

    overrideColormap.onChange([&]() {
        if (*overrideColormap) {
            colormapChanged_.reset();
            for (auto p : colormaps_) {
                p->setVisible(false);
            }
        } else {
            selectedColorAxis.propertyModified();
        }
        tf.setReadOnly(!*overrideColormap);
    });

    selectedColorAxis.onChange([&]() {
        if (selectedColorAxis.size() == 0 || colormaps_.size() != selectedColorAxis.size() ||
            overrideColormap)
            return;

        for (auto i = 0u; i < colormaps_.size(); i++) {
            colormaps_[i]->setVisible(i == static_cast<size_t>(*selectedColorAxis));
        }

        auto updateColormap = [&]() {
            if (colormaps_.size() <= static_cast<size_t>(*selectedColorAxis)) return;
            auto cm = dynamic_cast<const ColormapProperty*>(colormaps_[selectedColorAxis]);
            if (cm) *tf = cm->getTransferFunction();
        };
        colormapChanged_ = dynamic_cast<ColormapProperty*>(colormaps_[selectedColorAxis])
                               ->onChangeScoped(updateColormap);
        updateColormap();
    });

    addProperties(selectedColorAxis, overrideColormap, tf);

    port.onChange([this, portPtr = &port]() {
        if (portPtr->hasData()) {
            createOrUpdateProperties(portPtr->getData());
        }
    });
}

void DataFrameColormapProperty::createOrUpdateProperties(
    std::shared_ptr<const DataFrame> dataframe) {
    if (!dataframe || dataframe->getNumberOfColumns() <= 1) return;
    dataframe_ = dataframe;
    auto oldColormapProperties = colormaps_;
    colormaps_.clear();
    for (size_t i = 0; i < dataframe_->getNumberOfColumns(); i++) {
        auto c = dataframe_->getColumn(i);
        std::string displayName = "Settings";
        std::string identifier = util::stripIdentifier(c->getHeader());

        auto colormapProp = [&]() -> ColormapProperty* {
            if (auto p = getPropertyByIdentifier(identifier)) {
                if (auto prop = dynamic_cast<ColormapProperty*>(p)) {
                    return prop;
                }
                removeProperty(identifier);
            }
            auto newProp = std::make_unique<ColormapProperty>(identifier, displayName);
            auto ptr = newProp.get();
            newProp->setVisible(false);
            newProp->setSerializationMode(PropertySerializationMode::All);
            insertProperty(1, newProp.release());
            return ptr;
        }();
        colormaps_.push_back(colormapProp);
        colormapProp->setupForColumn(*c);
    }
    // Remove properties that were not reused
    for (auto p : oldColormapProperties) {
        if (util::find(colormaps_, p) == colormaps_.end()) {
            removeProperty(p);
        }
    }
    setCurrentStateAsDefault();
    selectedColorAxis.propertyModified();
}

}  // namespace inviwo
