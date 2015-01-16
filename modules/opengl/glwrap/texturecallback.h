/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_TEXTURECALLBACK_H
#define IVW_TEXTURECALLBACK_H

#include <inviwo/core/common/inviwo.h>

namespace inviwo {

class Texture;

class BaseTextureCallBack {
public:
    BaseTextureCallBack() {}
    virtual ~BaseTextureCallBack() {}
    virtual void invoke(Texture*) const=0;
};

template <typename T>
class MemberFunctionTextureCallback : public BaseTextureCallBack {
public:
    typedef void (T::*fPointerTexture)(Texture*);

    MemberFunctionTextureCallback(T* obj, fPointerTexture functionPtr)
        : functionPtr_(functionPtr)
        , obj_(obj) {}

    virtual ~MemberFunctionTextureCallback() {}

    void invoke(Texture* p) const {
        if (functionPtr_)(*obj_.*functionPtr_)(p);
    }

private:
    fPointerTexture functionPtr_;
    T* obj_;
};

class TextureCallback {
public:
    TextureCallback() : callBack_(0) {}
    virtual ~TextureCallback() {
        delete callBack_;
    }

    void invoke(Texture* p) const {
        if (callBack_)
            callBack_->invoke(p);
    }

    template <typename T>
    void addMemberFunction(T* o, void (T::*m)(Texture*)) {
        if(callBack_)
            delete callBack_;
        callBack_ = new MemberFunctionTextureCallback<T>(o,m);
    }

private:
    BaseTextureCallBack* callBack_;
};


} // namespace

#endif // IVW_TEXTURECALLBACK_H
