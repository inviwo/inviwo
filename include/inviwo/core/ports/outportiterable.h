/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

/* using type erasure
 * https://aherrmann.github.io/programming/2014/10/19/type-erasure-with-merged-concepts/
 */

template <typename T>
class DataOutport;

template <typename T>
struct OutportIterable {
    OutportIterable() = default;
    virtual ~OutportIterable() = default;

    class const_iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = std::shared_ptr<const T>;
        using reference = std::shared_ptr<const T>;
        using iterator_category = std::forward_iterator_tag;

        const_iterator() : self_(nullptr){};
        template <typename Wrapper>
        const_iterator(Wrapper wrapper) : self_(std::make_unique<Model<Wrapper>>(wrapper)) {}
        const_iterator(const const_iterator& rhs)
            : self_(rhs.self_ ? rhs.self_->clone() : nullptr) {}
        const_iterator& operator=(const const_iterator& that) {
            if (this != &that) {
                std::unique_ptr<Concept> s(that.self_ ? that.self_->clone() : nullptr);
                std::swap(s, self_);
            }
            return *this;
        }

        const_iterator& operator++() {
            self_->inc();
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator i{*this};
            self_->inc();
            return i;
        }

        reference operator*() const { return self_->get(); }
        pointer operator->() const { return self_->get(); }

        bool operator==(const const_iterator& rhs) const {
            if (!self_ && !(rhs.self_)) {
                return true;
            } else if (self_) {
                return self_->end();
            } else if (rhs.self_) {
                return rhs.self_->end();
            } else {
                return self_->equal(*(rhs.self_));
            }
        }
        bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

    private:
        struct Concept {
            virtual ~Concept() = default;
            virtual Concept* clone() = 0;
            virtual void inc() = 0;
            virtual std::shared_ptr<const T> get() = 0;
            virtual bool equal(const Concept& that) const = 0;
            virtual bool end() const = 0;
        };

        template <typename U>
        class Model : public Concept {
        public:
            Model(U data) : data_(data) {}

            virtual Model<U>* clone() override { return new Model<U>(*this); };
            virtual void inc() override { data_.inc(); };
            virtual std::shared_ptr<const T> get() override { return data_.get(); };
            virtual bool equal(const Concept& that) const override {
                return data_.equal(static_cast<const Model<U>&>(that).data_);
            };
            virtual bool end() const override { return data_.end(); }

        private:
            U data_;
        };

        std::unique_ptr<Concept> self_;
    };

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
};

namespace detail {

// Base template for single data ptr
template <typename T>
class OutportIterableWrapper {
public:
    OutportIterableWrapper() : data_(nullptr) {}
    OutportIterableWrapper(const DataOutport<T>& port) : data_{port.getData()} {}

    std::shared_ptr<const T> get() { return data_; };

    void inc() { data_.reset(); };
    bool equal(const OutportIterableWrapper& rhs) const {
        if (!data_ && !rhs.data_)
            return true;
        else if (!data_ != !rhs.data_)
            return false;
        else
            return data_.get() == data_.get();
    };
    bool end() const { return data_ == nullptr; }

private:
    std::shared_ptr<const T> data_;
};

// Specialization for vector of shared_ptr<data>.
template <typename T, typename Alloc>
class OutportIterableWrapper<std::vector<std::shared_ptr<T>, Alloc>> {
public:
    using Iter = typename std::vector<std::shared_ptr<T>, Alloc>::const_iterator;
    OutportIterableWrapper() : iter_(), iterEnd_(), end_(true) {}
    OutportIterableWrapper(const DataOutport<std::vector<std::shared_ptr<T>, Alloc>>& port)
        : iter_{port.hasData() ? port.getData()->begin() : Iter{}}
        , iterEnd_{port.hasData() ? port.getData()->end() : Iter{}}
        , end_{iter_ == iterEnd_} {}

    std::shared_ptr<const T> get() { return *iter_; };

    void inc() {
        ++iter_;
        if (iter_ == iterEnd_) end_ = true;
    };

    bool equal(const OutportIterableWrapper& rhs) const {
        if (end_ && rhs.end_)
            return true;
        else if (end_ != rhs.end_)
            return false;
        else
            return iter_ == rhs.iter_;
    }
    bool end() const { return end_; }

private:
    Iter iter_;
    Iter iterEnd_;
    bool end_;
};

// Specialization for vector of data
template <typename T, typename Alloc>
class OutportIterableWrapper<std::vector<T, Alloc>> {
public:
    using Iter = typename std::vector<T, Alloc>::const_iterator;

    OutportIterableWrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
    OutportIterableWrapper(const DataOutport<std::vector<T, Alloc>>& port)
        : data_{port.getData()}
        , iter_{port.hasData() ? port.getData()->begin() : Iter{}}
        , iterEnd_{port.hasData() ? port.getData()->end() : Iter{}}
        , end_{iter_ == iterEnd_} {}

    std::shared_ptr<const T> get() { return std::shared_ptr<const T>(data_, &(*iter_)); };

    void inc() {
        ++iter_;
        if (iter_ == iterEnd_) end_ = true;
    };

