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

#ifndef IVW_INPORTITERABLE_H
#define IVW_INPORTITERABLE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/outportiterable.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

template <typename InportType, typename T, bool Flat>
class InportIterable {
public:
    InportIterable() = default;
    virtual ~InportIterable() = default;

    template <typename Derived>
    class const_iterator_base {
    protected:
        using self = Derived;
        using PortIter = typename std::vector<Outport*>::const_iterator;
        using DataIter = typename OutportIterable<T>::const_iterator;

    public:
        explicit const_iterator_base(PortIter pIterBegin, PortIter pIterEnd)
            : pIter_{pIterBegin}, pEnd_{pIterEnd}, dIter_{}, dEnd_{} {
            if (pIter_ != pEnd_) {
                auto ptr = dynamic_cast<OutportIterable<T>*>(*pIter_);
                dIter_ = ptr->begin();
                dEnd_ = ptr->end();
                shouldGetNextPort();
            }
        }
        self& operator++() {
            ++dIter_;
            shouldGetNextPort();
            return *static_cast<Derived*>(this);
        }
        self operator++(int) {
            self i = *static_cast<Derived*>(this);
            ++dIter_;
            shouldGetNextPort();
            return i;
        }

        bool operator==(const self& rhs) const {
            return pIter_ == rhs.pIter_ && dIter_ == rhs.dIter_;
        }
        bool operator!=(const self& rhs) const { return !(*this == rhs); }

    protected:
        void shouldGetNextPort() {
            while (dIter_ == dEnd_ && pIter_ != pEnd_) {
                ++pIter_;
                if (pIter_ != pEnd_) {
                    auto ptr = dynamic_cast<OutportIterable<T>*>(*pIter_);
                    dIter_ = ptr->begin();
                    dEnd_ = ptr->end();
                }
            }
        }

        PortIter pIter_;
        PortIter pEnd_;
        DataIter dIter_;
        DataIter dEnd_;
    };

    class const_iterator : public const_iterator_base<const_iterator> {
        using Base = const_iterator_base<const_iterator>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::shared_ptr<const T>;
        using pointer = std::shared_ptr<const T>;
        using reference = std::shared_ptr<const T>;

        const_iterator(PortIter pIterBegin, PortIter pIterEnd) : Base(pIterBegin, pIterEnd) {}
        reference operator*() const { return *Base::dIter_; }
        pointer operator->() const { return *Base::dIter_; }
    };

    class const_iterator_port : public const_iterator_base<const_iterator_port> {
        using Base = const_iterator_base<const_iterator_port>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Outport*, std::shared_ptr<const T>>;
        using reference = std::pair<Outport*, std::shared_ptr<const T>>;
        using pointer = void;

        const_iterator_port(PortIter pIterBegin, PortIter pIterEnd) : Base(pIterBegin, pIterEnd) {}

        reference operator*() const { return reference{*Base::pIter_, *Base::dIter_}; }
    };

    class const_iterator_changed : public const_iterator_base<const_iterator_changed> {
        using Base = const_iterator_base<const_iterator_changed>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<bool, std::shared_ptr<const T>>;
        using reference = std::pair<bool, std::shared_ptr<const T>>;
        using pointer = void;

        const_iterator_changed(PortIter pIterBegin, PortIter pIterEnd, const InportType& inport)
            : Base(pIterBegin, pIterEnd), inport{inport} {}

        reference operator*() const {
            return reference{util::contains(inport.getChangedOutports(), *Base::pIter_),
                             *Base::dIter_};
        }

    private:
        const InportType& inport;
    };

    util::iter_range<const_iterator_port> outportAndData() const {
        return util::iter_range<const_iterator_port>{
            const_iterator_port{self().getConnectedOutports().begin(),
                                self().getConnectedOutports().end()},
            const_iterator_port{self().getConnectedOutports().end(),
                                self().getConnectedOutports().end()}};
    }

    util::iter_range<const_iterator_changed> changedAndData() const {
        return util::iter_range<const_iterator_changed>{
            const_iterator_changed{self().getConnectedOutports().begin(),
                                   self().getConnectedOutports().end(), self()},
            const_iterator_changed{self().getConnectedOutports().end(),
                                   self().getConnectedOutports().end(), self()}};
    }

