/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/networkannotationgraphicsitem.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/network/networkutils.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/util/document.h>

#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGradient>
#include <QFontMetrics>
#include <QGraphicsTextItem>
#include <QTextDocument>
#include <QFontDatabase>

#include <numeric>

namespace inviwo {

namespace {

int pointSizeToPixelSize(const int pointSize) {
    // compute pixel size for fonts by assuming 96 dpi as basis
    return ((pointSize * 4) / 3);
}

constexpr double labelMargin = 4.0;
constexpr double borderMargin = 16.0;
constexpr double penWidth = 2.0;

QFont getFont() {
    static const QFont name = []() {
        QFont f{QFontDatabase::systemFont(QFontDatabase::TitleFont)};
        f.setWeight(QFont::ExtraBold);
        f.setPixelSize(pointSizeToPixelSize(8));
        return f;
    }();
    return name;
}

QString elide(std::string_view text, double width) {
    const QFontMetricsF fm{getFont()};
    return fm.elidedText(utilqt::toQString(text), Qt::ElideMiddle, width);
}

QRectF processorRect(const std::vector<Processor*>& processors) {
    constexpr QRectF processor{ProcessorGraphicsItem::itemRect};

    auto [min, max] = util::getBoundingBox(processors);
    return QRectF{utilqt::toQPoint(min), utilqt::toQPoint(max)}.adjusted(
        processor.left(), processor.top(), processor.right(), processor.bottom());
}

QMarginsF margins(const NetworkAnnotation& annotation) {
    const int labelHeight = QFontMetrics{getFont()}.height();

    QMarginsF margins{borderMargin, borderMargin + labelMargin + labelHeight, borderMargin,
                      borderMargin};
    // adjust margins for descriptions
    if (annotation.description.has_value()) {
        const auto width = annotation.description->width;
        switch (annotation.description->alignment) {
            using enum Description::Alignment;
            case Left:
                margins.setLeft(margins.left() + width);
                break;
            case Top:
                margins.setTop(margins.top() + width);
                break;
            case Right:
                margins.setRight(margins.right() + width);
                break;
            case Bottom:
                margins.setBottom(margins.bottom() + width);
                break;
        }
    }
    return margins;
}

void highlightProcessors(const NetworkEditor* editor, const std::vector<Processor*>& processors,
                         bool highlight) {
    if (editor) {
        std::ranges::for_each(processors, [&](auto* processor) {
            if (auto* pgi = editor->getProcessorGraphicsItem(processor)) {
                pgi->setHighlight(highlight);
            }
        });
    }
}

}  // namespace

NetworkAnnotationGraphicsItem::NetworkAnnotationGraphicsItem(ProcessorNetwork* network,
                                                             const NetworkAnnotation& annotation)
    : QGraphicsItem{}
    , ProcessorMetaDataObserver{}
    , network_{network}
    , nameText_{}
    , description_{new QGraphicsTextItem{this}}
    , textDocument_{new QTextDocument{this}} {

    setZValue(depth::annotation);
    setFlags(ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);

    if (network_) {
        std::ranges::for_each(annotation.processors, [this, network](const auto& id) {
            if (auto* metaData = util::getMetaData(network->getProcessorByIdentifier(id))) {
                metaData->addObserver(this);
            }
        });
    }

    QTextOption opts{Qt::AlignLeft | Qt::AlignBaseline};
    opts.setWrapMode(QTextOption::NoWrap);
    nameText_.setTextOption(opts);
    nameText_.setTextFormat(Qt::PlainText);

    description_->setDocument(textDocument_);
    description_->setZValue(depth::annotationSelected);

    updateAnnotation(annotation);
}

NetworkAnnotationGraphicsItem::~NetworkAnnotationGraphicsItem() {
    if (auto* editor = getNetworkEditor()) {
        editor->getAnnotationManager().hideAnnotationDetails(this);
        if (isSelected()) {
            highlightProcessors(editor, processors_, false);
        }
    }
}

void NetworkAnnotationGraphicsItem::paint(QPainter* p,
                                          [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                                          [[maybe_unused]] QWidget* widget) {
    static constexpr float roundedCorners = 3.0f;

    static constexpr QColor transparent{0, 0, 0, 0};
    const QColor borderColor{color_()};
    const QColor fillColor{borderColor.red(), borderColor.green(), borderColor.blue(),
                           isSelected() ? 155 : 51};

    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    if (isSelected()) {
        p->setPen(QPen(QBrush(borderColor), penWidth, Qt::SolidLine));
    } else {
        p->setPen(QPen(QBrush(borderColor), penWidth, Qt::DashLine));
    }

    QLinearGradient gradient{QPointF{0, 0}, QPointF{30, 30}};
    gradient.setColorAt(0.0, fillColor);
    gradient.setColorAt(0.5, fillColor);
    gradient.setColorAt(0.50000001, transparent);
    gradient.setColorAt(1.0, transparent);
    gradient.setSpread(QGradient::RepeatSpread);
    p->setBrush(gradient);

    const QRectF r{rect()};
    p->drawRoundedRect(r, roundedCorners, roundedCorners);

    p->setFont(getFont());
    p->setBrush(QBrush{});
    p->setPen(Qt::white);

    p->drawStaticText(titlePos_(r), nameText_);

    p->restore();
}

QRectF NetworkAnnotationGraphicsItem::boundingRect() const {
    QRectF r{rect()};

    const double halfWidth = penWidth / 2.0;
    r.adjust(-halfWidth, -halfWidth, halfWidth, halfWidth);
    return r;
}

QVariant NetworkAnnotationGraphicsItem::itemChange(GraphicsItemChange change,
                                                   const QVariant& value) {
#include <warn/push>
#include <warn/ignore/switch-enum>
    switch (change) {
        case QGraphicsItem::ItemSelectedHasChanged:
            selectedHasChanged();
            break;
        case QGraphicsItem::ItemVisibleHasChanged: {
            if (auto* editor = getNetworkEditor()) {
                editor->updateSceneSize();
            }
            break;
        }
        case QGraphicsItem::ItemSceneChange:
            // value holds the new scene, old scene in scene()
            if (scene() && isSelected()) {
                highlightProcessors(getNetworkEditor(), processors_, false);
            }
            break;
        default:
            break;
    }
#include <warn/pop>
    return QGraphicsItem::itemChange(change, value);
}

void NetworkAnnotationGraphicsItem::onProcessorMetaDataPositionChange() { updateGeometry(); }

void NetworkAnnotationGraphicsItem::updateAnnotation(const NetworkAnnotation& annotation) {
    std::ranges::for_each(processors_, [this](auto* processor) {
        if (auto* metaData = util::getMetaData(processor)) {
            metaData->removeObserver(this);
        }
    });

    processors_ = annotation.processors | std::views::transform([this](const auto& id) {
                      return network_->getProcessorByIdentifier(id);
                  }) |
                  std::views::filter([](auto* p) { return p != nullptr; }) |
                  std::ranges::to<std::vector>();
    // observe processor metadata for position changes
    std::ranges::for_each(processors_, [this](auto* processor) {
        if (auto* metaData = util::getMetaData(processor)) {
            metaData->addObserver(this);
        }
    });

    margins_ = margins(annotation);

    const int descriptionWidth =
        annotation.description.transform([](auto& desc) { return desc.width; }).value_or(0);
    const auto alignment =
        annotation.description.transform([](auto& desc) { return desc.alignment; })
            .value_or(Description::Alignment::Left);

    titleFunc_ = [this, title = annotation.title.value_or("")]() {
        return elide(title, textWidth_());
    };
    titlePos_ = [this, descriptionWidth, alignment](const QRectF& rect) {
        QPointF labelPosition{rect.left() + labelMargin, rect.top() + labelMargin};
        if (description_->isVisible() && alignment == Description::Alignment::Right) {
            labelPosition.setX(rect.right() - (labelMargin + descriptionWidth));
        }
        return labelPosition;
    };

    descriptionPos_ = [alignment, descriptionWidth](const QRectF& rect) {
        switch (alignment) {
            using enum Description::Alignment;
            case Left:
                [[fallthrough]];
            case Top:
                return QPointF{rect.left() + labelMargin, rect.top() + borderMargin + labelMargin};
            case Right:
                return QPointF{rect.right() - (labelMargin + descriptionWidth),
                               rect.top() + labelMargin + borderMargin};
            case Bottom:
                return QPointF{rect.left() + labelMargin,
                               rect.bottom() - (labelMargin + descriptionWidth)};
        }
        return QPointF{};
    };
    textWidth_ = [this, alignment, descriptionWidth]() {
        if (alignment == Description::Alignment::Left ||
            alignment == Description::Alignment::Right) {
            return static_cast<double>(descriptionWidth);
        } else {
            return rect().width() - 2 * borderMargin;
        }
    };
    color_ = [color = annotation.color]() { return utilqt::toQColor(color); };

    if (annotation.description.has_value()) {
        textDocument_->setHtml(
            utilqt::toQString(util::unindentMd2doc(annotation.description->markdown).str()));
        description_->update();
    }
    description_->setVisible(annotation.description.has_value());

    updateGeometry();
    update();
}

void NetworkAnnotationGraphicsItem::updateGeometry() {
    prepareGeometryChange();
    auto rect = processorRect(processors_);
    setPos(rect.center());
    rect.moveCenter(QPointF{0.0, 0.0});

    rect += margins_;
    setRect(rect);

    nameText_.setText(titleFunc_());
    description_->setPos(descriptionPos_(rect));
    description_->setTextWidth(textWidth_());
}

void NetworkAnnotationGraphicsItem::selectedHasChanged() {
    if (isSelected()) {
        setZValue(depth::annotationSelected);
        if (auto* editor = getNetworkEditor()) {
            editor->getAnnotationManager().showAnnotationDetails(this);
            if (network_) {
                highlightProcessors(editor, processors_, true);
            }
        }
    } else {
        setZValue(depth::annotation);
        if (auto* editor = getNetworkEditor()) {
            editor->getAnnotationManager().hideAnnotationDetails(this);
            if (network_) {
                highlightProcessors(editor, processors_, false);
            }
        }
    }
}

void NetworkAnnotationGraphicsItem::setRect(const QRectF& r) { rect_ = r; }

const QRectF& NetworkAnnotationGraphicsItem::rect() const { return rect_; }

NetworkEditor* NetworkAnnotationGraphicsItem::getNetworkEditor() const {
    return qobject_cast<NetworkEditor*>(scene());
}

}  // namespace inviwo
