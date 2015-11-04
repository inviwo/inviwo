/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/inviwoqtutils.h>
#include <inviwo/core/util/logcentral.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QLocale>
#include <warn/pop>
#include <ios>
#include <exception>

namespace inviwo {

namespace utilqt {

std::locale getCurrentStdLocale() {
    std::locale loc;
    try {
        // use the system locale provided by Qt

#ifdef WIN32
        // need to change locale given by Qt from underscore to hyphenated ("sv_SE" to "sv-SE")
        // although std::locale should only accept locales with underscore, e.g. "sv_SE"
        std::string localeName(QLocale::system().name().replace('_', '-').toStdString());
#else
        std::string localeName(QLocale::system().name().toStdString());
#endif
        loc = std::locale(localeName.c_str());
    }
    catch (std::exception &e) {
        LogWarnCustom("getStdLocale", "Locale could not be set. " << e.what());
    }
    return loc;
}

std::ios_base& localizeStream(std::ios_base& stream) {
    stream.imbue(getCurrentStdLocale());
    return stream;
}

QString toLocalQString(const std::string& input) {
   return QString::fromLocal8Bit(input.c_str());
}

IVW_QTWIDGETS_API QString toQString(const std::string& input) {
   return QString::fromUtf8(input.c_str());
}

} // namespace utilqt

}  // namespace
