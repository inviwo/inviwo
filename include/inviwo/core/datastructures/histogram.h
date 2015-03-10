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

#ifndef IVW_HISTOGRAM_H
#define IVW_HISTOGRAM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

//The NormalizedHistogram has a array of bins and all bins are normalized.
//It can be de-normalized using the maxiumBinValue_.
class IVW_CORE_API NormalizedHistogram {

public:
    NormalizedHistogram(size_t);
    NormalizedHistogram(const NormalizedHistogram& rhs);
    NormalizedHistogram& operator=(const NormalizedHistogram& that);
    virtual NormalizedHistogram* clone() const;
    virtual ~NormalizedHistogram();

    std::vector<double>* getData();
    const std::vector<double>* getData() const;
    double& operator[](size_t i);
    const double& operator[](size_t i) const;

    bool exists() const;

    void setValid(bool);
    bool isValid() const;

    void resize(size_t);

    void performNormalization();
    void calculatePercentiles();
    void calculateHistStats();
    double getMaximumBinValue() const;

    struct Stats {
    public:
        double min;
        double max;
        double mean;
        double standardDeviation;
        std::vector<double> percentiles;
    };
    Stats stats_;
    Stats histStats_;
    dvec2 dataRange_;

protected:
    std::vector<double> data_;
    double maximumBinCount_; //The maximum count (used for normalization)
    bool valid_;
};

class IVW_CORE_API HistogramContainer {
public:
    HistogramContainer();
    HistogramContainer(const HistogramContainer& rhs);
    HistogramContainer& operator=(const HistogramContainer& that);

    HistogramContainer(HistogramContainer&& rhs);
    HistogramContainer& operator=(HistogramContainer&& that);

    NormalizedHistogram& operator[](size_t i) const;
    virtual HistogramContainer* clone() const;
    virtual ~HistogramContainer();

    size_t size() const;
    bool empty() const;
    NormalizedHistogram* get(size_t i) const;
    void add(NormalizedHistogram* hist);

    void setValid(bool valid);
    bool isValid() const;

private:
    std::vector<NormalizedHistogram*> histograms_;
};


} // namespace

#endif // IVW_HISTOGRAM_H
