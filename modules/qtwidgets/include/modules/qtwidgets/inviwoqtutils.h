/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/datastructures/image/imagetypes.h>  // for LayerType, LayerType::Color
#include <inviwo/core/util/glmvec.h>                      // for dvec2, ivec2, ivec4, ivec3, uvec4
#include <inviwo/qt/applicationbase/qtlocale.h>

#include <cstddef>      // for size_t
#include <ios>          // for ostringstream, istringstream
#include <locale>       // for locale
#include <sstream>      // for basic_ostream
#include <string>       // for string
#include <string_view>  // for string_view
#include <utility>      // for pair
#include <vector>       // for vector
#include <filesystem>
#include <charconv>
#include <optional>

#include <QByteArray>  // for QByteArray
#include <QColor>      // for QColor
#include <QIcon>       // for QIcon
#include <QImage>      // for QImage
#include <QLocale>     // for QLocale
#include <QObject>     // for QObject
#include <QPixmap>     // for QPixmap
#include <QPoint>      // for QPoint
#include <QPointF>     // for QPointF
#include <QRectF>      // for QRectF
#include <QSize>       // for QSize
#include <QSizeF>      // for QSizeF
#include <QString>     // for QString
#include <QModelIndex>
#include <QPen>
#include <QEvent>
#include <glm/vec2.hpp>  // for vec<>::(anonymous)
#include <glm/vec3.hpp>  // for vec<>::(anonymous), operator*
#include <glm/vec4.hpp>  // for vec<>::(anonymous), operator*

#if !(defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L)
#include <fast_float/fast_float.h>
#endif

class QEvent;
class QMainWindow;
class QMenu;
class QRectF;
class QWidget;
class QFontMetrics;

