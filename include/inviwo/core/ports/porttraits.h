/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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
#include <inviwo/core/ports/port.h>
#include <inviwo/core/util/introspection.h>

#include <string>

namespace inviwo {

/**
 * \class PortTraits
 * \brief A traits class for getting the class identifier from a Port.
 * This provides a customization point if one wants to generate the class identifier dynamically,
 * by specializing the traits for your kind of Port:
 * \code{.cpp}
 *     template <typename T>
 *     struct PortTraits<MyPort<T>> {
 *        static constexpr std::string_view classIdentifier() {
 *           return generateMyPortClassIdentifier<T>();
 *        }
 *     };
 * \endcode
 * The default behavior returns the static member "classIdentifier";
 */
template <typename T, typename = void>
struct PortTraits {
    /**
     * The Class Identifier has to be globally unique. Use a reverse DNS naming scheme.
     * Example: "org.someorg.myporttype"
     * The default implementation will look for a static std::string member T::classIdentifier.
     * In case it is not found an empty string will be returned. An empty class identifier will be
     * considered an error in various factories.
     */
    static constexpr std::string_view classIdentifier() { return util::classIdentifier<T>(); }
};

}  // namespace inviwo
