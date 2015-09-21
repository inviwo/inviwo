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
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

/* using type erasure
 * https://aherrmann.github.io/programming/2014/10/19/type-erasure-with-merged-concepts/
 */

template <typename T>
struct OutportIterable {
    OutportIterable() {}
    virtual ~OutportIterable() {}

    class const_iterator
        : public std::iterator<std::forward_iterator_tag, T, std::ptrdiff_t,
                               std::shared_ptr<const T>, std::shared_ptr<const T>> {
    public:
        const_iterator() : self_(nullptr){};
        template <typename Wrapper>
        const_iterator(Wrapper wrapper)
            : self_(util::make_unique<Model<Wrapper>>(wrapper)) {}
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

        std::shared_ptr<const T> operator*() { return self_->getref(); }
        std::shared_ptr<const T> operator->() { return self_->getptr(); }

        bool operator==(const const_iterator& rhs) const {
            if (!self_ && !(rhs.self_))
                return true;
            else if (!self_ || !(rhs.self_))
                return false;
            else
                return self_->equal(*(rhs.self_));
        }
        bool operator!=(const const_iterator& rhs) const {
            if (!self_ && !(rhs.self_))
                return false;
            else if (!self_ || !(rhs.self_))
                return true;
            else
                return !self_->equal(*(rhs.self_));
        }

    private:
        struct Concept {
            virtual ~Concept() = default;
            virtual Concept* clone() = 0;
            virtual void inc() = 0;
            virtual std::shared_ptr<const T> getref() = 0;
            virtual std::shared_ptr<const T> getptr() = 0;
            virtual bool equal(const Concept& that) = 0;
        };

        template <typename U>
        class Model : public Concept {
        public:
            Model(U data) : data_(data) {}

            virtual Model<U>* clone() override { return new Model<U>(*this); };
            virtual void inc() override { data_.inc(); };
            virtual std::shared_ptr<const T> getref() override { return data_.getref(); };
            virtual std::shared_ptr<const T> getptr() override { return data_.getptr(); };
            virtual bool equal(const Concept& that) override {
                return data_.equal(static_cast<const Model<U>&>(that).data_);
            };

        private:
            U data_;
        };

        std::unique_ptr<Concept> self_;
    };

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
};

template <typename T>
class DataOutport;

// Base template for single data ptr
template <typename T, typename Enable = void>
struct OutportIterableImpl : public OutportIterable<T> {
    OutportIterableImpl(DataOutport<T>* port) : port_(port) {}
    using const_iterator = typename OutportIterable<T>::const_iterator;

    virtual const_iterator begin() const override { return const_iterator(Wrapper(port_, false)); }
    virtual const_iterator end() const override { return const_iterator(Wrapper(port_, true)); }

private:
    class Wrapper {
    public:
        Wrapper() : port_(nullptr), end_(true) {}
        Wrapper(DataOutport<T>* port, bool end) : port_(port), end_(end || !port->getData()) {}

        void inc() { end_ = true; };
        std::shared_ptr<const T> getref() { return port_->getData(); };
        std::shared_ptr<const T> getptr() { return port_->getData(); };
        bool equal(const Wrapper& rhs) {
            if (end_ && rhs.end_)
                return true;
            else if (end_ != rhs.end_)
                return false;
            else
                return port_->getData().get() == port_->getData().get();
        };

    private:
        DataOutport<T>* port_;
        bool end_;
    };

    DataOutport<T>* port_;
};

// Specialization for vector of shared_ptr<data>.
template <typename T, typename Alloc>
struct OutportIterableImpl<std::vector<std::shared_ptr<T>, Alloc>> : public OutportIterable<T> {
    using container = std::vector<std::shared_ptr<T>, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    OutportIterableImpl(DataOutport<container>* port) : port_(port) {}

    virtual const_iterator begin() const override {
        auto data = port_->getData();
        if (!data)
            return const_iterator(Wrapper());
        else
            return const_iterator(Wrapper(data->begin(), data->end()));
    };
    virtual const_iterator end() const override {
        auto data = port_->getData();
        if (!data)
            return const_iterator(Wrapper());
        else
            return const_iterator(Wrapper(data->end(), data->end()));
    };

private:
    class Wrapper {
    public:
        using Iter = typename container::const_iterator;
        Wrapper() : iter_(), iterEnd_(), end_(true) {}
        Wrapper(Iter begin, Iter end) : iter_(begin), iterEnd_(end), end_(iter_ == iterEnd_) {}
        void inc() {
            iter_++;
            if (iter_ == iterEnd_) end_ = true;
        };

        std::shared_ptr<const T> getref() { return *iter_; };
        std::shared_ptr<const T> getptr() { return *iter_; };

        bool equal(const Wrapper& rhs) {
            if (end_ && rhs.end_)
                return true;
            else if (end_ != rhs.end_)
                return false;
            else
                return iter_ == rhs.iter_;
        }

