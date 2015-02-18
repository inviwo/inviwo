/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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
    virtual void invoke(const void*) const=0;
};

template <typename T, typename U>
class BaseModuleCallback  : public CallbackWithSingleArgument {
public:
    typedef void (T::*fPointer)(const U*);

    BaseModuleCallback(T* obj, fPointer functionPtr)
        : functionPtr_(functionPtr)
        , obj_(obj) {}

    virtual ~BaseModuleCallback() {}

    void invoke(const U* argument) const {
        if (!argument) {
            LogInfo("Callback function argument does not match");
            return;
        }

        if (functionPtr_)(*obj_.*functionPtr_)(argument);
    }

    virtual void invoke(const void* p) const {
        const U* argument = static_cast<const U*>(p);
        BaseModuleCallback::invoke(argument);
    }

private:
    fPointer functionPtr_;
    T* obj_;
};

class IVW_CORE_API ModuleCallback {
public:
    ModuleCallback() : callBack_(0) {}
    virtual ~ModuleCallback() {
        delete callBack_;
    };

    template <typename U>
    void invoke(const U* p) const {
        if (callBack_)
            callBack_->invoke(p);
    }

    template <typename T, typename U>
    void addMemberFunction(T* o, void (T::*m)(const U*)) {
        if (callBack_) delete callBack_;

        callBack_ = new BaseModuleCallback<T,U>(o,m);
    }

private:
    CallbackWithSingleArgument* callBack_;
};


} // namespace

#endif // IVW_MODULECALLBACK_H
