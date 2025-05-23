/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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

#include <inviwo/core/algorithm/camerautils.h>            // for Side, Side::XNegative, Side...
#include <inviwo/core/datastructures/image/image.h>       // for Image
#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, LayerType::Color
#include <inviwo/core/datastructures/image/layer.h>       // for Layer
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/datastructures/isovaluecollection.h>   // for IsoValueCollection
#include <inviwo/core/datastructures/tfprimitive.h>          // for TFPrimitive
#include <inviwo/core/datastructures/tfprimitiveset.h>       // for TFPrimitiveSet
#include <inviwo/core/datastructures/transferfunction.h>     // for TransferFunction
#include <inviwo/core/interaction/events/eventpropagator.h>  // for EventPropagator
#include <inviwo/core/interaction/events/viewevent.h>        // for ViewEvent, ViewEvent::FitData
#include <inviwo/core/io/imagewriterutil.h>                  // for saveLayer
#include <inviwo/core/network/processornetwork.h>            // for ProcessorNetwork
#include <inviwo/core/processors/exporter.h>
#include <inviwo/core/properties/isovalueproperty.h>          // for IsoValueProperty
#include <inviwo/core/properties/transferfunctionproperty.h>  // for TransferFunctionProperty
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/util/exception.h>          // for Exception
#include <inviwo/core/util/glmvec.h>             // for ivec2, vec4, dvec2
#include <inviwo/core/util/logcentral.h>         // for LogCentral
#include <inviwo/core/util/safecstr.h>           // for SafeCStr
#include <inviwo/core/util/stringconversion.h>   // for replaceInString, toString
#include <inviwo/core/util/transformiterator.h>  // for TransformIterator
#include <inviwo/core/util/zip.h>                // for enumerate, zipIterator, zipper
#include <inviwo/core/util/rendercontext.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>  // for TFPropertyConcept

#include <algorithm>      // for clamp, find_if, find
#include <cmath>          // for pow, round
#include <exception>      // for exception
#include <ios>            // for ostringstream, ios_base
#include <memory>         // for unique_ptr
#include <unordered_map>  // for unordered_map

#include <QAction>          // for QAction
#include <QApplication>     // for QApplication
#include <QBrush>           // for QBrush
#include <QBuffer>          // for QBuffer
#include <QByteArray>       // for QByteArray
#include <QClipboard>       // for QClipboard
#include <QEvent>           // for QEvent, QEvent::Close
#include <QEventLoop>       // for QEventLoop, QEventLoop::Exc...
#include <QFlags>           // for QFlags
#include <QFont>            // for QFont
#include <QFontDatabase>    // for QFontDatabase, QFontDatabas...
#include <QFontMetrics>     // for QFontMetrics
#include <QGuiApplication>  // for QGuiApplication
#include <QIODevice>        // for QIODevice
#include <QIODeviceBase>    // for QIODeviceBase::WriteOnly
#include <QLatin1String>    // for QLatin1String
#include <QLinearGradient>  // for QLinearGradient
#include <QList>            // for QList, QList<>::iterator
#include <QLocale>          // for QLocale
#include <QMainWindow>      // for QMainWindow
#include <QMenu>            // for QMenu
#include <QMenuBar>         // for QMenuBar
#include <QPainter>         // for QPainter, QPainter::Antiali...
#include <QPen>             // for QPen
#include <QPixmap>          // for QPixmap
#include <QPolygonF>        // for QPolygonF
#include <QRect>            // for QRect
#include <QScreen>          // for QScreen
#include <QStringList>      // for QStringList
#include <QStyle>           // for QStyle, QStyle::SH_TitleBar...
#include <QWidget>          // for QWidget
#include <Qt>               // for Tool, WindowFlags, WindowFu...
#include <fmt/core.h>       // for format

namespace inviwo::utilqt {

std::ios_base& localizeStream(std::ios_base& stream) {
    stream.imbue(getCurrentStdLocale());
    return stream;
}

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

void paintCheckerBoard(QPainter& painter, const QRectF& rect) {
    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();

    painter.fillRect(rect, QBrush(checkerBoard));
}

QPen cosmeticPen(const QBrush& brush, qreal width, Qt::PenStyle s, Qt::PenCapStyle c,
                 Qt::PenJoinStyle j) {
    QPen pen(brush, width, s, c, j);
    pen.setCosmetic(true);
    return pen;
}

void paint(const TransferFunction& tf, QPainter& painter, const QRectF& rect) {
    QLinearGradient gradient;
    for (const auto& point : tf) {
        vec4 curColor = point.getColor();
        // increase alpha to allow better visibility by 1 - (1 - a)^4
        curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);
        gradient.setColorAt(point.getPosition(), utilqt::toQColor(curColor));
    }

