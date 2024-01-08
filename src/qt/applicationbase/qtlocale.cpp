/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2024 Inviwo Foundation
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

#include <inviwo/qt/applicationbase/qtlocale.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/logcentral.h>

#include <fmt/format.h>

#include <QLocale>

namespace inviwo {

std::locale utilqt::getCurrentStdLocale() {
    static std::locale loc = []() -> std::locale {
#ifdef WIN32
        // need to change locale given by Qt from underscore to hyphenated ("sv_SE" to "sv-SE")
        // although std::locale should only accept locales with underscore, e.g. "sv_SE"
        std::string localeName(QLocale::system().name().replace('_', '-').toStdString());
#else
        std::string localeName(QLocale::system().name().toStdString());
#endif
        if (localeName.rfind('.') == std::string::npos) {
            localeName += ".UTF-8";
        }

        // try to use the system locale provided by Qt
        std::vector<std::string> localeNames = {localeName, "en_US.UTF-8", "en_US.UTF8",
                                                "en-US.UTF-8", "en-US.UTF8"};
        for (auto&& [i, name] : util::enumerate(localeNames)) {
            try {
                if (i != 0) {
                    LogWarnCustom("getStdLocale", fmt::format("Falling back to locale '{}'", name));
                }
                return std::locale(name);
            } catch (std::exception& e) {
                LogWarnCustom("getStdLocale",
                              fmt::format("Locale could not be set to '{}', {}", name, e.what()));
            }
        }
        LogWarnCustom("getStdLocale", "Using the default locale");
        return std::locale{};
    }();

    return loc;
}

}  // namespace inviwo
