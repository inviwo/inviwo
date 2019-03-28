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

#ifndef IVW_CALLBACK_H
#define IVW_CALLBACK_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/dispatcher.h>

#include <functional>

namespace inviwo {

using BaseCallBack = std::function<void()>;
// clang-format off
/**
 * Example usage
 * CallBackList list;
 * list.addMemberFunction(&myClassObject, &MYClassObject::myFunction);
 * or
 * list.addLambdaCallback([](){});
 */
class CallBackList {
public:
    CallBackList() = default;
    virtual ~CallBackList() = default;

    void startBlockingCallbacks() { ++callbacksBlocked_; }
    void stopBlockingCallbacks() { --callbacksBlocked_; }

    void invokeAll() const {
        if (callbacksBlocked_ == 0) dispatcher_.invoke();
    }


    template <typename T>
    [[deprecated("was declared deprecated. Use `addLambdaCallback(std::function<void()>)` instead")]]
    const BaseCallBack* addMemberFunction(T* o, void (T::*m)()) {
        auto cb = dispatcher_.add([o, m](){if (m) (*o.*m)();});
        callBackList_.push_back(cb);
        objMap_[static_cast<void*>(o)].push_back(cb.get());
        return cb.get();
    }

    const BaseCallBack* addLambdaCallback(std::function<void()> lambda) {
        auto cb = dispatcher_.add(std::move(lambda));
        callBackList_.push_back(cb);
        return cb.get();
    }
    std::shared_ptr<std::function<void()>> addLambdaCallbackRaii(std::function<void()> lambda) {
        return dispatcher_.add(std::move(lambda));
    }

    /**
     * \brief Removes callback if the callback was added before.
     * @param callback Callback to be removed.
     * @return bool True if removed, false otherwise.
     */
    bool remove(const BaseCallBack* callback) {
        return util::erase_remove_if(callBackList_,
                                     [&](const std::shared_ptr<std::function<void()>>& ptr) {
                                         return ptr.get() == callback;
                                     }) > 0;
    }
    /**
     * \brief Removes all added callbacks.
     */
    void clear() {
        callBackList_.clear();
        objMap_.clear();
    }

    /**
     * \brief Remove all callbacks associated with the object.
     */
    template <typename T>
    [[deprecated("was declared deprecated. Use `remove(const BaseCallBack* callback)` instead")]]
    void removeMemberFunction(T* o) {
        auto it = objMap_.find(static_cast<void*>(o));
        if (it != objMap_.end()) {
            for (auto ptr : it->second) {
                remove(ptr);
            }
            objMap_.erase(it);
        }
    }


private:
    int callbacksBlocked_{0};
    std::vector<std::shared_ptr<std::function<void()>>> callBackList_;
    std::unordered_map<void*, std::vector<const BaseCallBack*>> objMap_;
    mutable Dispatcher<void()> dispatcher_;
};

// clang-format on
}  // namespace inviwo

#endif  // IVW_CALLBACK_H
