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

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/datasequence.h>

#include <filesystem>

#include <fmt/format.h>
#include <fmt/std.h>
#include <fmt/chrono.h>

namespace inviwo {

template <>
struct DataTraits<std::filesystem::path> {
    static constexpr auto classId{"org.inviwo.path"};
    static constexpr std::string_view classIdentifier() { return classId; }
    static constexpr std::string_view dataName() { return "Path"; }
    static constexpr uvec3 colorCode() { return {129, 149, 33}; }
    static Document info(const std::filesystem::path& data) {
        Document doc;
        doc.append("p", dataName());
        doc.append("p", data.string());
        return doc;
    }
};

using PathInport = DataInport<std::filesystem::path>;
using PathOutport = DataOutport<std::filesystem::path>;
using PathSequenceInport = DataInport<DataSequence<std::filesystem::path>>;
using PathSequenceOutport = DataOutport<DataSequence<std::filesystem::path>>;

extern template class IVW_CORE_TMPL_EXP DataInport<std::filesystem::path>;
extern template class IVW_CORE_TMPL_EXP DataInport<std::filesystem::path, 0, false>;
extern template class IVW_CORE_TMPL_EXP DataInport<std::filesystem::path, 0, true>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<std::filesystem::path>>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<std::filesystem::path>, 0, false>;
extern template class IVW_CORE_TMPL_EXP DataInport<DataSequence<std::filesystem::path>, 0, true>;
extern template class IVW_CORE_TMPL_EXP DataOutport<std::filesystem::path>;
extern template class IVW_CORE_TMPL_EXP DataOutport<DataSequence<std::filesystem::path>>;

}  // namespace inviwo

template <>
struct fmt::formatter<std::filesystem::file_time_type, char> {
    fmt::formatter<std::chrono::sys_time<std::filesystem::file_time_type::duration>> formatter;

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return formatter.parse(ctx);
    }

    template <class FmtContext>
    constexpr FmtContext::iterator format(std::filesystem::file_time_type time,
                                          FmtContext& ctx) const {
#ifdef WIN32
        const auto systime = std::chrono::clock_cast<std::chrono::system_clock>(time);
#else
        const auto systime = std::filesystem::file_time_type::clock::to_sys(time);
#endif
        return formatter.format(systime, ctx);
    }
};
