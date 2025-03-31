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

#include <inviwo/dataframe/processors/dataframetomesh.h>

#include <inviwo/core/interaction/events/pickingevent.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/glmcomp.h>

#include <modules/base/algorithm/dataminmax.h>

#include <inviwo/dataframe/datastructures/column.h>

#include <ranges>

namespace inviwo {
namespace util {

namespace {

template <typename T = double, typename U = T>
OrdinalPropertyState<T> transformState(T val = T{1}) {
    return {val,
            util::filled<T>(static_cast<util::value_type_t<T>>(-10)),
            ConstraintBehavior::Ignore,
            util::filled<T>(static_cast<util::value_type_t<T>>(10)),
            ConstraintBehavior::Ignore,
            util::filled<T>(static_cast<util::value_type_t<T>>(0.01)),
            InvalidationLevel::InvalidOutput,
            PropertySemantics::Text};
}

}  // namespace

ColumnMapper::ColumnMapper(std::span<Info> infos) : infos_{infos} {}

bool ColumnMapper::isModified() const {
    bool update = false;
    for (auto& info : infos_) {
        update |= info.comp.isModified() && info.comp.getOwner();
    }
    return update;
}

void ColumnMapper::updateSources(const DataFrame& df) {
    for (auto& info : infos_) {
        info.visit([&](auto& i) {
            for (auto& source : i.sources) {
                source.setOptions(df);
            }
        });
    }
}

namespace {

void transform(const OrdinalProperty<dmat4>& mat, auto& dst, dvec2) {
    std::ranges::transform(dst, dst.begin(), [&]<typename T>(const T& val) {
        const auto dval = static_cast<util::same_extent_t<T, double>>(val);
        if constexpr (util::extent_v<T> == 1) {
            auto res = mat.get() * dvec4{dval, 0.0, 0.0, 1.0};
            return static_cast<T>(res[0]);
        } else if constexpr (util::extent_v<T> == 2) {
            auto res = mat.get() * dvec4{dval, 0.0, 1.0};
            return static_cast<T>(res);
        } else if constexpr (util::extent_v<T> == 3) {
            auto res = mat.get() * dvec4{dval, 1.0};
            return static_cast<T>(res);
        } else if constexpr (util::extent_v<T> == 4) {
            return static_cast<T>(mat.get() * dval);
        } else {
            throw Exception("no matching type");
        }
    });
}
void transform(ColumnMapper::OffsetAndPicking& so, auto& dst, dvec2 range) {
    so.pickingMapper.resize(static_cast<size_t>(range[1] - range[0] + 1.0));

    std::ranges::transform(dst, dst.begin(), [&]<typename T>(T& val) {
        auto i = util::glmcomp(val, 0) + so.offset.get() - static_cast<int>(range[0]);
        i = std::clamp(i, decltype(i){0}, static_cast<decltype(i)>(so.pickingMapper.getSize()));
        return static_cast<uint32_t>(so.pickingMapper.getPickingId(static_cast<size_t>(i)));
    });
}
void transform(const ColumnMapper::ScaleAndOffset& so, auto& dst, dvec2) {
    std::ranges::transform(dst, dst.begin(), [&]<typename T>(T& val) {
        const auto dval = static_cast<util::same_extent_t<T, double>>(val);
        return static_cast<T>(so.scale.get() * (dval + so.offset.get()));
    });
}
void transform(const TransferFunctionProperty& tf, auto& dst, dvec2 range) {
    std::ranges::transform(dst, dst.begin(), [&]<typename T>(T& val) {
        const auto n = (util::glmcomp(val, 0) - range[0]) / (range[1] - range[0]);
        return tf.get().sample(n);
    });
}

}  // namespace

Mesh::BufferVector ColumnMapper::getBuffers(const DataFrame& df) {
    Mesh::BufferVector buffers;
    for (auto& info : infos_) {
        info.range.set(dvec2{0, 0});

        if (!info.comp) {
            continue;
        }

        info.visit([&]<typename Type>(Type& self) {
            using T = typename Type::ElemType;
            const auto size = df.getNumberOfRows();
            auto buffer = std::make_shared<Buffer<T>>(size);
            buffers.emplace_back(info.bufferType, buffer);

            auto& dst = buffer->getEditableRAMRepresentation()->getDataContainer();

            for (auto&& [comp, s] : util::enumerate(self.sources)) {
                if (!s.isNoneSelected()) {
                    const auto col = df.getColumn(s.getSelectedValue());
                    col->getRAMRepresentation()
                        ->template dispatch<void, dispatching::filter::Scalars>(
                            [&](auto brprecision) {
                                const auto& src = brprecision->getDataContainer();
                                for (size_t i = 0; i < size; ++i) {
                                    util::glmcomp(dst[i], comp) =
                                        static_cast<util::value_type_t<T>>(src[i]);  // NOLINT
                                }
                            });
                }
            }

            if (!std::empty(dst)) {
                const auto range = util::dataMinMax(dst.data(), dst.size());
                info.range.set({range.first[0], range.second[0]});
            } else {
                info.range.set({0.0, 0.0});
            }

            if (info.doTransform) {
                transform(self.transform, dst, info.range.get());
            }
        });
    }
    return buffers;
}

ColumnMapper::Info::Info(BufferType bt, Types type)
    : type{std::move(type)}
    , bufferType{bt}
    , comp{toLower(enumToStr(bt)), enumToStr(bt)}
    , range("range", "Range", "Range of input data, before any transforms"_help, 0.0, 0.0,
            std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 0.001, 0.0,
            InvalidationLevel::Valid, PropertySemantics::Text)
    , doTransform("doTransform", "Apply transformation", false) {

    visit([&](auto& self) {
        for (auto& s : self.sources) {
            comp.addProperty(s);
        }
    });

    comp.addProperties(range, doTransform);
    range.setReadOnly(true);
    visit([&](auto& self) {
        util::overloaded{
            [&](Property& item) { doTransform.addProperty(item); },
            [&](ScaleAndOffset& item) {
                doTransform.addProperty(item.offset);
                doTransform.addProperty(item.scale);
            },
            [&](OffsetAndPicking& item) { doTransform.addProperty(item.offset); }}(self.transform);
    });
}

dvec2 ColumnMapper::Info::getDataRange() const {
    if (doTransform.isChecked()) {
        return visit([&](auto& self) {
            return util::overloaded{[&](const Property&) { return range.get(); },
                                    [&](const ScaleAndOffset& item) {
                                        return (range.get() + dvec2{item.offset}) *
                                               dvec2{item.scale};
                                    },
                                    [&](const OffsetAndPicking& item) {
                                        return range.get() + dvec2(item.offset.get());
                                    }}(self.transform);
        });
    } else {
        return range.get();
    }
}

ColumnMapper::ScaleAndOffset::ScaleAndOffset()
    : scale{"scale", "Scale", transformState<double>(1.0)}
    , offset{"offset", "Offset", transformState<double>(0.0)} {}

ColumnMapper::OffsetAndPicking::OffsetAndPicking(Processor* p, size_t size,
                                                 std::function<void(PickingEvent*)> callback)
    : offset{"offset", "Offset", transformState<int>(0)}
    , pickingMapper(p, size, std::move(callback)) {}

}  // namespace util

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo DataFrameToMesh::processorInfo_{
    "org.inviwo.DataFrameToMesh",  // Class identifier
    "DataFrame To Mesh",           // Display name
    "DataFrame",                   // Category
    CodeState::Stable,             // Code state
    Tags::CPU,                     // Tags
    R"(Convert DataFrame columns into a Mesh)"_unindentHelp,
};

const ProcessorInfo& DataFrameToMesh::getProcessorInfo() const { return processorInfo_; }

DataFrameToMesh::DataFrameToMesh()
    : Processor{}
    , dataFrame_{"dataFrame", ""_help}
    , bnl_{"brushingAndLinking"}
    , outport_{"outport_", ""_help}
    , drawType_{"drawType", "Draw Type", {{"points", "Points", 0}, {"lines", "Lines", 1}}, 0}
    , infos_{{{PositionAttrib, SpatialType{OrdinalProperty<dmat4>{"transform", "Transform",
                                                                  util::transformState<dmat4>()},
                                           std::array<std::string, 3>{"x", "y", "z"}}},
              {NormalAttrib, SpatialType{OrdinalProperty<dmat4>("transform", "Transform",
                                                                util::transformState<dmat4>())}},
              {ColorAttrib, ColorType{TransferFunctionProperty("transform", "Transform")}},
              {TexCoordAttrib, SpatialType{OrdinalProperty<dmat4>("transform", "Transform",
                                                                  util::transformState<dmat4>())}},
              {CurvatureAttrib, RealType{ScaleAndOffset{}}},
              {IndexAttrib, IntType{ScaleAndOffset{}}},
              {RadiiAttrib, RealType{ScaleAndOffset{}}},
              {PickingAttrib, PickingType{OffsetAndPicking{
                                  this, 0, [this](PickingEvent* event) { picking(event); }}}},
              {ScalarMetaAttrib, RealType{ScaleAndOffset{}}}}}

