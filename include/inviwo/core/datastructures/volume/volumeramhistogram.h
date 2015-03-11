/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMERAMHISTOGRAM_H
#define IVW_VOLUMERAMHISTOGRAM_H

#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/datastructures/volume/volumeoperation.h>
#include <inviwo/core/datastructures/histogram.h>

namespace inviwo {

class IVW_CORE_API VolumeRAMNormalizedHistogram : public VolumeOperation {
public:
    VolumeRAMNormalizedHistogram(const VolumeRepresentation* in,
                                 std::vector<NormalizedHistogram*>* oldHist = NULL,
                                 int sampleRate = 1, size_t maxNumberOfBins = 2048)
        : VolumeOperation(in)
        , sampleRate_(sampleRate)
        , histograms_(oldHist)
        , maxNumberOfBins_(maxNumberOfBins) {}

    virtual ~VolumeRAMNormalizedHistogram() {}

    template <typename T, size_t B>
    void evaluate();

    static inline std::vector<NormalizedHistogram*>* apply(
        const VolumeRepresentation* in, std::vector<NormalizedHistogram*>* oldHist = NULL,
        int sampleRate = 1, size_t maxNumberOfBins = 2048) {
        VolumeRAMNormalizedHistogram subsetOP =
            VolumeRAMNormalizedHistogram(in, oldHist, sampleRate, maxNumberOfBins);
        in->performOperation(&subsetOP);
        return subsetOP.getOutput<std::vector<NormalizedHistogram*> >();
    }

