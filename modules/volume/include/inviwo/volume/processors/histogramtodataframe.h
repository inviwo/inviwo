/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/volume/volumemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/datastructures/histogramtools.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

namespace inviwo {

template <typename T>
class HistogramToDataFrame : public Processor, public ProgressBarOwner {
public:
    HistogramToDataFrame();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;

private:
    DataInport<T> inport_;
    DataFrameOutport outport_;

    OptionProperty<HistogramMode> histogramMode_;

    std::shared_ptr<DataFrame> dataframe_;
    HistogramCache::Result histogramResult_;
};

template <typename T>
const ProcessorInfo& HistogramToDataFrame<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<HistogramToDataFrame<T>>::getProcessorInfo()};
    return info;
}

template <>
struct ProcessorTraits<HistogramToDataFrame<Layer>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.LayerHistogramToDataFrame",       // Class identifier
            "Layer Histogram To DataFrame",               // Display name
            "Layer Operation",                            // Category
            CodeState::Experimental,                      // Code state
            Tags::CPU | Tag{"Histogram"} | Tag{"Layer"},  // Tags
            R"(Returns the histograms of a Layer as a DataFrame)"_unindentHelp,
        };
    }
};

template <>
struct ProcessorTraits<HistogramToDataFrame<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.VolumeHistogramToDataFrame",       // Class identifier
            "Volume Histogram To DataFrame",               // Display name
            "Volume Operation",                            // Category
            CodeState::Experimental,                       // Code state
            Tags::CPU | Tag{"Histogram"} | Tag{"Volume"},  // Tags
            R"(Returns the histograms of a Volume as a DataFrame)"_unindentHelp,
        };
    }
};

namespace detail {

IVW_MODULE_VOLUME_API std::shared_ptr<DataFrame> createDataFrame(
    const std::vector<Histogram1D>& histograms, HistogramMode mode);

}  // namespace detail

template <typename T>
HistogramToDataFrame<T>::HistogramToDataFrame()
    : Processor{}
    , inport_{"inport", "Input data"_help}
    , outport_{"histogram", "DataFrame containing the histogram of the input data"_help}
    , histogramMode_{
          "histogramMode", "Histogram Mode",
          OptionPropertyState<HistogramMode>{.options = {{"all", "All", HistogramMode::All},
                                                         {"p99", "99%", HistogramMode::P99},
                                                         {"p99", "99%", HistogramMode::P95},
                                                         {"p99", "99%", HistogramMode::P90},
                                                         {"log", "Log", HistogramMode::Log}}}
              .setSelectedValue(HistogramMode::All)} {

    addPorts(inport_, outport_);
    addProperties(histogramMode_);
}

template <typename T>
void HistogramToDataFrame<T>::process() {
    histogramResult_ =
        inport_.getData()->calculateHistograms([this](const std::vector<Histogram1D>& histograms) {
            dataframe_ = detail::createDataFrame(histograms, histogramMode_);
            getProgressBar().finishProgress();
            outport_.setData(dataframe_);
            outport_.invalidate(InvalidationLevel::Valid);
        });

    if (histogramResult_.progress == HistogramCache::Progress::NoData) {
        dataframe_ = nullptr;
        getProgressBar().finishProgress();
    } else if (histogramResult_.progress == HistogramCache::Progress::Calculating) {
        dataframe_ = nullptr;
        updateProgress(0.0);
    }

    outport_.setData(dataframe_);
}

}  // namespace inviwo
