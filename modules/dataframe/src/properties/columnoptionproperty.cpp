/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/dataframe/properties/columnoptionproperty.h>

#include <inviwo/core/properties/optionproperty.h>      // for OptionPropertyOption, OptionPrope...
#include <inviwo/core/properties/property.h>            // for Property
#include <inviwo/core/properties/valuewrapper.h>        // for PropertySerializationMode, Proper...
#include <inviwo/core/util/utilities.h>                 // for stripIdentifier
#include <inviwo/core/util/zip.h>                       // for enumerate, zipIterator, zipper
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrameInport, DataFrame

#include <type_traits>  // for remove_extent_t
#include <utility>      // for move
#include <vector>       // for vector

namespace inviwo {

const std::string ColumnOptionProperty::classIdentifier = "org.inviwo.DataFrameColumnProperty";
std::string_view ColumnOptionProperty::getClassIdentifier() const { return classIdentifier; }

ColumnOptionProperty::ColumnOptionProperty(std::string_view identifier,
                                           std::string_view displayName,
                                           AddNoneOption emptySelection, int defaultIndex)
    : OptionPropertyInt(std::string{identifier}, std::string{displayName})
    , noneOption_(emptySelection)
    , defaultColumnIndex_(defaultIndex) {

    setSerializationMode(PropertySerializationMode::All);
    if (noneOption_ == AddNoneOption::Yes) {
        addOption("none", "None", -1);
    }
}

ColumnOptionProperty::ColumnOptionProperty(std::string_view identifier,
                                           std::string_view displayName, DataFrameInport& port,
                                           AddNoneOption emptySelection, int defaultIndex)
    : OptionPropertyInt(std::string{identifier}, std::string{displayName})
    , noneOption_(emptySelection)
    , defaultColumnIndex_(defaultIndex) {

    setSerializationMode(PropertySerializationMode::All);
    setPort(port);
}

ColumnOptionProperty::ColumnOptionProperty(const ColumnOptionProperty& rhs)
    : OptionPropertyInt(rhs)
    , noneOption_(rhs.noneOption_)
    , defaultColumnIndex_(rhs.defaultColumnIndex_) {

    if (rhs.inport_) {
        setPort(*rhs.inport_);
    }
}

ColumnOptionProperty* ColumnOptionProperty::clone() const {
    return new ColumnOptionProperty(*this);
}

void ColumnOptionProperty::setPort(DataFrameInport& inport) {
    inport_ = &inport;

    onChangeCallback_ = inport_->onChangeScoped([&]() {
        if (inport_->hasData()) {
            setOptions(*inport_->getData());
        }
    });
    if (inport_->hasData()) {
        setOptions(*inport_->getData());
    }
}

void ColumnOptionProperty::setOptions(const DataFrame& dataframe) {
    if (dataframe.getNumberOfColumns() <= 1) return;

    std::vector<OptionPropertyIntOption> options;
    if (noneOption_ == AddNoneOption::Yes) {
        options.emplace_back("none", "None", -1);
    }

    for (auto&& [idx, col] : util::enumerate<int>(dataframe)) {
        const auto header = col->getHeader();
        options.emplace_back(util::stripIdentifier(header), header, idx);
    }

    const bool wasEmpty =
        options_.empty() || ((options_.size() == 1) && (options_[0].id_ == "none"));
    replaceOptions(std::move(options));
    if (wasEmpty) {
        setSelectedValue(defaultColumnIndex_);
    }
    setCurrentStateAsDefault();
}

void ColumnOptionProperty::setDefaultSelectedIndex(int index) {
    if (index < 0) {
        if (noneOption_ == AddNoneOption::Yes) {
            index = -1;
        } else {
            index = 0;
        }
    }
    defaultColumnIndex_ = index;
}

const std::string& ColumnOptionProperty::getSelectedColumnHeader() const {
    return getSelectedDisplayName();
}

bool ColumnOptionProperty::isNoneSelected() const { return size() && (getSelectedValue() == -1); }

void ColumnOptionProperty::set(const Property* srcProperty) {
    if (auto src = dynamic_cast<const ColumnOptionProperty*>(srcProperty)) {
        if ((src->options_.size() == 0) || (options_.size() == 0)) {
            return;
        }

        if (src->isNoneSelected() && (noneOption_ == AddNoneOption::No)) {
            // Do Nothing
        } else {
            setSelectedIdentifier(src->getSelectedIdentifier());
        }
    } else {
        OptionPropertyInt::set(srcProperty);
    }
}

}  // namespace inviwo
