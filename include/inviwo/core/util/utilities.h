/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/util/stdextensions.h>
#include <string_view>
#include <filesystem>

namespace inviwo {

class ProcessorNetwork;

class Property;
class ProcessorWidget;

namespace util {

IVW_CORE_API void saveNetwork(ProcessorNetwork* network, std::string_view filename);

IVW_CORE_API void saveAllCanvases(ProcessorNetwork* network, const std::filesystem::path& dir,
                                  std::string_view name = "UPN", std::string_view ext = ".png",
                                  bool onlyActiveCanvases = false);

[[deprecated("use util::validateIdentifier")]] IVW_CORE_API bool isValidIdentifierCharacter(
    char c, std::string_view extra = "");

/**
 * Validate the given @p identifier.  If @p identifier is invalid, throws an Exception with the
 * given exception context @p context and identifier @p type for a detailed error message.
 *
 * Valid identifiers follow the C++ and Python variable naming rules. That is
 *   + case sensitive
 *   + start with letter or underscore
 *   + contain letters, numbers, and underscores
 *   + length > 0
 *
 * Matching regex: [a-zA-Z_][a-zA-Z0-9_]*
 *
 * @param identifier  name to be validated
 * @param type        identifier type, only used in case of exception
 * @param context     used when throwing an exception
 * @throws Exception if identifier is not valid
 *
 * @see stripIdentifier
 */
IVW_CORE_API void validateIdentifier(std::string_view identifier, std::string_view type,
                                     SourceContext context = std::source_location::current());

/**
 * Utility to augment an identifier with a number to make it unique. Will add an increasing number
 * to the end of the given identifier until the isUnique test returns true.
 * Example for a processor identifier:
 *     auto uniqueIdentifier = util::findUniqueIdentifier(
 *         startIdentifier,
 *         [&](std::string_view id) {
 *             return processorNetwork->getProcessorByIdentifier(id) == nullptr; },
 *         ""
 *     );
 */
IVW_CORE_API std::string findUniqueIdentifier(std::string_view identifier,
                                              std::function<bool(std::string_view)> isUnique,
                                              std::string_view sep = " ");

[[deprecated("use util::stripIdentifier")]] IVW_CORE_API std::string cleanIdentifier(
    std::string_view identifier, std::string_view extra = "");

/**
 * \brief Removes inviwo-module from module library file name.
 * Turns "/path/to/inviwo-module-yourmodule.dll" into "yourmodule".
 * Returns filename without extension if inviwo-module was not found.
 *
 * @param  filePath Path to module file
 * @return name of the module
 */
IVW_CORE_API std::string stripModuleFileNameDecoration(const std::filesystem::path& filePath);

/**
 * Strip all invalid characters from the given @p identifier. If the resulting identifier is empty,
 * a single underscore <tt>_</tt> is returned.
 *
 * Valid identifiers follow the C++ and Python variable naming rules. That is
 *   + case sensitive
 *   + start with letter or underscore
 *   + contain letters, numbers, and underscores
 *   + length > 0
 *
 * Matching regex: [a-zA-Z_][a-zA-Z0-9_]*
 *
 * @param identifier  name to be stripped and validated
 * @returns valid identifier
 *
 * @see validateIdentifier
 */
IVW_CORE_API std::string stripIdentifier(std::string_view identifier);

namespace detail {

struct IVW_CORE_API Shower {
    void operator()(Property& p);
    void operator()(ProcessorWidget& p);
};

struct IVW_CORE_API Hideer {
    void operator()(Property& p);
    void operator()(ProcessorWidget& p);
};

}  // namespace detail

template <typename... Args>
void show(Args&&... args) {
    for_each_argument(detail::Shower{}, std::forward<Args>(args)...);
}

template <typename... Args>
void hide(Args&&... args) {
    for_each_argument(detail::Hideer{}, std::forward<Args>(args)...);
}

}  // namespace util

}  // namespace inviwo
