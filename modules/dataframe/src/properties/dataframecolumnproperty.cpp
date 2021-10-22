/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <inviwo/dataframe/properties/dataframecolumnproperty.h>
#include <inviwo/core/util/zip.h>

namespace inviwo {

const std::string DataFrameColumnProperty::classIdentifier = "org.inviwo.DataFrameColumnProperty";
std::string DataFrameColumnProperty::getClassIdentifier() const { return classIdentifier; }

DataFrameColumnProperty::DataFrameColumnProperty(std::string identifier, std::string displayName,
                                                 EmptySelection emptySelection, size_t defaultIndex)
    : OptionPropertyInt(identifier, displayName)
    , emptySelection_(emptySelection)
    , defaultIndex_(defaultIndex) {

    setSerializationMode(PropertySerializationMode::All);
    if (emptySelection_ == EmptySelection::Yes) {
        addOption("none", "None", -1);
    }
}

DataFrameColumnProperty::DataFrameColumnProperty(std::string identifier, std::string displayName,
                                                 DataFrameInport& port,
                                                 EmptySelection emptySelection, size_t defaultIndex)
    : OptionPropertyInt(identifier, displayName)
    , emptySelection_(emptySelection)
    , defaultIndex_(defaultIndex) {

    setSerializationMode(PropertySerializationMode::All);
    setPort(port);
}

DataFrameColumnProperty::DataFrameColumnProperty(const DataFrameColumnProperty& rhs)
    : OptionPropertyInt(rhs)
    , inport_(rhs.inport_)
    , emptySelection_(rhs.emptySelection_)
    , defaultIndex_(rhs.defaultIndex_) {

    if (inport_) {
        onChangeCallback_ = inport_->onChangeScoped([&]() {
            if (inport_->hasData()) {
                setOptions(inport_->getData());
            }
        });
    }
}

DataFrameColumnProperty* DataFrameColumnProperty::clone() const {
    return new DataFrameColumnProperty(*this);
}

void DataFrameColumnProperty::setPort(DataFrameInport& inport) {
    inport_ = &inport;

    onChangeCallback_ = inport_->onChangeScoped([&]() {
        if (inport_->hasData()) {
            setOptions(inport_->getData());
        }
    });
    setOptions(inport_->getData());
}

void DataFrameColumnProperty::setOptions(std::shared_ptr<const DataFrame> dataframe) {
    if (!dataframe || dataframe->getNumberOfColumns() <= 1) return;

    std::vector<OptionPropertyIntOption> options;
    if (emptySelection_ == EmptySelection::Yes) {
        options.emplace_back("none", "None", -1);
    }

    auto createIdentifier = [](std::string str) {
        util::erase_remove_if(str, [](char cc) {
            return !(cc >= -1) || !(std::isalnum(cc) || cc == '_' || cc == '-');
        });
        return str;
    };

    for (const auto&& [idx, col] : util::enumerate<int>(*dataframe)) {
        const auto header = col->getHeader();
        options.emplace_back(createIdentifier(header), header, idx);
    }

    const bool wasEmpty =
        options_.empty() || ((options_.size() == 1) && (options_[0].id_ == "none"));
    replaceOptions(std::move(options));
    if (wasEmpty) {
        setSelectedIndex(defaultIndex_);
    }
    setCurrentStateAsDefault();
}

void DataFrameColumnProperty::setDefaultIndex(int index) { defaultIndex_ = index; }

const std::string& DataFrameColumnProperty::getColumnHeader() const {
    if (getSelectedIndex() < 0) {
        static std::string empty = {};
        return empty;
    }
    return getSelectedDisplayName();
}

void DataFrameColumnProperty::set(const Property* srcProperty) {
    if (auto src = dynamic_cast<const DataFrameColumnProperty*>(srcProperty)) {
        if ((src->options_.size() == 0) || (options_.size() == 0)) {
            return;
        }

        if ((src->getSelectedValue() < 0) && (emptySelection_ == EmptySelection::No)) {
            // Do Nothing
        } else {
            setSelectedIdentifier(src->getSelectedIdentifier());
        }
    } else {
        OptionPropertyInt::set(srcProperty);
    }
}

}  // namespace inviwo
