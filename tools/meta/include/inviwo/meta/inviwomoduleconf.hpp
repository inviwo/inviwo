/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/meta/inviwometadefine.hpp>
#include <inviwo/meta/util.hpp>

#include <string>
#include <filesystem>

#include <fmt/format.h>

namespace inviwo::meta {

class INVIWO_META_API ModuleConf {
public:
    using path = std::filesystem::path;

    ModuleConf(const std::string& name, std::string_view org) : name_{name}, org_{org} {}
    std::string name() const { return name_; }
    std::string org() const { return org_; }
    std::string api() const { return fmt("IVW_MODULE_{uname}_API"); }
    path moduleInclude() const { return fmt("{org}{lname}/{lname}module.h"); }
    path defineInclude() const { return fmt("{org}{lname}/{lname}moduledefine.h"); }
    path registrationFile() const { return fmt("src/{lname}module.cpp"); }
    path listFile() const { return cmlists; }
    path incPath() const { return fmt("include/{org}{lname}"); }
    path srcPath() const { return fmt("src"); }
    path includePrefix() const { return fmt("{org}{lname}"); }

protected:
    const path cmlists{"CMakeLists.txt"};
    std::string fmt(std::string_view str) const {
        using namespace fmt::literals;
        return fmt::format(str, "name"_a = name_, "uname"_a = util::toUpper(name_),
                           "lname"_a = util::toLower(name_),
                           "org"_a = (org_.empty() ? "" : org_ + "/"));
    }
    std::string name_;
    std::string org_;
};

class INVIWO_META_API LegacyModuleConf : public ModuleConf {
public:
    LegacyModuleConf(const std::string& name) : ModuleConf(name, "modules") {}
    path registrationFile() const { return fmt("{lname}module.cpp"); }
    path listFile() const { return cmlists; }
    path incPath() const { return ""; }
    path srcPath() const { return ""; }
};

class INVIWO_META_API InviwoConf : public ModuleConf {
public:
    InviwoConf() : ModuleConf("Core", "inviwo") {}
    std::string api() const { return "IVW_CORE_API"; }
    path moduleInclude() const { return "inviwo/core/common/inviwocore.h"; }
    path defineInclude() const { return "inviwo/core/common/inviwocoredefine.h"; }
    path registrationFile() const { return "src/core/common/inviwocore.cpp"; }
    path listFile() const { return path{"src"} / "core" / cmlists; }
    path incPath() const { return "include/inviwo/core"; }
    path srcPath() const { return "src/core"; }
    path includePrefix() const { return "inviwo/core"; }
};

class INVIWO_META_API QtEditorConf : public ModuleConf {
public:
    QtEditorConf() : ModuleConf("QtEditor", "inviwo") {}
    std::string api() const { return "IVW_QTEDITOR_API"; }
    path moduleInclude() const { return "!!SHOULD NOT BE USED!!"; }
    path defineInclude() const { return "inviwo/qt/editor/inviwoqteditordefine.h"; }
    path registrationFile() const { return "!!SHOULD NOT BE USED!!"; }
    path listFile() const { return path{"src"} / "qt" / "editor" / cmlists; }
    path incPath() const { return "include/inviwo/qt/editor"; }
    path srcPath() const { return "src/qt/editor"; }
    path includePrefix() const { return "inviwo/qt/editor"; }
};

}  // namespace inviwo::meta
