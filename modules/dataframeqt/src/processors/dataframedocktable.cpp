/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <inviwo/dataframeqt/processors/dataframedocktable.h>

#include <inviwo/core/io/datawriterutil.h>                 // for saveData
#include <inviwo/core/metadata/processorwidgetmetadata.h>  // for ProcessorWidgetMet...
#include <inviwo/core/network/networklock.h>               // for NetworkLock
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorstate.h>     // for CodeState, CodeSta...
#include <inviwo/core/processors/processortags.h>      // for Tags, Tag, Tags::CPU
#include <inviwo/core/processors/processorwidget.h>    // for ProcessorWidget
#include <inviwo/core/properties/boolproperty.h>       // for BoolProperty
#include <inviwo/core/properties/invalidationlevel.h>  // for InvalidationLevel
#include <inviwo/core/properties/ordinalproperty.h>    // for IntSize2Property
#include <inviwo/core/properties/property.h>           // for Property, Property...
#include <inviwo/core/properties/propertysemantics.h>  // for PropertySemantics
#include <inviwo/core/properties/valuewrapper.h>       // for PropertySerializat...

#include <inviwo/core/util/exception.h>         // for Exception
#include <inviwo/core/util/sourcecontext.h>     // for SourceContext
#include <inviwo/core/util/statecoordinator.h>  // for StateCoordinator

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>  // for BrushingAndLinking...
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrameInport
#include <inviwo/dataframeqt/dataframetableview.h>

#include <functional>  // for __base
#include <limits>      // for numeric_limits
#include <map>         // for map
#include <utility>     // for move
#include <memory>      // for shared_ptr, unique_ptr

#include <glm/vec2.hpp>  // for operator!=
#include <QObject>       // for Q_OBJECT
#include <QVBoxLayout>
#include <QMainWindow>
#include <QImage>

namespace inviwo {

DataFrameDockTableWidget::DataFrameDockTableWidget(Processor* p)
    : ProcessorDockWidgetQt(p, utilqt::toQString(p->getDisplayName()),
                            utilqt::getApplicationMainWindow())
    , tableview_{new DataFrameTableView(this)} {

    tableview_->setMouseTracking(true);
    tableview_->setAttribute(Qt::WA_OpaquePaintEvent);

    QSizePolicy sp(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sp.setVerticalStretch(5);
    sp.setHorizontalStretch(5);
    tableview_->setSizePolicy(sp);

    setFocusProxy(tableview_);

    auto* mainWidget = new QWidget();
    auto* layout = new QVBoxLayout(mainWidget);
    layout->setAlignment(Qt::AlignTop);
    const auto space = utilqt::refSpacePx(this);
    layout->setContentsMargins(0, space, 0, space);
    layout->setSpacing(space);

    layout->addWidget(tableview_);

    setWidget(mainWidget);
}

DataFrameDockTableWidget::~DataFrameDockTableWidget() = default;

void DataFrameDockTableWidget::setManager(BrushingAndLinkingManager& manager) {
    tableview_->setManager(manager);
}
void DataFrameDockTableWidget::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                            bool categoryIndices) {
    tableview_->setDataFrame(std::move(dataframe), categoryIndices);
}
void DataFrameDockTableWidget::setIndexColumnVisible(bool visible) {
    tableview_->setIndexColumnVisible(visible);
}
void DataFrameDockTableWidget::setFilteredRowsVisible(bool visible) {
    tableview_->setFilteredRowsVisible(visible);
}
void DataFrameDockTableWidget::brushingUpdate() { tableview_->brushingUpdate(); }

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameDockTable::processorInfo_{
    "org.inviwo.DataFrameDockTable",  // Class identifier
    "DataFrame Dock Table",           // Display name
    "Data Output",                    // Category
    CodeState::Stable,                // Code state
    Tags::CPU | Tag{"DataFrame"},     // Tags
    "Shows the content of a DataFrame in a tabular view."_help};

const ProcessorInfo& DataFrameDockTable::getProcessorInfo() const { return processorInfo_; }

DataFrameDockTable::DataFrameDockTable()
    : Processor()
    , inport_("inport", "DataFrame contents to be shown in the processor widget"_help)
    , brushLinkPort_("brushingAndLinking", "Inport for brushing & linking interactions"_help)

