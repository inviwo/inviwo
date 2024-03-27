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
#include <inviwo/core/util/assertion.h>

#include <memory>

namespace inviwo {

class IVW_CORE_API TFData {
public:
    using OnChangeHandle = std::array<std::shared_ptr<std::function<void()>>, 3>;

    struct Base {
        virtual ~Base() = default;
        Base() = default;
        Base(const Base&) = delete;
        Base(Base&&) = delete;
        Base& operator=(const Base&) = delete;
        Base& operator=(Base&&) = delete;

        virtual std::unique_ptr<Base> clone() const = 0;

        virtual const DataMapper* getDataMap() const = 0;

        virtual HistogramCache::Result calculateHistograms(
            const std::function<void(const std::vector<Histogram1D>&)>& whenDone) const = 0;

        virtual OnChangeHandle onChange(const std::function<void()>& callback) const = 0;
    };

    template <typename T>
    struct Implementation : Base {
        explicit Implementation(T* toWrap) : Base{}, port{toWrap} {
            IVW_ASSERT(port != nullptr, "port should never be null");
        }
        Implementation(const Implementation&) = delete;
        Implementation(Implementation&&) = delete;
        Implementation& operator=(const Implementation&) = delete;
        Implementation& operator=(Implementation&&) = delete;
        virtual ~Implementation() override = default;

        virtual std::unique_ptr<Base> clone() const override {
            return std::make_unique<Implementation<T>>(port);
        }

        virtual const DataMapper* getDataMap() const override {
            if (auto data = port->getData()) {
                return &data->dataMap;
            } else {
                return nullptr;
            }
        }

        virtual HistogramCache::Result calculateHistograms(
            const std::function<void(const std::vector<Histogram1D>&)>& whenDone) const override {
            if (auto data = port->getData()) {
                return data->calculateHistograms(whenDone);
            } else {
                return {.progress = HistogramCache::Progress::NoData};
            }
        }

        virtual OnChangeHandle onChange(const std::function<void()>& callback) const override {
            return {port->onChangeScoped(callback), port->onConnectScoped(callback),
                    port->onDisconnectScoped(callback)};
        }

        T* port;
    };

    TFData() = default;

    // NOLINTBEGIN(google-explicit-constructor
    template <typename T>
    TFData(T* data) : base_{data ? std::make_unique<Implementation<T>>(data) : nullptr} {}
    // NOLINTEND

    TFData(const TFData& rhs) : base_{rhs.base_ ? rhs.base_->clone() : nullptr} {}
    TFData(TFData&& rhs) noexcept : base_{std::exchange(rhs.base_, nullptr)} {}
    TFData& operator=(const TFData& that) {
        if (this != &that) {
            base_ = that.base_ ? that.base_->clone() : nullptr;
        }
        return *this;
    }
    TFData& operator=(TFData&& that) noexcept {
        if (this != &that) {
            base_ = std::exchange(that.base_, nullptr);
        }
        return *this;
    }
    ~TFData() = default;

    const DataMapper* getDataMap() const {
        if (base_) {
            return base_->getDataMap();
        } else {
            return nullptr;
        }
    }

    HistogramCache::Result calculateHistograms(
        const std::function<void(const std::vector<Histogram1D>&)>& whenDone) const {
        if (base_) {
            return base_->calculateHistograms(whenDone);
        } else {
            return {};
        }
    }

    OnChangeHandle onChange(const std::function<void()>& callback) const {
        if (base_) {
            return base_->onChange(callback);
        } else {
            return {};
        }
    }

    explicit operator bool() const { return base_ != nullptr; }

private:
    std::unique_ptr<Base> base_;
};

}  // namespace inviwo
