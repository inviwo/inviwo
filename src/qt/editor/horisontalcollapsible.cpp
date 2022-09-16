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

#include <inviwo/qt/editor/horisontalcollapsible.h>
#include <inviwo/qt/editor/verticallabel.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QToolButton>
#include <QVBoxLayout>
#include <warn/pop>

namespace inviwo {

    
HorisontalCollipsable::HorisontalCollipsable(const QString& title, QWidget* content,
                                             QWidget* parent)
    : QWidget(parent), content_(content) {  
    toggleButton_ = new QToolButton(this);
    toggleButton_->setObjectName("collapseButton");
    toggleButton_->setArrowType(Qt::ArrowType::LeftArrow);
    toggleButton_->setCheckable(true);
    toggleButton_->setChecked(true);

    collapsibleTitle_ = new VerticalLabel(title, this);
    collapsibleTitle_->setObjectName("collapseTitle");

    toggleLayout_ = new QVBoxLayout(this);
    toggleLayout_->addWidget(collapsibleTitle_);
    toggleLayout_->addWidget(toggleButton_);

    toggleLayout_->setContentsMargins(0, 0, 0, 0);
    toggleLayout_->setAlignment(Qt::AlignCenter);
    toggleLayout_->setAlignment(toggleButton_, Qt::AlignHCenter);
    toggleLayout_->setSizeConstraint(QLayout::SetFixedSize);

    toggleArea_ = new QWidget(this);
    toggleArea_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    toggleArea_->setLayout(toggleLayout_);

    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(toggleArea_);
    mainLayout->addWidget(content_);
    mainLayout->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(mainLayout);

    connect(toggleButton_, &QToolButton::clicked, this, &HorisontalCollipsable::collapse);
}

 bool HorisontalCollipsable::isCollapsed() const { return toggleButton_->isChecked(); }

void HorisontalCollipsable::collapse(bool collapse) {
    setUpdatesEnabled(false);
    toggleButton_->setChecked(collapse);
    toggleButton_->setArrowType(collapse ? Qt::ArrowType::LeftArrow : Qt::ArrowType::RightArrow);
    collapsibleTitle_->setVisible(collapse);
    content_->setVisible(!collapse);
    setUpdatesEnabled(true);
    adjustSize();
    
    emit toggled(collapse);
}




}  // namespace inviwo
