/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_TOOLTIPHELPER_H
#define IVW_TOOLTIPHELPER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

/**
 * \class ToolTipHelper
 * \brief Helper class to create html tooltips
 */
class IVW_CORE_API ToolTipHelper {
public:
    ToolTipHelper(std::string item = "");
    operator std::string();
    ~ToolTipHelper();

    ToolTipHelper& tableTop();
    ToolTipHelper& tableBottom();

    template <typename T, typename U>
    ToolTipHelper& row(T item, std::vector<U> vals, bool tablehead = false);

    template <typename T, typename U>
    ToolTipHelper& row(T item, std::initializer_list<U> vals, bool tablehead = false);

    template <typename T, typename U>
    ToolTipHelper& row(T item, U val, bool tablehead = false);

private:
    std::stringstream ss;
};

template <typename T, typename U>
ToolTipHelper& inviwo::ToolTipHelper::row(T item, U val, bool tablehead) {
    std::string td = (tablehead ? "th" : "td");
    ss << "<tr><" << td << " style='color:#bbb;padding-right:8px;'>" << item << "</" << td << ">";
    ss << "<" << td << "><nobr>" << val << "</nobr></" << td << ">";
    ss << "</tr>" << std::endl;
    return *this;
}

template <typename T, typename U>
ToolTipHelper& inviwo::ToolTipHelper::row(T item, std::vector<U> vals, bool tablehead) {
    std::string td = (tablehead ? "th" : "td");
    ss << "<tr><" << td << " style='color:#bbb;padding-right:8px;'>" << item << "</" << td << ">";

    for (auto& val : vals) {
        ss << "<" << td << " align=center><nobr>" << val << "</nobr></" << td << ">";
    }
    ss << "</tr>" << std::endl;
    return *this;
}

template <typename T, typename U>
ToolTipHelper& inviwo::ToolTipHelper::row(T item, std::initializer_list<U> vals, bool tablehead /*= false*/) {
    std::string td = (tablehead ? "th" : "td");
    ss << "<tr><" << td << " style='color:#bbb;padding-right:8px;'>" << item << "</" << td << ">";

    for (auto& val : vals) {
        ss << "<" << td << " align=center><nobr>" << val << "</nobr></" << td << ">";
    }
    ss << "</tr>" << std::endl;
    return *this;
}

} // namespace

#endif // IVW_TOOLTIPHELPER_H

