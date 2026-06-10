/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2026 Inviwo Foundation
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

#include <modules/qtwidgets/tf/tfpropertydialog.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwoapplicationutil.h>
#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/histogram.h>
#include <inviwo/core/datastructures/isovaluecollection.h>
#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/datastructures/tfprimitiveset.h>
#include <inviwo/core/datastructures/transferfunction.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/network/networklock.h>
#include <modules/qtwidgets/colorwheel.h>
#include <modules/qtwidgets/inviwodockwidgettitlebar.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/properties/propertyeditorwidgetqt.h>
#include <modules/qtwidgets/qtwidgetsmodule.h>
#include <modules/qtwidgets/rangesliderqt.h>
#include <modules/qtwidgets/tf/tfcoloredit.h>
#include <modules/qtwidgets/tf/tfeditor.h>
#include <modules/qtwidgets/tf/tfeditorview.h>
#include <modules/qtwidgets/tf/tflineedit.h>
#include <modules/qtwidgets/tf/tfpropertyconcept.h>
#include <modules/qtwidgets/tf/tfselectionwatcher.h>
#include <modules/qtwidgets/tf/tfmovemode.h>
#include <modules/qtwidgets/inviwoeditmenu.h>

#include <algorithm>
#include <string_view>
#include <type_traits>
#include <utility>

#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QObject>
#include <QPoint>
#include <QPolygonF>
#include <QRect>
#include <QRectF>
#include <QSettings>
#include <QSignalBlocker>
#include <QSizeF>
#include <QSizePolicy>
#include <QString>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <QScrollBar>
#include <Qt>
#include <QMainWindow>
#include <QMimeData>
#include <fmt/core.h>
#include <glm/common.hpp>
#include <glm/vec2.hpp>

class QHBoxLayout;
class QVBoxLayout;

