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

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QKeyEvent>
#include <QLabel>
#include <QThread>
#include <QCoreApplication>
#include <warn/pop>

#include <inviwo/core/common/inviwo.h>
#include <inviwo/qt/editor/consolewidget.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/processors/processor.h>

namespace inviwo {

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : InviwoDockWidget(tr("Console"), parent)
    , infoTextColor_(153, 153, 153)
    , warnTextColor_(221, 165, 8)
    , errorTextColor_(255, 107, 107)
    , errorsLabel_(nullptr)
    , warningsLabel_(nullptr)
    , infoLabel_(nullptr)
    , numErrors_(0)
    , numWarnings_(0)
    , numInfos_(0) {
    setObjectName("ConsoleWidget");
    setAllowedAreas(Qt::BottomDockWidgetArea);
    textField_ = new QTextEdit(this);
    textField_->setReadOnly(true);
    textField_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    textField_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(textField_, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    QHBoxLayout *statusBar = new QHBoxLayout();

    errorsLabel_ = new QLabel("0", this);
    warningsLabel_ = new QLabel("0", this);
    infoLabel_ = new QLabel("0", this);

    QFrame* line1 = new QFrame();
    QFrame* line2 = new QFrame();
    QFrame* line3 = new QFrame();
    line1->setFrameShape(QFrame::VLine);
    line2->setFrameShape(QFrame::VLine);
    line3->setFrameShape(QFrame::VLine);
    
    line1->setFrameShadow(QFrame::Raised);
    line2->setFrameShadow(QFrame::Raised);
    line3->setFrameShadow(QFrame::Raised);

    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/error.png'>", this));
    statusBar->addWidget(errorsLabel_);
    statusBar->addWidget(line1);
    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/warning.png'>", this));
    statusBar->addWidget(warningsLabel_);
    statusBar->addWidget(line2);
    statusBar->addWidget(new QLabel( "<img width='16' height='16' src=':/icons/info.png'>", this));
    statusBar->addWidget(infoLabel_);
    statusBar->addWidget(line3);
    

    statusBar->addItem(new QSpacerItem(40, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(textField_);
    layout->addLayout(statusBar);

    layout->setContentsMargins(3, 0, 0, 3);


    QWidget* w = new QWidget();
    w->setLayout(layout);
    setWidget(w);


    connect(this, SIGNAL(logMessageSignal(int, QString)), this, SLOT(logMessage(int, QString)));
    connect(this, SIGNAL(clearSignal()), this, SLOT(clear()));
}

ConsoleWidget::~ConsoleWidget() {
}

void ConsoleWidget::showContextMenu(const QPoint& pos) {
    QMenu* menu = textField_->createStandardContextMenu();
    QAction* clearAction = menu->addAction("Clear console");
    clearAction->setShortcut(Qt::ControlModifier + Qt::Key_E);
    menu->addAction(clearAction);
    QAction* result = menu->exec(QCursor::pos());

    if (result == clearAction) {
        clear();
    }

    delete menu;
}

void ConsoleWidget::clear(){
    if (QThread::currentThread() != QCoreApplication::instance()->thread()){
        emit clearSignal();
        return;
    }

    textField_->clear();
    errorsLabel_->setText("0");
    warningsLabel_->setText("0");
    infoLabel_->setText("0");
    numErrors_ = numWarnings_ = numInfos_ = 0;
}

void ConsoleWidget::logMessage(int level, QString message){
    ConsoleWidget::logMessage(static_cast<LogLevel>(level), message);
}

void ConsoleWidget::logMessage(LogLevel level, QString message) {
    if (QThread::currentThread() != QCoreApplication::instance()->thread()){
        emit logMessageSignal(static_cast<int>(level), message);
        return; 
    }

    switch (level) {
        case LogLevel::Error: {
            textField_->setTextColor(errorTextColor_);
            errorsLabel_->setText(toString(++numErrors_).c_str());
            break;
        }
        case LogLevel::Warn: {
            textField_->setTextColor(warnTextColor_);
            warningsLabel_->setText(toString(++numWarnings_).c_str());

            break;
        }
        case LogLevel::Info: {
            textField_->setTextColor(infoTextColor_);
            infoLabel_->setText(toString(++numInfos_).c_str());
            break;
        }
    }
    textField_->append(message);
    QTextCursor c =  textField_->textCursor();
    c.movePosition(QTextCursor::End);
    textField_->setTextCursor(c);

}

void ConsoleWidget::log(std::string logSource, LogLevel level, LogAudience audience,
                        const char* fileName, const char* function, int line, std::string msg) {
    IVW_UNUSED_PARAM(function);

    QString message;
    std::string lineNo = toString(line);
    switch (audience) {
        case inviwo::LogAudience::User: {
            message = QString::fromStdString("(" + logSource + ") " + msg);

            break;
        }
        case inviwo::LogAudience::Developer: {
            switch (level) {
                case LogLevel::Error:
                case LogLevel::Warn: {
                    message =
                        QString::fromStdString("(" + logSource + ") [" + std::string(fileName) +
                                               ", " + lineNo + "]: " + msg);
                    break;
                }
                case LogLevel::Info:
                default: {
                    message = QString::fromStdString("(" + logSource + ") " + msg);
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    logMessage(level, message);
}

void ConsoleWidget::logProcessor(Processor* processor, LogLevel level, LogAudience audience,
                                 std::string msg, const char* file, const char* function,
                                 int line) {
    QString message = QString::fromStdString("Processor " + 
        (parseTypeIdName(std::string(typeid(processor).name()))) + "/" + processor->getIdentifier()
         + ": " + msg);
    logMessage(level, message);
}

void ConsoleWidget::logNetwork(LogLevel level, LogAudience audience,
                              std::string msg, const char* file, const char* function,
                              int line) {
    
    QString message = QString::fromStdString("ProcessorNetwork: " + msg);
    logMessage(level, message);
}


void ConsoleWidget::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_E && keyEvent->modifiers() == Qt::ControlModifier){
        clear();
    }
}

} // namespace