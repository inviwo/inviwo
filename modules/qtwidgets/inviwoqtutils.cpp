/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/document.h>
#include <inviwo/core/properties/property.h>

#include <inviwo/core/util/logcentral.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QLocale>
#include <QApplication>
#include <QDesktopWidget>
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

QString toQString(const std::string& input) {
   return QString::fromUtf8(input.c_str());
}

QPointF toQPoint(dvec2 v) {
    return QPointF(v.x, v.y);
}

QPoint toQPoint(ivec2 v) {
    return QPoint(v.x, v.y);
}

dvec2 toGLM(QPointF v) {
    return dvec2(v.x(), v.y());
}

ivec2 toGLM(QPoint v) {
    return ivec2(v.x(), v.y());
}

dvec2 toGLM(QSizeF v) {
    return dvec2(v.width(), v.height());
}

ivec2 toGLM(QSize v) {
    return ivec2(v.width(), v.height());
}

QSizeF toQSize(dvec2 v) {
    return QSizeF(v.x, v.y);
}

QSize toQSize(ivec2 v) {
    return QSize(v.x, v.y);
}

QMainWindow* getApplicationMainWindow() {
    auto widgets = QApplication::allWidgets();
    auto it = std::find_if(widgets.begin(), widgets.end(), [](const auto& item) {
        return item->objectName().compare("InviwoMainWindow") == 0;
    });
    if (it != widgets.end()) {
        return dynamic_cast<QMainWindow*>(*it);
    }
    else {
        return nullptr;
    }
}

QPoint movePointOntoDesktop(const QPoint& point, const QSize& size, bool decorationOffset /*= true*/) {
#ifdef WIN32
    // Only non zero on windows, due to a QT bug in window decoration handling.
    static QPoint windowDecorationOffset = []() {
        // Fix window offset when restoring positions saved during shutdown.
        // Determined the window frame/border size once.
        QWidget w(nullptr, Qt::Tool);
        // Move the widget out of sight.
        w.move(-5000, -5000);
        // Need to show the widget, otherwise no border exists, i.e. this->pos() ==
        // this->geometry().topLeft()
        w.show();
        QPoint widgetPos = w.pos();
        QRect widgetGeo = w.geometry();
        QPoint offset(widgetGeo.left() - widgetPos.x(), widgetGeo.top() - widgetPos.y());
        w.hide();
        return offset;
    }();
#else 
    static QPoint windowDecorationOffset = QPoint(0, 0);
#endif

    QPoint pos(point);
    QDesktopWidget* desktop = QApplication::desktop();
    int primaryScreenIndex = desktop->primaryScreen();
    QRect wholeScreen = desktop->screenGeometry(primaryScreenIndex);

    for (int i = 0; i < desktop->screenCount(); i++) {
        if (i != primaryScreenIndex)
            wholeScreen = wholeScreen.united(desktop->screenGeometry(i));
    }

    wholeScreen.setRect(wholeScreen.x() - 10, wholeScreen.y() - 10, wholeScreen.width() + 20,
        wholeScreen.height() + 20);
    QPoint bottomRight = QPoint(point.x() + size.width(), point.y() + size.height());
    auto mainWindow = getApplicationMainWindow();
    QPoint appPos;
    if (mainWindow) {
        appPos = mainWindow->pos();
    }

    if (decorationOffset) {
        QPoint offset = windowDecorationOffset;
        pos -= offset;
    }

    if (!wholeScreen.contains(pos) || !wholeScreen.contains(bottomRight)) {
        // If the widget is outside visible screen
        pos = appPos;
        pos += offsetWidget();
    }
    return pos;
}

QPoint offsetWidget() {
    static int offsetCounter = 0;
    static ivec2 baseOffset(350, 100);

    ivec2 pos(0, 0);
    pos += baseOffset + ivec2(40 * offsetCounter++);

    if (offsetCounter == 10) {  // reset offset
        offsetCounter = 0;
        baseOffset.x += 200;
        if (baseOffset.x >= 800) {
            baseOffset.x = 350;
            baseOffset.y += 100;
            if (baseOffset.y >= 800) {
                baseOffset.y = 100;
            }
        }
    }
    return QPoint(pos.x, pos.y);
}

} // namespace utilqt

}  // namespace