    /**
     * Values put into bins [x, x+delta)
     */
    static size_t calculateBins(const VolumeRAM* volumeRAM, size_t maxNumberOfBins) {
        const Volume* volume = reinterpret_cast<const Volume*>(volumeRAM->getOwner());
        dvec2 dataRange = volume->dataMap_.dataRange;

        double delta = (dataRange.y - dataRange.x) / static_cast<double>(maxNumberOfBins-1);

        switch (volumeRAM->getDataFormat()->getNumericType()) {
        case DataFormatEnums::NOT_SPECIALIZED_TYPE:
            break;
        case DataFormatEnums::FLOAT_TYPE:
            break;
        case DataFormatEnums::UNSIGNED_INTEGER_TYPE:
            if (delta < 1.0) {
                delta = 1.0;
            }
            break;
        case DataFormatEnums::SIGNED_INTEGER_TYPE:
            if (delta < 1.0) {
                delta = 1.0;
            }
            break;
        }
        return std::min(maxNumberOfBins,static_cast<size_t>(std::ceil((dataRange.y - dataRange.x) / delta) + 1));
    }

private:
    int sampleRate_;
    std::vector<NormalizedHistogram*>* histograms_;  // One histogram per channel in the data
    size_t maxNumberOfBins_;
};


template <typename T, typename V, typename vec, typename ivec, size_t B, int N>
void calculateVecHist(std::vector<NormalizedHistogram*>* histograms, const VolumeRAM* volumeRAM,
                      int sampleRate, size_t numberOfBinsInHistogram) {
    std::vector<std::vector<double>*> histData;

    for (int channel = 0; channel < N; ++channel) {
        if (static_cast<int>(histograms->size()) < channel + 1) {
            histograms->push_back(new NormalizedHistogram(numberOfBinsInHistogram));
        } else {
            histograms->at(channel)->resize(numberOfBinsInHistogram);
        }

        histData.push_back(histograms->at(channel)->getData());
    }

    V* src = const_cast<V*>(reinterpret_cast<const V*>(volumeRAM->getData()));
    const uvec3& dim = volumeRAM->getDimensions();
    const Volume* volume = reinterpret_cast<const Volume*>(volumeRAM->getOwner());
    dvec2 dataRange = volume->dataMap_.dataRange;

    glm::uvec3 pos(0);

    vec min(std::numeric_limits<double>::max());
    vec max(std::numeric_limits<double>::min());
    vec sum(0);
    vec sum2(0);
    ivec outsideOfDataRange(0);
    double count(0);
    V elem;

    for (pos.z = 0; pos.z < dim.z; pos.z += sampleRate) {
        for (pos.y = 0; pos.y < dim.y; pos.y += sampleRate) {
            for (pos.x = 0; pos.x < dim.x; pos.x += sampleRate) {
                if (volumeRAM->shouldStopHistogramCalculation()) break;

                elem = src[VolumeRAM::posToIndex(pos, dim)];
                vec val(elem);

                min = glm::min(min, val);
                max = glm::max(max, val);
                sum += val;
                sum2 += val * val;
                count++;

                ivec ind =
                    static_cast<ivec>((val - vec(dataRange.x)) / (vec(dataRange.y - dataRange.x)) *
                                      vec(numberOfBinsInHistogram - 1));

                for (int channel = 0; channel < N; ++channel) {
                    if (ind[channel] >= 0 &&
                        ind[channel] < static_cast<int>(numberOfBinsInHistogram)) {
                        histData[channel]->at(ind[channel])++;
                    } else {
                        outsideOfDataRange[channel]++;
                    }
                }
            }
        }
    }

    for (int channel = 0; channel < N; ++channel) {
        histograms->at(channel)->dataRange_ = dataRange;
        histograms->at(channel)->stats_.min = min[channel];
        histograms->at(channel)->stats_.max = max[channel];
        histograms->at(channel)->stats_.mean = sum[channel] / count;
        histograms->at(channel)->stats_.standardDeviation = std::sqrt(
            (count * sum2[channel] - sum[channel] * sum[channel]) / (count * (count - 1)));

        histograms->at(channel)->calculatePercentiles();
        histograms->at(channel)->performNormalization();
        histograms->at(channel)->calculateHistStats();
        histograms->at(channel)->setValid(true);
    }
}


template <typename T, size_t B, int N>
class HistogramCalculator {
public:
    static void calculate(std::vector<NormalizedHistogram*>* histograms, const VolumeRAM* volumeRAM,
                          int sampleRate, size_t numberOfBinsInHistogram) {
        // Calculate histogram data

        if (histograms->empty()) {
            histograms->push_back(new NormalizedHistogram(numberOfBinsInHistogram));
        } else {
            histograms->at(0)->resize(numberOfBinsInHistogram);
        }

        NormalizedHistogram* histogram = histograms->at(0);

        std::vector<double>* histData = histogram->getData();
        T* src = const_cast<T*>(reinterpret_cast<const T*>(volumeRAM->getData()));
        const uvec3& dim = volumeRAM->getDimensions();

        const Volume* volume = reinterpret_cast<const Volume*>(volumeRAM->getOwner());
        dvec2 dataRange = volume->dataMap_.dataRange;

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();
        double sum = 0;
        double sum2 = 0;
        double count = 0;
        uvec3 pos(0);

        size_t outsideOfDataRange = 0;

        for (pos.z = 0; pos.z < dim.z; pos.z += sampleRate) {
            for (pos.y = 0; pos.y < dim.y; pos.y += sampleRate) {
                for (pos.x = 0; pos.x < dim.x; pos.x += sampleRate) {
                    if (volumeRAM->shouldStopHistogramCalculation()) break;
                    double val = static_cast<double>(src[VolumeRAM::posToIndex(pos, dim)]);
                    min = std::min(min, val);
                    max = std::max(max, val);
                    sum += val;
                    sum2 += val * val;
                    count++;

                    int ind =
                        static_cast<int>((val - dataRange.x) / (dataRange.y - dataRange.x) *
                                            (numberOfBinsInHistogram - 1));

                    if (ind >= 0 && ind < static_cast<int>(numberOfBinsInHistogram)) {
                        histData->at(ind)++;
                    } else {
                        outsideOfDataRange++;
                    }
                }
            }
        }

        histogram->dataRange_ = dataRange;
        histogram->stats_.min = min;
        histogram->stats_.max = max;
        histogram->stats_.mean = sum / count;
        histogram->stats_.standardDeviation =
            std::sqrt((count * sum2 - sum * sum) / (count * (count - 1)));

        histogram->calculatePercentiles();
        histogram->performNormalization();
        histogram->calculateHistStats();
        histogram->setValid(true);
    };
};

template <typename T, size_t B, int N>
class HistogramCalculator<glm::detail::tvec2<T, glm::defaultp>, B, N> {
public:
    typedef glm::detail::tvec2<T, glm::defaultp> V;
    typedef glm::detail::tvec2<double, glm::defaultp> vec;
    typedef glm::detail::tvec2<int, glm::defaultp> ivec;

