/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QString>
#include <warn/pop>

class QToolButton;
class QHBoxLayout;
class QVBoxLayout;

namespace inviwo {

class VerticalLabel;

/**
 * Collapse/expand content.
 * Content will be placed to the right of the collapse label/button.
 * L  |  
 * a  |
 * b  |  Content
 * e  |
 * l  |
 * <  |
 */
class IVW_QTEDITOR_API HorisontalCollipsable : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>

public:
    explicit HorisontalCollipsable(const QString& title = "", QWidget* content = nullptr,
                                   QWidget* parent = nullptr);

    bool isCollapsed() const;

    void collapse(bool collapsed);

signals:
    void toggled(bool collapsed);

private:
    QWidget* toggleArea_;
    VerticalLabel* collapsibleTitle_;
    QToolButton* toggleButton_;
    QVBoxLayout* toggleLayout_;

    QHBoxLayout* mainLayout;
    QWidget* content_;
};

}  // namespace inviwo
