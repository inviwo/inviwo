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

#include <inviwo/core/util/tooltiphelper.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

ToolTipHelper::ToolTipHelper(std::string item /*= ""*/) : ss() {
    auto locale = InviwoApplication::getPtr()->getUILocale();
    ss.imbue(locale);

    ss << "<html><head>"
       << "<style>"
       << "table { border-color:white;white-space:pre;margin-top:0px;margin-bottom:0px; }"
       << "table > tr > td { padding-left:0px; padding-right:0px; }"
       << "</style><head/><body>";

    if (!item.empty()) ss << "<b style='color:white;'>" << item << "</b>";
}

ToolTipHelper::~ToolTipHelper() {}

ToolTipHelper& ToolTipHelper::tableTop() {
    ss << "<table border='0' cellspacing='0' cellpadding='0'"
       << "style='border-color:white;white-space:pre;margin: 5px 0;'>";
    return *this;
}

ToolTipHelper& ToolTipHelper::tableBottom() {
    ss << "</table>";
    return *this;
}

ToolTipHelper::operator std::string() { return ss.str() + "</body></html>"; }

}  // namespace
