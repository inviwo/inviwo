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

#ifndef IVW_RAIIUTILS_H
#define IVW_RAIIUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <functional>

namespace inviwo {

namespace util {

/**
 * \class KeepTrueWhileInScope
 * \brief sets the given bool variable to true and restores its state when leaving the scope
 *
 * An instance of this class will set a given bool variable to true upon construction.
 * It restores the previous state of the bool variable when leaving the current scope.
 */
class IVW_CORE_API KeepTrueWhileInScope {
public:
    KeepTrueWhileInScope(bool* b) : variable_(b), prevValue_(b ? *b : false) {
        if (variable_) {
            (*variable_) = true;
        }
    }
    ~KeepTrueWhileInScope() {
        if (variable_) (*variable_) = prevValue_;
    }

private:
    bool* variable_;
    bool prevValue_;
};

/**
 * \class OnScopeExit
 * \brief calls the given function when leaving the current scope
 *
 * An instance of this class will call the provided action, i.e. a void function, when leaving the
 * current scope. The action will also be called in case the constructor fails.
 *
 * The action can be changed at any time by calling setAction() or release().
 */
struct IVW_CORE_API OnScopeExit {
    typedef std::function<void(void)> ExitAction;

    OnScopeExit() = delete;
    OnScopeExit(OnScopeExit const&) = delete;
    OnScopeExit& operator=(OnScopeExit const& that) = delete;

    OnScopeExit(OnScopeExit&& rhs) : action_(std::move(rhs.action_)) { rhs.action_ = nullptr; };
    OnScopeExit& operator=(OnScopeExit&& that) {
        if (this != &that) {
            action_ = nullptr;
            std::swap(action_, that.action_);
        }
        return *this;
    }

    OnScopeExit(ExitAction action) try : action_(action) {
    } catch (...) {
        action();
    }
    ~OnScopeExit() {
        if (action_) action_();
    }

    void setAction(ExitAction action = nullptr) { action_ = action; }
    void release() { setAction(); }

private:
#include <warn/push>
#include <warn/ignore/dll-interface>
    ExitAction action_;
#include <warn/pop>
};

template <typename T>
void SetValue(T& t, T value) {
    t = value;
}

template <typename T>
OnScopeExit::ExitAction RevertValue(T& t) {
    return std::bind(SetValue<T>, std::ref(t), t);
}

}  // namespace util

}  // namespace inviwo

#endif  // IVW_RAIIUTILS_H
