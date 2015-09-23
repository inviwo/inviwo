/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_CLONEABLEPTR_H
#define IVW_CLONEABLEPTR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace util {
/**
 * \class cloneable_ptr
 * A resource handle for classes that should be cloned on copy and assignment
 */
template <typename T>
class cloneable_ptr { 
public:
    cloneable_ptr() = default;
    cloneable_ptr(T* ptr);
    cloneable_ptr(std::unique_ptr<T>&& ptr);
    cloneable_ptr(const cloneable_ptr<T>& rhs);
    cloneable_ptr<T>& operator=(const cloneable_ptr<T>& that);

    cloneable_ptr(cloneable_ptr<T>&& rhs);
    cloneable_ptr<T>& operator=(cloneable_ptr<T>&& that);

    ~cloneable_ptr() = default;

    void reset(T* ptr = nullptr);
    T* get() const;

    typename std::add_lvalue_reference<T>::type operator*() const;
    T* operator->() const;
    explicit operator bool() const;


private:
    std::unique_ptr<T> ptr_;
};

template <typename T>
cloneable_ptr<T>::cloneable_ptr(T* ptr) : ptr_(ptr) {}

template <typename T>
cloneable_ptr<T>& cloneable_ptr<T>::operator=(const cloneable_ptr<T>& that) {
    if (this != &that) {
        ptr_.reset(that.ptr_? that.ptr_->clone() : nullptr);
    }
    return *this;
}
template <typename T>
cloneable_ptr<T>::cloneable_ptr(cloneable_ptr<T>&& rhs) : ptr_(std::move(rhs.ptr_)) {}

template <typename T>
cloneable_ptr<T>& cloneable_ptr<T>::operator=(cloneable_ptr<T>&& that) {
    ptr_ = std::move(that.ptr_);
}

template <typename T>
cloneable_ptr<T>::cloneable_ptr(const cloneable_ptr<T>& rhs) : 
    ptr_(rhs.ptr_? rhs.ptr_->clone() : nullptr) {}

template <typename T>
cloneable_ptr<T>::cloneable_ptr(std::unique_ptr<T>&& ptr) : ptr_(std::move(ptr)) {}

template <typename T>
T* cloneable_ptr<T>::operator->() const {
    return ptr_.operator->();
}

template <typename T>
typename std::add_lvalue_reference<T>::type cloneable_ptr<T>::operator*() const {
    return ptr_.operator*();
}

template <typename T>
inviwo::util::cloneable_ptr<T>::operator bool() const {
    return ptr_.operator bool();
}

template <typename T>
void inviwo::util::cloneable_ptr<T>::reset(T* ptr) {
    ptr_.reset(ptr);
}


template <typename T>
T* inviwo::util::cloneable_ptr<T>::get() const {
    return ptr_.get();
}


} // namespace

} // namespace

#endif // IVW_CLONEABLEPTR_H