    static void calculate(std::vector<NormalizedHistogram*>* histograms, const VolumeRAM* volumeRAM,
                          int sampleRate, size_t numberOfBinsInHistogram) {
        calculateVecHist<T, V, vec, ivec, B, 2>(histograms, volumeRAM, sampleRate,
                                                numberOfBinsInHistogram);
    }
};

template <typename T, size_t B, int N>
class HistogramCalculator<glm::detail::tvec3<T, glm::defaultp>, B, N> {
public:
    typedef glm::detail::tvec3<T, glm::defaultp> V;
    typedef glm::detail::tvec3<double, glm::defaultp> vec;
    typedef glm::detail::tvec3<int, glm::defaultp> ivec;

    static void calculate(std::vector<NormalizedHistogram*>* histograms, const VolumeRAM* volumeRAM,
                          int sampleRate, size_t numberOfBinsInHistogram) {
        calculateVecHist<T, V, vec, ivec, B, 3>(histograms, volumeRAM, sampleRate,
                                                numberOfBinsInHistogram);
    }
};
template <typename T, size_t B, int N>
class HistogramCalculator<glm::detail::tvec4<T, glm::defaultp>, B, N> {
public:
    typedef glm::detail::tvec4<T, glm::defaultp> V;
    typedef glm::detail::tvec4<double, glm::defaultp> vec;
    typedef glm::detail::tvec4<int, glm::defaultp> ivec;

    static void calculate(std::vector<NormalizedHistogram*>* histograms, const VolumeRAM* volumeRAM,
                          int sampleRate, size_t numberOfBinsInHistogram) {
        calculateVecHist<T, V, vec, ivec, B, 4>(histograms, volumeRAM, sampleRate,
                                                numberOfBinsInHistogram);
    }
};


template <typename T>
class VolumeRAMPrecision;

template <typename T, size_t B>
class VolumeRAMCustomPrecision;

template <typename T, size_t B>
void VolumeRAMNormalizedHistogram::evaluate() {
    const VolumeRAMPrecision<T>* volumeRAM =
        dynamic_cast<const VolumeRAMPrecision<T>*>(getInputVolume());

    if (!volumeRAM || !volumeRAM->getOwner()) {
        setOutput(NULL);
        return;
    }

    size_t numberOfBinsInHistogram = calculateBins(volumeRAM, maxNumberOfBins_);

    // Create Normalized Histogram
    if (!histograms_) {
        histograms_ = new std::vector<NormalizedHistogram*>();
    }

    switch (volumeRAM->getDataFormat()->getComponents()) {
        case 1:
            HistogramCalculator<T, B, 1>::calculate(histograms_, volumeRAM, sampleRate_,
                                                     numberOfBinsInHistogram);
            break;
        case 2:
            HistogramCalculator<T, B, 2>::calculate(histograms_, volumeRAM, sampleRate_,
                                                     numberOfBinsInHistogram);
            break;
        case 3:
            HistogramCalculator<T, B, 3>::calculate(histograms_, volumeRAM, sampleRate_,
                                                     numberOfBinsInHistogram);
            break; 
        case 4:
            HistogramCalculator<T, B, 4>::calculate(histograms_, volumeRAM, sampleRate_,
                                                     numberOfBinsInHistogram);
            break; 
        default:
            break;
    }

    if (volumeRAM->shouldStopHistogramCalculation()) {
        for (std::vector<NormalizedHistogram*>::iterator it = histograms_->begin();
             it != histograms_->end(); ++it) {
            delete *it;
        }
        histograms_->clear();
        delete histograms_;
        histograms_ = NULL;
    }

    setOutput(histograms_);
}

}  // namespace

#endif  // IVW_VOLUMERAMHISTOGRAM_H
