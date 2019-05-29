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

#ifndef IVW_SINGLETON_H
#define IVW_SINGLETON_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/stringconversion.h>

#include <sstream>
#include <vector>

namespace inviwo {

class SingletonException : public Exception {
public:
    SingletonException(const std::string& message = "",
                       ExceptionContext context = ExceptionContext())
        : Exception(message, context) {}
};

/**
 * T must have a static T* instance_ member variable.
 */
template <class T>
class Singleton {
public:
    Singleton<T>() = default;
    Singleton<T>(Singleton<T> const&) = delete;
    void operator=(Singleton<T> const&) = delete;

    static void init() {
        if (T::instance_) {
            throw SingletonException(name() + " Singleton already initialized",
                                     IVW_CONTEXT_CUSTOM("Singleton"));
        }
        T::instance_ = util::defaultConstructType<T>();
        if (!T::instance_) {
            throw SingletonException("Was not able to initialize " + name() + "singleton",
                                     IVW_CONTEXT_CUSTOM("Singleton"));
        }
    };

    static void init(T* instance) {
        if (T::instance_) {
            throw SingletonException(name() + " Singleton already initialized",
                                     IVW_CONTEXT_CUSTOM("Singleton"));
        }
        if (!instance) {
            throw SingletonException("Null pointer passed", IVW_CONTEXT_CUSTOM("Singleton"));
        }
        T::instance_ = instance;
    };

    static T* getPtr() {
        if (!T::instance_) {
            throw SingletonException(
                name() +
                    " Singleton not initialized. Ensure that init() is called in a thread-safe "
                    "environment. ",
                IVW_CONTEXT_CUSTOM("Singleton"));
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
    static std::string name() { return parseTypeIdName(typeid(T).name()); }
};

}  // namespace inviwo

#endif  // IVW_SINGLETON_H
