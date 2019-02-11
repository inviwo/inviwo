/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_MODULEUTILS_H
#define IVW_MODULEUTILS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/exception.h>

namespace inviwo {

namespace module {

/**
 * \brief return the path for a specific type located within the requested module
 *
 * @param identifier   name of the module
 * @param pathType     type of the requested path
 * @return subdirectory of the module matching the type
 */
IVW_CORE_API std::string getModulePath(const std::string &identifier, ModulePath pathType);

/**
 * \brief return the path for a specific type located within the requested module of type T
 *
 * @param pathType     type of the requested path
 * @return subdirectory of the module matching the type
 */
template <typename T>
std::string getModulePath(ModulePath pathType);

template <typename T>
std::string getModulePath(ModulePath pathType) {
    std::string path;
    if (auto m = InviwoApplication::getPtr()->getModuleByType<T>()) {
        path = m->getPath(pathType);
        if (path.empty() || path == m->getPath()) {
            throw Exception("Could not locate module path for specified path type");
        }
    } else {
        throw Exception("Could not locate module");
    }
    return path;
}

}  // namespace module

}  // namespace inviwo

#endif  // IVW_MODULEUTILS_H
