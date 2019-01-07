/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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

namespace inviwo {

template <typename T, bool Flat>
class InportIterable {
public:
    InportIterable(std::vector<Outport*>* connections) : connections_(connections) {}
    virtual ~InportIterable() = default;

    class const_iterator {
        using self = const_iterator;
        using PortIter = typename std::vector<Outport*>::const_iterator;
        using DataIter = typename OutportIterable<T>::const_iterator;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = std::shared_ptr<const T>;
        using reference = std::shared_ptr<const T>;
        using iterator_category = std::forward_iterator_tag;

        explicit const_iterator(PortIter pIterBegin, PortIter pIterEnd)
            : pIter_(pIterBegin), pEnd_(pIterEnd), dIter_(), dEnd_() {
            if (pIter_ != pEnd_) {
                auto ptr = dynamic_cast<OutportIterable<T>*>(*pIter_);
                dIter_ = ptr->begin();
                dEnd_ = ptr->end();

                while (dIter_ == dEnd_ && pIter_ != pEnd_) {
                    pIter_++;
                    if (pIter_ != pEnd_) {
                        ptr = dynamic_cast<OutportIterable<T>*>(*pIter_);
                        dIter_ = ptr->begin();
                        dEnd_ = ptr->end();
                    } else {
                        dIter_ = dEnd_;
                    }
                }
            }
        }
        self& operator++() {
            incrementIter();
            return *this;
        }
        self operator++(int) {
            self i = *this;
            incrementIter();
            return i;
        }

        std::shared_ptr<const T> operator*() const { return *dIter_; }
        std::shared_ptr<const T> operator->() const { return *dIter_; }

        bool operator==(const self& rhs) const {
            return pIter_ == rhs.pIter_ && dIter_ == rhs.dIter_;
        }
        bool operator!=(const self& rhs) const {
            return pIter_ != rhs.pIter_ && dIter_ != rhs.dIter_;
        }

    private:
        void incrementIter() {
            dIter_++;
            while (dIter_ == dEnd_ && pIter_ != pEnd_) {
                pIter_++;
                if (pIter_ != pEnd_) {
                    auto ptr = dynamic_cast<OutportIterable<T>*>(*pIter_);
                    dIter_ = ptr->begin();
                    dEnd_ = ptr->end();
                } else {
                    dIter_ = dEnd_;
                }
            }
        }

        PortIter pIter_;
        PortIter pEnd_;
        DataIter dIter_;
        DataIter dEnd_;
    };

    const_iterator begin() const {
        return const_iterator(connections_->begin(), connections_->end());
    }
    const_iterator end() const { return const_iterator(connections_->end(), connections_->end()); }

private:
    std::vector<Outport*>* connections_;
};

// Specialization for non flat case.
template <typename T>
class InportIterable<T, false> {
public:
    InportIterable(std::vector<Outport*>* connections) : connections_(connections) {}
    virtual ~InportIterable() = default;

    class const_iterator {
        using self = const_iterator;
        using PortIter = typename std::vector<Outport*>::const_iterator;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = std::shared_ptr<const T>;
        using reference = std::shared_ptr<const T>;
        using iterator_category = std::forward_iterator_tag;

        explicit const_iterator(PortIter pIterBegin) : pIter_(pIterBegin) {}
        self& operator++() {
            pIter_++;
            return *this;
        }
        self operator++(int) {
            self i = *this;
            pIter_++;
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

    private:
        PortIter pIter_;
    };

    const_iterator begin() const { return const_iterator(connections_->begin()); }
    const_iterator end() const { return const_iterator(connections_->end()); }

private:
    std::vector<Outport*>* connections_;
};

}  // namespace inviwo

#endif  // IVW_INPORTITERABLE_H
