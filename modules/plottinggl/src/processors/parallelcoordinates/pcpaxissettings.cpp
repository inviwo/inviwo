/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>
#include <modules/plotting/utils/axisutils.h>

#include <fmt/format.h>
#include <fmt/printf.h>

namespace inviwo {
namespace plot {

namespace detail {
enum class FilterResult { Upper, Lower, None };
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

}  // namespace detail

const std::string PCPAxisSettings::classIdentifier =
    "org.inviwo.parallelcoordinates.axissettingsproperty";
std::string PCPAxisSettings::getClassIdentifier() const { return classIdentifier; }

PCPAxisSettings::PCPAxisSettings(std::string_view identifier, std::string_view displayName,
                                 size_t columnId)
    : BoolCompositeProperty(identifier, displayName, true)
    , range("range", "Axis Range")
    , invertRange("invertRange", "Invert Range")
    , captionSettings_(this)
    , labelSettings_(this)
    , major_(this)
    , minor_(this)
    , columnId_{static_cast<uint32_t>(columnId)} {

    addProperties(range, invertRange);

    setCollapsed(true);
    setSerializationMode(PropertySerializationMode::All);
    range.setSerializationMode(PropertySerializationMode::All);
    invertRange.setSerializationMode(PropertySerializationMode::All);

    range.onRangeChange([this]() {
        updateLabels();
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
    , captionSettings_(this)
    , labelSettings_(this)
    , major_(this)
    , minor_(this)
    , columnId_{rhs.columnId_} {

    addProperties(range, invertRange);

    range.onRangeChange([this]() {
        updateLabels();
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
            at = [vec = &dataVector](size_t idx) { return static_cast<double>(vec->at(idx)); };
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

void PCPAxisSettings::setParallelCoordinates(ParallelCoordinates* pcp) {
    pcp_ = pcp;

    labelUpdateCallback_ = pcp_->labelFormat_.onChangeScoped([this]() { updateLabels(); });
    updateLabels();
}

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
        const auto filtered = detail::filterValue(at(i), rangeTmp);
        if (filtered == detail::FilterResult::None) {
            brushed_[i] = false;
            continue;
        }
        brushed_[i] = true;
        if (filtered == detail::FilterResult::Upper) upperBrushed_ = true;
        if (filtered == detail::FilterResult::Lower) lowerBrushed_ = true;
    }
}

void PCPAxisSettings::updateLabels() {
    if (!pcp_) return;

    const auto tickmarks = plot::getMajorTickPositions(major_, range.getRange());
    const auto& format = pcp_->labelFormat_.get();

    labels_.clear();
    std::transform(tickmarks.begin(), tickmarks.end(), std::back_inserter(labels_),
                   [&](auto tick) { return fmt::sprintf(format, tick); });
}

dvec2 PCPAxisSettings::getRange() const {
    if (catCol_) {
        return {0.0, static_cast<double>(catCol_->getCategories().size()) - 1.0};
    } else {
        return {range.getRangeMin(), range.getRangeMax()};
    }
}

bool PCPAxisSettings::getUseDataRange() const { return false; }

bool PCPAxisSettings::getAxisVisible() const { return BoolCompositeProperty::isChecked(); }

bool PCPAxisSettings::getFlipped() const { return invertRange.get(); }

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

AxisSettings::Orientation PCPAxisSettings::getOrientation() const { return Orientation::Vertical; }
AxisSettings::Placement PCPAxisSettings::getPlacement() const { return Placement::Inside; }
const std::string& PCPAxisSettings::getCaption() const { return caption_; }
const PlotTextSettings& PCPAxisSettings::getCaptionSettings() const { return captionSettings_; }
const std::vector<std::string>& PCPAxisSettings::getLabels() const {
    return catCol_ ? catCol_->getCategories() : labels_;
}

const PlotTextSettings& PCPAxisSettings::getLabelSettings() const { return labelSettings_; }
const MajorTickSettings& PCPAxisSettings::getMajorTicks() const { return major_; }
const MinorTickSettings& PCPAxisSettings::getMinorTicks() const { return minor_; }

bool PCPCaptionSettings::isEnabled() const {
    return settings_->pcp_->captionPosition_.get() != ParallelCoordinates::LabelPosition::None;
}
vec4 PCPCaptionSettings::getColor() const { return settings_->pcp_->captionColor_; }
float PCPCaptionSettings::getPosition() const {
    return (settings_->pcp_->captionPosition_.get() == ParallelCoordinates::LabelPosition::Above) !=
                   settings_->getFlipped()
               ? 1.0f
               : 0.0f;
}

vec2 PCPCaptionSettings::getOffset() const {
    return {0.0f, settings_->pcp_->captionOffset_ * (settings_->getFlipped() ? -1.0f : 1.0f)};
}
float PCPCaptionSettings::getRotation() const { return 270.f; }
const FontSettings& PCPCaptionSettings::getFont() const {
    return settings_->pcp_->captionSettings_;
}

bool PCPLabelSettings::isEnabled() const { return settings_->pcp_->showLabels_; }
vec4 PCPLabelSettings::getColor() const { return settings_->pcp_->labelColor_; }
float PCPLabelSettings::getPosition() const { return 0.0f; }
vec2 PCPLabelSettings::getOffset() const { return {settings_->pcp_->labelOffset_, 0.0f}; }
float PCPLabelSettings::getRotation() const { return 0.0f; }
const FontSettings& PCPLabelSettings::getFont() const { return settings_->pcp_->labelSettings_; }

TickStyle PCPMajorTickSettings::getStyle() const { return TickStyle::Both; }
vec4 PCPMajorTickSettings::getColor() const { return settings_->getColor(); }
float PCPMajorTickSettings::getTickLength() const { return settings_->pcp_->axisSize_ * 2.0f; }
float PCPMajorTickSettings::getTickWidth() const { return settings_->getWidth(); }
double PCPMajorTickSettings::getTickDelta() const { return settings_->catCol_ ? 1.0 : 0.0; }
bool PCPMajorTickSettings::getRangeBasedTicks() const { return false; }

TickStyle PCPMinorTickSettings::getStyle() const { return TickStyle::None; }
bool PCPMinorTickSettings::getFillAxis() const { return false; }
vec4 PCPMinorTickSettings::getColor() const { return settings_->getColor(); }
float PCPMinorTickSettings::getTickLength() const { return 0.0f; }
float PCPMinorTickSettings::getTickWidth() const { return 0.0f; }
int PCPMinorTickSettings::getTickFrequency() const { return 0; }

}  // namespace plot

}  // namespace inviwo
