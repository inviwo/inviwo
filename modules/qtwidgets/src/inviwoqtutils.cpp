/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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
#include <inviwo/core/properties/property.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/io/imagewriterutil.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/canvasprocessor.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/tfpropertyconcept.h>

#include <inviwo/core/util/logcentral.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QLocale>
#include <QApplication>
#include <QClipboard>
#include <QPixmap>
#include <QDesktopWidget>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QPainter>
#include <QLinearGradient>
#include <QBuffer>
#include <QByteArray>
#include <QStyle>
#include <QScreen>
#include <warn/pop>

#include <ios>
#include <exception>

namespace inviwo {

namespace utilqt {

std::locale getCurrentStdLocale() {
    auto warnOnce = [](auto message) {
        static bool hasWarned = false;
        if (!hasWarned) {
            LogWarnCustom("getStdLocale", message);
            hasWarned = true;
        }
    };

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
    } catch (std::exception& e) {
        warnOnce(std::string("Locale could not be set. ") + e.what());
        try {
            loc = std::locale("en_US.UTF8");
        } catch (...) {
        }
    }
    return loc;
}

std::ios_base& localizeStream(std::ios_base& stream) {
    stream.imbue(getCurrentStdLocale());
    return stream;
}

QString toLocalQString(const std::string& input) { return QString::fromLocal8Bit(input.c_str()); }
std::string fromLocalQString(const QString& input) {
    return std::string(input.toLocal8Bit().constData());
}

QString toQString(const std::string& input) { return QString::fromUtf8(input.c_str()); }
std::string fromQString(const QString& input) { return std::string(input.toUtf8().constData()); }

QPointF toQPoint(dvec2 v) { return QPointF(v.x, v.y); }

QPoint toQPoint(ivec2 v) { return QPoint(v.x, v.y); }

dvec2 toGLM(QPointF v) { return dvec2(v.x(), v.y()); }

ivec2 toGLM(QPoint v) { return ivec2(v.x(), v.y()); }

dvec2 toGLM(QSizeF v) { return dvec2(v.width(), v.height()); }

ivec2 toGLM(QSize v) { return ivec2(v.width(), v.height()); }

QSizeF toQSize(dvec2 v) { return QSizeF(v.x, v.y); }

QSize toQSize(ivec2 v) { return QSize(v.x, v.y); }

vec3 tovec3(const QColor& c) { return vec3(c.redF(), c.greenF(), c.blueF()); }

ivec3 toivec3(const QColor& c) { return ivec3(c.red(), c.green(), c.blue()); }

vec4 tovec4(const QColor& c) { return vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }

ivec4 toivec4(const QColor& c) { return ivec4(c.red(), c.green(), c.blue(), c.alpha()); }

QColor toQColor(const vec3& v) { return toQColor(ivec3(v * 255.0f)); }

QColor toQColor(const ivec3& v) { return QColor(v.r, v.g, v.b); }

QColor toQColor(const uvec3& v) { return QColor(v.r, v.g, v.b); }

QColor toQColor(const vec4& v) { return toQColor(ivec4(v * 255.0f)); }

QColor toQColor(const ivec4& v) { return QColor(v.r, v.g, v.b, v.a); }

QColor toQColor(const uvec4& v) { return QColor(v.r, v.g, v.b, v.a); }

QMainWindow* getApplicationMainWindow() {
    auto widgets = QApplication::allWidgets();
    auto it = std::find_if(widgets.begin(), widgets.end(), [](const auto& item) {
        return item->objectName().compare("InviwoMainWindow") == 0;
    });
    if (it != widgets.end()) {
        return dynamic_cast<QMainWindow*>(*it);
    } else {
        return nullptr;
    }
}

QPixmap toQPixmap(const TransferFunction& tf, const QSize& size) {
    QVector<QGradientStop> gradientStops;
    for (const auto& tfpoint : tf) {
        vec4 curColor = tfpoint.getColor();
        // increase alpha to allow better visibility by 1 - (1 - a)^4
        curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);

        gradientStops.append(QGradientStop(tfpoint.getPosition(), utilqt::toQColor(curColor)));
    }

    // set bounds of the gradient
    QLinearGradient gradient;
    gradient.setStops(gradientStops);

    // gradient should stretch entire pixmap from left to right
    gradient.setStart(QPointF(0.0, 0.0));
    gradient.setFinalStop(QPointF(size.width(), 0.0));

    QPixmap tfPixmap(size);
    QPainter tfPainter(&tfPixmap);

    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();

    QRect r(QPoint(0, 0), size);
    // fill pixmap background with a checkerboard
    tfPainter.fillRect(r, QBrush(checkerBoard));
    // draw TF gradient on top
    tfPainter.fillRect(r, gradient);