    , dimensions_{"dimensions", "Canvas Size",
                  util::ordinalCount(size2_t(512, 300), size2_t(10000, 10000))
                      .setMin(size2_t(1, 1))
                      .set(InvalidationLevel::Valid)}
    , position_{"position",
                "Canvas Position",
                ivec2(128, 128),
                ivec2(std::numeric_limits<int>::lowest()),
                ivec2(std::numeric_limits<int>::max()),
                ivec2(1, 1),
                InvalidationLevel::Valid,
                PropertySemantics::Text}
    , visible_{"visible", "Visible", true}
    , parent_{"parent", "Widget Parent", ""}
    , dockArea_{"dockArea",
                "Dock Area",
                {{"NoDockWidgetArea", "No Dock Area", Qt::NoDockWidgetArea},
                 {"LeftDockWidgetArea", "Left Dock Area", Qt::LeftDockWidgetArea},
                 {"RightDockWidgetArea", "Right Dock Area", Qt::RightDockWidgetArea},
                 {"TopDockWidgetArea", "Top Dock Area", Qt::TopDockWidgetArea},
                 {"BottomDockWidgetArea", "Bottom Dock Area", Qt::BottomDockWidgetArea}},
                1}
    , showIndexColumn_("showIndexColumn", "Show Index Column",
                       "show/hide index column in table"_help, false, InvalidationLevel::Valid)
    , showCategoryIndices_("showCategoryIndices", "Show Category Indices",
                           "show integral category indices for categorical columns"_help, false)
    , showFilteredRowCols_("showFilteredItems", "Show Filtered Items", true)
    , widgetMetaData_{createMetaData<ProcessorWidgetMetaData>(
          ProcessorWidgetMetaData::classIdentifier)}
    , currentParent_{} {

    widgetMetaData_->addObserver(this);

    addPort(inport_);
    addPort(brushLinkPort_);
    addProperties(dimensions_, position_, visible_, parent_, dockArea_, showIndexColumn_,
                  showCategoryIndices_, showFilteredRowCols_);

    // this is serialized in the widget metadata
    dimensions_.setSerializationMode(PropertySerializationMode::None);
    position_.setSerializationMode(PropertySerializationMode::None);
    visible_.setSerializationMode(PropertySerializationMode::None);

    dimensions_.onChange([this]() { widgetMetaData_->setDimensions(dimensions_.get()); });
    position_.onChange([this]() { widgetMetaData_->setPosition(position_.get()); });
    visible_.onChange([this]() { widgetMetaData_->setVisible(visible_.get()); });

    showIndexColumn_.onChange([this]() {
        if (auto* w = getWidget()) {
            w->setIndexColumnVisible(showIndexColumn_);
        }
    });
    showFilteredRowCols_.onChange([this]() {
        if (auto* w = getWidget()) {
            w->setFilteredRowsVisible(showFilteredRowCols_);
        }
    });

    parent_.onChange([this]() {
        if (auto* widget = getWidget()) {
            setWidgetParent(widget, parent_.get());
        }
    });

    dockArea_.onChange([this]() {
        if (auto* widget = getWidget()) {
            setDockArea(widget, dockArea_.get());
        }
    });
}

DataFrameDockTable::~DataFrameDockTable() {
    if (processorWidget_) {
        processorWidget_->setVisible(false);
    }
}

void DataFrameDockTable::setNetwork(ProcessorNetwork* network) {
    Processor::setNetwork(network);
    if (network) {
        network->addObserver(this);
    }
}

void DataFrameDockTable::process() {
    if (auto* w = getWidget()) {
        setWidgetParent(w, parent_.get());

        if (inport_.isChanged() || showCategoryIndices_.isModified()) {
            w->setDataFrame(inport_.getData(), showCategoryIndices_);
        }
        w->brushingUpdate();
    }
}

void DataFrameDockTable::setProcessorWidget(std::unique_ptr<ProcessorWidget> processorWidget) {
    auto* widget = getWidget();
    if (processorWidget && !widget) {
        throw Exception(
            "Expected DataFrameDockTableWidget in DataFrameDockTable::setProcessorWidget");
    }

    if (widget) {
        widget->setManager(brushLinkPort_.getManager());
        widget->setIndexColumnVisible(showIndexColumn_);
        widget->setFilteredRowsVisible(showFilteredRowCols_);

        setWidgetParent(widget, parent_.get());
    }

    Processor::setProcessorWidget(std::move(processorWidget));
    isSink_.update();
    isReady_.update();
}

void DataFrameDockTable::setWidgetParent(DataFrameDockTableWidget* widget,
                                         std::string_view processorId) {
    if (processorId != currentParent_) {
        if (processorId.empty()) {
            currentParent_.clear();
            if (auto* mw = utilqt::getApplicationMainWindow()) {
                widget->setParent(mw);
                mw->addDockWidget(Qt::LeftDockWidgetArea, widget);
                widget->setFloating(true);
            } else {
                widget->setParent(nullptr);
                widget->setFloating(true);
            }
        } else if (auto* p = getNetwork()->getProcessorByIdentifier(processorId)) {
            if (auto* parent = dynamic_cast<QMainWindow*>(p->getProcessorWidget())) {
                widget->setParent(parent);
                setDockArea(widget, dockArea_.get());
                currentParent_ = processorId;
            }
        }
    }
}

void DataFrameDockTable::setDockArea(DataFrameDockTableWidget* widget, Qt::DockWidgetArea area) {
    if (auto* mw = dynamic_cast<QMainWindow*>(widget->parent())) {
        mw->addDockWidget(area, widget);
        widget->setFloating(area == Qt::NoDockWidgetArea);

        if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea) {
            mw->resizeDocks({widget}, {static_cast<int>(widgetMetaData_->getDimensions().x)},
                            Qt::Horizontal);
        } else if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
            mw->resizeDocks({widget}, {static_cast<int>(widgetMetaData_->getDimensions().y)},
                            Qt::Vertical);
        }
    }
}

