/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/algorithm/searchdsl.h>
#include <inviwo/core/processors/processor.h>

#include <functional>
#include <vector>
#include <unordered_map>
#include <string>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

class QLineEdit;

namespace inviwo {
class InviwoApplication;
class InviwoMainWindow;
class Processor;

/**
 * Widget for searching the processor network. Will highlight matching processors.
 */
class IVW_QTEDITOR_API NetworkSearch : public QWidget {
public:
    NetworkSearch(InviwoMainWindow* win);

    void updateSearch(const QString& str);

    virtual ~NetworkSearch() = default;

    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    SearchDSL<Processor> dsl_;

    static std::unordered_map<std::string, std::string> getModuleMap(InviwoApplication* app);
    static bool find(std::string_view cont, std::string_view s);
    InviwoMainWindow* win_;
    QLineEdit* edit_;
};

}  // namespace inviwo