namespace inviwo {

namespace {

constexpr QSize colorWheelSize{150, 150};

QComboBox* createHistogramComboBox(int selected) {
    auto* cb = new QComboBox();
    cb->addItem("Histogram: Off");
    cb->addItem("Histogram: 100%");
    cb->addItem("Histogram: 99%");
    cb->addItem("Histogram: 95%");
    cb->addItem("Histogram: 90%");
    cb->addItem("Histogram: Log");
    cb->setCurrentIndex(selected);
    return cb;
}

QComboBox* createPointMoveModeComboBox() {
    auto* cb = new QComboBox();
    cb->addItem("Point Movement: Free");
    cb->addItem("Point Movement: Restrict");
    cb->addItem("Point Movement: Push");
    cb->setCurrentIndex(0);
    return cb;
}
QComboBox* createModeComboBox(bool absolute) {
    auto* cb = new QComboBox();
    cb->addItem(utilqt::toQString(fmt::to_string(static_cast<PrimitiveSetMode>(0))));
    cb->addItem(utilqt::toQString(fmt::to_string(static_cast<PrimitiveSetMode>(1))));
    cb->setCurrentIndex(absolute ? 1 : 0);
    return cb;
}
}  // namespace

TFPropertyDialog::TFPropertyDialog(TransferFunctionProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<TransferFunctionProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(IsoValueProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<IsoValueProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(IsoTFProperty* property)
    : TFPropertyDialog(std::make_unique<TFPropertyModel<IsoTFProperty>>(property)) {}

TFPropertyDialog::TFPropertyDialog(std::unique_ptr<TFPropertyConcept> model)
    : PropertyEditorWidgetQt(model->getProperty(), "Transfer Function Editor", "TFEditorWidget")
    , preview_{new QLabel()}
    , concept_{std::move(model)}
    , colorWheel_{std::make_unique<ColorWheel>(colorWheelSize)}
    , editor_{std::make_unique<TFEditor>(concept_.get(), this)}
    , tfSelectionWatcher_{std::make_unique<TFSelectionWatcher>(concept_->getProperty(),
                                                               concept_->sets())}
    , view_{new TFEditorView(concept_.get(), editor_.get())}

    , chkShowHistogram_{createHistogramComboBox(static_cast<int>(concept_->getHistogramMode()))}
    , pointMoveMode_{createPointMoveModeComboBox()}
    , tfMode_{createModeComboBox(!concept_->allRelative())}

    , scalar_{new QLabel("Scalar")}
    , domainMin_{new QLabel("0.0")}
    , domainMax_{new QLabel("1.0")} {

    if (auto titlebar = dynamic_cast<InviwoDockWidgetTitleBar*>(titleBarWidget())) {
        if (auto layout = dynamic_cast<QHBoxLayout*>(titlebar->layout())) {
            QToolButton* helpBtn = new QToolButton();
            helpBtn->setIcon(QIcon(":/svgicons/dock-help.svg"));
            const auto iconsize =
                utilqt::emToPx(this, QSizeF(titlebar->getIconSize(), titlebar->getIconSize()));
            helpBtn->setIconSize(iconsize);
            helpBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
            layout->insertWidget(1, helpBtn);

            if (auto* qtModule = util::getModuleByType<QtWidgetsModule>()) {
                QObject::connect(helpBtn, &QToolButton::clicked, this,
                                 [qtModule]() { qtModule->showTFHelpWindow(); });
            }
        }
    }

    if (auto owner = concept_->getProperty()->getOwner()) {
        if (auto p = owner->getProcessor()) {
            onNameChange_ = p->onDisplayNameChange(
                [this](std::string_view, std::string_view) { updateTitleFromProperty(); });
        }
    }

    preview_->setMinimumSize(1, 20);
    QSizePolicy sliderPol = preview_->sizePolicy();
    sliderPol.setHorizontalStretch(3);
    preview_->setSizePolicy(sliderPol);

    concept_->addObserver(this);
    for (auto* set : concept_->sets()) {
        set->addObserver(this);
    }

    connect(editor_.get(), &TFEditor::selectionChanged, this,
            [this]() { tfSelectionWatcher_->updateSelection(editor_->getSelectedPrimitives()); });
    connect(editor_.get(), &TFEditor::updateBegin, this, [&]() { ongoingUpdate_ = true; });
    connect(editor_.get(), &TFEditor::updateEnd, this, [&]() {
        ongoingUpdate_ = false;
        updateTFPreview();
    });

    if (auto* editMenu = utilqt::getInviwoEditMenu()) {
        editActionsHandle_ = editMenu->registerItem(std::make_shared<MenuItem>(
            view_,
            [this](MenuItemType t) -> bool {
                switch (t) {
                    case MenuItemType::cut:
                    case MenuItemType::copy:
                    case MenuItemType::del:
                        return !editor_->selectedItems().isEmpty();
                    case MenuItemType::paste: {
                        auto* clipboard = QApplication::clipboard();
                        const auto* mimeData = clipboard->mimeData();
                        return mimeData &&
                               mimeData->hasFormat(QString::fromUtf8(TFEditor::mimeTFPrimitives));
                    }
                    case MenuItemType::select:
                        return true;
                    default:
                        return false;
                }
            },
            [this](MenuItemType t) -> void {
                switch (t) {
                    case MenuItemType::cut:
                        editor_->cut();
                        break;
                    case MenuItemType::copy:
                        editor_->copy();
                        break;
                    case MenuItemType::paste:
                        editor_->paste();
                        break;
                    case MenuItemType::del:
                        editor_->deleteSelection();
                        break;
                    case MenuItemType::select:
                        editor_->selectAll();
                        break;
                    default:
                        break;
                }
            }));
    }

    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor, colorWheel_.get(),
            [cw = colorWheel_.get()](const QColor& c, bool /*ambiguous*/) {
                QSignalBlocker block(cw);
                cw->setColor(c);
            });
    connect(colorWheel_.get(), &ColorWheel::colorChange, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setColor);

    connect(chkShowHistogram_,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int i) { concept_->setHistogramMode(static_cast<HistogramMode>(i)); });

    connect(pointMoveMode_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int i) { editor_->setMoveMode(static_cast<TFMoveMode>(i)); });
    connect(editor_.get(), &TFEditor::moveModeChange, this,
            [this](TFMoveMode m) { pointMoveMode_->setCurrentIndex(static_cast<int>(m)); });