void DataFrameDockTable::onProcessorWidgetPositionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getPosition() != position_.get()) {
        const Property::OnChangeBlocker blocker{position_};
        position_.set(widgetMetaData_->getPosition());
    }
}

void DataFrameDockTable::onProcessorWidgetDimensionChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->getDimensions() != dimensions_.get()) {
        const Property::OnChangeBlocker blocker{dimensions_};
        dimensions_.set(widgetMetaData_->getDimensions());
    }
}

void DataFrameDockTable::onProcessorWidgetVisibilityChange(ProcessorWidgetMetaData*) {
    if (widgetMetaData_->isVisible() != visible_.get()) {
        const Property::OnChangeBlocker blocker{visible_};
        visible_.set(widgetMetaData_->isVisible());
    }
    isSink_.update();
    isReady_.update();
    invalidate(InvalidationLevel::InvalidOutput);
}

void DataFrameDockTable::onProcessorNetworkDidAddProcessor(Processor* p) {
    if (parent_.get() != currentParent_ && p->getIdentifier() == parent_.get()) {
        if (auto* w = getWidget()) {
            setWidgetParent(w, parent_.get());
        }
    }
}
void DataFrameDockTable::onProcessorNetworkWillRemoveProcessor(Processor* p) {
    if (p->getIdentifier() == parent_.get()) {
        if (auto* w = getWidget()) {
            setWidgetParent(w, "");
        }
    }
}

DataFrameDockTableWidget* DataFrameDockTable::getWidget() const {
    return dynamic_cast<DataFrameDockTableWidget*>(processorWidget_.get());
}

void DataFrameDockTable::setWidgetSize(size2_t dim) {
    const NetworkLock lock{this};
    dimensions_.set(dim);
}

size2_t DataFrameDockTable::getWidgetSize() const { return dimensions_.get(); }

std::optional<std::filesystem::path> DataFrameDockTable::exportFile(
    const std::filesystem::path& path, std::string_view name,
    const std::vector<FileExtension>& candidateExtensions, Overwrite overwrite) const {

    if (auto data = inport_.getData()) {
        return util::saveData(*data, path, name, candidateExtensions, overwrite);
    }

    throw Exception("Inport has no data");
}

std::shared_ptr<const Image> DataFrameDockTable::getImage() const {
    if (auto* w = getWidget()) {
        QPixmap pm(w->size());
        w->render(&pm);

        return std::make_shared<Image>(utilqt::toLayer(pm.toImage()));
    }

    return nullptr;
}

}  // namespace inviwo
