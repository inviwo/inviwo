/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/demangle.h>

#include <string>

namespace inviwo {

class SingletonException : public Exception {
public:
    using Exception::Exception;
};
/**
 * T must have a static T* instance_ member variable.
 */
template <class T>
class Singleton {
public:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    void operator=(const Singleton&) = delete;

    static void init() {
        if (T::instance_) {
            throw SingletonException(IVW_CONTEXT_CUSTOM("Singleton"),
                                     "{} singleton already initialized", name());
        }
        T::instance_ = new T();
        if (!T::instance_) {
            throw SingletonException(IVW_CONTEXT_CUSTOM("Singleton"),
                                     "Was not able to initialize {} singleton", name());
        }
    };

    static void init(T* instance) {
        if (T::instance_) {
            throw SingletonException(IVW_CONTEXT_CUSTOM("Singleton"),
                                     "{} singleton already initialized", name());
        }
        if (!instance) {
            throw SingletonException(IVW_CONTEXT_CUSTOM("Singleton"), "Null pointer passed");
        }
        T::instance_ = instance;
    };

    static T* getPtr() {
        if (!T::instance_) {
            throw SingletonException(
                IVW_CONTEXT_CUSTOM("Singleton"),
                "{} Singleton not initialized. Ensure that init() is called in a thread-safe "
                "environment.",
                name());
        }
        return T::instance_;
    };

    static void deleteInstance() {
        delete T::instance_;
        T::instance_ = nullptr;
    };

    static bool isInitialized() { return T::instance_ != nullptr; }

    virtual ~Singleton() {
        if (this == T::instance_) {
            T::instance_ = nullptr;
        }
    };

private:
    static std::string name() { return util::parseTypeIdName(typeid(T).name()); }
};

}  // namespace inviwo
