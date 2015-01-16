/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IWE_ACTION_H
#define IWE_ACTION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/ivwserializable.h>

namespace inviwo {

class Event;

class IVW_CORE_API Action {
public:
    Action();
    template <typename T>
    Action(T* obj, void (T::*m)(Event*));
    Action(const Action& rhs);
    Action& operator=(const Action& that);
    virtual Action* clone() const;
    virtual ~Action();

    virtual void invoke(Event* event);

    template <typename T>
    void setAction(T* o, void (T::*m)(Event*)) {
        if (callback_) delete callback_;
        callback_ = new ActionCallback<T>(o, m);
    }

protected:
    class BaseActionCallback {
    public:
        BaseActionCallback() {}
        virtual void invoke(Event*) = 0;
        virtual ~BaseActionCallback(){};
    };

    template <typename T>
    class ActionCallback : public BaseActionCallback {
    public:
        typedef void (T::*fPointer)(Event*);
        ActionCallback(T* obj, fPointer function)
            : BaseActionCallback(), function_(function), obj_(obj) {}
        virtual ~ActionCallback() {}

        virtual void invoke(Event* event) { (*obj_.*function_)(event); }

    private:
        const fPointer function_;
        T* obj_;
    };

    BaseActionCallback* callback_;
};

template <typename T>
Action::Action(T* obj, void (T::*m)(Event*))
    : callback_(new ActionCallback<T>(obj, m)) {}

}  // namespace

#endif  // IWE_ACTION_H