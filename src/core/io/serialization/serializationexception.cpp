/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <inviwo/core/io/serialization/serializationexception.h>

#include <fmt/format.h>

namespace inviwo {

SerializationException::SerializationException(std::string message, ExceptionContext context,
                                               std::string key, std::string type, std::string id,
                                               TxElement* node)
    : Exception(message, context), data_(key, type, id, node) {}

const std::string& SerializationException::getKey() const noexcept { return data_.key; }

const std::string& SerializationException::getType() const noexcept { return data_.type; }

const std::string& SerializationException::getId() const noexcept { return data_.id; }

const SerializationException::SerializationExceptionData& SerializationException::getData() const
    noexcept {
    return data_;
}

std::string util::formatSerializationError(const std::string& name, const std::string& src,
                                           const std::string& dst, const std::string& err) {
    return fmt::format(
        "Could not create {name} from:\n"
        "   {src}\n"
        "to\n"
        "   {dst}\n"
        "{err}",
        fmt::arg("name", name), fmt::arg("src", src), fmt::arg("dst", dst), fmt::arg("err", err));
}

}  // namespace inviwo
