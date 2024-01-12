/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2024 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframemodel.h>

#include <inviwo/core/datastructures/bitset.h>                          // for BitSet
#include <inviwo/core/datastructures/buffer/bufferram.h>                // for BufferRAM, Buffer...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/util/formatdispatching.h>                         // for Scalars, Vecs
#include <inviwo/core/util/formats.h>                                   // for DataFormatBase
#include <inviwo/core/util/glmvec.h>                                    // for dvec2
#include <inviwo/core/util/raiiutils.h>                                 // for OnScopeExit, OnSc...
#include <inviwo/core/util/stringconversion.h>                          // for toString
#include <inviwo/core/util/zip.h>                                       // for enumerate, zipIte...
#include <inviwo/dataframe/datastructures/column.h>                     // for Column, ColumnType
#include <inviwo/dataframe/datastructures/dataframe.h>                  // for DataFrame
#include <modules/brushingandlinking/brushingandlinkingmanager.h>       // for BrushingAndLinkin...
#include <modules/brushingandlinking/datastructures/brushingaction.h>   // for BrushingTarget
#include <modules/qtwidgets/inviwoqtutils.h>                            // for toQString

#include <optional>       // for optional
#include <string>         // for string
#include <type_traits>    // for remove_extent_t
#include <unordered_set>  // for unordered_set

#include <QBrush>          // for QBrush
#include <QColor>          // for QColor
#include <QList>           // for QList<>::const_it...
#include <QMetaType>       // for QMetaType, QMetaT...
#include <QString>         // for QString
#include <QtGlobal>        // for qulonglong, qlong...
#include <fmt/core.h>      // for basic_string_view
#include <glm/gtx/io.hpp>  // for operator<<
#include <glm/vec2.hpp>    // for vec<>::(anonymous)

class QModelIndex;

namespace inviwo {

DataFrameModel::DataFrameModel(QObject* parent) : QAbstractTableModel(parent), manager_(nullptr) {}

DataFrameModel::~DataFrameModel() = default;

void DataFrameModel::setManager(BrushingAndLinkingManager& manager) { manager_ = &manager; }

void DataFrameModel::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                  bool categoryIndices) {
    util::OnScopeExit onscopeexit([&]() { endResetModel(); });

    beginResetModel();
    data_ = dataframe;
    valueFuncs_.clear();
    tooltipFuncs_.clear();

    if (!data_) {
        return;
    }

    auto getValueFunc = [categoryIndices](const Column* col) -> ValueFunc {
        switch (col->getColumnType()) {
            case ColumnType::Categorical:
                if (!categoryIndices) {
                    return [cc = static_cast<const CategoricalColumn*>(col)](int row) -> QVariant {
                        return utilqt::toQString(cc->getAsString(row));
                    };
                } else {
                    return col->getBuffer()
                        ->getRepresentation<BufferRAM>()
                        ->template dispatch<ValueFunc, dispatching::filter::Scalars>([](auto br) {
                            return [br](int row) -> QVariant {
                                auto val = br->getDataContainer()[row];
                                return QVariant{static_cast<qulonglong>(val)};
                            };
                        });
                }
                break;
            case ColumnType::Index:
            case ColumnType::Ordinal: {
                auto df = col->getBuffer()->getDataFormat();
                if (df->getComponents() == 1) {
                    return col->getBuffer()
                        ->getRepresentation<BufferRAM>()
                        ->template dispatch<ValueFunc, dispatching::filter::Scalars>([](auto br) {
                            return [br](int row) -> QVariant {
                                auto val = br->getDataContainer()[row];
                                if constexpr (std::is_floating_point_v<decltype(val)>) {
                                    return QVariant{val};
                                } else if constexpr (std::is_signed_v<decltype(val)>) {
                                    return QVariant{static_cast<qlonglong>(val)};
                                } else {
                                    return QVariant{static_cast<qulonglong>(val)};
                                }
                            };
                        });
                } else {
                    // more than one component, convert to string
                    return col->getBuffer()
                        ->getRepresentation<BufferRAM>()
                        ->template dispatch<ValueFunc, dispatching::filter::Vecs>([](auto br) {
                            return [br](int row) {
                                return QVariant{
                                    utilqt::toQString(toString(br->getDataContainer()[row]))};
                            };
                        });
                }
                break;
            }
            default:
                return [](int) { return QVariant(); };
        }
    };
    auto getToolTipFunc = [categoryIndices](const Column* col) -> std::function<QVariant(int)> {
        switch (col->getColumnType()) {
            case ColumnType::Categorical:
                if (!categoryIndices) {
                    return [cc = static_cast<const CategoricalColumn*>(col)](int row) -> QVariant {
                        return QString("Key: %1").arg(cc->getId(row));
                    };
                } else {
                    return [](int) { return QVariant(); };
                }
                break;
            case ColumnType::Ordinal:
            case ColumnType::Index:
            default:
                return [](int) { return QVariant(); };
        }
    };

    for (auto col : *data_) {
        valueFuncs_.push_back(getValueFunc(col.get()));
        tooltipFuncs_.push_back(getToolTipFunc(col.get()));
    }
}

