/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_MODULECALLBACK_H
#define IVW_MODULECALLBACK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class IVW_CORE_API CallbackWithSingleArgument {
public:
    CallbackWithSingleArgument() {}
    virtual ~CallbackWithSingleArgument() {}
    virtual void invoke(void*) const = 0;
};

template <typename T, typename U>
class BaseModuleCallback : public CallbackWithSingleArgument {
public:
    using fPointer = void (T::*)(U*);

    BaseModuleCallback(T* obj, fPointer functionPtr) : functionPtr_(functionPtr), obj_(obj) {}

    virtual ~BaseModuleCallback() = default;

    void invoke(U* argument) const {
        if (!argument) {
            throw Exception("Callback function argument does not match", IVW_CONTEXT);
        }

        if (functionPtr_) {
            (*obj_.*functionPtr_)(argument);
        }
    }

    virtual void invoke(void* p) const { BaseModuleCallback::invoke(static_cast<U*>(p)); }

private:
    fPointer functionPtr_;
    T* obj_;
};

/**
 *	A callback for use in ModuleCallbackAction
 */
class IVW_CORE_API ModuleCallback {
public:
    ModuleCallback() : callBack_{} {}
    virtual ~ModuleCallback() = default;

    template <typename U>
    void invoke(U* p) const {
        if (callBack_) {
            callBack_->invoke(p);
        }
    }
    template <typename T, typename U>
    void addMemberFunction(T* o, void (T::*m)(U*)) {
        callBack_ = std::make_unique<BaseModuleCallback<T, U>>(o, m);
    }

private:
    std::unique_ptr<CallbackWithSingleArgument> callBack_;
};

}  // namespace inviwo

#endif  // IVW_MODULECALLBACK_H
