/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2026 Inviwo Foundation
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

#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>

#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/formatdispatching.h>
#include <inviwo/core/util/glm.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <modules/brushingandlinking/datastructures/brushingaction.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>
#include <modules/fontrendering/properties/fontproperty.h>
#include <modules/plotting/utils/axisutils.h>
#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <type_traits>
#include <unordered_set>

#include <fmt/core.h>
#include <fmt/printf.h>
#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace inviwo {
class FontSettings;

namespace plot {
class MinorTickSettings;
class PlotTextSettings;

namespace {
enum class FilterResult : unsigned char { Upper, Lower, None };
/**
 * Helper for brushing data
 * @param value to filter
 * @param range to use for filtering
 * @return true if value is outside range and not missing data.
 */
FilterResult filterValue(const double& value, const dvec2& range) {
    // Do not filter missing data (NaN)
    if (util::isnan(value)) return FilterResult::None;
    if (value < range.x) {
        return FilterResult::Lower;
    }
    if (value > range.y) {
        return FilterResult::Upper;
    }
    return FilterResult::None;
}

}  // namespace

std::string_view PCPAxisSettings::getClassIdentifier() const { return classIdentifier; }

PCPAxisSettings::PCPAxisSettings(std::string_view identifier, std::string_view displayName,
                                 size_t columnId)
    : BoolCompositeProperty(identifier, displayName, true)
    , range("range", "Axis Range")
    , invertRange("invertRange", "Invert Range")
    , columnId_{static_cast<uint32_t>(columnId)} {

    addProperties(range, invertRange);

    setCollapsed(true);
    setSerializationMode(PropertySerializationMode::All);
    range.setSerializationMode(PropertySerializationMode::All);
    invertRange.setSerializationMode(PropertySerializationMode::All);

    range.onRangeChange([this]() {
        if (pcp_) pcp_->updateAxisRange(*this);
    });

    range.onChange([this]() {
        updateBrushing();
        if (pcp_) pcp_->updateBrushing(*this);
    });
}

PCPAxisSettings::PCPAxisSettings(const PCPAxisSettings& rhs)
    : BoolCompositeProperty(rhs)
    , range{rhs.range}
    , invertRange(rhs.invertRange)
    , columnId_{rhs.columnId_} {

    addProperties(range, invertRange);

    range.onRangeChange([this]() {
        if (pcp_) pcp_->updateAxisRange(*this);
    });

    range.onChange([this]() {
        updateBrushing();
        if (pcp_) pcp_->updateBrushing(*this);
    });
}

PCPAxisSettings* PCPAxisSettings::clone() const { return new PCPAxisSettings(*this); }

void PCPAxisSettings::update(std::shared_ptr<const DataFrame> frame) {
    col_ = frame->getColumn(columnId_);
    catCol_ = dynamic_cast<const CategoricalColumn*>(col_.get());

    caption_ = col_->getColumnType() == ColumnType::Ordinal
                   ? fmt::format("{} {: [sys}", col_->getHeader(), col_->getUnit())
                   : col_->getHeader();

    col_->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto ram) -> void {
            auto& dataVector = ram->getDataContainer();
            at = [vec = &dataVector](size_t idx) { return static_cast<double>(vec->at(idx)); };

            dvec2 minmax = col_->getRange();
            if (std::abs(minmax.y - minmax.x) < glm::epsilon<double>()) {
                minmax += dvec2(-1.0, 1.0);
            }

            const dvec2 prevVal = range.get();
            const dvec2 prevRange = range.getRange();
            const double l = prevRange.y - prevRange.x;
            const double prevMinRatio = (prevVal.x - prevRange.x) / l;
            const double prevMaxRatio = (prevVal.y - prevRange.x) / l;

            Property::OnChangeBlocker block{range};
            range.setRange(minmax);

            const double minV = minmax.x;
            const double maxV = minmax.y;
            if (l > 0 && maxV != minV) {
                range.set(
                    {minV + prevMinRatio * (maxV - minV), minV + prevMaxRatio * (maxV - minV)});
            }
        });

    range.propertyModified();
}

double PCPAxisSettings::getNormalized(double v) const {
    if (range.getRangeMax() == range.getRangeMin()) {
        return 0.5;
    } else if (v <= range.getRangeMin()) {
        return 0.0;
    } else if (v >= range.getRangeMax()) {
        return 1.0;
    } else {
        return (v - range.getRangeMin()) / (range.getRangeMax() - range.getRangeMin());
    }
}

double PCPAxisSettings::getNormalizedAt(size_t idx) const { return getNormalized(at(idx)); }

double PCPAxisSettings::getValue(double v) const {
    if (invertRange) {
        v = 1.0 - v;
    }

    if (v <= 0) {
        return range.getRangeMin();
    } else if (v >= 1) {
        return range.getRangeMax();
    } else {
        return range.getRangeMin() + v * (range.getRangeMax() - range.getRangeMin());
    }
}

void PCPAxisSettings::moveHandle(bool upper, double mouseY) {
    const double value = std::clamp(getValue(mouseY), range.getRangeMin(), range.getRangeMax());
    const auto newRange =
        upper ? dvec2{range.getRangeMin(), value} : dvec2{value, range.getRangeMax()};
    range.set(newRange);
}

void PCPAxisSettings::setParallelCoordinates(ParallelCoordinates* pcp) { pcp_ = pcp; }