namespace inviwo {

class EventPropagator;
class Image;
class IsoValueProperty;
class IsoValueCollection;
class Layer;
class ProcessorNetwork;
class TransferFunction;
class TransferFunctionProperty;
class TFPropertyConcept;
class IsoTFProperty;

namespace utilqt {

/**
 * @brief localize
 * The given stream is imbued with the currently set system locale provided by Qt.
 *
 * @param stream   the locale is imbued onto this stream
 * @return std::ios_base&  reference to the input stream
 */
IVW_MODULE_QTWIDGETS_API std::ios_base& localizeStream(std::ios_base& stream);

/**
 * @brief create a QString from a UTF8-encoded std::string
 */
inline QString toLocalQString(std::string_view str) {
    return QString::fromLocal8Bit(str.data(), str.size());
}
/**
 * @brief create a QString from a UTF8-encoded std::string
 */
inline QString toLocalQString(const std::string& str) {
    return QString::fromLocal8Bit(str.data(), str.size());
}
/**
 * @brief convert a QString to a localized 8bit std::string
 */
inline std::string fromLocalQString(const QString& str) { return {str.toLocal8Bit().constData()}; }

/**
 * @brief create a QString from a UTF8-encoded std::string
 */
inline QString toQString(std::string_view str) { return QString::fromUtf8(str.data(), str.size()); }
/**
 * @brief create a QString from a UTF8-encoded std::string
 */
inline QString toQString(const std::string& str) {
    return QString::fromUtf8(str.data(), str.size());
}

/**
 * @brief create a UTF8-encoded std::string from a QString
 */
inline std::string fromQString(const QString& str) { return {str.toUtf8().constData()}; }

/**
 * @brief Extract a numeric value of type \p T from \p str using the C locale.
 *
 * Extract a numeric value of type \p T from \p str using the C locale. For floating point values
 * '.' and ',' are accepted as decimal point. The string must contain only the value.
 *
 * @tparam T   type of returned value
 * @param str  string containing the numeric value
 * @return extracted value if conversion was successful, nullopt otherwise
 */
template <typename T>
std::optional<T> numericValueFromString(const QString& str) {
    QByteArray arrayBuf{str.trimmed().toUtf8()};
    if constexpr (std::is_floating_point_v<T>) {
        // replace commas with period, thus accepting both 7.34 and 7,34
        for (auto& c : arrayBuf) {
            if (c == ',') {
                c = '.';
            }
        }
    }
    T value{0};

#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    auto [ptr, ec] = std::from_chars(arrayBuf.constBegin(), arrayBuf.constEnd(), value);
#else
    auto [ptr, ec] = fast_float::from_chars(arrayBuf.constBegin(), arrayBuf.constEnd(), value);
#endif
    if (ec == std::errc{} && ptr == arrayBuf.constEnd()) {
        return value;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Formats a floating point number as QString.
 *
 * Formats the given floating point number \p value to a QString using the C locale.
 * The default representation has eight decimals after the decimal point, e.g. a value of
 * 0.123456789 yields "0.12345679". Each order of magnitude reduces the number of decimals by
 * one, i.e. 100.12345678 will be represented as "100.123457". Numbers smaller than 1e-6 are
 * shown in scientific representation.
 *
 * @param value   floating point number
 * @return formatted number string with significant digits, avoiding scientific notation if possible
 */
IVW_MODULE_QTWIDGETS_API QString formatAsNonscientific(double value);

IVW_MODULE_QTWIDGETS_API QString toQString(const std::filesystem::path& path);
IVW_MODULE_QTWIDGETS_API std::filesystem::path toPath(const QString& str);

constexpr QPointF toQPoint(dvec2 v) { return {v.x, v.y}; }
constexpr QPoint toQPoint(ivec2 v) { return {v.x, v.y}; }

constexpr dvec2 toGLM(QPointF v) { return {v.x(), v.y()}; }
constexpr ivec2 toGLM(QPoint v) { return {v.x(), v.y()}; }
constexpr dvec2 toGLM(QSizeF v) { return {v.width(), v.height()}; }
constexpr ivec2 toGLM(QSize v) { return {v.width(), v.height()}; }

constexpr QSizeF toQSize(dvec2 v) { return {v.x, v.y}; }
constexpr QSize toQSize(ivec2 v) { return {v.x, v.y}; }

inline vec3 tovec3(const QColor& c) { return {c.redF(), c.greenF(), c.blueF()}; }
inline ivec3 toivec3(const QColor& c) { return {c.red(), c.green(), c.blue()}; }
inline vec4 tovec4(const QColor& c) { return {c.redF(), c.greenF(), c.blueF(), c.alphaF()}; }
inline ivec4 toivec4(const QColor& c) { return {c.red(), c.green(), c.blue(), c.alpha()}; }

constexpr QColor toQColor(const ivec3& v) { return QColor(v.r, v.g, v.b); }
constexpr QColor toQColor(const uvec3& v) { return QColor(v.r, v.g, v.b); }
constexpr QColor toQColor(const vec3& v) { return toQColor(ivec3(v * 255.0f)); }

constexpr QColor toQColor(const ivec4& v) { return QColor(v.r, v.g, v.b, v.a); }
constexpr QColor toQColor(const uvec4& v) { return QColor(v.r, v.g, v.b, v.a); }
constexpr QColor toQColor(const vec4& v) { return toQColor(ivec4(v * 255.0f)); }

IVW_MODULE_QTWIDGETS_API QPen cosmeticPen(const QBrush& brush, qreal width,
                                          Qt::PenStyle s = Qt::SolidLine,
                                          Qt::PenCapStyle c = Qt::SquareCap,
                                          Qt::PenJoinStyle j = Qt::BevelJoin);

IVW_MODULE_QTWIDGETS_API void paintCheckerBoard(QPainter& painter, const QRectF& rect);
IVW_MODULE_QTWIDGETS_API void paint(const TransferFunction& tf, QPainter& painter,
                                    const QRectF& rect);
IVW_MODULE_QTWIDGETS_API void paint(const IsoValueCollection& isoValues, QPainter& painter,
                                    const QRectF& rect);
IVW_MODULE_QTWIDGETS_API void paintMask(const dvec2& mask, const dvec2& range, QPainter& painter,
                                        const QRectF& rect);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunction& tf, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TransferFunctionProperty& tfproperty,
                                           const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const IsoValueProperty& property, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const IsoTFProperty& property, const QSize& size);

IVW_MODULE_QTWIDGETS_API QPixmap toQPixmap(const TFPropertyConcept& propertyConcept,
                                           const QSize& size);

IVW_MODULE_QTWIDGETS_API QPointF clamp(const QPointF& pos, const QRectF& rect);

/**
 * @brief Retrieve the QMainWindow named "InviwoMainWindow" from QApplication.
 * @return The main window if the application contains the widget, otherwise null.
 */
IVW_MODULE_QTWIDGETS_API QMainWindow* getApplicationMainWindow();

/**
 * @brief Moves point to become relative to the main window and thereby visible.
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
 * @brief Offset widgets based on order added such that they do not end up on top of each other.
 * Base offset of (350, 100). The offset will be increased by (40, 40) every time the function is
 * called. Furthermore, the horizontal offset will be increased by 200 every tenth time.
 * @return
 */
IVW_MODULE_QTWIDGETS_API QPoint offsetWidget();

IVW_MODULE_QTWIDGETS_API QMenu* addMenu(std::string_view menuName, const std::string& before);
IVW_MODULE_QTWIDGETS_API QMenu* addMenu(std::string_view menuName, QMenu* before = nullptr);
IVW_MODULE_QTWIDGETS_API QMenu* getMenu(std::string_view menuName, bool createIfNotFound = false);

IVW_MODULE_QTWIDGETS_API QImage layerToQImage(const Layer& layer);

IVW_MODULE_QTWIDGETS_API std::shared_ptr<Layer> toLayer(const QImage& image);

/*
 * @brief save the given QImage \p image as base64-encoded string using the image file format \p
 * format and image \p quality.
 *
 * @param image    image to be encoded
 * @param format   image file format
 * @param quality  image quality [0,100] (0 - small compressed, 100 - large uncompressed),
 *                 -1 uses default settings
 * @return base64 string of the corresponding image
 *
 * @see QImage::save()
 */
IVW_MODULE_QTWIDGETS_API std::string toBase64(const QImage& image, std::string_view format = "PNG",
                                              int quality = -1);
/*
 * @brief Convert base64-encoded string to QImage.
 *
 * @param base64 byte array to be converted
 * @param format of data (e.g. "jpeg")
 * @return QImage representation of the string
 *
 * @see toBase64
 */
IVW_MODULE_QTWIDGETS_API QImage fromBase64(std::string_view base64,
                                           std::string_view format = "PNG");

/*
 * @brief Convert base64-encoded string to QIcon.
 *
 * @param base64 byte array to be converted
 * @param format of data (e.g. "jpeg"), auto-detected if empty.
 * @return QIcon representation of the string
 *
 * @see QPixmap::loadFromData
 */
IVW_MODULE_QTWIDGETS_API QIcon fromBase64ToIcon(std::string_view base64,
                                                std::string_view format = "PNG");

/*
 * @brief retrieve the contents of all visible canvases as QImage. A canvas must be ready and
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
 * @brief formats a title string similar to QWidget::setWindowTitle, i.e. "[*]" is replaced with
 * either nothing or '*' depending on the modification state of \p widget
 *
 * @return title where "[*]" is replaced with '*' if the widget is modified, or '' otherwise
 * @see QWidget::setWindowTitle
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
    explicit WidgetCloseEventFilter(QObject* parent);
    virtual bool eventFilter(QObject* obj, QEvent* ev) override;
};

IVW_MODULE_QTWIDGETS_API void setFullScreenAndOnTop(QWidget* widget, bool fullScreen, bool onTop);

template <typename T>
int decimals([[maybe_unused]] T inc) {
    if constexpr (std::is_floating_point_v<T>) {
        if (inc == T{0.0} || inc >= T{1.0}) return 0;
        return static_cast<int>(-std::log10(inc));
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

template <typename T>
struct Save {
    explicit Save(T* item) : item_(item) { item->save(); }
    Save(const Save&) = delete;
    Save(Save&& rhs) noexcept : item_{rhs.item_} { rhs.item_ = nullptr; };
    Save& operator=(const Save&) = delete;
    Save& operator=(Save&& that) noexcept {
        if (this != &that) {
            std::swap(item_, that.item_);
            if (that.item_) {
                that.item_->disable();
                that.item_ = nullptr;
            }
        }
        return *this;
    };
    ~Save() {
        if (item_) item_->restore();
    }

private:
    T* item_;
};

template <typename Enum>
QVariant getData(const QModelIndex& index, Enum e) {
    return index.data(static_cast<std::underlying_type_t<Enum>>(e));
}
inline QVariant getData(const QModelIndex& index, Qt::ItemDataRole r) { return index.data(r); }

}  // namespace utilqt

template <class T>
std::string toLocalizedString(T value) {
    std::ostringstream stream;
    stream.imbue(utilqt::getCurrentStdLocale());
    stream << value;
    return std::move(stream).str();
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

class QActionEvent;
class QChildEvent;
class QCloseEvent;
class QContextMenuEvent;
class QDeferredDeleteEvent;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QEnterEvent;
class QFileOpenEvent;
class QFocusEvent;
class QGestureEvent;
class QGraphicsItem;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneDragDropEvent;
class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneMoveEvent;
class QGraphicsSceneResizeEvent;
class QGraphicsSceneWheelEvent;
class QHelpEvent;
class QHideEvent;
class QHoverEvent;
class QIconDragEvent;
class QInputMethodEvent;
class QInputMethodQueryEvent;
class QKeyEvent;
class QMouseEvent;
class QMoveEvent;
class QNativeGestureEvent;
class QPaintEvent;
class QPlatformSurfaceEvent;
class QResizeEvent;
class QScreenOrientationChangeEvent;
class QScrollEvent;
class QScrollPrepareEvent;
class QShortcutEvent;
class QShowEvent;
class QStateMachine;
class QStatusTipEvent;
class QTabletEvent;
class QTimerEvent;
class QTouchEvent;
class QWheelEvent;
class QWindowStateChangeEvent;

namespace inviwo::utilqt {
namespace detail {
    template <QEvent::Type Type>
    auto qEventMap() {
        if constexpr (Type == QEvent::None) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ActionAdded) {
            return static_cast<QActionEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ActionChanged) {
            return static_cast<QActionEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ActionRemoved) {
            return static_cast<QActionEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ActivationChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ApplicationFontChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ApplicationLayoutDirectionChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ApplicationPaletteChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ApplicationStateChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ApplicationWindowIconChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ChildAdded) {
            return static_cast<QChildEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ChildPolished) {
            return static_cast<QChildEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ChildRemoved) {
            return static_cast<QChildEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ChildWindowAdded) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ChildWindowRemoved) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Clipboard) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Close) {
            return static_cast<QCloseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::CloseSoftwareInputPanel) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ContentsRectChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ContextMenu) {
            return static_cast<QContextMenuEvent*>(nullptr);
        } else if constexpr (Type == QEvent::CursorChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DeferredDelete) {
            return static_cast<QDeferredDeleteEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DevicePixelRatioChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DragEnter) {
            return static_cast<QDragEnterEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DragLeave) {
            return static_cast<QDragLeaveEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DragMove) {
            return static_cast<QDragMoveEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Drop) {
            return static_cast<QDropEvent*>(nullptr);
        } else if constexpr (Type == QEvent::DynamicPropertyChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::EnabledChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Enter) {
            return static_cast<QEnterEvent*>(nullptr);
#ifdef QT_KEYPAD_NAVIGATION
        } else if constexpr (Type == QEvent::EnterEditFocus) {
            return static_cast<QEvent*>(nullptr);
#endif
        } else if constexpr (Type == QEvent::EnterWhatsThisMode) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Expose) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::FileOpen) {
            return static_cast<QFileOpenEvent*>(nullptr);
        } else if constexpr (Type == QEvent::FocusIn) {
            return static_cast<QFocusEvent*>(nullptr);
        } else if constexpr (Type == QEvent::FocusOut) {
            return static_cast<QFocusEvent*>(nullptr);
        } else if constexpr (Type == QEvent::FocusAboutToChange) {
            return static_cast<QFocusEvent*>(nullptr);
        } else if constexpr (Type == QEvent::FontChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Gesture) {
            return static_cast<QGestureEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GestureOverride) {
            return static_cast<QGestureEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GrabKeyboard) {
            return static_cast<QGraphicsItem*>(nullptr);
        } else if constexpr (Type == QEvent::GrabMouse) {
            return static_cast<QGraphicsItem*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneContextMenu) {
            return static_cast<QGraphicsSceneContextMenuEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneDragEnter) {
            return static_cast<QGraphicsSceneDragDropEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneDragLeave) {
            return static_cast<QGraphicsSceneDragDropEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneDragMove) {
            return static_cast<QGraphicsSceneDragDropEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneDrop) {
            return static_cast<QGraphicsSceneDragDropEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneHelp) {
            return static_cast<QHelpEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneHoverEnter) {
            return static_cast<QGraphicsSceneHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneHoverLeave) {
            return static_cast<QGraphicsSceneHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneHoverMove) {
            return static_cast<QGraphicsSceneHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneMouseDoubleClick) {
            return static_cast<QGraphicsSceneMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneMouseMove) {
            return static_cast<QGraphicsSceneMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneMousePress) {
            return static_cast<QGraphicsSceneMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneMouseRelease) {
            return static_cast<QGraphicsSceneMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneMove) {
            return static_cast<QGraphicsSceneMoveEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneResize) {
            return static_cast<QGraphicsSceneResizeEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneWheel) {
            return static_cast<QGraphicsSceneWheelEvent*>(nullptr);
        } else if constexpr (Type == QEvent::GraphicsSceneLeave) {
            return static_cast<QGraphicsSceneWheelEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Hide) {
            return static_cast<QHideEvent*>(nullptr);
        } else if constexpr (Type == QEvent::HideToParent) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::HoverEnter) {
            return static_cast<QHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::HoverLeave) {
            return static_cast<QHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::HoverMove) {
            return static_cast<QHoverEvent*>(nullptr);
        } else if constexpr (Type == QEvent::IconDrag) {
            return static_cast<QIconDragEvent*>(nullptr);
        } else if constexpr (Type == QEvent::InputMethod) {
            return static_cast<QInputMethodEvent*>(nullptr);
        } else if constexpr (Type == QEvent::InputMethodQuery) {
            return static_cast<QInputMethodQueryEvent*>(nullptr);
        } else if constexpr (Type == QEvent::KeyboardLayoutChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::KeyPress) {
            return static_cast<QKeyEvent*>(nullptr);
        } else if constexpr (Type == QEvent::KeyRelease) {
            return static_cast<QKeyEvent*>(nullptr);
        } else if constexpr (Type == QEvent::LanguageChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::LayoutDirectionChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::LayoutRequest) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Leave) {
            return static_cast<QEvent*>(nullptr);
#ifdef QT_KEYPAD_NAVIGATION
        } else if constexpr (Type == QEvent::LeaveEditFocus) {
            return static_cast<QEvent*>(nullptr);
#endif
        } else if constexpr (Type == QEvent::LeaveWhatsThisMode) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::LocaleChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::NonClientAreaMouseButtonDblClick) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::NonClientAreaMouseButtonPress) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::NonClientAreaMouseButtonRelease) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::NonClientAreaMouseMove) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MacSizeChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MetaCall) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ModifiedChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MouseButtonDblClick) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MouseButtonPress) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MouseButtonRelease) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MouseMove) {
            return static_cast<QMouseEvent*>(nullptr);
        } else if constexpr (Type == QEvent::MouseTrackingChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Move) {
            return static_cast<QMoveEvent*>(nullptr);
        } else if constexpr (Type == QEvent::NativeGesture) {
            return static_cast<QNativeGestureEvent*>(nullptr);
        } else if constexpr (Type == QEvent::OrientationChange) {
            return static_cast<QScreenOrientationChangeEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Paint) {
            return static_cast<QPaintEvent*>(nullptr);
        } else if constexpr (Type == QEvent::PaletteChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ParentAboutToChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ParentChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ParentWindowAboutToChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ParentWindowChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::PlatformPanel) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::PlatformSurface) {
            return static_cast<QPlatformSurfaceEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Polish) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::PolishRequest) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::QueryWhatsThis) {
            return static_cast<QHelpEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Quit) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ReadOnlyChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::RequestSoftwareInputPanel) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Resize) {
            return static_cast<QResizeEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ScrollPrepare) {
            return static_cast<QScrollPrepareEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Scroll) {
            return static_cast<QScrollEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Shortcut) {
            return static_cast<QShortcutEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ShortcutOverride) {
            return static_cast<QKeyEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Show) {
            return static_cast<QShowEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ShowToParent) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::SockAct) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::StateMachineSignal) {
            return static_cast<QStateMachine*>(nullptr);
        } else if constexpr (Type == QEvent::StateMachineWrapped) {
            return static_cast<QStateMachine*>(nullptr);
        } else if constexpr (Type == QEvent::StatusTip) {
            return static_cast<QStatusTipEvent*>(nullptr);
        } else if constexpr (Type == QEvent::StyleChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletMove) {
            return static_cast<QTabletEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletPress) {
            return static_cast<QTabletEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletRelease) {
            return static_cast<QTabletEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletEnterProximity) {
            return static_cast<QTabletEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletLeaveProximity) {
            return static_cast<QTabletEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TabletTrackingChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ThreadChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Timer) {
            return static_cast<QTimerEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ToolBarChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ToolTip) {
            return static_cast<QHelpEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ToolTipChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TouchBegin) {
            return static_cast<QTouchEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TouchCancel) {
            return static_cast<QTouchEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TouchEnd) {
            return static_cast<QTouchEvent*>(nullptr);
        } else if constexpr (Type == QEvent::TouchUpdate) {
            return static_cast<QTouchEvent*>(nullptr);
        } else if constexpr (Type == QEvent::UngrabKeyboard) {
            return static_cast<QGraphicsItem*>(nullptr);
        } else if constexpr (Type == QEvent::UngrabMouse) {
            return static_cast<QGraphicsItem*>(nullptr);
        } else if constexpr (Type == QEvent::UpdateLater) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::UpdateRequest) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WhatsThis) {
            return static_cast<QHelpEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WhatsThisClicked) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::Wheel) {
            return static_cast<QWheelEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WinEventAct) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowActivate) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowBlocked) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowDeactivate) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowIconChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowStateChange) {
            return static_cast<QWindowStateChangeEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowTitleChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WindowUnblocked) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::WinIdChange) {
            return static_cast<QEvent*>(nullptr);
        } else if constexpr (Type == QEvent::ZOrderChange) {
            return static_cast<QEvent*>(nullptr);
        } else {
            return static_cast<QEvent*>(nullptr);
        }
    }
}  // namespace detail

template <QEvent::Type Type>
auto isEvent(QEvent* e) -> decltype(detail::qEventMap<Type>()) {
    if (!e) {
        return nullptr;
    } else if (Type == e->type()) {
        return static_cast<decltype(detail::qEventMap<Type>())>(e);
    } else {
        return nullptr;
    }
}
}  // namespace inviwo::utilqt
