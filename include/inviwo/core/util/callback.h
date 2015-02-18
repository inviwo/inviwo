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

namespace inviwo {

class BaseCallBack {
public:
    BaseCallBack() {};
    virtual ~BaseCallBack() {};
    virtual void invoke() const=0;
};

template <typename T>
class MemberFunctionCallBack : public BaseCallBack {
public:
    typedef void (T::*fPointerType)();
    virtual ~MemberFunctionCallBack() {}
    MemberFunctionCallBack(T* obj, fPointerType functionPtr)
        : functionPtr_(functionPtr)
        , obj_(obj) {}

    virtual void invoke() const {
        if (functionPtr_)(*obj_.*functionPtr_)();
    }

private:
    fPointerType functionPtr_;
    T* obj_;
};


// Example usage
// CallBackList cbList;
// cbList.addMemberFunction(&myClassObject, &MYClassObject::myFunction);
class CallBackList {
public:
    CallBackList() {}
    virtual ~CallBackList() {
        std::map<void*, BaseCallBack*>::iterator it;

        for (it = callBackList_.begin(); it != callBackList_.end(); ++it) delete it->second;

        callBackList_.clear();
    }

    void invokeAll() const {
        std::map<void*, BaseCallBack*>::const_iterator it;
        for (it = callBackList_.begin(); it != callBackList_.end(); ++it) it->second->invoke();
    }

    template <typename T>
    void addMemberFunction(T* o, void (T::*m)()) {
        std::map<void*, BaseCallBack*>::iterator it = callBackList_.find(o);

        if (it != callBackList_.end()) {
            delete it->second;
            it->second = new MemberFunctionCallBack<T>(o, m);
        } else {
            callBackList_[o] = new MemberFunctionCallBack<T>(o, m);
        }
    }

    template <typename T>
    void removeMemberFunction(T* o) {
        std::map<void*, BaseCallBack*>::iterator it = callBackList_.find(o);

        if (it != callBackList_.end()) {
            delete it->second;
            callBackList_.erase(it);
        }
    }

private:
    std::map<void*, BaseCallBack*> callBackList_;
};

class SingleCallBack {
public:
    SingleCallBack() : callBack_(0) {}

    virtual ~SingleCallBack() {
        delete callBack_;
    }

    void invoke() const {
        if (callBack_)
            callBack_->invoke();
    }

    template <typename T>
    void addMemberFunction(T* o, void (T::*m)()) {
        if (callBack_)
            delete callBack_;

        callBack_ = new MemberFunctionCallBack<T>(o,m);
    }

private:
    BaseCallBack* callBack_;
};


} // namespace

#endif // IVW_CALLBACK_H
