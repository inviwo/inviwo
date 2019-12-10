/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/dataframeqt/dataframetableview.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/datastructures/column.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/raiiutils.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <type_traits>

#include <warn/push>
#include <warn/ignore/all>
#include <QHeaderView>
#include <warn/pop>

namespace inviwo {

DataFrameTableView::DataFrameTableView(QWidget* parent) : QTableWidget(3, 3, parent) {
    horizontalHeader()->setStretchLastSection(true);

    // make it read-only
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);

    QObject::connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
                     [this](const QItemSelection&, const QItemSelection&) {
                         if (ignoreEvents_) return;
                         util::KeepTrueWhileInScope ignore(&ignoreUpdate_);
                         if (data_) {
                             // translate model indices to row IDs
                             const auto& indexCol = data_->getIndexColumn()
                                                        ->getTypedBuffer()
                                                        ->getRAMRepresentation()
                                                        ->getDataContainer();

                             std::unordered_set<size_t> selection;
                             for (auto& index : selectionModel()->selection().indexes()) {
                                 selection.insert(indexCol[index.row()]);
                             }
                             emit rowSelectionChanged(selection);
                         }
                     });
}

void DataFrameTableView::setDataFrame(std::shared_ptr<const DataFrame> dataframe,
                                      bool vectorsIntoColumns) {
    if (data_ == dataframe) return;

    data_ = dataframe;
    if (!dataframe) {
        clear();
        return;
    }

    vectorsIntoCols_ = vectorsIntoColumns;
    auto headers = generateHeaders();

    // empty existing table, and disable sorting temporarily (messes up insertion otherwise)
    clear();
    setColumnCount(static_cast<int>(headers.size()));
    setRowCount(static_cast<int>(dataframe->getNumberOfRows()));

    setHorizontalHeaderLabels(headers);

    int colIndex = 0;
    for (auto col : *dataframe) {
        std::vector<std::function<QVariant(size_t)>> getValueFunc;
        auto df = col->getBuffer()->getDataFormat();
        if (auto cc = dynamic_cast<const CategoricalColumn*>(col.get())) {
            getValueFunc.push_back(
                [cc](size_t index) { return utilqt::toQString(cc->getAsString(index)); });
        } else if (df->getComponents() == 1) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Scalars>([&getValueFunc](auto br) {
                    getValueFunc.push_back([br](size_t index) {
                        auto val = br->getDataContainer()[index];
                        if constexpr (std::is_floating_point_v<decltype(val)>) {
                            return QVariant{val};
                        } else if constexpr (std::is_signed_v<decltype(val)>) {
                            return QVariant{static_cast<qlonglong>(val)};
                        } else {
                            return QVariant{static_cast<qulonglong>(val)};
                        }
                    });
                });
        } else if (df->getComponents() > 1 && vectorsIntoColumns) {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&getValueFunc](auto br) {
                    using ValueType = util::PrecisionValueType<decltype(br)>;
                    for (size_t i = 0; i < util::flat_extent<ValueType>::value; ++i) {
                        getValueFunc.push_back([br, i](size_t index) {
                            auto val = br->getDataContainer()[index][i];
                            if constexpr (std::is_floating_point_v<decltype(val)>) {
                                return QVariant{val};
                            } else if constexpr (std::is_signed_v<decltype(val)>) {
                                return QVariant{static_cast<qlonglong>(val)};
                            } else {
                                return QVariant{static_cast<qulonglong>(val)};
                            }
                        });
                    }
                });
        } else {
            col->getBuffer()
                ->getRepresentation<BufferRAM>()
                ->dispatch<void, dispatching::filter::Vecs>([&getValueFunc](auto br) {
                    getValueFunc.push_back([br](size_t index) {
                        return QVariant{utilqt::toQString(toString(br->getDataContainer()[index]))};
                    });
                });
        }

        for (auto& func : getValueFunc) {
            for (size_t row = 0; row < col->getSize(); ++row) {
                const QString str = [val = func(row)]() {
                    switch (static_cast<QMetaType::Type>(val.type())) {
                        case QMetaType::Double:
                            return QString::number(val.toDouble(), 'g', 6);
                        case QMetaType::Float:
                            return QString::number(val.toFloat());
                        case QMetaType::QString:
                        default:
                            return val.toString();
                    }
                }();

                setItem(static_cast<int>(row), colIndex, new QTableWidgetItem(str));
            }
            ++colIndex;
        }
    }

    if (columnCount() > 0) {
        horizontalHeader()->setSectionHidden(0, !indexVisible_);
    }
}

void DataFrameTableView::setIndexColumnVisible(bool visible) {
    indexVisible_ = visible;
    if (columnCount() > 0) {
        horizontalHeader()->setSectionHidden(0, !indexVisible_);
    }
}

bool DataFrameTableView::isIndexColumnVisible() const { return indexVisible_; }

void DataFrameTableView::selectColumns(const std::unordered_set<size_t>& columns) {
    if (!data_ || ignoreUpdate_) return;

    setHorizontalHeaderLabels(generateHeaders(columns));
}

void DataFrameTableView::selectRows(const std::unordered_set<size_t>& rows) {
    if (!data_ || ignoreUpdate_) return;

    util::KeepTrueWhileInScope ignore(&ignoreEvents_);
    selectionModel()->clearSelection();

    if (rows.empty()) return;

    // translate row IDs into row numbers and fetch corresponding model indices
    const auto& indexCol =
        data_->getIndexColumn()->getTypedBuffer()->getRAMRepresentation()->getDataContainer();

    QItemSelection s;
    for (size_t i = 0; i < indexCol.size(); ++i) {
        if (rows.count(indexCol[i]) > 0) {
            QModelIndex start{model()->index(static_cast<int>(i), 0)};
            QModelIndex end{model()->index(static_cast<int>(i), columnCount() - 1)};
            s.select(start, end);
        }
    }
    selectionModel()->select(s, QItemSelectionModel::Select);
}

QStringList DataFrameTableView::generateHeaders(
    const std::unordered_set<size_t>& selectedCols) const {

    const std::array<char, 4> componentNames = {'X', 'Y', 'Z', 'W'};
    QStringList headers;
    size_t colIndex = 0;
    for (const auto& col : *data_) {
        const std::string selected = (selectedCols.count(colIndex) > 0) ? " [+]" : "";
        const auto components = col->getBuffer()->getDataFormat()->getComponents();
        if (components > 1 && vectorsIntoCols_) {
            for (size_t k = 0; k < components; k++) {
                headers.push_back(
                    utilqt::toQString(col->getHeader() + ' ' + componentNames[k] + selected));
            }
        } else {
            headers.push_back(utilqt::toQString(col->getHeader() + selected));
        }
        ++colIndex;
    }

    return headers;
}

}  // namespace inviwo