    bool equal(const OutportIterableWrapper& rhs) const {
        if (end_ && rhs.end_)
            return true;
        else if (end_ != rhs.end_)
            return false;
        else
            return iter_ == rhs.iter_;
    }

    bool end() const { return end_; }

private:
    std::shared_ptr<const std::vector<T, Alloc>> data_;
    Iter iter_;
    Iter iterEnd_;
    bool end_;
};

// Specialization for vector of data pointer
template <typename T, typename Alloc>
struct OutportIterableWrapper<std::vector<T*, Alloc>> {
public:
    using Iter = typename std::vector<T*, Alloc>::const_iterator;

    OutportIterableWrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
    OutportIterableWrapper(const DataOutport<std::vector<T*, Alloc>>& port)
        : data_{port.getData()}
        , iter_{port.hasData() ? port.getData()->begin() : Iter{}}
        , iterEnd_{port.hasData() ? port.getData()->end() : Iter{}}
        , end_{iter_ == iterEnd_} {}

    std::shared_ptr<const T> get() { return std::shared_ptr<const T>(data_, *iter_); };

    void inc() {
        ++iter_;
        if (iter_ == iterEnd_) end_ = true;
    };

    bool equal(const OutportIterableWrapper& rhs) const {
        if (end_ && rhs.end_)
            return true;
        else if (end_ != rhs.end_)
            return false;
        else
            return iter_ == rhs.iter_;
    }

    bool end() const { return end_; }

private:
    std::shared_ptr<const std::vector<T*, Alloc>> data_;
    Iter iter_;
    Iter iterEnd_;
    bool end_;
};

// Specialization for vector of data unique pointer
template <typename T, typename Alloc>
class OutportIterableWrapper<std::vector<std::unique_ptr<T>, Alloc>> {
public:
    using Iter = typename std::vector<std::unique_ptr<T>, Alloc>::const_iterator;

    OutportIterableWrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
    OutportIterableWrapper(const DataOutport<std::vector<std::unique_ptr<T>, Alloc>>& port)
        : data_{port.getData()}
        , iter_{port.hasData() ? port.getData()->begin() : Iter{}}
        , iterEnd_{port.hasData() ? port.getData()->end() : Iter{}}
        , end_{iter_ == iterEnd_} {}

    std::shared_ptr<const T> get() { return std::shared_ptr<const T>(data_, (*iter_).get()); };

    void inc() {
        ++iter_;
        if (iter_ == iterEnd_) end_ = true;
    };

    bool equal(const OutportIterableWrapper& rhs) const {
        if (end_ && rhs.end_)
            return true;
        else if (end_ != rhs.end_)
            return false;
        else
            return iter_ == rhs.iter_;
    }

    bool end() const { return end_; }

private:
    std::shared_ptr<const std::vector<std::unique_ptr<T>, Alloc>> data_;
    Iter iter_;
    Iter iterEnd_;
    bool end_;
};

}  // namespace detail

// Base template for single data ptr
template <typename Derived, typename T, typename Enable = void>
struct OutportIterableImpl : public OutportIterable<T> {
    using Container = T;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return detail::OutportIterableWrapper<Container>{*static_cast<const Derived*>(this)};
    }
    virtual const_iterator end() const override {
        return detail::OutportIterableWrapper<Container>{};
    }
};

// Specialization for vector of shared_ptr<data>.
template <typename Derived, typename T, typename Alloc>
struct OutportIterableImpl<Derived, std::vector<std::shared_ptr<T>, Alloc>>
    : public OutportIterable<T> {
    using Container = std::vector<std::shared_ptr<T>, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return detail::OutportIterableWrapper<Container>{*static_cast<const Derived*>(this)};
    }
    virtual const_iterator end() const override {
        return detail::OutportIterableWrapper<Container>{};
    }
};

// Specialization for vector of data
template <typename Derived, typename T, typename Alloc>
struct OutportIterableImpl<Derived, std::vector<T, Alloc>> : public OutportIterable<T> {
    using Container = std::vector<T, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return detail::OutportIterableWrapper<Container>{*static_cast<const Derived*>(this)};
    }
    virtual const_iterator end() const override {
        return detail::OutportIterableWrapper<Container>{};
    }
};

// Specialization for vector of data pointer
template <typename Derived, typename T, typename Alloc>
struct OutportIterableImpl<Derived, std::vector<T*, Alloc>> : public OutportIterable<T> {
    using Container = std::vector<T*, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return detail::OutportIterableWrapper<Container>{*static_cast<const Derived*>(this)};
    }
    virtual const_iterator end() const override {
        return detail::OutportIterableWrapper<Container>{};
    }
};

// Specialization for vector of data pointer
template <typename Derived, typename T, typename Alloc>
struct OutportIterableImpl<Derived, std::vector<std::unique_ptr<T>, Alloc>>
    : public OutportIterable<T> {
    using Container = std::vector<std::unique_ptr<T>, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override {
        return detail::OutportIterableWrapper<Container>{*static_cast<const Derived*>(this)};
    }
    virtual const_iterator end() const override {
        return detail::OutportIterableWrapper<Container>{};
    }
};

}  // namespace inviwo

#endif  // IVW_OUTPORTITERATOR_H
