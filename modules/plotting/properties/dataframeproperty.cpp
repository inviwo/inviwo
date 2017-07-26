/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#include <modules/plotting/properties/dataframeproperty.h>

namespace inviwo {

namespace plot {

PropertyClassIdentifier(DataFrameColumnProperty, "org.inviwo.DataFrameColumnProperty");

DataFrameColumnProperty::DataFrameColumnProperty(std::string identifier, std::string displayName,
                                                 bool allowNone, size_t firstIndex)
    : OptionPropertyInt(identifier, displayName)
    , dataframe_(nullptr)
    , allowNone_(allowNone)
    , firstIndex_(firstIndex) {
    setSerializationMode(PropertySerializationMode::All);
}

DataFrameColumnProperty::DataFrameColumnProperty(std::string identifier, std::string displayName,
                                                 DataInport<DataFrame> &port, bool allowNone,
                                                 size_t firstIndex)
    : OptionPropertyInt(identifier, displayName)
    , dataframe_(port.getData())
    , allowNone_(allowNone)
    , firstIndex_(firstIndex) {
    setSerializationMode(PropertySerializationMode::All);
    auto portPtr = &port;

    port.onChange([=]() {
        if (portPtr->hasData()) {
            setOptions(portPtr->getData());
        }
    });
}

void DataFrameColumnProperty::setOptions(std::shared_ptr<const DataFrame> dataframe) {
    if (!dataframe || dataframe->getNumberOfColumns() <= 1) return;
    dataframe_ = dataframe;
    std::string prevID = "";
    if (options_.size() != 0) {
        prevID = getSelectedIdentifier();
    }

    clearOptions();

    if (allowNone_) {
        addOption("none", "None", -1);
    }

    int idx = 0;
    for (const auto &col : *dataframe) {
        if (idx == 0) {
            idx++;
            continue;
        }
        auto header = col->getHeader();
        auto identifier = header;
        util::erase_remove_if(identifier, [](char cc) {
            return !(cc >= -1) || !(std::isalnum(cc) || cc == '_' || cc == '-');
        });
        addOption(identifier, header, idx);
        if ((prevID == "" && static_cast<int>(firstIndex_) == idx) || identifier == prevID) {
            setSelectedIdentifier(identifier);
        }
        idx++;
    }

    propertyModified();
    setCurrentStateAsDefault();
}

std::shared_ptr<const Column> DataFrameColumnProperty::getColumn() {
    if (!dataframe_) {
        return nullptr;
    }
    auto id = get();
    if (id == -1) {
        return nullptr;
    };  // None is selected
    return dataframe_->getColumn(id);
}

std::shared_ptr<const BufferBase> DataFrameColumnProperty::getBuffer() {
    if (auto col = getColumn()) {
        return col->getBuffer();
    }
    return nullptr;
}

void DataFrameColumnProperty::set(const Property *p) {
    if (auto dfcp = dynamic_cast<const DataFrameColumnProperty *>(p)) {
        if (dfcp->dataframe_ != dataframe_) {
            return;
        }
        if (dfcp->options_.size() == 0) return;
        if (options_.size() == 0) return;

        auto i = dfcp->getSelectedValue();
        if (i == -1 && !allowNone_) {
            // Do Nothing
        } else {
            auto id = dfcp->getSelectedIdentifier();
            if (id != getSelectedIdentifier()) {
                setSelectedIdentifier(id);
            }
        }
    } else {
        OptionPropertyInt::set(p);
    }
}

}  // namespace plot

}  // namespace inviwo
