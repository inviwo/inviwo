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

#pragma once

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/meshport.h>
#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <inviwo/core/io/serialization/serializer.h>
#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/stdextensions.h>

#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrame
#include <inviwo/dataframe/properties/columnoptionproperty.h>

#include <flags/allow_flags.h>
#include <flags/flags.h>

#include <array>
#include <variant>
#include <span>

namespace inviwo {

namespace util {

/**
 * This class manages the properties required for creating Buffers from a DataFrame
 * dataset. In particular, it allows to select specific columns to be used for vertex data,
 * normals, colors, etc. Data values can also be transformed.
 */
class IVW_MODULE_DATAFRAME_API ColumnMapper {
public:
    struct Info;

    explicit ColumnMapper(std::span<Info> infos);

    /**
     * Query the isModified() status of all nested composite properties
     * @return true if any of the composite info properties is modified
     */
    bool isModified() const;

    /**
     * Update the internal state to match the DataFrame columns of \p df and their names.
     * @param df   DataFrame used as source for generating mesh buffers and updating the internal
     * properties.
     * @see getBuffers
     */
    void updateSources(const DataFrame& df);

    /**
     * Generate mesh buffers for the given DataFrame set \p df based on the internal properties.
     * @param df   DataFrame used as data source
     * @see updateSources
     */
    Mesh::BufferVector getBuffers(const DataFrame& df);

    struct IVW_MODULE_DATAFRAME_API ScaleAndOffset {
        ScaleAndOffset();
        DoubleProperty scale;
        DoubleProperty offset;
    };

    struct IVW_MODULE_DATAFRAME_API OffsetAndPicking {
        OffsetAndPicking(Processor* p, size_t size, std::function<void(PickingEvent*)> callback);
        IntProperty offset;
        PickingMapper pickingMapper;
    };

    template <size_t N, typename T, typename Transform>
    struct Type {
        using ElemType = T;

        explicit Type(Transform transform,
                      std::optional<std::array<std::string, N>> defaults = std::nullopt)
            : sources{util::make_array<N>([](auto i) {
                static constexpr std::array<std::string_view, 4> labels = {"x", "y", "z", "w"};
                return ColumnOptionProperty{fmt::format("source{}", i),
                                            fmt::format("Source {}", labels[i]),
                                            ColumnOptionProperty::AddNoneOption::Yes};
            })}
            , transform{std::move(transform)}
            , defaultColumns{std::move(defaults)} {}

        std::array<ColumnOptionProperty, N> sources;
        Transform transform;
        std::optional<std::array<std::string, N>> defaultColumns;
    };

    using SpatialType = Type<3, vec3, OrdinalProperty<dmat4>>;
    using RealType = Type<1, float, ScaleAndOffset>;
    using IntType = Type<1, uint32_t, ScaleAndOffset>;
    using PickingType = Type<1, uint32_t, OffsetAndPicking>;
    using ColorType = Type<1, vec4, TransferFunctionProperty>;

    using Types = std::variant<SpatialType, RealType, IntType, PickingType, ColorType>;

    struct IVW_MODULE_DATAFRAME_API Info {
        Info(BufferType bt, Types type);
        glm::dvec2 getDataRange() const;
        Types type;
        BufferType bufferType;
        BoolCompositeProperty comp;
        DoubleMinMaxProperty range;
        BoolCompositeProperty doTransform;

        template <typename F>
        auto visit(F&& f) {
            return std::visit(std::forward<F>(f), type);
        }
        template <typename F>
        auto visit(F&& f) const {
            return std::visit(std::forward<F>(f), type);
        }
    };

private:
    std::span<Info> infos_;
};

}  // namespace util

class PickingEvent;

class IVW_MODULE_DATAFRAME_API DataFrameToMesh : public Processor {
public:
    DataFrameToMesh();

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    using Info = util::ColumnMapper::Info;

    using ScaleAndOffset = util::ColumnMapper::ScaleAndOffset;
    using OffsetAndPicking = util::ColumnMapper::OffsetAndPicking;

    using SpatialType = util::ColumnMapper::SpatialType;
    using RealType = util::ColumnMapper::RealType;
    using IntType = util::ColumnMapper::IntType;
    using PickingType = util::ColumnMapper::PickingType;
    using ColorType = util::ColumnMapper::ColorType;

    using enum BufferType;

    void picking(PickingEvent* event);

    DataInport<DataFrame> dataFrame_;
    BrushingAndLinkingInport bnl_;
    MeshOutport outport_;

    OptionPropertyInt drawType_;

    std::array<Info, 10> infos_;
    util::ColumnMapper mapper_;

    OrdinalProperty<mat4> modelMatrix_;
    OrdinalProperty<mat4> worldMatrix_;
};

}  // namespace inviwo
