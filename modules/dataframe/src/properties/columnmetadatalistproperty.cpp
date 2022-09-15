/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <inviwo/dataframe/properties/columnmetadatalistproperty.h>

#include <inviwo/core/network/networklock.h>                     // for NetworkLock
#include <inviwo/core/properties/invalidationlevel.h>            // for InvalidationLevel
#include <inviwo/core/properties/listproperty.h>                 // for ListProperty, ListProper...
#include <inviwo/core/properties/property.h>                     // for OverwriteState, Overwrit...
#include <inviwo/core/properties/propertyowner.h>                // for PropertyOwner
#include <inviwo/core/properties/propertysemantics.h>            // for PropertySemantics
#include <inviwo/core/properties/valuewrapper.h>                 // for PropertySerializationMode
#include <inviwo/core/util/exception.h>                          // for Exception
#include <inviwo/core/util/sourcecontext.h>                      // for IVW_CONTEXT
#include <inviwo/core/util/zip.h>                                // for enumerate, zipIterator
#include <inviwo/dataframe/datastructures/dataframe.h>           // for DataFrameInport, DataFrame
#include <inviwo/dataframe/properties/columnmetadataproperty.h>  // for ColumnMetaDataProperty

namespace inviwo {

const std::string ColumnMetaDataListProperty::classIdentifier =
    "org.inviwo.ColumnMetaDataListProperty";
std::string ColumnMetaDataListProperty::getClassIdentifier() const { return classIdentifier; }

ColumnMetaDataListProperty::ColumnMetaDataListProperty(std::string_view identifier,
                                                       std::string_view displayName,
                                                       InvalidationLevel invalidationLevel,
                                                       PropertySemantics semantics)
    : ListProperty(identifier, displayName,
                   std::make_unique<ColumnMetaDataProperty>("column1", "Column 1"), 0,
                   ListPropertyUIFlag::Static, invalidationLevel, semantics) {
    setSerializationMode(PropertySerializationMode::All);
}

ColumnMetaDataListProperty::ColumnMetaDataListProperty(std::string_view identifier,
                                                       std::string_view displayName,
                                                       DataFrameInport& port,
                                                       InvalidationLevel invalidationLevel,
                                                       PropertySemantics semantics)
    : ColumnMetaDataListProperty(identifier, displayName, invalidationLevel, semantics) {
    setPort(port);
}

ColumnMetaDataListProperty::ColumnMetaDataListProperty(const ColumnMetaDataListProperty& rhs)
    : ListProperty(rhs) {}

ColumnMetaDataListProperty* ColumnMetaDataListProperty::clone() const {
    return new ColumnMetaDataListProperty(*this);
}

void ColumnMetaDataListProperty::setPort(DataFrameInport& inport) {
    inport_ = &inport;

    onChangeCallback_ = inport_->onChangeScoped([&]() {
        if (inport_->hasData()) {
            updateForNewDataFrame(*inport_->getData(), util::OverwriteState::Yes);
        }
    });
    if (inport_->hasData()) {
        updateForNewDataFrame(*inport_->getData(), util::OverwriteState::Yes);
    }
}

ColumnMetaDataProperty& ColumnMetaDataListProperty::meta(size_t i) {
    if (i >= size()) {
        throw Exception(IVW_CONTEXT, "ColumnIndex {} greater then size {}", i, size());
    }
    return *static_cast<ColumnMetaDataProperty*>((*this)[i]);
}
const ColumnMetaDataProperty& ColumnMetaDataListProperty::meta(size_t i) const {
    if (i >= size()) {
        throw Exception(IVW_CONTEXT, "ColumnIndex {} greater then size {}", i, size());
    }
    return *static_cast<const ColumnMetaDataProperty*>((*this)[i]);
}

void ColumnMetaDataListProperty::updateForNewDataFrame(const DataFrame& dataFrame,
                                                       util::OverwriteState overwrite) {
    if (dataFrame.getNumberOfColumns() <= 1) return;

    NetworkLock lock(this);
    for (const auto&& [idx, col] : util::enumerate(dataFrame)) {
        while (idx >= size()) constructProperty(0);
        meta(idx).updateForNewColumn(*col, overwrite);
    }

    while (size() > dataFrame.getNumberOfColumns()) {
        removeProperty(size() - 1);
    }
}

void ColumnMetaDataListProperty::updateDataFrame(DataFrame& dataFrame) const {
    if (dataFrame.getNumberOfColumns() == 0) return;

    // Iterate in reverse to be able to drop columns without affecting the order
    for (ptrdiff_t i = dataFrame.getNumberOfColumns() - 1; i >= 0; --i) {
        auto idx = static_cast<size_t>(i);
        if (idx < size()) {
            if (meta(idx).getDrop()) {
                dataFrame.dropColumn(idx);
            } else {
                meta(idx).updateColumn(*dataFrame.getColumn(idx));
            }
        }
    }
}

}  // namespace inviwo
