/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinatesaxissettingsproperty.h>
#include <modules/plotting/datastructures/column.h>
#include <modules/base/algorithm/dataminmax.h>
#include <modules/plotting/utils/statsutils.h>

namespace inviwo {
namespace plot {

namespace detail {
enum class FilterResult { Upper, Lower, None };
/**
 * Helper for brushing data
 * @param value to filter
 * @param range to use for filtering
 * @return true if if value is outside range and not missing data.
 */
FilterResult filterValue(const double &value, const dvec2 &range) {
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

const std::string ParallelCoordinatesAxisSettingsProperty::classIdentifier =
    "org.inviwo.parallelcoordinates.axissettingsproperty";
std::string ParallelCoordinatesAxisSettingsProperty::getClassIdentifier() const {
    return classIdentifier;
}

ParallelCoordinatesAxisSettingsProperty::ParallelCoordinatesAxisSettingsProperty(
    std::string identifier, std::string displayName)
    : BoolCompositeProperty(identifier, displayName, true)
    , usePercentiles_("usePercentiles", "Use Percentiles", false)
    , range_("range", "Axis Range") {
    addProperty(range_);
    addProperty(usePercentiles_);

    setCollapsed(true);
    setSerializationMode(PropertySerializationMode::All);
    range_.setSerializationMode(PropertySerializationMode::All);
    usePercentiles_.setSerializationMode(PropertySerializationMode::All);
}

void ParallelCoordinatesAxisSettingsProperty::updateFromColumn(std::shared_ptr<const Column> col) {
    col_ = col;
    util::KeepTrueWhileInScope updating(&updating_);
    col->getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto ram) -> void {
            using T = typename util::PrecsionValueType<decltype(ram)>;
            auto &dataVector = ram->getDataContainer();

            auto minMax = util::bufferMinMax(ram, IgnoreSpecialValues::Yes);
            double minV = minMax.first.x;
            double maxV = minMax.second.x;

            dvec2 prevVal = range_.get();
            dvec2 prevRange = range_.getRange();
            double l = prevRange.y - prevRange.x;

            double prevMinRatio = (prevVal.x - prevRange.x) / (l);
            double prevMaxRatio = (prevVal.y - prevRange.x) / (l);

            range_.setRange(glm::tvec2<T>(minV, maxV));
            if (l > 0 && maxV != minV) {
                range_.set(
                    {minV + prevMinRatio * (maxV - minV), minV + prevMaxRatio * (maxV - minV)});
            }
            auto pecentiles = statsutil::percentiles(dataVector, {0., 0.25, 0.75, 1.});
            p0_ = static_cast<double>(pecentiles[0]);
            p25_ = static_cast<double>(pecentiles[1]);
            p75_ = static_cast<double>(pecentiles[2]);
            p100_ = static_cast<double>(pecentiles[3]);
            at = [vec = &dataVector](size_t idx) { return static_cast<double>(vec->at(idx)); };
        });
}

double ParallelCoordinatesAxisSettingsProperty::getNormalized(double v) const {
    if (range_.getRangeMax() == range_.getRangeMin()) {
        return 0.5;
    }
    const auto range = range_.getRange();
    if (v <= range.x) {
        return 0;
    }
    if (v >= range.y) {
        return 1;
    }
    if (!usePercentiles_.get()) {
        return (v - range.x) / (range.y - range.x);
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

double ParallelCoordinatesAxisSettingsProperty::getNormalizedAt(size_t idx) const {
    return getNormalized(at(idx));
}

double ParallelCoordinatesAxisSettingsProperty::getValue(double v) const {
    const auto range = range_.getRange();
    if (v <= 0) {
        return range.x;
    }
    if (v >= 1) {
        return range.y;
    }
    if (!usePercentiles_.get()) {
        return range.x + v * (range.y - range.x);
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

void ParallelCoordinatesAxisSettingsProperty::moveHandle(bool upper, double mouseY) {
    auto range = range_.get();
    double value = getValue(mouseY);

    if (upper) {
        if (value < range.x) {
            value = range.x;
        }
        range.y = value;
    } else {
        if (value > range.y) {
            value = range.y;
        }
        range.x = value;
    }
    range_.set(range);
}

void ParallelCoordinatesAxisSettingsProperty::updateBrushing(std::unordered_set<size_t> &brushed) {
    if (updating_) return;
    auto range = range_.get();
    // Increase range to avoid conversion issues
    const static dvec2 off(-std::numeric_limits<float>::epsilon(),
                           std::numeric_limits<float>::epsilon());
    range += off;
    upperBrushed_ = false;
    lowerBrushed_ = false;
    for (size_t i = 0; i < col_->getSize(); i++) {
        auto filtered = detail::filterValue(at(i), range);
        if (filtered == detail::FilterResult::None) continue;
        brushed.insert(i);
        if (filtered == detail::FilterResult::Upper) upperBrushed_ = true;
        if (filtered == detail::FilterResult::Lower) lowerBrushed_ = true;
    }
}

}  // namespace plot
}  // namespace inviwo