    // gradient should stretch entire pixmap from left to right
    gradient.setStart(rect.bottomLeft());
    gradient.setFinalStop(rect.bottomRight());

    painter.fillRect(rect, gradient);
}

void paint(const IsoValueCollection& isoValues, QPainter& painter, const QRectF& rect) {
    const auto range = isoValues.getRange();
    auto normalize = [&](double pos) { return (pos - range.x) / (range.y - range.x); };

    // draw a small hour glass for each iso value
    const double halfWidth = 6.0;

    const Save saved(&painter);
    painter.setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
    painter.setRenderHint(QPainter::Antialiasing);
    // add vertical lines for each iso value
    for (const auto& isoValue : isoValues) {
        vec4 curColor = isoValue.getColor();
        // increase alpha to allow better visibility by 1 - (1 - a)^4
        curColor.a = 1.0f - std::pow(1.0f - curColor.a, 4.0f);
        const auto pos = normalize(isoValue.getPosition()) * rect.width();

        painter.setBrush(utilqt::toQColor(curColor));
        painter.drawPolygon(QPolygonF(
            {QPointF(pos - halfWidth, rect.height()), QPointF(pos + halfWidth, rect.height()),
             QPointF(pos - halfWidth, 0.0), QPointF(pos + halfWidth, 0.0)}));
    }
}

void paintMask(const dvec2& mask, const dvec2& range, QPainter& painter, const QRectF& rect) {
    // draw masking indicators
    const QColor maskColor(25, 25, 25, 150);
    if (mask.x > range.x) {
        // normalize mask position with respect to TF range
        const double maskPos = (mask.x - range.x) / (range.y - range.x);
        const QRectF rectMask{rect.topLeft(), QSizeF{maskPos * rect.width(), rect.height()}};
        painter.fillRect(rectMask, maskColor);
        painter.drawLine(rectMask.bottomRight(), rectMask.topRight());
    }

    if (mask.y < range.y) {
        // normalize mask position with respect to TF range
        const double maskPos = (mask.y - range.x) / (range.y - range.x);
        const QRectF rectMask{rect.topLeft() + QPointF{maskPos * rect.width(), 0.0},
                              rect.bottomRight()};
        painter.fillRect(rectMask, maskColor);
        painter.drawLine(rectMask.bottomLeft(), rectMask.topLeft());
    }
}

QPixmap toQPixmap(const TransferFunction& tf, const QSize& size) {
    QPixmap pixmap{size};
    QPainter painter{&pixmap};
    const QRectF rect{QPointF(0, 0), size};
    paintCheckerBoard(painter, rect);
    paint(tf, painter, rect);
    return pixmap;
}

QPixmap toQPixmap(const TransferFunctionProperty& property, const QSize& size) {
    QPixmap pixmap{size};
    QPainter painter{&pixmap};
    const QRectF rect{QPointF(0, 0), size};
    paintCheckerBoard(painter, rect);
    paint(property.get(), painter, rect);
    paintMask(property.getMask(), property.get().getRange(), painter, rect);
    return pixmap;
}

QPixmap toQPixmap(const IsoValueProperty& property, const QSize& size) {
    QPixmap pixmap{size};
    QPainter painter{&pixmap};
    const QRectF rect{QPointF(0, 0), size};
    paintCheckerBoard(painter, rect);
    paint(property.get(), painter, rect);
    return pixmap;
}

QPixmap toQPixmap(const IsoTFProperty& property, const QSize& size) {
    QPixmap pixmap{size};
    QPainter painter{&pixmap};
    const QRectF rect{QPointF(0, 0), size};
    paintCheckerBoard(painter, rect);
    paint(property.tf_.get(), painter, rect);
    paint(property.isovalues_.get(), painter, rect);
    paintMask(property.tf_.getMask(), property.tf_.get().getRange(), painter, rect);
    return pixmap;
}

QPixmap toQPixmap(const TFPropertyConcept& propertyConcept, const QSize& size) {
    QPixmap pixmap{size};
    QPainter painter{&pixmap};
    const QRectF rect{QPointF(0, 0), size};
    paintCheckerBoard(painter, rect);

    if (auto* tf = propertyConcept.getTransferFunction()) {
        paint(*tf, painter, rect);
    }
    if (auto* isoValues = propertyConcept.getIsovalues()) {
        paint(*isoValues, painter, rect);
    }
    if (auto* tf = propertyConcept.getTransferFunction()) {
        if (propertyConcept.supportsMask()) {
            paintMask(propertyConcept.getMask(), tf->getRange(), painter, rect);
        }
    }
    return pixmap;
}

