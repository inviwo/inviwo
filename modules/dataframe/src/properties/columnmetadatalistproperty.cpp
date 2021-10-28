/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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
#include <inviwo/core/metadata/metadataowner.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/base/algorithm/dataminmax.h>

namespace inviwo {

const std::string ColumnMetaDataListProperty::classIdentifier =
    "org.inviwo.ColumnMetaDataListProperty";
std::string ColumnMetaDataListProperty::getClassIdentifier() const { return classIdentifier; }

ColumnMetaDataListProperty::ColumnMetaDataListProperty(std::string_view identifier,
                                                       std::string_view displayName,
                                                       InvalidationLevel invalidationLevel,
                                                       PropertySemantics semantics)
    : ListProperty(std::string(identifier), std::string(displayName),
                   std::make_unique<ColumnMetaDataProperty>("column1", "Column 1"), 0,
                   ListPropertyUIFlag::Static, invalidationLevel, semantics) {}

ColumnMetaDataListProperty::ColumnMetaDataListProperty(std::string_view identifier,
                                                       std::string_view displayName,
                                                       DataFrameInport& port,
                                                       InvalidationLevel invalidationLevel,
                                                       PropertySemantics semantics)
    : ListProperty(std::string(identifier), std::string(displayName),
                   std::make_unique<ColumnMetaDataProperty>("column1", "Column 1"), 0,
                   ListPropertyUIFlag::Static, invalidationLevel, semantics) {

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
            updateColumnProperties(*inport_->getData());
        }
    });
    if (inport_->hasData()) {
        updateColumnProperties(*inport_->getData());
    }
}

MetaDataOwner ColumnMetaDataListProperty::getColumnMetaData(size_t columnIndex) const {
    MetaDataOwner metadata;

    auto p = util::find_if_or_null(getProperties(), [columnIndex](const Property* p) {
        return static_cast<const ColumnMetaDataProperty*>(p)->getColumnIndex() == columnIndex;
    });
    if (p) {
        auto colprop = static_cast<const ColumnMetaDataProperty*>(p);
        metadata.setMetaData<DoubleVec2MetaData>("DataRange", colprop->getRange());
    }

    return metadata;
}

ColumnMetaDataListProperty& ColumnMetaDataListProperty::resetToDefaultState() {
    NetworkLock lock(this);
    clear();
    if (inport_ && inport_->hasData()) {
        updateColumnProperties(*inport_->getData());
    }
    return *this;
}

void ColumnMetaDataListProperty::updateColumnProperties(const DataFrame& dataframe) {
    if (dataframe.getNumberOfColumns() <= 1) return;

    auto columnRange = [](auto& col) {
        return col->getBuffer()
            ->getRepresentation<BufferRAM>()
            ->dispatch<dvec2, dispatching::filter::Scalars>([&](auto br) {
                auto [min, max] = util::bufferMinMax(br, IgnoreSpecialValues::No);
                return dvec2{min.x, max.x};
            });
    };

    auto toRemove = util::transform(
        getProperties(), [](Property* p) { return static_cast<ColumnMetaDataProperty*>(p); });

    NetworkLock lock(this);
    for (const auto&& [idx, col] : util::enumerate<int>(dataframe)) {
        auto it = util::find_if(toRemove, [header = col->getHeader()](auto p) {
            return p->getDisplayName() == header;
        });
        if (it != toRemove.end()) {
            (*it)->setColumnIndex(idx);
            (*it)->setRange(columnRange(col));
            util::erase_remove(toRemove, *it);
        } else {
            auto p = static_cast<ColumnMetaDataProperty*>(constructProperty(0));
            p->setDisplayName(col->getHeader());
            p->setColumnIndex(idx);
            p->setRange(columnRange(col));
        }
    }
    for (auto* p : toRemove) {
        removeProperty(p);
    }
}

}  // namespace inviwo
