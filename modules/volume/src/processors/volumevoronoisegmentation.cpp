/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <inviwo/volume/processors/volumevoronoisegmentation.h>
#include <modules/base/algorithm/volume/volumevoronoi.h>

#include <inviwo/core/util/zip.h>
#include <algorithm>
#include <functional>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeVoronoiSegmentation::processorInfo_{
    "org.inviwo.VolumeVoronoiSegmentation",                       // Class identifier
    "Volume Voronoi Segmentation",                                // Display name
    "Volume Operation",                                           // Category
    CodeState::Stable,                                            // Code state
    Tags::CPU | Tag{"Volume"} | Tag{"Atlas"} | Tag{"DataFrame"},  // Tags
    R"(Processor which calculates the voronoi segmentation of a volume,
       given the volume together with seed points (and optional weights) as input.
    )"_unindentHelp};
const ProcessorInfo& VolumeVoronoiSegmentation::getProcessorInfo() const { return processorInfo_; }

VolumeVoronoiSegmentation::VolumeVoronoiSegmentation()
    : PoolProcessor(pool::Option::DelayDispatch)
    , volume_("inputVolume", "The input volume"_help)
    , dataFrame_("seedPoints", R"(
        Seed points together with indices and optional weights. The seed points are
        expected to be in the model space. The column with the weights should have
        header name "r".)"_unindentHelp)
    , outport_("outport", R"(
        Volume where each voxel has the value of the closest seed point, according to the
        Voronoi algorithm)"_unindentHelp)
    , weighted_("weighted", "Weighted voronoi", "Use the weighted version of voronoi or not."_help,
                false)
    , iCol_{"iCol", "Segment Index Column", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 0}
    , indexOffset_{"indexOffset",
                   "Index Offset",
                   0,
                   {0, ConstraintBehavior::Ignore},
                   {100, ConstraintBehavior::Ignore}}
    , xCol_{"xCol", "X Coordinate Column", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 1}
    , yCol_{"yCol", "Y Coordinate Column", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 2}
    , zCol_{"zCol", "Z Coordinate Column", dataFrame_, ColumnOptionProperty::AddNoneOption::No, 3}
    , wCol_{"wCol", "Weight Column", dataFrame_, ColumnOptionProperty::AddNoneOption::Yes, 4} {

    addPort(volume_);
    addPort(dataFrame_);
    addPort(outport_);

    addProperties(weighted_, iCol_, indexOffset_, xCol_, yCol_, zCol_, wCol_);
}

namespace {
constexpr auto copyColumn = [](const Column& col, auto& dstContainer, auto assign) {
    col.getBuffer()->getRepresentation<BufferRAM>()->dispatch<void, dispatching::filter::Scalars>(
        [&](auto buf) {
            const auto& data = buf->getDataContainer();
            for (auto&& [src, dst] : util::zip(data, dstContainer)) {
                std::invoke(assign, src, dst);
            }
        });
};
}

void VolumeVoronoiSegmentation::process() {
    auto calc = [dataFrame = dataFrame_.getData(), volume = volume_.getData(), iCol = iCol_.get(),
                 xCol = xCol_.get(), yCol = yCol_.get(), zCol = zCol_.get(), wCol = wCol_.get(),
                 weighted = weighted_.get(), offset = indexOffset_.get()]() {
        if (iCol < 0 || static_cast<size_t>(iCol) >= dataFrame->getNumberOfColumns()) {
            throw Exception("Missing column", IVW_CONTEXT_CUSTOM("VolumeVoronoiSegmentation"));
        }

        const auto nrows = dataFrame->getIndexColumn()->getSize();
        std::vector<std::pair<uint32_t, vec3>> seedPointsWithIndices(nrows);

        copyColumn(
            *dataFrame->getColumn(iCol), seedPointsWithIndices,
            [&](const auto& src, auto& dst) { dst.first = static_cast<uint32_t>(offset + src); });

        const auto setColumnDataAsFloats = [&](int column, int idx) {
            if (column < 0 || static_cast<size_t>(column) >= dataFrame->getNumberOfColumns()) {
                throw Exception("Missing column", IVW_CONTEXT_CUSTOM("VolumeVoronoiSegmentation"));
            }
            copyColumn(
                *dataFrame->getColumn(column), seedPointsWithIndices,
                [idx](const auto& src, auto& dst) { dst.second[idx] = static_cast<float>(src); });
        };
        setColumnDataAsFloats(xCol, 0);
        setColumnDataAsFloats(yCol, 1);
        setColumnDataAsFloats(zCol, 2);

        std::optional<std::vector<float>> radii;
        if (weighted && wCol >= 0 && static_cast<size_t>(wCol) < dataFrame->getNumberOfColumns()) {
            radii = std::vector<float>(nrows);
            copyColumn(*dataFrame->getColumn(wCol), *radii,
                       [](const auto& src, auto& dst) { dst = static_cast<float>(src); });
        }

        const auto voronoiVolume = util::voronoiSegmentation(
            volume->getDimensions(), volume->getCoordinateTransformer().getIndexToDataMatrix(),
            volume->getCoordinateTransformer().getDataToModelMatrix(), seedPointsWithIndices,
            volume->getWrapping(), radii);

        voronoiVolume->setModelMatrix(volume->getModelMatrix());
        voronoiVolume->setWorldMatrix(volume->getWorldMatrix());

        return voronoiVolume;
    };

    outport_.setData(nullptr);
    dispatchOne(calc, [this](std::shared_ptr<Volume> result) {
        outport_.setData(result);
        newResults();
    });
}

}  // namespace inviwo
