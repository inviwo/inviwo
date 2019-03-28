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

#ifndef IVW_INVIWOQTUTILS_H
#define IVW_INVIWOQTUTILS_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/image/imagetypes.h>

#include <locale>
#include <ios>
#include <sstream>
#include <warn/push>
#include <warn/ignore/all>
#include <QString>
#include <QPointF>
#include <QPoint>
#include <QSizeF>
#include <QSize>
#include <QColor>
#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <warn/pop>

class QFontMetrics;

namespace inviwo {

class Property;
class ProcessorNetwork;

class TransferFunction;
class TransferFunctionProperty;
class IsoTFProperty;
class IsoValueProperty;

class Layer;
class Image;

namespace util {
struct TFPropertyConcept;
}

namespace utilqt {

/**
 * \brief getCurrentStdLocale
 * This function returns the current system locale provided by Qt.
 * If the Qt application has not been initialized, the returned
 * value is the environment's default locale.
 *
 * @return std::locale   Qt locale converted to std::locale
 */
IVW_MODULE_QTWIDGETS_API std::locale getCurrentStdLocale();

/**
 * \brief localize
 * The given stream is imbued with the currently set system locale provided by Qt.
 *
 * @param stream   the locale is imbued onto this stream
 * @return std::ios_base&  reference to the input stream
 */
IVW_MODULE_QTWIDGETS_API std::ios_base& localizeStream(std::ios_base& stream);

/**
 * \brief convert a std::string to a localized QString using the currently set system locale
 */
IVW_MODULE_QTWIDGETS_API QString toLocalQString(const std::string&);
/**
 * \brief convert a QString to a localized 8bit std::string
 */
IVW_MODULE_QTWIDGETS_API std::string fromLocalQString(const QString& input);

/**
 * \brief create a QString from a UTF8-encoded std::string
 */
IVW_MODULE_QTWIDGETS_API QString toQString(const std::string&);
/**
 * \brief create a UTF8-encoded std::string from a QString
 */
IVW_MODULE_QTWIDGETS_API std::string fromQString(const QString& input);

IVW_MODULE_QTWIDGETS_API dvec2 toGLM(QPointF);
IVW_MODULE_QTWIDGETS_API ivec2 toGLM(QPoint);

IVW_MODULE_QTWIDGETS_API QPointF toQPoint(dvec2);
IVW_MODULE_QTWIDGETS_API QPoint toQPoint(ivec2);

IVW_MODULE_QTWIDGETS_API dvec2 toGLM(QSizeF);
IVW_MODULE_QTWIDGETS_API ivec2 toGLM(QSize);

IVW_MODULE_QTWIDGETS_API QSizeF toQSize(dvec2);
IVW_MODULE_QTWIDGETS_API QSize toQSize(ivec2);

IVW_MODULE_QTWIDGETS_API vec3 tovec3(const QColor&);
IVW_MODULE_QTWIDGETS_API ivec3 toivec3(const QColor&);

IVW_MODULE_QTWIDGETS_API vec4 tovec4(const QColor&);
IVW_MODULE_QTWIDGETS_API ivec4 toivec4(const QColor&);

IVW_MODULE_QTWIDGETS_API QColor toQColor(const vec3&);
IVW_MODULE_QTWIDGETS_API QColor toQColor(const ivec3&);
IVW_MODULE_QTWIDGETS_API QColor toQColor(const uvec3&);
IVW_MODULE_QTWIDGETS_API QColor toQColor(const vec4&);
IVW_MODULE_QTWIDGETS_API QColor toQColor(const ivec4&);
IVW_MODULE_QTWIDGETS_API QColor toQColor(const uvec4&);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunction& tf, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunctionProperty& tfproperty,
                                           const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const IsoValueProperty& property, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const util::TFPropertyConcept& propertyConcept,
                                           const QSize& size);

/**
 * \brief Retrieve the QMainWindow named "InviwoMainWindow" from QApplication.
 * @return The main window if the application contains the widget, otherwise null.
 */
IVW_MODULE_QTWIDGETS_API QMainWindow* getApplicationMainWindow();

/**
 * \brief Moves point to become relative to the main window and thereby visible.
 *
 * Positions saved for one screen setup may end up outside on other setups.
 * Apply this function to make sure that the widget ends up visible on the screen.
 * @see CanvasProcessorWidgetQt
 * @param point The previous screen position of the widget
 * @param size The size of the widget
 * @param decorationOffset Offset widget below top horizontal window bar
 * @return The adapted point on the screen, same as input point if no adjustment was necessary
 */
IVW_MODULE_QTWIDGETS_API QPoint movePointOntoDesktop(const QPoint& point, const QSize& size,
                                                     bool decorationOffset = true);

/**
 * \brief Offset widgets based on order added such that they do not end up on top of each other.
 * Base offset of (350, 100). The offset will be increased by (40, 40) every time the function is
 * called. Furthermore, the horizontal offset will be increased by 200 every tenth time.
 * @return
 */
IVW_MODULE_QTWIDGETS_API QPoint offsetWidget();

IVW_MODULE_QTWIDGETS_API QMenu* addMenu(std::string menuName, std::string before);
IVW_MODULE_QTWIDGETS_API QMenu* addMenu(std::string menuName, QMenu* before = nullptr);
IVW_MODULE_QTWIDGETS_API QMenu* getMenu(std::string menuName, bool createIfNotFound = false);

IVW_MODULE_QTWIDGETS_API QImage layerToQImage(const Layer& layer);

/*
 * \brief save the given QImage \p image as base64-encoded string using the image file format \p
 * format and image \p quality.
 *
 * @param image    image to be encoded
 * @param format   image file format
 * @param quality  image quality [0,100] (0 - small compressed, 100 - large uncompressed),
 *                 -1 uses default settings
 * @return base64 string of the corresponding image
 *
 * \see QImage::save()
 */
IVW_MODULE_QTWIDGETS_API std::string toBase64(const QImage& image,
                                              const std::string& format = "PNG", int quality = -1);

/*
 * \brief retrieve the contents of all visible canvases as QImage. A canvas must be ready and
 * visible in order to be considered.
 *
 * @param network    visible canvases are extracted from this processor network
 * @param alpha      the resulting images will retain their alpha channel if true
 * @return vector of pairs representing the display name and contents of the respective canvases
 */
IVW_MODULE_QTWIDGETS_API std::vector<std::pair<std::string, QImage>> getCanvasImages(
    ProcessorNetwork* network, bool alpha = true);

IVW_MODULE_QTWIDGETS_API void addImageActions(QMenu& menu, const Image& image,
                                              LayerType visibleLayer = LayerType::Color,
                                              size_t visibleIndex = 10000);

/*
 * \brief formats a title string similar to QWidget::setWindowTitle, i.e. "[*]" is replaced with
 * either nothing or '*' depending on the modification state of \p widget
 *
 * @return title where "[*]" is replaced with '*' if the widget is modified, or '' otherwise
 * \see QWidget::setWindowTitle
 */
IVW_MODULE_QTWIDGETS_API QString windowTitleHelper(const QString& title, const QWidget* widget);

// In a non high dpi system an 'M' measures 11 px. Hence all our old pixels sizes, can be converted
// to Em sizes by dividing by 11
constexpr int refEm() { return 11; }
constexpr double refSpaceEm() { return 7.0 / refEm(); }
IVW_MODULE_QTWIDGETS_API int refSpacePx(const QWidget* w);
IVW_MODULE_QTWIDGETS_API QSize emToPx(const QWidget* w, QSizeF);
IVW_MODULE_QTWIDGETS_API int emToPx(const QWidget* w, double em);
IVW_MODULE_QTWIDGETS_API int emToPx(const QFontMetrics& m, double em);

}  // namespace utilqt

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

}  // namespace inviwo

#endif  // IVW_INVIWOQTUTILS_H
