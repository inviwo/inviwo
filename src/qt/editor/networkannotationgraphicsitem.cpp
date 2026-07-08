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
#include <QTextDocument.h>

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
        QFont f("Segoe", 10, QFont::ExtraBold, false);
        f.setPixelSize(pointSizeToPixelSize(8));
        return f;
    }();
    return name;
}

QString elide(std::string_view text, double width) {
    const QFontMetricsF fm{getFont()};
    return fm.elidedText(utilqt::toQString(text), Qt::ElideMiddle, width);
}

QRectF processorRect(const NetworkAnnotation& annotation, const NetworkEditor* editor) {
    if (!editor) return {};

    if (const auto* network = editor->getNetwork()) {
        QRectF rect{};
        for (const auto& id : annotation.processors) {
            if (const auto* metaData = util::getMetaData(network->getProcessorByIdentifier(id))) {
                QRectF r{ProcessorGraphicsItem::itemRect};
                r.moveCenter(utilqt::toQPoint(metaData->getPosition()));
                rect = rect.united(r);
            }
        }
        return rect;
    }
    return {};
}

QMarginsF margins(const NetworkAnnotation& annotation) {
    const int labelHeight = QFontMetrics{getFont()}.height();

    QMarginsF margins{borderMargin, borderMargin + labelMargin + labelHeight, borderMargin,
                      borderMargin};
    // adjust margins for descriptions
    if (annotation.description.markdown.has_value()) {
        switch (annotation.description.alignment) {
            using enum Description::TextAlignment;
            case Left:
                margins.setLeft(margins.left() + annotation.description.width);
                break;
            case Top:
                margins.setTop(margins.top() + annotation.description.width);
                break;
            case Right:
                margins.setRight(margins.right() + annotation.description.width);
                break;
            case Bottom:
                margins.setBottom(margins.bottom() + annotation.description.width);
                break;
        }
    }
    return margins;
}

void highlightProcessors(const NetworkEditor* editor, const NetworkAnnotation& annotation,
                         bool highlight) {
    if (editor && editor->getNetwork()) {
        std::ranges::for_each(annotation.processors, [&](const auto& id) {
            if (auto* pgi = editor->getProcessorGraphicsItem(
                    editor->getNetwork()->getProcessorByIdentifier(id))) {
                pgi->setHighlight(highlight);
            }
        });
    }
}

}  // namespace

NetworkAnnotationGraphicsItem::NetworkAnnotationGraphicsItem(ProcessorNetwork* network,
                                                             size_t index)
    : QGraphicsItem{}
    , ProcessorMetaDataObserver{}
    , index_{index}
    , annotations_{network->getNetworkAnnotations()}
    , nameText_{}
    , description_{new QGraphicsTextItem{this}}
    , textDocument_{new QTextDocument{this}} {

    if (annotations_) {
        annotations_->Observable<NetworkAnnotationsObserver>::addObserver(this);
    }

    setZValue(depth::annotation);
    setFlags(ItemIsSelectable | ItemIsFocusable | ItemSendsGeometryChanges);

    if (annotations_ && network) {
        std::ranges::for_each(
            annotations_->getAnnotation(index).processors, [this, network](const auto& id) {
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

    if (annotations_) {
        annotations_->addObserver(this);
    }

    updateGeometry();
    updateState();
}

NetworkAnnotationGraphicsItem::~NetworkAnnotationGraphicsItem() {
    if (isSelected() && annotations_) {
        highlightProcessors(getNetworkEditor(), annotations_->getAnnotation(index_), false);
    }
    if (annotations_) {
        annotations_->Observable<NetworkAnnotationsObserver>::removeObserver(this);
    }
}

size_t NetworkAnnotationGraphicsItem::getAnnotationIndex() const { return index_; }

void NetworkAnnotationGraphicsItem::paint(QPainter* p,
                                          [[maybe_unused]] const QStyleOptionGraphicsItem* options,
                                          [[maybe_unused]] QWidget* widget) {
    if (!annotations_) return;
    const auto& annotation = annotations_->getAnnotation(index_);

    static constexpr float roundedCorners = 3.0f;

    static constexpr QColor transparent{0, 0, 0, 0};
    const QColor borderColor{utilqt::toQColor(annotation.color)};
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

    QPointF labelPosition{r.left() + labelMargin, r.top() + labelMargin};
    if (description_->isVisible() &&
        annotation.description.alignment == Description::TextAlignment::Right) {
        labelPosition.setX(r.right() - (labelMargin + annotation.description.width));
    }
    p->drawStaticText(labelPosition, nameText_);

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
            if (scene() && isSelected() && annotations_) {
                highlightProcessors(getNetworkEditor(), annotations_->getAnnotation(index_), false);
            }
            break;
        case QGraphicsItem::ItemSceneHasChanged:
            // value holds new scene, new scene in scene()
            updateGeometry();
            break;
        default:
            break;
    }
#include <warn/pop>
    return QGraphicsItem::itemChange(change, value);
}

void NetworkAnnotationGraphicsItem::onProcessorMetaDataPositionChange() { updateGeometry(); }

void NetworkAnnotationGraphicsItem::onNetworkAnnotationChanged(NetworkAnnotation&, size_t index) {
    if (index != index_) {
        return;
    }
    updateGeometry();
    updateState();
}

void NetworkAnnotationGraphicsItem::updateGeometry() {
    if (!annotations_) return;
    const auto& annotation = annotations_->getAnnotation(index_);

    prepareGeometryChange();
    auto rect = processorRect(annotation, getNetworkEditor());
    setPos(rect.center());
    rect.moveCenter(QPointF{0.0, 0.0});

    margins_ = margins(annotation);
    rect += margins_;
    setRect(rect);

    nameText_.setText(elide(annotation.title.value_or(""), rect.width() - (2.0 * labelMargin)));

    switch (annotation.description.alignment) {
        using enum Description::TextAlignment;
        case Left:
        case Top:
            description_->setPos(rect.left() + labelMargin,
                                 rect.top() + borderMargin + labelMargin);
            break;
        case Right:
            description_->setPos(rect.right() - (labelMargin + annotation.description.width),
                                 rect.top() + labelMargin + borderMargin);
            break;
        case Bottom:
            description_->setPos(rect.left() + labelMargin,
                                 rect.bottom() - (labelMargin + annotation.description.width));
            break;
    }
}

void NetworkAnnotationGraphicsItem::updateState() {
    if (!annotations_) return;
    const auto& annotation = annotations_->getAnnotation(index_);

    if (annotation.description.markdown.has_value()) {
        textDocument_->setHtml(
            utilqt::toQString(util::unindentMd2doc(*annotation.description.markdown).str()));
        description_->update();
    }
    description_->setVisible(annotation.description.markdown.has_value());

    using enum Description::TextAlignment;
    if (auto alignment = annotation.description.alignment;
        alignment == Left || alignment == Right) {
        description_->setTextWidth(annotation.description.width);
    } else {
        description_->setTextWidth(rect().width() - 2 * borderMargin);
    }

    update();
}

void NetworkAnnotationGraphicsItem::selectedHasChanged() {
    if (isSelected()) {
        setZValue(depth::annotationSelected);
        if (auto* editor = getNetworkEditor()) {
            editor->addNetworkAnnotationWidgets(index_);
            if (annotations_) {
                highlightProcessors(editor, annotations_->getAnnotation(index_), true);
            }
        }
    } else {
        setZValue(depth::annotation);
        if (auto* editor = getNetworkEditor()) {
            editor->removeNetworkAnnotationWidgets(index_);
            if (annotations_) {
                highlightProcessors(editor, annotations_->getAnnotation(index_), false);
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
