/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

class IVW_CORE_API KeepTrueWhileInScope {
public:
    KeepTrueWhileInScope(bool* b) : variable_(b) { (*variable_) = true; }
    ~KeepTrueWhileInScope() { (*variable_) = false; }

private:
    bool* variable_;
};

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
    ExitAction action_;
};

template <typename T>
void SetValue(T& t, T value) {
    t = value;
}

template <typename T>
OnScopeExit::ExitAction RevertValue(T& t) {
    return std::bind(SetValue<T>, std::ref(t), t);
}

}  // namespace

}  // namespace

#endif  // IVW_RAIIUTILS_H