QPointF clamp(const QPointF& pos, const QRectF& rect) {
    return QPointF{std::clamp(pos.x(), rect.left(), rect.right()),
                   std::clamp(pos.y(), rect.top(), rect.bottom())};
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
    static const QPoint windowDecorationOffset = QPoint(0, 0);
#endif

    QPoint pos(point);

    if (decorationOffset) {
        const QPoint offset = windowDecorationOffset;
        pos -= offset;
    }

    // Check if point is within any desktops rect (with some extra padding applied).
    const bool withinAnyDesktop = [&]() {
        if (auto* screen = QGuiApplication::screenAt(pos)) {
            static constexpr QMargins padding{0, 0, 10, 10};
            return screen->availableGeometry().marginsRemoved(padding).contains(pos);
        }
        return false;
    }();

    if (!withinAnyDesktop) {
        // If the widget is outside visible screen
        auto* mainWindow = getApplicationMainWindow();
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
    return {pos.x, pos.y};
}

QMenu* addMenu(std::string_view menuName, const std::string& before) {
    return addMenu(menuName, getMenu(before));
}

QMenu* addMenu(std::string_view menuName, QMenu* before) {
    if (auto* mainwin = utilqt::getApplicationMainWindow()) {
        if (!before) {
            before = getMenu("&Help", false);
        }

        auto* menuBar = mainwin->menuBar();

        if (before) {
            auto* menu = new QMenu(utilqt::toQString(menuName), menuBar);
            menuBar->insertMenu(before->menuAction(), menu);
            return menu;
        } else {  // No menu specified or couldn't find a help menu
            return menuBar->addMenu(utilqt::toQString(menuName));
        }
    }
    throw Exception("No Qt main window found");
}

QMenu* getMenu(std::string_view menuName, bool createIfNotFound) {
    if (auto* mainwin = utilqt::getApplicationMainWindow()) {
        auto* menuBar = mainwin->menuBar();
        auto menus = menuBar->findChildren<QMenu*>();

        std::string menuNoAnd{menuName};
        replaceInString(menuNoAnd, "&", "");

        auto menuItem = std::find_if(menus.begin(), menus.end(), [&](auto& m) {
            std::string title = utilqt::fromQString(m->title());
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

std::shared_ptr<Layer> toLayer(const QImage& image) {
    // Note: it is critical that the QImage format "Format_RGBA8888" is binary compatible with
    // our "LayerRAMPrecision<glm::u8vec4>".
#if (QT_VERSION < QT_VERSION_CHECK(6, 9, 0))
    const auto qImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
#else
    const auto qImage = image.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
#endif
    auto ram = std::make_shared<LayerRAMPrecision<glm::u8vec4>>(utilqt::toGLM(qImage.size()));
    const auto ramMemSize =
        glm::compMul(ram->getDimensions()) * ram->getDataFormat()->getSizeInBytes();

    if (static_cast<size_t>(qImage.sizeInBytes()) != ramMemSize) {
        throw Exception(
            SourceContext{},
            "Expected the QImage byte size {}, and Inviwo Layer byte size {}, to be equal",
            qImage.sizeInBytes(), ramMemSize);
    }

    std::memcpy(ram->getData(), qImage.bits(), qImage.sizeInBytes());

    return std::make_shared<Layer>(ram);
}

void addImageActions(QMenu& menu, const Image& image, LayerType visibleLayer, size_t visibleIndex) {
    QMenu* copy = menu.addMenu(QIcon(":svgicons/edit-copy.svg"), "Copy");
    QMenu* save = menu.addMenu(QIcon(":svgicons/save-as.svg"), "Save");

    auto addAction = [&copy, &save](const std::string& name, const Layer* layer, bool visible) {
        std::ostringstream oss;
        oss << name << (visible ? " (Visible)" : "");
        auto* copyAction = copy->addAction(oss.str().c_str());
        QAction::connect(copyAction, &QAction::triggered, [layer]() {
            rendercontext::activateDefault();
            QApplication::clipboard()->setPixmap(QPixmap::fromImage(utilqt::layerToQImage(*layer)));
        });

        auto* saveAction = save->addAction(oss.str().c_str());
        QAction::connect(saveAction, &QAction::triggered, [layer]() { util::saveLayer(*layer); });
    };

    const auto nLayers = image.getNumberOfColorLayers();
    for (size_t i = 0; i < nLayers; i++) {
        addAction("Color Layer " + (nLayers > 1 ? toString(i) : ""), image.getColorLayer(i),
                  visibleLayer == LayerType::Color && visibleIndex == i);
    }

    addAction("Picking Layer", image.getPickingLayer(), visibleLayer == LayerType::Picking);
    addAction("Depth Layer", image.getDepthLayer(), visibleLayer == LayerType::Depth);
}

void addViewActions(QMenu& menu, EventPropagator* ep) {
    auto prop = [&](auto action) {
        return [ep, action]() {
            ViewEvent e{action};
            ep->propagateEvent(&e, nullptr);
        };
    };
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-fit-to-data.svg"), "Fit to data"),
                   &QAction::triggered, prop(ViewEvent::FitData{}));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-x-p.svg"), "View from X+"),
                   &QAction::triggered, prop(camerautil::Side::XPositive));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-x-m.svg"), "View from X-"),
                   &QAction::triggered, prop(camerautil::Side::XNegative));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-y-p.svg"), "View from Y+"),
                   &QAction::triggered, prop(camerautil::Side::YPositive));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-y-m.svg"), "View from Y-"),
                   &QAction::triggered, prop(camerautil::Side::YNegative));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-z-p.svg"), "View from Z+"),
                   &QAction::triggered, prop(camerautil::Side::ZPositive));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-z-m.svg"), "View from Z-"),
                   &QAction::triggered, prop(camerautil::Side::ZNegative));
    QMenu::connect(menu.addAction(QIcon(":svgicons/view-flip.svg"), "Flip Up Vector"),
                   &QAction::triggered, prop(ViewEvent::FlipUp{}));
}