    return tfPixmap;
}

QPixmap toQPixmap(const TransferFunctionProperty& tfproperty, const QSize& size) {
    auto tfPixmap = toQPixmap(tfproperty.get(), size);
    QPainter tfPainter(&tfPixmap);

    const auto tfRange = tfproperty.get().getRange();

    // draw masking indicators
    const QColor maskColor(25, 25, 25, 150);
    if (tfproperty.getMask().x > tfRange.x) {
        // normalize mask position with respect to TF range
        const double maskPos = (tfproperty.getMask().x - tfRange.x) / (tfRange.y - tfRange.x);
        QRect rectMask(QPoint(0, 0),
                       QSize(static_cast<int>(maskPos * size.width()), size.height()));
        tfPainter.fillRect(rectMask, maskColor);
        tfPainter.drawLine(rectMask.bottomRight(), rectMask.topRight());
    }

    if (tfproperty.getMask().y < tfRange.y) {
        // normalize mask position with respect to TF range
        const double maskPos = (tfproperty.getMask().y - tfRange.x) / (tfRange.y - tfRange.x);
        QRect rectMask(QPoint(static_cast<int>(maskPos * size.width()), 0),
                       QPoint(size.width(), size.height()));
        tfPainter.fillRect(rectMask, maskColor);
        tfPainter.drawLine(rectMask.bottomLeft(), rectMask.topLeft());
    }
    return tfPixmap;
}

QPixmap toQPixmap(const IsoValueProperty& property, const QSize& size) {
    QPixmap tfPixmap(size);
    QPainter painter(&tfPixmap);

    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();

    QRect r(QPoint(0, 0), size);
    // fill pixmap background with a checkerboard
    painter.fillRect(r, QBrush(checkerBoard));

    const auto tfRange = property.get().getRange();
    auto normalize = [tfRange](double pos) { return (pos - tfRange.x) / (tfRange.y - tfRange.x); };

    // draw a small hour glass for each isovalue
    const double halfWidth = 6.0;

    painter.setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
    painter.setRenderHint(QPainter::Antialiasing);
    // add vertical lines for each isovalue
    for (const auto& isovalue : property.get()) {
        vec4 curColor = isovalue.getColor();
        // increase alpha to allow better visibility by 1 - (1 - a)^4
        curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);
        double pos = normalize(isovalue.getPosition()) * size.width();

        painter.setBrush(utilqt::toQColor(curColor));
        painter.drawPolygon(QPolygonF(
            {QPointF(pos - halfWidth, size.height()), QPointF(pos + halfWidth, size.height()),
             QPointF(pos - halfWidth, 0.0), QPointF(pos + halfWidth, 0.0)}));
    }
    return tfPixmap;
}

QPixmap toQPixmap(const util::TFPropertyConcept& propertyConcept, const QSize& size) {

    QPixmap tfPixmap(size);
    QPainter painter(&tfPixmap);

    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();

    QRect r(QPoint(0, 0), size);
    // fill pixmap background with a checkerboard
    painter.fillRect(r, QBrush(checkerBoard));

    if (propertyConcept.hasTF() && !propertyConcept.getTransferFunction()->empty()) {
        // draw TF gradient on top

        QVector<QGradientStop> gradientStops;
        for (const auto& tfpoint : *propertyConcept.getTransferFunction()) {
            vec4 curColor = tfpoint.getColor();
            // increase alpha to allow better visibility by 1 - (1 - a)^4
            curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);

            gradientStops.append(QGradientStop(tfpoint.getPosition(), utilqt::toQColor(curColor)));
        }

        // set bounds of the gradient
        QLinearGradient gradient;
        gradient.setStops(gradientStops);

        // gradient should stretch entire pixmap from left to right
        gradient.setStart(QPointF(0.0, 0.0));
        gradient.setFinalStop(QPointF(size.width(), 0.0));
        painter.fillRect(r, gradient);
    }

    if (propertyConcept.hasIsovalues()) {
        // draw a small hour glass for each isovalue
        const double halfWidth = 6.0;
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);

        const auto tfRange = propertyConcept.getIsovalues()->getRange();
        auto normalize = [tfRange](double pos) {
            return (pos - tfRange.x) / (tfRange.y - tfRange.x);
        };

        painter.setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
        // add vertical lines for each isovalue
        for (const auto& isovalue : *propertyConcept.getIsovalues()) {
            vec4 curColor = isovalue.getColor();
            // increase alpha to allow better visibility by 1 - (1 - a)^4
            curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);
            double pos = normalize(isovalue.getPosition()) * size.width();

            painter.setBrush(utilqt::toQColor(curColor));
            painter.drawPolygon(QPolygonF(
                {QPointF(pos - halfWidth, size.height()), QPointF(pos + halfWidth, size.height()),
                 QPointF(pos - halfWidth, 0.0), QPointF(pos + halfWidth, 0.0)}));
        }

        painter.restore();
    }

    if (propertyConcept.supportsMask()) {
        const auto tfRange = propertyConcept.getTransferFunction()->getRange();
        // draw masking indicators
        const QColor maskColor(25, 25, 25, 150);
        if (propertyConcept.getMask().x > tfRange.x) {
            // normalize mask position with respect to TF range
            const double maskPos =
                (propertyConcept.getMask().x - tfRange.x) / (tfRange.y - tfRange.x);
            QRect rectMask(QPoint(0, 0),
                           QSize(static_cast<int>(maskPos * size.width()), size.height()));
            painter.fillRect(rectMask, maskColor);
            painter.drawLine(rectMask.bottomRight(), rectMask.topRight());
        }

        if (propertyConcept.getMask().y < tfRange.y) {
            // normalize mask position with respect to TF range
            const double maskPos =
                (propertyConcept.getMask().y - tfRange.x) / (tfRange.y - tfRange.x);
            QRect rectMask(QPoint(static_cast<int>(maskPos * size.width()), 0),
                           QPoint(size.width(), size.height()));
            painter.fillRect(rectMask, maskColor);
            painter.drawLine(rectMask.bottomLeft(), rectMask.topLeft());
        }
    }

    return tfPixmap;
}

