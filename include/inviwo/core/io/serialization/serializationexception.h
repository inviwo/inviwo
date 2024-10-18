/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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
#include <inviwo/core/util/exception.h>
#include <inviwo/core/io/serialization/nodedebugger.h>

#include <string>

class TiXmlElement;

namespace ticpp {
class Element;
}
using TxElement = ticpp::Element;

namespace inviwo {

class IVW_CORE_API SerializationException : public Exception {
public:
    struct SerializationExceptionData {
        SerializationExceptionData(std::string_view k = "", std::string_view t = "",
                                   std::string_view i = "", TxElement* n = nullptr)
            : key(k), type(t), id(i), nd(n) {}

        SerializationExceptionData(std::string_view k, std::string_view t, std::string_view i,
                                   TiXmlElement* n)
            : key(k), type(t), id(i), nd(n) {}
        std::string key;
        std::string type;
        std::string id;
        NodeDebugger nd;
    };

    SerializationException(std::string_view message = "",
                           ExceptionContext context = ExceptionContext(), std::string_view key = "",
                           std::string_view type = "", std::string_view id = "",
                           TxElement* n = nullptr);
    SerializationException(std::string_view message, ExceptionContext context, std::string_view key,
                           std::string_view type, std::string_view id, TiXmlElement* n);
    SerializationException(std::string_view format, fmt::format_args&& args,
                           ExceptionContext context);
    template <typename... Args>
    SerializationException(ExceptionContext context, std::string_view format, Args&&... args)
        : Exception{format, fmt::make_format_args(std::forward<Args>(args)...),
                    std::move(context)} {}

    virtual ~SerializationException() noexcept = default;

    virtual const std::string& getKey() const noexcept;
    virtual const std::string& getType() const noexcept;
    virtual const std::string& getId() const noexcept;
    virtual const SerializationExceptionData& getData() const noexcept;

private:
    SerializationExceptionData data_;
};

}  // namespace inviwo
