/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_DESERIALIZATIONERRORHANDLER_H
#define IVW_DESERIALIZATIONERRORHANDLER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/serializationexception.h>

namespace inviwo {

class IVW_CORE_API BaseDeserializationErrorHandler {
public:
    BaseDeserializationErrorHandler();
    virtual ~BaseDeserializationErrorHandler();

    virtual void handleError(SerializationException&) = 0;
    virtual std::string getKey() = 0;
};

template <typename T>
class DeserializationErrorHandler : public BaseDeserializationErrorHandler {
public:
    typedef void (T::*Callback)(SerializationException&);

    DeserializationErrorHandler(std::string type, T* obj, Callback callback);
    virtual ~DeserializationErrorHandler() {}

    virtual void handleError(SerializationException&);
    virtual std::string getKey();

private:
    std::string key_;
    T* obj_;
    Callback callback_;
};

template <typename T>
DeserializationErrorHandler<T>::DeserializationErrorHandler(std::string type, T* obj,
                                                            Callback callback)
    : key_(type), obj_(obj), callback_(callback) {}

template <typename T>
std::string DeserializationErrorHandler<T>::getKey() {
    return key_;
}

template <typename T>
void inviwo::DeserializationErrorHandler<T>::handleError(SerializationException& e) {
    (*obj_.*callback_)(e);
}

}  // namespace

#endif  // IVW_DESERIALIZATIONERRORHANDLER_H
