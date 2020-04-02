/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2020 Inviwo Foundation
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

#ifndef IVW_ORDINALBASEWIDGET_H
#define IVW_ORDINALBASEWIDGET_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/properties/constraintbehaviour.h>

namespace inviwo {

template <typename T>
class OrdinalBaseWidget {
public:
    virtual ~OrdinalBaseWidget() = default;

    virtual T getValue() const = 0;
    virtual void setValue(T value) = 0;
    virtual void initValue(T value) = 0;
    virtual void setMinValue(T minValue, ConstraintBehaviour cb) = 0;
    virtual void setMaxValue(T maxValue, ConstraintBehaviour cb) = 0;
    virtual void setIncrement(T increment) = 0;

    static int decimals(double inc) {
        if constexpr (std::is_floating_point_v<T>) {
            const static QLocale locale;
            std::ostringstream buff;
            utilqt::localizeStream(buff);
            buff << inc;
            const std::string str(buff.str());
            auto periodPosition = str.find(locale.decimalPoint().toLatin1());
            if (periodPosition == std::string::npos) {
                return 0;
            } else {
                return static_cast<int>(str.length() - periodPosition) - 1;
            }
        } else {
            return 0;
        }
    }
};

}  // namespace inviwo

#endif  // IVW_ORDINALBASEWIDGET_H
