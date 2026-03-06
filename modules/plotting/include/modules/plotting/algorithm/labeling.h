/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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
#pragma once

#include <modules/plotting/plottingmoduledefine.h>

#include <inviwo/core/util/glmvec.h>

#include <vector>
#include <array>
#include <span>
#include <string>

namespace inviwo::plot {

enum class LabelingAlgorithm : unsigned char {
    Heckbert,
    Matplotlib,
    ExtendedWilkinson,
    Limits,
};

struct IVW_MODULE_PLOTTING_API LinearRange {
    double start{0.0};
    double stop{1.0};
    double step{1.0};
};

constexpr LabelingAlgorithm defaultLabeling = LabelingAlgorithm::Matplotlib;
constexpr std::string_view defaultFormat = "{:.1f}";

IVW_MODULE_PLOTTING_API void updateLabelPositions(std::vector<double>& majorPositions,
                                                  std::vector<double>& minorPositions,
                                                  LabelingAlgorithm algorithm, const dvec2& range,
                                                  int maxTicks, int minorTickFrequency, bool fillAxis);

IVW_MODULE_PLOTTING_API void updateLabels(std::vector<std::string>& labels,
                                          const std::vector<double>& positions,
                                          std::string_view format);

IVW_MODULE_PLOTTING_API void updateLabels(std::vector<std::string>& labels,
                                          std::span<const std::string> srcLabels);

// set of nice numbers suggested by Talbot et al. for their labeling (Extended Wilkinson)
constexpr std::array stepsTalbot = {1.0, 5.0, 2.0, 2.5, 4.0, 3.0};
// set of nice numbers as used by matplotlib
constexpr std::array stepsMatplotlib = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 8.0, 10.0};

/**
 * Returns an inclusive linear range starting at @p start and ending at @p stop.
 *
 * @param start  starting value of the range
 * @param stop   last value of the range
 * @param step   increment between each element of the range
 * @return linear range [start, start + step, ..., stop]
 * @note it the range is invalid (start > stop and step > 0) or similar, the result is empty
 */
IVW_MODULE_PLOTTING_API void linearRange(const LinearRange& range, std::vector<double>& positions);

IVW_MODULE_PLOTTING_API void linearRange(const dvec2& range, const LinearRange& optRange,
                                         int minorTickFrequency, std::vector<double>& major,
                                         std::vector<double>& minor);

/**
 * Heckbert's labeling algorithm
 *
 * The algorithm was modified so that the ticks and labels are always within
 * [@p valueMin, @p valueMax]
 *
 * Heckbert, P. S. (1990) Nice numbers for graph labels, Graphics Gems I,
 * Academic Press Professional, Inc.
 *
 * @param valueMin  lower bound of the input range
 * @param valueMax  upper bound of the input range
 * @param numTicks  optimal number of ticks
 * @return computed labels and ticks positions
 */
IVW_MODULE_PLOTTING_API LinearRange labelingHeckbert(double valueMin, double valueMax,
                                                     int numTicks);

/**
 * Extended Wilkinson algorithm by Justin Talbot
 *
 * This is an enhanced version of Wilkinson's optimization-based axis labeling approach
 * as described by Talbot et al.
 *
 * Talbot, J., Lin, S., Hanrahan, P. (2010) An Extension of Wilkinson's Algorithm
 * for Positioning Tick Labels on Axes, InfoVis 2010.
 *
 * Implementation based on https://github.com/cran/labeling/blob/master/R/labeling.R
 *
 * @param valueMin  lower bound of the input range
 * @param valueMax  upper bound of the input range
 * @param numTicks  optimal number of ticks
 * @param steps     set of "nice" numbers, e.g. [1, 5, 2, 2.5, 4, 3]
 * @return computed labels and ticks positions
 */
IVW_MODULE_PLOTTING_API LinearRange labelingExtendedWilkinson(
    double valueMin, double valueMax, int numTicks, std::span<const double> steps = stepsTalbot);

/**
 * matplotlib labeling algorithm
 *
 * Place evenly spaced ticks, with a cap on the total number of ticks.
 *
 * Finds nice tick locations with no more than @p maxTicks ticks being within the
 * view limits. Locations beyond the limits are added to support autoscaling.
 *
 * Implementation based on
 * https://github.com/matplotlib/matplotlib/blob/main/lib/matplotlib/ticker.py
 *
 * @param valueMin  lower bound of the input range
 * @param valueMax  upper bound of the input range
 * @param maxTicks  maximum number of ticks
 * @param steps     set of "nice" numbers, e.g. [1, 1.5, 2, 2.5, 3, 4, 5, 6, 8, 10]
 * @param integerTicks  if true, ticks will only take integer values, as long as at least @p
 *                   minNTicks integers are found within the range
 * @param minNTicks relax @p maxTicks and @p integerTicks constraints if necessary
 * @return computed labels and ticks positions
 */
IVW_MODULE_PLOTTING_API LinearRange labelingMatplotlib(
    double valueMin, double valueMax, int maxTicks, std::span<const double> steps = stepsMatplotlib,
    bool integerTicks = false, int minNTicks = 2);

/**
 * Basic axis labeling showing only the limits and an optional zero.
 *
 * @param valueMin  lower bound of the input range
 * @param valueMax  upper bound of the input range
 * @param includeZero  a label for 0.0 is included if true
 * @return computed labels and ticks positions
 */
IVW_MODULE_PLOTTING_API void labelingLimits(double valueMin, double valueMax,
                                            std::vector<double>& positions,
                                            bool includeZero = true);

}  // namespace inviwo::plot