    private:
        Iter iter_;
        Iter iterEnd_;
        bool end_;
    };

    DataOutport<container>* port_;
};

// Specialization for vector of data
template <typename T, typename Alloc>
struct OutportIterableImpl<std::vector<T, Alloc>> : public OutportIterable<T> {
    using container = std::vector<T, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    OutportIterableImpl(DataOutport<container>* port) : port_(port) {}

    virtual const_iterator begin() const override {
        auto data = port_->getData();
        if (!data) return const_iterator(Wrapper());
        return const_iterator(Wrapper(data));
    };
    virtual const_iterator end() const override { return const_iterator(Wrapper()); };

private:
    class Wrapper {
    public:
        using Iter = typename container::const_iterator;

        Wrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
        Wrapper(std::shared_ptr<const container> data)
            : data_(data), iter_(data->begin()), iterEnd_(data->end()), end_(iter_ == iterEnd_) {}

        void inc() {
            iter_++;
            if (iter_ == iterEnd_) end_ = true;
        };
        std::shared_ptr<const T> getref() { return std::shared_ptr<const T>(data_, &(*iter_)); };
        std::shared_ptr<const T> getptr() { return std::shared_ptr<const T>(data_, &(*iter_)); };
        bool equal(const Wrapper& rhs) {
            if (end_ && rhs.end_)
                return true;
            else if (end_ != rhs.end_)
                return false;
            else
                return iter_ == rhs.iter_;
        }

    private:
        std::shared_ptr<const container> data_;
        Iter iter_;
        Iter iterEnd_;
        bool end_;
    };

    DataOutport<container>* port_;
};

// Specialization for vector of data pointer
template <typename T, typename Alloc>
struct OutportIterableImpl<std::vector<T*, Alloc>> : public OutportIterable<T> {
    using container = std::vector<T*, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    OutportIterableImpl(DataOutport<container>* port) : port_(port) {}

    virtual const_iterator begin() const override {
        auto data = port_->getData();
        if (!data) return const_iterator(Wrapper());
        return const_iterator(Wrapper(data));
    };
    virtual const_iterator end() const override { return const_iterator(Wrapper()); };

private:
    class Wrapper {
    public:
        using Iter = typename container::const_iterator;

        Wrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
        Wrapper(std::shared_ptr<const container> data)
            : data_(data), iter_(data->begin()), iterEnd_(data->end()), end_(iter_ == iterEnd_) {}

        void inc() {
            iter_++;
            if (iter_ == iterEnd_) end_ = true;
        };

        std::shared_ptr<const T> getref() { return std::shared_ptr<const T>(data_, *iter_); };
        std::shared_ptr<const T> getptr() { return std::shared_ptr<const T>(data_, *iter_); };

        bool equal(const Wrapper& rhs) {
            if (end_ && rhs.end_)
                return true;
            else if (end_ != rhs.end_)
                return false;
            else
                return iter_ == rhs.iter_;
        }

    private:
        std::shared_ptr<const container> data_;
        Iter iter_;
        Iter iterEnd_;
        bool end_;
    };

    DataOutport<container>* port_;
};

// Specialization for vector of data pointer
template <typename T, typename Alloc>
struct OutportIterableImpl<std::vector<std::unique_ptr<T>, Alloc>> : public OutportIterable<T> {
    using container = std::vector<std::unique_ptr<T>, Alloc>;
    using const_iterator = typename OutportIterable<T>::const_iterator;

    OutportIterableImpl(DataOutport<container>* port) : port_(port) {}

    virtual const_iterator begin() const override {
        auto data = port_->getData();
        if (!data) return const_iterator(Wrapper());
        return const_iterator(Wrapper(data));
    };
    virtual const_iterator end() const override { return const_iterator(Wrapper()); };

private:
    class Wrapper {
    public:
        using Iter = typename container::const_iterator;

        Wrapper() : data_(), iter_(), iterEnd_(), end_(true) {}
        Wrapper(std::shared_ptr<const container> data)
            : data_(data), iter_(data->begin()), iterEnd_(data->end()), end_(iter_ == iterEnd_) {}

        void inc() {
            iter_++;
            if (iter_ == iterEnd_) end_ = true;
        };

        std::shared_ptr<const T> getref() { return std::shared_ptr<const T>(data_, (*iter_).get()); };
        std::shared_ptr<const T> getptr() { return std::shared_ptr<const T>(data_, (*iter_).get()); };

        bool equal(const Wrapper& rhs) {
            if (end_ && rhs.end_)
                return true;
            else if (end_ != rhs.end_)
                return false;
            else
                return iter_ == rhs.iter_;
        }

    private:
        std::shared_ptr<const container> data_;
        Iter iter_;
        Iter iterEnd_;
        bool end_;
    };

    DataOutport<container>* port_;
};

}  // namespace

#endif  // IVW_OUTPORTITERATOR_H
