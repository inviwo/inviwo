/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/util/fileextension.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>

namespace inviwo::util {

inline void updateReaderFromFile(const FileProperty& filePath,
                                 OptionProperty<FileExtension>& extensions) {
    if (extensions.empty()) return;

    if ((filePath.getSelectedExtension() == FileExtension::all() &&
         !extensions.getSelectedValue().matches(filePath)) ||
        filePath.getSelectedExtension().empty()) {
        const auto& opts = extensions.getOptions();
        const auto it =
            std::ranges::find_if(opts, [&](const OptionPropertyOption<FileExtension>& opt) {
                return opt.value_.matches(filePath.get());
            });
        extensions.setSelectedValue(it != opts.end() ? it->value_ : FileExtension{});
    } else {
        extensions.setSelectedValue(filePath.getSelectedExtension());
    }
}

template <typename... Types>
void updateReaderFromFileAndType(const FileProperty& filePath,
                                 OptionProperty<FileExtension>& extensions,
                                 const DataReaderFactory& rf) {
    if (extensions.empty()) return;

    if ((filePath.getSelectedExtension() == FileExtension::all() &&
         !extensions.getSelectedValue().matches(filePath)) ||
        filePath.getSelectedExtension().empty()) {

        // We look for matching FileExtensions for each type in order
        const auto& exts = extensions.getOptions();

        const auto match = [&]<typename T>() {
            return std::ranges::find_if(exts, [&](const OptionPropertyOption<FileExtension>& opt) {
                return opt.value_.matches(filePath.get()) &&
                       rf.hasReaderForTypeAndExtension<T>(opt.value_);
            });
        };

        for (auto& it : {match.template operator()<Types>()...}) {
            if (it != exts.end()) {
                extensions.setSelectedValue(it->value_);
                return;
            }
        }
        extensions.setSelectedValue(FileExtension{});
    } else {
        extensions.setSelectedValue(filePath.getSelectedExtension());
    }
}

template <typename ExtView>
std::vector<OptionPropertyOption<FileExtension>> optionsForFileExtensions(ExtView extensions) {
    return extensions | std::views::transform([](const FileExtension& ext) {
               return OptionPropertyOption<FileExtension>{ext};
           }) |
           std::ranges::to<std::vector>();
}

template <typename... Types>
std::vector<OptionPropertyOption<FileExtension>> optionsForTypes(const DataReaderFactory& rf) {
    return optionsForFileExtensions(rf.getExtensionsForTypesView<Types...>());
}
template <typename... Types>
std::vector<OptionPropertyOption<FileExtension>> optionsForTypes(const DataWriterFactory& rf) {
    return optionsForFileExtensions(rf.getExtensionsForTypesView<Types...>());
}

template <typename ExtView>
void updateOptions(OptionProperty<FileExtension>& optionProperty, ExtView extensions) {
    optionProperty.updateOptions(
        [&](std::vector<OptionPropertyOption<FileExtension>>& opts) -> bool {
            bool modified = false;
            for (auto&& [opt, ext] : std::views::zip(opts, extensions)) {
                if (opt.value_ != ext) {
                    opt = ext;
                    modified = true;
                }
            }
            const auto size = std::ranges::distance(extensions);
            if (std::ssize(opts) > size) {
                opts.erase(opts.begin() + size, opts.end());
                modified = true;
            }
            for (auto&& item : extensions | std::views::drop(opts.size())) {
                opts.emplace_back(item);
                modified = true;
            }
            return modified;
        });
}

template <typename ExtView>
void updateNameFilters(FileProperty& filePath, ExtView extensions) {
    auto& nameFilters = filePath.getNameFilters();
    nameFilters.resize(std::ranges::distance(extensions) + 1uz);
    nameFilters.front() = FileExtension::all();
    std::ranges::copy(extensions, std::ranges::begin(nameFilters | std::views::drop(1)));
}
template <typename... Types>
void updateNameFilters(const DataReaderFactory& rf, FileProperty& filePath) {
    updateNameFilters(filePath, rf.getExtensionsForTypesView<Types...>());
}
template <typename... Types>
void updateNameFilters(const DataWriterFactory& rf, FileProperty& filePath) {
    updateNameFilters(filePath, rf.getExtensionsForTypesView<Types...>());
}

template <typename... Types>
void updateFilenameFilters(const DataReaderFactory& rf, FileProperty& filePath,
                           OptionProperty<FileExtension>& optionProperty) {

    const auto extensions = rf.getExtensionsForTypesView<Types...>();
    updateOptions(optionProperty, extensions);
    updateNameFilters(filePath, extensions);
}

}  // namespace inviwo::util