    connect(tfMode_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int i) {
                const auto mode = static_cast<PrimitiveSetMode>(i);
                const NetworkLock lock(concept_->getProperty());
                for (auto* set : concept_->sets()) {
                    if (set->getMode() != mode) {
                        if (const auto* dataMap = concept_->getDataMap()) {
                            set->setMode(mode, *dataMap);
                        } else {
                            set->setMode(mode);
                        }
                    }
                }
                editor_->updateSceneRect();
                view_->fitViewToScene();
            });

    const auto dataChange = [this]() {
        if (const auto* dataMap = concept_->getDataMap()) {
            scalar_->setText(utilqt::toQString(
                fmt::format("{}{: [}", dataMap->valueAxis.name, dataMap->valueAxis.unit)));
            domainMin_->setText(QString("%1").arg(dataMap->mapFromNormalizedToValue(0.0)));
            domainMax_->setText(QString("%1").arg(dataMap->mapFromNormalizedToValue(1.0)));
        } else {
            scalar_->setText("Scalar");
            domainMin_->setText("0.0");
            domainMax_->setText("1.0");
        }
        // ensure that the range of primitive scalar is matching value range of volume data
        onTFModeChangedInternal();
    };
    dataChangeHandle_ = concept_->onDataChange(dataChange);

    // set up TF primitive widgets
    primitivePos_ = new TFLineEdit();
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetPosition, primitivePos_,
            &TFLineEdit::setValue);
    connect(primitivePos_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setPosition);

    primitiveAlpha_ = new TFLineEdit();
    // only accept values in [0, 1]
    primitiveAlpha_->setValidRange(dvec2(0.0, 1.0), 0.0001);
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetAlpha, primitiveAlpha_,
            &TFLineEdit::setValue);
    connect(primitiveAlpha_, &TFLineEdit::valueChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setAlpha);

    primitiveColor_ = new TFColorEdit();
    primitiveColor_->setColor(QColor(Qt::black), true);
    connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor, primitiveColor_,
            &TFColorEdit::setColor);
    connect(primitiveColor_, &TFColorEdit::colorChanged, tfSelectionWatcher_.get(),
            &TFSelectionWatcher::setColor);

    dataChange();

    auto* leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(utilqt::refSpacePx(this));
    leftLayout->addWidget(view_);

    auto* preview = new QHBoxLayout();
    preview->setContentsMargins(0, 0, view_->verticalScrollBar()->width(), 0);
    preview->addWidget(preview_);
    leftLayout->addLayout(preview);

    auto* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(utilqt::refSpacePx(this));
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(tfMode_);
    rightLayout->addWidget(chkShowHistogram_);
    rightLayout->addWidget(pointMoveMode_);
    rightLayout->addWidget(colorWheel_.get());

    auto primitivePropLayout = new QGridLayout();
    primitivePropLayout->setColumnStretch(0, 0);
    primitivePropLayout->setColumnStretch(1, 2);

    primitivePropLayout->addWidget(scalar_, 0, 0, 1, 2);
    primitivePropLayout->addWidget(new QLabel("Scalar"), 1, 0);
    primitivePropLayout->addWidget(primitivePos_, 1, 1);
    primitivePropLayout->addWidget(new QLabel("Alpha"), 2, 0);
    primitivePropLayout->addWidget(primitiveAlpha_, 2, 1);
    primitivePropLayout->addWidget(new QLabel("Color"), 3, 0);
    primitivePropLayout->addWidget(primitiveColor_, 3, 1);

    primitivePropLayout->addWidget(new QLabel("Min"), 4, 0);
    primitivePropLayout->addWidget(domainMin_, 4, 1);

    primitivePropLayout->addWidget(new QLabel("Max"), 5, 0);
    primitivePropLayout->addWidget(domainMax_, 5, 1);
    rightLayout->addLayout(primitivePropLayout);
    // rightLayout->addStretch(3);

    auto* mainPanel = new QWidget(this);
    auto* mainLayout = new QHBoxLayout();
    const auto space = utilqt::refSpacePx(this);
    mainLayout->setContentsMargins(space, space, space, space);
    mainLayout->setSpacing(space);
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretch(0, 3);
    mainLayout->setStretch(1, 0);
    mainPanel->setLayout(mainLayout);

    setWidget(mainPanel);

    // set up color dialog
    {
        colorDialog_ = std::make_unique<QColorDialog>(this);
        colorDialog_->hide();
        colorDialog_->setAttribute(Qt::WA_DeleteOnClose, false);
        // we don't want to see alpha in the color dialog
        colorDialog_->setOption(QColorDialog::ShowAlphaChannel, false);
        colorDialog_->setOption(QColorDialog::NoButtons, true);
        colorDialog_->setWindowModality(Qt::NonModal);
        colorDialog_->setWindowTitle(
            QString("TF Primitive Color - %1")
                .arg(utilqt::toQString(concept_->getProperty()->getDisplayName())));

        connect(editor_.get(), &TFEditor::showColorDialog, colorDialog_.get(),
                [dialog = colorDialog_.get()]() {
#ifdef __APPLE__
                    // OSX Bug workaround: hide the dialog, due to some Mac issues
                    dialog->hide();
#endif  // __APPLE__
                    dialog->show();
                });

        connect(tfSelectionWatcher_.get(), &TFSelectionWatcher::updateWidgetColor,
                colorDialog_.get(),
                [dialog = colorDialog_.get()](const QColor& c, bool /*ambiguous*/) {
                    QSignalBlocker block(dialog);
                    if (c.isValid()) {
                        dialog->setCurrentColor(c);
                    } else {
                        // nothing selected
                        dialog->setCurrentColor(QColor("#95baff"));
                    }
                });
        connect(colorDialog_.get(), &QColorDialog::currentColorChanged, tfSelectionWatcher_.get(),
                &TFSelectionWatcher::setColor);

        colorDialog_->installEventFilter(new utilqt::WidgetCloseEventFilter(this));
    }

    // ensure that the TF dialog has its minimal size when showing up for the first time
    resize(utilqt::emToPx(this, 14.0), utilqt::emToPx(this, 12.0));

    {
        // make sure the help dialog for the TF editor is shown once
        QSettings settings;
        settings.beginGroup(objectName());
        if (!settings.contains("shownonce") || !settings.value("shownonce").toBool()) {
            settings.setValue("shownonce", true);

            auto* app = util::getInviwoApplication(concept_->getProperty());
            util::getModuleByTypeOrThrow<QtWidgetsModule>(app).showTFHelpWindow();
        }
        settings.endGroup();
    }

    updateFromProperty();
    updateTitleFromProperty();

    loadState();
}

