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

#ifndef IVW_INVIWOQTUTILS_H
#define IVW_INVIWOQTUTILS_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <locale>
#include <ios>
#include <sstream>

namespace inviwo {

namespace utilqt {
    
/** 
 * \brief getCurrentStdLocale
 * This function returns the current system locale provided by Qt.
 * If the Qt application has not been initialized, the returned 
 * value is the environment's default locale.
 * 
 * @return std::locale   Qt locale converted to std::locale
 */
IVW_QTWIDGETS_API std::locale getCurrentStdLocale();

/** 
 * \brief localize
 * The given stream is imbued with the currently set system locale provided by Qt.
 *
 * @param stream   the locale is imbued onto this stream
 * @return std::ios_base&  reference to the input stream
 */
IVW_QTWIDGETS_API std::ios_base& localizeStream(std::ios_base& stream);

} // namespace utilqt


template <class T>
std::string toLocalizedString(T value) {
    std::ostringstream stream;
    stream.imbue(utilqt::getCurrentStdLocale());
    stream << value;
    return stream.str();
}

template <class T>
T localizedStringTo(const std::string& str) {
    T result;
    std::istringstream stream;
    stream.imbue(utilqt::getCurrentStdLocale());
    stream.str(str);
    stream >> result;
    return result;
}

}  // namespace

#endif // IVW_INVIWOQTUTILS_H
