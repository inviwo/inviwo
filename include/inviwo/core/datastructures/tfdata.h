/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/histogram.h>

#include <inviwo/core/datastructures/histogramtools.h>

#include <memory>

namespace inviwo {

class IVW_CORE_API TFData {
public:
    struct Base {
        virtual ~Base() = default;
        virtual std::unique_ptr<Base> clone() const = 0;

        virtual const DataMapper& getDataMap() const = 0;

        virtual DispatcherHandle<HistogramCache::Callback> calculateHistograms(
            std::function<void(const std::vector<Histogram1D>&)> whenDone) const = 0;
    };

    template <typename T>
    struct Implementation : Base {
        Implementation(T toWrap) : wrapped{toWrap} {}

        virtual std::unique_ptr<Base> clone() const override {
            return std::make_unique<Implementation<T>>(*this);
        }

        virtual const DataMapper& getDataMap() const override {
            static const DataMapper default_;
            if (auto data = wrapped->getData()) {
                return data->dataMap;
            }
            return default_;
        }

        virtual DispatcherHandle<HistogramCache::Callback> calculateHistograms(
            std::function<void(const std::vector<Histogram1D>&)> whenDone) const override {
            if (auto data = wrapped->getData()) {
                return data->calculateHistograms(whenDone);
            } else {
                whenDone({});
            }
        }

        T wrapped;
    };

    template <typename T>
    TFData(T data) : base_{std::make_unique<Implementation<T>>(data)} {}

    TFData(const TFData& rhs) : base_{rhs.base_->clone()} {}
    TFData(TFData&& rhs) noexcept : base_{std::exchange(rhs.base_, nullptr)} {}
    TFData& operator=(const TFData& that) {
        if (this != &that) {
            base_ = that.base_->clone();
        }
        return *this;
    }
    TFData& operator=(TFData&& that) noexcept {
        if (this != &that) {
            base_ = std::exchange(that.base_, nullptr);
        }
        return *this;
    }

    const DataMapper& getDataMap() const { return base_->getDataMap(); }

    DispatcherHandle<HistogramCache::Callback> calculateHistograms(
        std::function<void(const std::vector<Histogram1D>&)> whenDone) const {
        return base_->calculateHistograms(whenDone);
    }

private:
    std::unique_ptr<Base> base_;
};

}  // namespace inviwo