    const_iterator begin() const {
        return const_iterator(self().getConnectedOutports().begin(),
                              self().getConnectedOutports().end());
    }
    const_iterator end() const {
        return const_iterator(self().getConnectedOutports().end(),
                              self().getConnectedOutports().end());
    }

private:
    InportType& self() { return static_cast<InportType&>(*this); }
    const InportType& self() const { return static_cast<const InportType&>(*this); }
};

// Specialization for non flat case.
template <typename InportType, typename T>
class InportIterable<InportType, T, false> {
public:
    InportIterable() = default;
    virtual ~InportIterable() = default;

    template <typename Derived>
    class const_iterator_base {
    protected:
        using self = Derived;
        using PortIter = typename std::vector<Outport*>::const_iterator;

    public:
        explicit const_iterator_base(PortIter pIterBegin) : pIter_(pIterBegin) {}
        self& operator++() {
            ++pIter_;
            return *static_cast<Derived*>(this);
        }
        self operator++(int) {
            self i = *static_cast<Derived*>(this);
            ++pIter_;
            return i;
        }
        std::shared_ptr<const T> operator*() const {
            return static_cast<DataOutport<T>*>(*pIter_)->getData();
        }
        std::shared_ptr<const T> operator->() const {
            return static_cast<DataOutport<T>*>(*pIter_)->getData();
        }
        bool operator==(const self& rhs) const { return pIter_ == rhs.pIter_; }
        bool operator!=(const self& rhs) const { return pIter_ != rhs.pIter_; }

    protected:
        PortIter pIter_;
    };

    class const_iterator : public const_iterator_base<const_iterator> {
        using Base = const_iterator_base<const_iterator>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::shared_ptr<const T>;
        using pointer = std::shared_ptr<const T>;
        using reference = std::shared_ptr<const T>;

        const_iterator(PortIter pIterBegin) : Base(pIterBegin) {}

        std::shared_ptr<const T> operator*() const {
            return static_cast<DataOutport<T>*>(*Base::pIter_)->getData();
        }
        std::shared_ptr<const T> operator->() const {
            return static_cast<DataOutport<T>*>(*Base::pIter_)->getData();
        }
    };

    class const_iterator_port : public const_iterator_base<const_iterator_port> {
        using Base = const_iterator_base<const_iterator_port>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Outport*, std::shared_ptr<const T>>;
        using reference = std::pair<Outport*, std::shared_ptr<const T>>;
        using pointer = void;

        const_iterator_port(PortIter pIterBegin) : Base(pIterBegin) {}

        reference operator*() const {
            return reference{*Base::pIter_, static_cast<DataOutport<T>*>(*Base::pIter_)->getData()};
        }
    };

    class const_iterator_changed : public const_iterator_base<const_iterator_changed> {
        using Base = const_iterator_base<const_iterator_changed>;
        using PortIter = typename Base::PortIter;

    public:
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<bool, std::shared_ptr<const T>>;
        using reference = std::pair<bool, std::shared_ptr<const T>>;
        using pointer = void;

        const_iterator_changed(PortIter pIterBegin, const InportType& inport)
            : Base(pIterBegin), inport{inport} {}

        reference operator*() const {
            return reference{util::contains(inport.getChangedOutports(), *Base::pIter_),
                             static_cast<DataOutport<T>*>(*Base::pIter_)->getData()};
        }

        const InportType& inport;
    };

    util::iter_range<const_iterator_port> outportAndData() const {
        return util::iter_range<const_iterator_port>{
            const_iterator_port{self().getConnectedOutports().begin()},
            const_iterator_port{self().getConnectedOutports().end()}};
    }

    util::iter_range<const_iterator_changed> changedAndData() const {
        return util::iter_range<const_iterator_changed>{
            const_iterator_changed{self().getConnectedOutports().begin(), self()},
            const_iterator_changed{self().getConnectedOutports().end(), self()}};
    }

    const_iterator begin() const { return const_iterator(self().getConnectedOutports().begin()); }
    const_iterator end() const { return const_iterator(self().getConnectedOutports().end()); }

private:
    InportType& self() { return static_cast<InportType&>(*this); }
    const InportType& self() const { return static_cast<const InportType&>(*this); }
};

}  // namespace inviwo

#endif  // IVW_INPORTITERABLE_H