std::string toBase64(const QImage& image, std::string_view format, int quality) {
    QByteArray byteArray;
    QBuffer buffer{&byteArray};
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.empty() ? nullptr : SafeCStr{format}.c_str(), quality);
    return std::string{byteArray.toBase64().data()};
}

QImage fromBase64(std::string_view base64, std::string_view format) {
    const QByteArray barray =
        QByteArray::fromBase64(QByteArray{base64.data(), static_cast<int>(base64.size())});
    QImage image;
    image.loadFromData(barray, format.empty() ? nullptr : SafeCStr{format}.c_str());
    return image;
}

QIcon fromBase64ToIcon(std::string_view base64, std::string_view format) {
    QPixmap pm;
    pm.loadFromData(
        QByteArray::fromBase64(QByteArray{base64.data(), static_cast<int>(base64.size())}),
        format.empty() ? nullptr : SafeCStr{format}.c_str());
    return QIcon(pm);
}

std::vector<std::pair<std::string, QImage>> getCanvasImages(ProcessorNetwork* network, bool alpha) {
    std::vector<std::pair<std::string, QImage>> images;

    network->forEachProcessor([&](Processor* p) {
        if (const auto* exporter = dynamic_cast<const ImageExporter*>(p)) {
            if (p->isSink() && p->isReady()) {
                if (auto img = exporter->getImage()) {
                    if (const auto* layer = img->getColorLayer()) {
                        auto qimg = utilqt::layerToQImage(*layer).scaledToHeight(256);
                        images.emplace_back(p->getDisplayName(), qimg);
                    }
                }
            }
        }
    });

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

    const QLatin1String placeHolder("[*]");
    int index = static_cast<int>(cap.indexOf(placeHolder));

    // here the magic begins
    while (index != -1) {
        index += static_cast<int>(placeHolder.size());
        int count = 1;
        while (cap.indexOf(placeHolder, index) == index) {
            ++count;
            index += static_cast<int>(placeHolder.size());
        }
        if ((count % 2) != 0) {  // odd number of [*] -> replace last one
            const int lastIndex = static_cast<int>(cap.lastIndexOf(placeHolder, index - 1));
            if (widget->isWindowModified()) {
                cap.replace(lastIndex, 3, QWidget::tr("*"));
            } else {
                cap.remove(lastIndex, 3);
            }
        }
        index = static_cast<int>(cap.indexOf(placeHolder, index));
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

std::vector<std::string> getMonoSpaceFonts() {
    std::vector<std::string> fonts;

    for (auto& font : QFontDatabase::families()) {
        if (QFontDatabase::isFixedPitch(font)) {
            fonts.push_back(utilqt::fromQString(font));
        }
    }

    return fonts;
}

size_t getDefaultMonoSpaceFontIndex() {
    const auto fonts = getMonoSpaceFonts();
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    auto it = std::find(fonts.begin(), fonts.end(), utilqt::fromQString(fixedFont.family()));
    if (it != fonts.end()) {
        return it - fonts.begin();
    } else {
        return 0;
    }
}

WidgetCloseEventFilter::WidgetCloseEventFilter(QObject* parent) : QObject(parent) {}

bool WidgetCloseEventFilter::eventFilter(QObject* obj, QEvent* ev) {
    if (ev->type() == QEvent::Close) {
        auto* dialog = qobject_cast<QWidget*>(obj);
        dialog->hide();
        ev->ignore();
        return true;
    } else {
        return false;
    }
}

void setFullScreen(QWidget* widget, bool fullScreen) {
    if (widget->windowState().testFlag(Qt::WindowFullScreen) == fullScreen) return;
    const auto visible = widget->isVisible();
    if (fullScreen) {
        // Prevent Qt resize event with incorrect size when going full screen.
        // Reproduce error by loading a workspace with a full screen canvas.
        // This is equivalent to suggested solution using QTimer
        // https://stackoverflow.com/questions/19817881/qt-fullscreen-on-startup
        // No need to process user events, i.e. mouse/keyboard etc.
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        widget->setWindowFlags(
            widget->windowFlags().setFlag(Qt::Tool, false).setFlag(Qt::Window, true));
        widget->setWindowState(widget->windowState() | Qt::WindowFullScreen);
    } else {
        widget->setWindowState(widget->windowState() & ~Qt::WindowFullScreen);
        widget->setWindowFlags(widget->windowFlags().setFlag(Qt::Tool, true));
    }
    widget->setVisible(visible);
}

void setFullScreenAndOnTop(QWidget* widget, bool fullScreen, bool onTop) {
    if (widget->windowFlags().testFlag(Qt::Tool) == onTop &&
        widget->windowState().testFlag(Qt::WindowFullScreen) == fullScreen) {
        return;
    }

    // setWindowFlag will alwyas hide the widget
    // https://doc.qt.io/qt-5/qwidget.html#windowFlags-prop
    const auto visible = widget->isVisible();

    // Prevent Qt resize event with incorrect size when going full screen.
    // Reproduce error by loading a workspace with a full screen canvas.
    // This is equivalent to suggested solution using QTimer
    // https://stackoverflow.com/questions/19817881/qt-fullscreen-on-startup
    // No need to process user events, i.e. mouse/keyboard etc.
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    if (fullScreen) {
        // Always use window mode for fullscreen
        widget->setWindowFlags(
            widget->windowFlags().setFlag(Qt::Tool, false).setFlag(Qt::Window, true));
        widget->setWindowState(widget->windowState() | Qt::WindowFullScreen);
    } else {
        widget->setWindowState(widget->windowState() & ~Qt::WindowFullScreen);

        if (onTop) {
            widget->setWindowFlags(widget->windowFlags().setFlag(Qt::Tool, true));
        } else {
            widget->setWindowFlags(
                widget->windowFlags().setFlag(Qt::Tool, false).setFlag(Qt::Window, true));
        }
    }

    widget->setVisible(visible);
}

QString formatAsNonscientific(double value) {
    // QString always uses QLocale::C
    const int visibleDigits = 8;
    const double scientificRepThreshold = 1e-6;

    if (std::abs(value) < scientificRepThreshold) {
        return QString::number(value, 'g', visibleDigits + 1);
    } else {
        const int int_digits =
            static_cast<int>(std::floor(std::log10(std::max(std::abs(value), 1.0)))) + 1;

        QString str = QString::number(value, 'f', std::max(visibleDigits + 1 - int_digits, 1));
        if (str.contains('.')) {
            // remove trailing zeros
            while (*str.rbegin() == '0') {
                str.chop(1);
            }
            if (*str.rbegin() == '.') {
                str.chop(1);
            }
        }
        return str;
    }
}

QString toQString(const std::filesystem::path& path) {
    auto str = path.generic_u8string();
    return QString::fromUtf8(reinterpret_cast<char*>(str.data()), str.size());
}

std::filesystem::path toPath(const QString& str) {
    auto buffer = str.toUtf8();
    const std::u8string_view u8str{reinterpret_cast<const char8_t*>(buffer.constData()),
                                   static_cast<size_t>(buffer.size())};
    return std::filesystem::path{u8str};
}

}  // namespace inviwo::utilqt