TFPropertyDialog::~TFPropertyDialog() {
    editor_->disconnect();
    hide();
}

Property* TFPropertyDialog::getProperty() const { return concept_->getProperty(); }

QSize TFPropertyDialog::minimumSizeHint() const { return TFPropertyDialog::sizeHint(); }

QSize TFPropertyDialog::sizeHint() const { return layout()->sizeHint(); }

void TFPropertyDialog::updateFromProperty() { updateTFPreview(); }

TFEditorView* TFPropertyDialog::getEditorView() const { return view_; }

void TFPropertyDialog::resizeEvent(QResizeEvent* event) {
    PropertyEditorWidgetQt::resizeEvent(event);
    updateTFPreview();
}

void TFPropertyDialog::showEvent(QShowEvent* event) {
    updateTFPreview();
    view_->update();
    PropertyEditorWidgetQt::showEvent(event);
}

void TFPropertyDialog::updateTitleFromProperty() {
    if (!getProperty()->getOwner()) return;

    const auto processorName = getProperty()->getOwner()->getProcessor()->getDisplayName();
    const auto windowTitle =
        fmt::format("Transfer Function Editor - {} ({}){}", getProperty()->getDisplayName(),
                    processorName, (getProperty()->getReadOnly() ? " - Read Only" : ""));
    setWindowTitle(utilqt::toQString(windowTitle));
}

void TFPropertyDialog::onSetDisplayName(Property*, const std::string&) {
    updateTitleFromProperty();
}

void TFPropertyDialog::onTFPrimitiveAdded(const TFPrimitiveSet&, TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveRemoved(const TFPrimitiveSet&, TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFPrimitiveChanged(const TFPrimitiveSet&, const TFPrimitive&) {
    updateFromProperty();
}

void TFPropertyDialog::onTFModeChanged(const TFPrimitiveSet&, PrimitiveSetMode mode) {
    const QSignalBlocker block(tfMode_);
    tfMode_->setCurrentIndex(static_cast<int>(mode));
    onTFModeChangedInternal();
}

void TFPropertyDialog::onTFModeChangedInternal() {
    // adjust value mapping in primitive widget for position
    dvec2 valueRange{0.0, 1.0};
    if (const auto* dataMap = concept_->getDataMap()) {
        valueRange = dataMap->valueRange;
    }

    // make increment depending on the size of the underlying TF texture
    const double incr =
        concept_->hasTF()
            ? 1.0 / static_cast<double>(concept_->getTFProperty()->getLookUpTableSize())
            : 0.01;

    primitivePos_->setValueMapping(concept_->allRelative(), valueRange,
                                   incr * (valueRange.y - valueRange.x));
}

void TFPropertyDialog::onHistogramModeChange(HistogramMode mode) {
    chkShowHistogram_->setCurrentIndex(static_cast<int>(mode));
}

void TFPropertyDialog::setReadOnly(bool readonly) {
    colorWheel_->setDisabled(readonly);
    view_->setDisabled(readonly);
    primitivePos_->setDisabled(readonly);
    primitiveAlpha_->setDisabled(readonly);
    primitiveColor_->setDisabled(readonly);
    pointMoveMode_->setDisabled(readonly);
    tfMode_->setDisabled(readonly);
}

void TFPropertyDialog::updateTFPreview() {
    if (ongoingUpdate_) return;

    auto pixmap = utilqt::toQPixmap(*concept_, QSize(preview_->width(), 20));
    preview_->setPixmap(pixmap);
}

}  // namespace inviwo