    , mapper_{infos_}
    , modelMatrix_{"modelMatrix", "Model Matrix", util::transformState(mat4{1.0})}
    , worldMatrix_{"worldMatrix", "World Matrix", util::transformState(mat4{1.0})} {

    addPorts(dataFrame_, bnl_, outport_);
    addProperties(drawType_);

    infos_[0].comp.setChecked(true);
    infos_[0].comp.setCurrentStateAsDefault();

    for (auto& info : infos_) {
        addProperty(info.comp);
    }

    addProperties(modelMatrix_, worldMatrix_);
}

void DataFrameToMesh::process() {
    const auto data = dataFrame_.getData();

    if (dataFrame_.isChanged()) {
        mapper_.updateSources(*data);
    }

    auto mesh = std::make_shared<Mesh>(mapper_.getBuffers(*data), Mesh::IndexVector{});

    if (drawType_.getSelectedValue() == 0) {
        mesh->setDefaultMeshInfo({DrawType::Points, ConnectivityType::None});
    } else if (drawType_.getSelectedValue() == 1) {
        mesh->setDefaultMeshInfo({DrawType::Lines, ConnectivityType::Strip});
    }

    {
        auto& pos = std::get<SpatialType>(infos_[0].type);
        for (size_t i = 0; i < pos.sources.size(); ++i) {
            if (!pos.sources[i].isNoneSelected()) {
                const auto col = data->getColumn(pos.sources[i].getSelectedValue());
                mesh->axes[i].name = col->getHeader();
                mesh->axes[i].unit = col->getUnit();
            }
        }
    }

    mesh->setModelMatrix(modelMatrix_.get());
    mesh->setWorldMatrix(worldMatrix_.get());

    outport_.setData(mesh);
}

void DataFrameToMesh::picking(PickingEvent* event) {
    const auto id = static_cast<uint32_t>(event->getPickedId());

    // Show tooltip for current item
    if (event->getHoverState() == PickingHoverState::Enter) {
        event->setToolTip(fmt::format("{}", event->getPickedId()));

        const BitSet highlight{id};
        bnl_.highlight(highlight);
    } else if (event->getHoverState() == PickingHoverState::Exit) {
        // unset tooltip at all times in case one was set prior disabling tooltips
        event->setToolTip("");
        bnl_.highlight({});
    }

    if ((event->getPressState() == PickingPressState::Release) &&
        (event->getPressItem() == PickingPressItem::Primary) && !event->getMovedSincePressed()) {

        BitSet selected(bnl_.getSelectedIndices());
        selected.flip(id);
        bnl_.select(selected);
        event->setUsed(true);
    }
}

}  // namespace inviwo
