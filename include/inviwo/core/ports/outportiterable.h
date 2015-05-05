/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_OUTPORTITERATOR_H
#define IVW_OUTPORTITERATOR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

template <typename T>
struct OutportIterable {
    OutportIterable() {}
    virtual ~OutportIterable() {}

    class const_iterator : public std::iterator<std::forward_iterator_tag, T> {
        using self = const_iterator;
        using ContIter = typename std::vector<T>::const_iterator;

    public:
        const_iterator() : container_(false), end_(true), ptr_(nullptr), iter_() {}
        explicit const_iterator(const T* ptr, bool end)
            : container_(false), end_(end), ptr_(ptr), iter_() {}
        explicit const_iterator(ContIter iter, bool end)
            : container_(true), end_(end), ptr_(nullptr), iter_(iter) {}
        self& operator++() {
            if (container_) {
                iter_++;
            } else {
                end_ = true;
                ptr_ = nullptr;
            }
            return *this;
        }
        self operator++(int) {
            self i = *this;
            if (container_) {
                iter_++;
            } else {
                end_ = true;
                ptr_ = nullptr;
            }
            return i;
        }

        const T& operator*() { return container_ ? *iter_ : *ptr_; }
        const T* operator->() { return container_ ? &(*iter_) : ptr_; }

        bool operator==(const self& rhs) const {
            if (end_ && rhs.end_) return true;

            if (container_ == rhs.container_) {
                return container_ ? iter_ == rhs.iter_ : ptr_ == rhs.ptr_;
            } else {
                return false;
            }
        }
        bool operator!=(const self& rhs) const {
            if (end_ && rhs.end_) return false;

            if (container_ == rhs.container_) {
                return container_ ? iter_ != rhs.iter_ : ptr_ != rhs.ptr_;
            } else {
                return true;
            }
        }

    private:
        bool container_;
        bool end_;
        const T* ptr_;
        ContIter iter_;
    };

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
};

template <typename T>
class DataOutport;

template <typename T>
struct OutportIterableImpl : public OutportIterable<T> {
    OutportIterableImpl(DataOutport<T>* port) : port_(port) {}
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override { return const_iterator(port_->getConstData(), false); }
    virtual const_iterator end() const override { return const_iterator(nullptr, true); }

private:
    DataOutport<T>* port_;
};
template <typename T>
struct OutportIterableImpl<std::vector<T>> : public OutportIterable<T> {
    OutportIterableImpl(DataOutport<std::vector<T>>* port) : port_(port) {}
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return const_iterator(port_->getConstData()->begin(), false);
    };
    virtual const_iterator end() const override {
        return const_iterator(port_->getConstData()->end(), true);
    };

private:
    DataOutport<std::vector<T>>* port_;
};

}  // namespace

#endif  // IVW_OUTPORTITERATOR_H