QPoint movePointOntoDesktop(const QPoint& point, const QSize& /*size*/,
                            bool decorationOffset /*= true*/) {
#ifdef WIN32
    // Only non zero on windows, due to a QT bug in window decoration handling.
    static QPoint windowDecorationOffset = []() {
        // Fix window offset when restoring positions saved during shutdown.
        // Determined the window frame/border size once.
        QWidget w(nullptr, Qt::Tool);
        // ensure that widget is not positioned on a visible desktop
        w.setAttribute(Qt::WA_DontShowOnScreen);
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

    if (decorationOffset) {
        QPoint offset = windowDecorationOffset;
        pos -= offset;
    }

    // Check if point is within any desktops rect (with some extra padding applied).
    static constexpr int leftPadding = 0;
    static constexpr int topPadding = 0;
    static constexpr int rightPadding = 10;
    static constexpr int bottomPadding = 10;
    const bool withinAnyDesktop = [&]() {
        QList<QScreen*> screens = QGuiApplication::screens();
        for (QScreen* screen : screens) {
            auto geom = screen->geometry().marginsRemoved(
                {leftPadding, topPadding, rightPadding, bottomPadding});
            if (geom.contains(pos)) {
                return true;
            }
        }
        return false;
    }();

    if (!withinAnyDesktop) {
        // If the widget is outside visible screen
        auto mainWindow = getApplicationMainWindow();
        QPoint appPos(0, 0);
        if (mainWindow) {
            appPos = mainWindow->pos();
        }
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

QMenu* addMenu(std::string menuName, std::string before) {
    return addMenu(menuName, getMenu(before));
}

QMenu* addMenu(std::string menuName, QMenu* before) {
    if (auto mainwin = utilqt::getApplicationMainWindow()) {
        if (!before) {
            before = getMenu("&Help", false);
        }

        auto menuBar = mainwin->menuBar();

        if (before) {
            auto menu = new QMenu(menuName.c_str(), menuBar);
            menuBar->insertMenu(before->menuAction(), menu);
            return menu;
        } else {  // No menu specified or couldn't find a help menu
            return menuBar->addMenu(menuName.c_str());
        }
    }
    throw Exception("No Qt main window found");
}

QMenu* getMenu(std::string menuName, bool createIfNotFound) {
    if (auto mainwin = utilqt::getApplicationMainWindow()) {
        auto menuBar = mainwin->menuBar();
        auto menus = menuBar->findChildren<QMenu*>();

        auto menuNoAnd = menuName;
        replaceInString(menuNoAnd, "&", "");

        auto menuItem = std::find_if(menus.begin(), menus.end(), [&](auto& m) {
            std::string title = m->title().toLocal8Bit().constData();
            replaceInString(title, "&", "");
            return menuNoAnd == title;
        });
        if (menuItem != menus.end()) {
            return *menuItem;
        } else if (createIfNotFound) {
            return addMenu(menuName);
        } else {
            return nullptr;
        }
    }
    throw Exception("No Qt main window found");
}

QImage layerToQImage(const Layer& layer) {
    auto data = layer.getAsCodedBuffer("png");
    return QImage::fromData(data->data(), static_cast<int>(data->size()), "png");
}

void addImageActions(QMenu& menu, const Image& image, LayerType visibleLayer, size_t visibleIndex) {
    QMenu* copy = menu.addMenu(QIcon(":svgicons/edit-copy.svg"), "Copy");
    QMenu* save = menu.addMenu(QIcon(":svgicons/save-as.svg"), "Save");

    auto addAction = [&copy, &save](const std::string& name, const Layer* layer, bool visible) {
        std::ostringstream oss;
        oss << name << (visible ? " (Visible)" : "");
        auto copyAction = copy->addAction(oss.str().c_str());
        copyAction->connect(copyAction, &QAction::triggered, [layer]() {
            QApplication::clipboard()->setPixmap(QPixmap::fromImage(utilqt::layerToQImage(*layer)));
        });

        auto saveAction = save->addAction(oss.str().c_str());
        saveAction->connect(saveAction, &QAction::triggered,
                            [layer]() { util::saveLayer(*layer); });
    };

    const auto nLayers = image.getNumberOfColorLayers();
    for (size_t i = 0; i < nLayers; i++) {
        addAction("Color Layer " + (nLayers > 1 ? toString(i) : ""), image.getColorLayer(i),
                  visibleLayer == LayerType::Color && visibleIndex == i);
    }

    addAction("Picking Layer", image.getPickingLayer(), visibleLayer == LayerType::Picking);
    addAction("Depth Layer", image.getDepthLayer(), visibleLayer == LayerType::Depth);
}

std::string toBase64(const QImage& image, const std::string& format, int quality) {
    QByteArray byteArray;
    QBuffer buffer{&byteArray};
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.empty() ? nullptr : format.c_str(), quality);
    return std::string{byteArray.toBase64().data()};
}

std::vector<std::pair<std::string, QImage>> getCanvasImages(ProcessorNetwork* network, bool alpha) {
    std::vector<std::pair<std::string, QImage>> images;
    for (auto* p : network->getProcessorsByType<CanvasProcessor>()) {
        if (p->isSink() && p->isReady()) {
            auto img = utilqt::layerToQImage(*p->getVisibleLayer()).scaledToHeight(256);
            images.push_back({p->getDisplayName(), img});
        }
    }

    if (!alpha) {
        for (auto& elem : images) {
            QImage& img = elem.second;
            if (img.hasAlphaChannel()) {
                switch (img.format()) {
                    case QImage::Format_Alpha8:
                        img = img.convertToFormat(QImage::Format_Grayscale8);
                        break;
                    case QImage::Format_RGBA8888:
                    case QImage::Format_RGBA8888_Premultiplied:
                        img = img.convertToFormat(QImage::Format_RGBX8888);
                        break;
                    default:
                        img = img.convertToFormat(QImage::Format_RGB32);
                        break;
                }
            }
        }
    }

    return images;
}

QString windowTitleHelper(const QString& title, const QWidget* widget) {
    if (title.isEmpty() || !widget) {
        return title;
    }

    // implementation based on qt_setWindowTitle_helperHelper() in qwidget.cpp
    QString cap = title;

    QLatin1String placeHolder("[*]");
    int index = cap.indexOf(placeHolder);

    // here the magic begins
    while (index != -1) {
        index += placeHolder.size();
        int count = 1;
        while (cap.indexOf(placeHolder, index) == index) {
            ++count;
            index += placeHolder.size();
        }
        if (count % 2) {  // odd number of [*] -> replace last one
            int lastIndex = cap.lastIndexOf(placeHolder, index - 1);
            if (widget->isWindowModified() &&
                widget->style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, widget))
                cap.replace(lastIndex, 3, QWidget::tr("*"));
            else
                cap.remove(lastIndex, 3);
        }
        index = cap.indexOf(placeHolder, index);
    }
    cap.replace(QLatin1String("[*][*]"), placeHolder);

    return cap;
}

int refSpacePx(const QWidget* w) { return emToPx(w, refSpaceEm()); }

QSize emToPx(const QWidget* w, QSizeF ems) {
    return QSize{emToPx(w, ems.width()), emToPx(w, ems.height())};
}

int emToPx(const QWidget* w, double em) {
    w->ensurePolished();
    return emToPx(w->fontMetrics(), em);
}

int emToPx(const QFontMetrics& m, double em) {
    const auto pxPerEm = m.boundingRect(QString(100, 'M')).width() / 100.0;
    return static_cast<int>(std::round(pxPerEm * em));
}

}  // namespace utilqt

}  // namespace inviwo