int DataFrameModel::rowCount(const QModelIndex&) const {
    if (data_) return static_cast<int>(data_->getNumberOfRows());
    return 0;
}

int DataFrameModel::columnCount(const QModelIndex&) const {
    if (data_) return static_cast<int>(data_->getNumberOfColumns());
    return 0;
}

QVariant DataFrameModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();

    switch (role) {
        case Qt::DisplayRole: {
            QVariant val = valueFuncs_[index.column()](index.row());
            switch (val.typeId()) {
                case QMetaType::Double:
                    return QString::number(val.toDouble(), 'g', 6);
                case QMetaType::Float:
                    return QString::number(val.toFloat());
                case QMetaType::QString:
                default:
                    return val.toString();
            }
        }
        case Roles::Data:
            return valueFuncs_[index.column()](index.row());
        case Qt::ToolTipRole:
            return tooltipFuncs_[index.column()](index.row());
        case Qt::BackgroundRole: {
            const auto& indexCol = data_->getIndexColumn()
                                       ->getTypedBuffer()
                                       ->getRAMRepresentation()
                                       ->getDataContainer();

            const bool highlighted =
                manager_ ? (manager_->isHighlighted(indexCol[index.row()]) ||
                            manager_->isHighlighted(index.column(), BrushingTarget::Column))
                         : false;
            const bool selected = manager_ ? manager_->isSelected(indexCol[index.row()]) : false;

            if (highlighted) {
                return QBrush(QColor(102, 87, 50));
            } else if (selected) {
                return QBrush(QColor("#47477b"));
            } else {
                return QBrush();
            }
        }
        default:
            return QVariant();
    }
}

void DataFrameModel::brushingUpdate() {
    if (!data_ || !manager_) return;

    if (manager_->isSelectionModified(BrushingTarget::Column)) {
        emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
    }
    if (manager_->isSelectionModified(BrushingTarget::Row) ||
        manager_->isHighlightModified(BrushingTarget::Row)) {
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                         {Qt::BackgroundRole});
    }
}

QVariant DataFrameModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (!data_) return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            // use regular row numbers starting at 1
            return section + 1;
        } else {
            const std::string selected =
                manager_ ? (manager_->isSelected(section, BrushingTarget::Column) ? " [+]" : "")
                         : "";
            const auto& col = data_->getColumn(section);
            const auto header =
                fmt::format("{}{}{: [}", selected, col->getHeader(), col->getUnit());
            return utilqt::toQString(header);
        }
    } else if ((role == Qt::ToolTipRole) && (orientation == Qt::Horizontal)) {
        const Column* col = data_->getColumn(section).get();
        auto tooltip = [col, section]() {
            if (col->getColumnType() == ColumnType::Categorical) {
                return QString("<b>%0</b> (Column %1)\nCategorical (%2 categories)")
                    .arg(utilqt::toQString(col->getHeader()))
                    .arg(section)
                    .arg(static_cast<const CategoricalColumn*>(col)->getCategories().size());
            } else {
                return QString("<b>%0</b> (Column %1)\nOrdinal (%2)")
                    .arg(utilqt::toQString(col->getHeader()))
                    .arg(section)
                    .arg(col->getBuffer()->getDataFormat()->getString());
            }
        }();
        if (col->getCustomRange()) {
            const dvec2 range = col->getCustomRange().value();
            tooltip.append(QString("\nColumn Range [%1, %2]").arg(range.x).arg(range.y));
        }
        return tooltip;
    }
    return QVariant();
}

void DataFrameModel::highlightRow(const QModelIndex& index) {
    if (!data_ || !manager_) return;

    if (!index.isValid()) {
        manager_->brush(BrushingAction::Highlight, BrushingTarget::Row, BitSet());
        return;
    }

    // translate model indices to row IDs
    const auto& indexCol =
        data_->getIndexColumn()->getTypedBuffer()->getRAMRepresentation()->getDataContainer();
    BitSet highlighted;
    highlighted.add(indexCol[index.row()]);
    manager_->brush(BrushingAction::Highlight, BrushingTarget::Row, highlighted);
}

void DataFrameModel::selectRows(const QModelIndexList& indices) {
    if (!data_ || !manager_) return;

    // translate model indices to row IDs
    const auto& indexCol =
        data_->getIndexColumn()->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    BitSet selection;
    for (auto& index : indices) {
        selection.add(indexCol[index.row()]);
    }
    manager_->brush(BrushingAction::Select, BrushingTarget::Row, selection);
}

std::vector<int> DataFrameModel::getSelectedRows() const {
    if (!data_ || !manager_) return {};

    const BitSet& selected = manager_->getSelectedIndices(BrushingTarget::Row);
    if (selected.empty()) return {};

    const auto& indexCol =
        data_->getIndexColumn()->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    std::vector<int> rows;
    for (auto&& [i, index] : util::enumerate<int>(indexCol)) {
        if (selected.contains(index)) {
            rows.push_back(i);
        }
    }
    return rows;
}

}  // namespace inviwo
