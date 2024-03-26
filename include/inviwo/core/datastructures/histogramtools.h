/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2024 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/util/dispatcher.h>

#include <memory>
#include <vector>
#include <mutex>

namespace inviwo {

class IVW_CORE_API HistogramCache {
public:
    using Callback = void(const std::vector<Histogram1D>&);

    enum class Progress { Done, Calculating, NoData };
    struct Result {
        DispatcherHandle<Callback> handle = nullptr;
        Progress progress = Progress::NoData;
    };

    HistogramCache();
    HistogramCache(const HistogramCache& rhs);
    HistogramCache(HistogramCache&& rhs) noexcept;
    HistogramCache& operator=(const HistogramCache& that);
    HistogramCache& operator=(HistogramCache&& that) noexcept;
    ~HistogramCache() = default;

    Result calculateHistograms(const std::function<std::vector<Histogram1D>()>& calculate,
                               const std::function<Callback>& whenDone) const;

    void forEach(const std::function<void(const Histogram1D&, size_t)>&) const;
    void discard(const std::function<std::vector<Histogram1D>()>& calculate);

private:
    enum class Status { Valid, Calculating, NotSet };
    struct State {
        std::mutex mutex;
        std::vector<Histogram1D> histograms;
        Dispatcher<Callback> callbacks;
        Status status = Status::NotSet;
    };

    std::shared_ptr<State> state_;
};

}  // namespace inviwo
