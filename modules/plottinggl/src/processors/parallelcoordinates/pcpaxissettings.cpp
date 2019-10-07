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

#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>

#include <inviwo/dataframe/datastructures/column.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/plotting/utils/statsutils.h>
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

PCPAxisSettings::PCPAxisSettings(std::string identifier, std::string displayName, size_t columnId)
    : BoolCompositeProperty(identifier, displayName, true)
    , usePercentiles("usePercentiles", "Use Percentiles", false)
    , invertRange("invertRange", "Invert Range")
    , range("range", "Axis Range")
    , columnId_{columnId} {

    addProperty(range);
    addProperty(invertRange);
    addProperty(usePercentiles);

    setCollapsed(true);
    setSerializationMode(PropertySerializationMode::All);
    range.setSerializationMode(PropertySerializationMode::All);
    invertRange.setSerializationMode(PropertySerializationMode::All);
    usePercentiles.setSerializationMode(PropertySerializationMode::All);

    range.onChange([this]() {
        updateBrushing();
        if (pcp_) pcp_->updateBrushing(*this);
    });
}

PCPAxisSettings::PCPAxisSettings(const PCPAxisSettings& rhs)
    : BoolCompositeProperty(rhs)
    , usePercentiles{rhs.usePercentiles}
    , invertRange(rhs.invertRange)
    , range{rhs.range}
    , columnId_{rhs.columnId_} {

    addProperty(range);
    addProperty(invertRange);
    addProperty(usePercentiles);

    range.onChange([this]() {
        updateBrushing();
        if (pcp_) pcp_->updateBrushing(*this);
    });
}

PCPAxisSettings* PCPAxisSettings::clone() const { return new PCPAxisSettings(*this); }

void PCPAxisSettings::updateFromColumn(std::shared_ptr<const Column> col) {
    col_ = col;
    catCol_ = dynamic_cast<const CategoricalColumn*>(col.get());

    col->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto ram) -> void {
            using T = typename util::PrecisionValueType<decltype(ram)>;
            auto& dataVector = ram->getDataContainer();

            auto minMax = util::bufferMinMax(ram, IgnoreSpecialValues::Yes);
            double minV = minMax.first.x;
            double maxV = minMax.second.x;

            if (std::abs(maxV - minV) == 0.0) {
                minV -= 1.0;
                maxV += 1.0;
            }

            dvec2 prevVal = range.get();
            dvec2 prevRange = range.getRange();
            double l = prevRange.y - prevRange.x;

            double prevMinRatio = (prevVal.x - prevRange.x) / (l);
            double prevMaxRatio = (prevVal.y - prevRange.x) / (l);

            Property::OnChangeBlocker block{range};
            range.setRange(glm::tvec2<T>(minV, maxV));
            if (l > 0 && maxV != minV) {
                range.set(
                    {minV + prevMinRatio * (maxV - minV), minV + prevMaxRatio * (maxV - minV)});
            }
            auto pecentiles = statsutil::percentiles(dataVector, {0., 0.25, 0.75, 1.});
            p0_ = static_cast<double>(pecentiles[0]);
            p25_ = static_cast<double>(pecentiles[1]);
            p75_ = static_cast<double>(pecentiles[2]);
            p100_ = static_cast<double>(pecentiles[3]);
            at = [vec = &dataVector](size_t idx) { return static_cast<double>(vec->at(idx)); };
        });

    range.propertyModified();
}

double PCPAxisSettings::getNormalized(double v) const {
    if (range.getRangeMax() == range.getRangeMin()) {
        return 0.5;
    }
    const auto rangeTmp = range.getRange();
    if (v <= rangeTmp.x) {
        return 0;
    }
    if (v >= rangeTmp.y) {
        return 1;
    }
    if (!usePercentiles.get()) {
        return (v - rangeTmp.x) / (rangeTmp.y - rangeTmp.x);
    } else {
        double minV, maxV;
        double o, r;
        if (v < p25_) {
            minV = p0_;
            maxV = p25_;
            o = 0;
            r = 0.25f;
        } else if (v < p75_) {
            minV = p25_;
            maxV = p75_;
            o = 0.25;
            r = 0.5;
        } else {
            minV = p75_;
            maxV = p100_;
            o = 0.75;
            r = 0.25;
        }

        double t = (v - minV) / (maxV - minV);
        return o + t * r;
    }
}

double PCPAxisSettings::getNormalizedAt(size_t idx) const { return getNormalized(at(idx)); }

double PCPAxisSettings::getValue(double v) const {
    if (invertRange) {
        v = 1.0 - v;
    }

    const auto rangeTmp = range.getRange();
    if (v <= 0) {
        return rangeTmp.x;
    }
    if (v >= 1) {
        return rangeTmp.y;
    }
    if (!usePercentiles.get()) {
        return rangeTmp.x + v * (rangeTmp.y - rangeTmp.x);
    } else {
        if (v < 0.25) {
            v /= 0.25;
            return p0_ + v * (p25_ - p0_);
        } else if (v < 0.75) {
            v -= 0.25;
            v /= 0.5;
            return p25_ + v * (p75_ - p25_);
        } else {
            v -= 0.75;
            v /= 0.25;
            return p75_ + v * (p100_ - p75_);
        }
    }
}

void PCPAxisSettings::moveHandle(bool upper, double mouseY) {
    auto rangeTmp = range.get();
    double value = getValue(mouseY);

    if (upper) {
        if (value < rangeTmp.x) {
            value = rangeTmp.x;
        }
        rangeTmp.y = value;
    } else {
        if (value > rangeTmp.y) {
            value = rangeTmp.y;
        }
        rangeTmp.x = value;
    }
    range.set(rangeTmp);
}

void PCPAxisSettings::setParallelCoordinates(ParallelCoordinates* pcp) {
    pcp_ = pcp;
    labelSettings_.setSettings(this);
    captionSettings_.setSettings(this);
    major_.setSettings(this);
    minor_.setSettings(this);

    auto updateLabels = [this]() {
        const auto tickmarks = plot::getMajorTickPositions(major_, range.getRange());
        labels_.clear();
        const auto& format = pcp_->labelFormat_.get();
        std::transform(tickmarks.begin(), tickmarks.end(), std::back_inserter(labels_),
                       [&](auto tick) { return fmt::sprintf(format, tick); });
    };
    labelUpdateCallback_ = pcp_->labelFormat_.onChangeScoped(updateLabels);
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

dvec2 PCPAxisSettings::getRange() const {
    if (catCol_) {
        return {0.0, static_cast<double>(catCol_->getCategories().size()) - 1.0};
    } else {
        return dvec2{range.getRangeMin(), range.getRangeMax()};
    }
}

bool PCPAxisSettings::getUseDataRange() const { return false; }

bool PCPAxisSettings::getAxisVisible() const { return BoolCompositeProperty::isChecked(); }

bool PCPAxisSettings::getFlipped() const { return invertRange.get(); }

vec4 PCPAxisSettings::getColor() const {
    const auto hover = pcp_->getHoveredAxis() == static_cast<int>(columnId_);
    const auto selected = pcp_->brushingAndLinking_.isColumnSelected(columnId_);

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
    } else if (pcp_->brushingAndLinking_.isColumnSelected(columnId_)) {
        return 1.5f * pcp_->axisSize_;
    } else {
        return 1.0f * pcp_->axisSize_;
    }
}

AxisSettings::Orientation PCPAxisSettings::getOrientation() const { return Orientation::Vertical; }
AxisSettings::Placement PCPAxisSettings::getPlacement() const { return Placement::Inside; }
const std::string& PCPAxisSettings::getCaption() const { return col_->getHeader(); }
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