void PCPAxisSettings::updateBrushing() {
    if (!col_) return;

    // Increase range to avoid conversion issues
    const dvec2 off{-std::numeric_limits<float>::epsilon(), std::numeric_limits<float>::epsilon()};
    const auto rangeTmp = range.get() + off;
    const auto nRows = col_->getSize();

    upperBrushed_ = false;
    lowerBrushed_ = false;

    brushed_.resize(nRows, false);

    for (size_t i = 0; i < nRows; i++) {
        const auto filtered = filterValue(at(i), rangeTmp);
        if (filtered == FilterResult::None) {
            brushed_[i] = false;
            continue;
        }
        brushed_[i] = true;
        if (filtered == FilterResult::Upper) upperBrushed_ = true;
        if (filtered == FilterResult::Lower) lowerBrushed_ = true;
    }
}

dvec2 PCPAxisSettings::getRange() const {
    if (catCol_) {
        return {0.0, static_cast<double>(catCol_->getCategories().size()) - 1.0};
    } else {
        return {range.getRangeMin(), range.getRangeMax()};
    }
}
vec4 PCPAxisSettings::getColor() const {
    const auto hover = pcp_->getHoveredAxis() == static_cast<int>(columnId_);
    const auto selected = pcp_->brushingAndLinking_.isSelected(columnId_, BrushingTarget::Column);

    if (hover && selected) {
        return glm::mix(pcp_->axisHoverColor_.get(), pcp_->axisSelectedColor_.get(), 0.5f);
    } else if (hover) {
        return pcp_->axisHoverColor_;
    } else if (selected) {
        return pcp_->axisSelectedColor_;
    } else {
        return pcp_->axisColor_;
    }
}

float PCPAxisSettings::getWidth() const {
    if (pcp_->getHoveredAxis() == static_cast<int>(columnId_)) {
        return 1.5f * pcp_->axisSize_;
    } else if (pcp_->brushingAndLinking_.isSelected(columnId_, BrushingTarget::Column)) {
        return 1.5f * pcp_->axisSize_;
    } else {
        return 1.0f * pcp_->axisSize_;
    }
}

bool PCPAxisSettings::dataModified() const {
    bool modified = false;

    if (!catCol_) modified |= range.isModified();
    modified |= invertRange.isModified();
    modified |= getBoolProperty()->isModified();
    modified |= pcp_->axisHoverColor_.isModified();
    modified |= pcp_->axisSelectedColor_.isModified();
    modified |= pcp_->axisColor_.isModified();
    modified |= pcp_->axisSize_.isModified();
    modified |= pcp_->captionPosition_.isModified();
    modified |= pcp_->captionColor_.isModified();
    modified |= pcp_->captionOffset_.isModified();
    modified |= pcp_->captionSettings_.isModified();
    modified |= pcp_->showLabels_.isModified();
    modified |= pcp_->labelColor_.isModified();
    modified |= pcp_->labelOffset_.isModified();
    modified |= pcp_->labelSettings_.isModified();

    return modified;
}

void PCPAxisSettings::update(AxisData& data) const {

    data.range = getRange();
    data.visible = isChecked();
    data.mirrored = invertRange.get();
    data.color = getColor();
    data.width = getWidth();
    data.caption = caption_;

    data.captionSettings.enabled =
        pcp_->captionPosition_.get() != ParallelCoordinates::LabelPosition::None;
    data.captionSettings.color = pcp_->captionColor_.get();
    data.captionSettings.position = (pcp_->captionPosition_.get() ==
                                     ParallelCoordinates::LabelPosition::Above) != invertRange.get()
                                        ? 1.0f
                                        : 0.0f;
    data.captionSettings.offset =
        vec2{0.0f, pcp_->captionOffset_ * (invertRange.get() ? -1.0f : 1.0f)};
    data.captionSettings.rotation = -90.0f;

    pcp_->captionSettings_.update(data.captionSettings.font);

    if (catCol_) {
        linearRange({.start = 0.0,
                     .stop = static_cast<double>(catCol_->getCategories().size()) - 1.0,
                     .step = 1.0},
                    data.majorPositions);
        data.minorPositions.clear();
        updateLabels(data.labels, catCol_->getCategories());
    } else {
        updateLabelPositions(data.majorPositions, data.minorPositions, LabelingAlgorithm::Limits,
                             data.range, 10, 0, true);
        updateLabels(data.labels, data.majorPositions,
                     pcp_ ? pcp_->labelFormat_ : ParallelCoordinates::defaultLabelFormat);
    }

    data.labelSettings.enabled = pcp_->showLabels_;
    data.labelSettings.color = pcp_->labelColor_.get();
    data.labelSettings.position = 0.0f;
    data.labelSettings.offset = vec2(pcp_->labelOffset_, 0.0f);
    data.labelSettings.rotation = -90.0f;
    pcp_->labelSettings_.update(data.labelSettings.font);

    data.major.style = TickData::Style::Both;
    data.major.color = getColor();
    data.major.length = pcp_->axisSize_ * 2.0f;
    data.major.width = getWidth();

    data.minor.style = TickData::Style::None;
    data.minor.color = vec4(0.0f);
    data.minor.length = 0.0f;
    data.minor.width = 0.0f;
}

}  // namespace plot

}  // namespace inviwo
