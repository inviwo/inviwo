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

#ifndef IVW_CALLBACK_H
#define IVW_CALLBACK_H

#include <inviwo/core/common/inviwo.h>
#include <functional>

namespace inviwo {

class BaseCallBack {
public:
    BaseCallBack(){};
    virtual ~BaseCallBack(){};
    virtual void invoke() const = 0;
    virtual bool involvesObject(void*) const { return false; }
};

template <typename T>
class MemberFunctionCallBack : public BaseCallBack {
public:
    typedef void (T::*fPointerType)();

    MemberFunctionCallBack(T* obj, fPointerType func) : func_(func), obj_(obj) {}
    virtual ~MemberFunctionCallBack() {}
    virtual void invoke() const  override {
        if (func_) (*obj_.*func_)();
    }

    bool involvesObject(void* ptr) const override { return static_cast<void*>(obj_) == ptr; }

private:
    fPointerType func_;
    T* obj_;
};

class LambdaCallBack : public BaseCallBack {
public:
    LambdaCallBack(std::function<void()> func) : func_{func} {}
    virtual ~LambdaCallBack(){};
    virtual void invoke() const {
        if (func_) func_();
    }

private:
    std::function<void()> func_;
};

// Example usage
// CallBackList list;
// list.addMemberFunction(&myClassObject, &MYClassObject::myFunction);

class CallBackList {
public:
    CallBackList() {}
    virtual ~CallBackList() {
        clear();
    }

    void invokeAll() const {
        for (BaseCallBack* cb : callBackList_) cb->invoke();
    }

    template <typename T>
    const BaseCallBack* addMemberFunction(T* o, void (T::*m)()) {
        MemberFunctionCallBack<T>* callBack = new MemberFunctionCallBack<T>(o, m);
        callBackList_.push_back(callBack);
        return callBack;
    }
    const BaseCallBack* addLambdaCallback(std::function<void()> lambda) {
        LambdaCallBack* callBack = new LambdaCallBack(lambda);
        callBackList_.push_back(callBack);
        return callBack;
    }

    /** 
     * \brief Deletes and removes callback if the callback was added before.
     * 
     * @note Callback pointer is invalid after calling remove if true is returned.
     * @param callback Callback to be removed.
     * @return bool True if removed, false otherwise.
     */
    bool remove(const BaseCallBack* callback) {
        auto it = std::find(callBackList_.begin(), callBackList_.end(), callback);
        if (it != callBackList_.end()) {
            delete *it;
            callBackList_.erase(it);
            return true;
        }
        return false;
    }
    /** 
     * \brief Deletes and removes all added callbacks.
     */
    void clear() {
        for (BaseCallBack* cb : callBackList_) delete cb;
        callBackList_.clear();
    }

    /** 
     * \brief Delete and remove all callbacks associated with the object.
     * 
     */
    template <typename T>
    void removeMemberFunction(T* o) {
        callBackList_.erase(std::remove_if(callBackList_.begin(), callBackList_.end(),
                                           [o](BaseCallBack* cb) -> bool {
                                if (cb->involvesObject(o)) {
                                    delete cb;
                                    return true;
                                } else {
                                    return false;
                                }
                            }),
                            callBackList_.end());
    }

private:
    std::vector<BaseCallBack*> callBackList_;
};

// SingleCallBack removed use std::function instead.

}  // namespace

#endif  // IVW_CALLBACK_H
