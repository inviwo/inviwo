/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2021 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/util/glmvec.h>
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
#include <QLocale>
#include <warn/pop>

#include <string_view>

class QFontMetrics;

namespace inviwo {

class Property;
class ProcessorNetwork;
class EventPropagator;

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
 * \brief create a QString from a UTF8-encoded std::string
 */
inline QString toLocalQString(std::string_view str) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QString::fromLocal8Bit(str.data(), str.size());
#else
    return QString::fromLocal8Bit(str.data(), static_cast<int>(str.size()));
#endif
}
/**
 * \brief create a QString from a UTF8-encoded std::string
 */
inline QString toLocalQString(const std::string& str) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QString::fromLocal8Bit(str.data(), str.size());
#else
    return QString::fromLocal8Bit(str.data(), static_cast<int>(str.size()));
#endif
}
/**
 * \brief convert a QString to a localized 8bit std::string
 */
inline std::string fromLocalQString(const QString& str) {
    return std::string(str.toLocal8Bit().constData());
}

/**
 * \brief create a QString from a UTF8-encoded std::string
 */
inline QString toQString(std::string_view str) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QString::fromUtf8(str.data(), str.size());
#else
    return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
#endif
}
/**
 * \brief create a QString from a UTF8-encoded std::string
 */
inline QString toQString(const std::string& str) {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QString::fromUtf8(str.data(), str.size());
#else
    return QString::fromUtf8(str.data(), static_cast<int>(str.size()));
#endif
}
/**
 * \brief create a UTF8-encoded std::string from a QString
 */
inline std::string fromQString(const QString& str) { return std::string(str.toUtf8().constData()); }

constexpr QPointF toQPoint(dvec2 v) { return QPointF(v.x, v.y); }
constexpr QPoint toQPoint(ivec2 v) { return QPoint(v.x, v.y); }

constexpr dvec2 toGLM(QPointF v) { return dvec2(v.x(), v.y()); }
constexpr ivec2 toGLM(QPoint v) { return ivec2(v.x(), v.y()); }
constexpr dvec2 toGLM(QSizeF v) { return dvec2(v.width(), v.height()); }
constexpr ivec2 toGLM(QSize v) { return ivec2(v.width(), v.height()); }

constexpr QSizeF toQSize(dvec2 v) { return QSizeF(v.x, v.y); }
constexpr QSize toQSize(ivec2 v) { return QSize(v.x, v.y); }

inline vec3 tovec3(const QColor& c) { return vec3(c.redF(), c.greenF(), c.blueF()); }
inline ivec3 toivec3(const QColor& c) { return ivec3(c.red(), c.green(), c.blue()); }
inline vec4 tovec4(const QColor& c) { return vec4(c.redF(), c.greenF(), c.blueF(), c.alphaF()); }
inline ivec4 toivec4(const QColor& c) { return ivec4(c.red(), c.green(), c.blue(), c.alpha()); }

constexpr QColor toQColor(const ivec3& v) { return QColor(v.r, v.g, v.b); }
constexpr QColor toQColor(const uvec3& v) { return QColor(v.r, v.g, v.b); }
constexpr QColor toQColor(const vec3& v) { return toQColor(ivec3(v * 255.0f)); }

constexpr QColor toQColor(const ivec4& v) { return QColor(v.r, v.g, v.b, v.a); }
constexpr QColor toQColor(const uvec4& v) { return QColor(v.r, v.g, v.b, v.a); }
constexpr QColor toQColor(const vec4& v) { return toQColor(ivec4(v * 255.0f)); }

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunction& tf, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunctionProperty& tfproperty,
                                           const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const IsoValueProperty& property, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const util::TFPropertyConcept& propertyConcept,
                                           const QSize& size);

IVW_MODULE_QTWIDGETS_API QPointF clamp(const QPointF& pos, const QRectF& rect);

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
IVW_MODULE_QTWIDGETS_API std::string toBase64(const QImage& image, std::string_view format = "PNG",
                                              int quality = -1);
/*
 * \brief Convert base64-encoded string to QImage.
 *
 * @param base64 byte array to be converted
 * @param format of data (e.g. "jpeg")
 * @return QImage representation of the string
 *
 * \see toBase64
 */
IVW_MODULE_QTWIDGETS_API QImage fromBase64(std::string_view base64,
                                           std::string_view format = "PNG");

/*
 * \brief Convert base64-encoded string to QIcon.
 *
 * @param base64 byte array to be converted
 * @param format of data (e.g. "jpeg"), auto-detected if empty.
 * @return QIcon representation of the string
 *
 * \see QPixmap::loadFromData
 */
IVW_MODULE_QTWIDGETS_API QIcon fromBase64ToIcon(std::string_view base64,
                                                std::string_view format = "PNG");

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

IVW_MODULE_QTWIDGETS_API void addViewActions(QMenu& menu, EventPropagator* ep);

/*
 * \brief formats a title string similar to QWidget::setWindowTitle, i.e. "[*]" is replaced with
 * either nothing or '*' depending on the modification state of \p widget
 *
 * @return title where "[*]" is replaced with '*' if the widget is modified, or '' otherwise
 * \see QWidget::setWindowTitle
 */
QString windowTitleHelper(const QString& title, const QWidget* widget);

// In a non high dpi system an 'M' measures 11 px. Hence all our old pixels sizes, can be converted
// to Em sizes by dividing by 11
constexpr int refEm() { return 11; }
constexpr double refSpaceEm() { return 7.0 / refEm(); }
IVW_MODULE_QTWIDGETS_API int refSpacePx(const QWidget* w);
IVW_MODULE_QTWIDGETS_API QSize emToPx(const QWidget* w, QSizeF);
IVW_MODULE_QTWIDGETS_API int emToPx(const QWidget* w, double em);
IVW_MODULE_QTWIDGETS_API int emToPx(const QFontMetrics& m, double em);

/**
 * Workaround for closing Widget using the "X" button in the titlebar
 * This filter intercepts the CloseEvent and ignores it, and then hides the widget
 */
struct IVW_MODULE_QTWIDGETS_API WidgetCloseEventFilter : QObject {
    WidgetCloseEventFilter(QObject* parent);
    virtual bool eventFilter(QObject* obj, QEvent* ev) override;
};

IVW_MODULE_QTWIDGETS_API void setFullScreenAndOnTop(QWidget* widget, bool fullScreen, bool onTop);

template <typename T>
int decimals([[maybe_unused]] double inc) {
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

/**
 * @brief Get a list of all available mono space font
 *
 * Queries the @c QFontDatabase for all font that @c isFixedPitch
 */
IVW_MODULE_QTWIDGETS_API std::vector<std::string> getMonoSpaceFonts();

/**
 * @brief Index of the system mono space font
 *
 * Returns the index of the default system mono space font in the list given by getMonoSpaceFonts
 * @see getMonoSpaceFonts()
 * @return the index or 0 if the default font was not found
 */
IVW_MODULE_QTWIDGETS_API size_t getDefaultMonoSpaceFontIndex();

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
