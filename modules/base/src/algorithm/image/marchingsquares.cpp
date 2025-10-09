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

#include <modules/base/algorithm/image/marchingsquares.h>

#include <inviwo/core/datastructures/geometry/geometrytype.h>  // for ConnectivityType
#include <inviwo/core/datastructures/geometry/typedmesh.h>     // for BasicMesh, TypedMesh
#include <inviwo/core/datastructures/geometry/mesh.h>          // for Mesh
#include <inviwo/core/datastructures/image/layerram.h>         // IWYU pragma: keep
#include <inviwo/core/datastructures/image/layer.h>            // IWYU pragma: keep
#include <inviwo/core/datastructures/buffer/buffer.h>          // for util::makeBuffer
#include <inviwo/core/datastructures/isovaluecollection.h>     // for IsoValueCollection
#include <inviwo/core/util/formatdispatching.h>                // for dispatch, All
#include <inviwo/core/util/formats.h>                          // for DataFormatBase
#include <inviwo/core/util/glmcomp.h>                          // for glmcomp
#include <inviwo/core/util/glmconvert.h>                       // for glm_convert
#include <inviwo/core/util/glmutils.h>                         // for extent
#include <inviwo/core/util/glmvec.h>                           // for vec3, vec4
#include <inviwo/core/util/indexmapper.h>                      // for IndexMapper, Inde...
#include <inviwo/core/util/interpolation.h>                    // for Interpolation
#include <inviwo/core/util/exception.h>                        // for Exception

#include <span>
#include <ranges>
#include <algorithm>
#include <vector>
#include <stack>
#include <tuple>

namespace inviwo::util {

namespace {

namespace detail {

// Marching Squares cases indicating edges with intersections
// (edge numbers correspond to bottom, right, top, left)
constexpr std::array<std::pair<size_t, std::array<std::pair<int, int>, 2>>, 16> msCases = {{
    {0, {}},                  // case 0
    {1, {{{0, 3}}}},          // case 1
    {1, {{{0, 1}}}},          // case 2
    {1, {{{1, 3}}}},          // case 3
    {1, {{{1, 2}}}},          // case 4
    {2, {{{0, 1}, {2, 3}}}},  // case 5
    {1, {{{0, 2}}}},          // case 6
    {1, {{{2, 3}}}},          // case 7
    {1, {{{2, 3}}}},          // case 8
    {1, {{{0, 2}}}},          // case 9
    {2, {{{1, 2}, {3, 0}}}},  // case 10
    {1, {{{1, 2}}}},          // case 11
    {1, {{{1, 3}}}},          // case 12
    {1, {{{0, 1}}}},          // case 13
    {1, {{{0, 3}}}},          // case 14
    {0, {}},                  // case 15
}};

}  // namespace detail

constexpr std::array<std::span<const std::pair<int, int>>, 16> marchingSquareCases = []() {
    std::array<std::span<const std::pair<int, int>>, detail::msCases.size()> result;
    std::ranges::transform(detail::msCases, result.begin(),
                           [](auto& item) -> std::span<const std::pair<int, int>> {
                               return {item.second.data(), item.first};
                           });
    return result;
}();

enum class PointType : std::uint8_t { Undefined, Start, End, Mid };

struct Intersection {
    size_t line = 0;
    size_t index = 0;
    PointType type = PointType::Undefined;
};

struct Cell {
    double scalar = 0.0;
    std::optional<Intersection> intersect;
};

struct LineUpdate {
    size_t oldIndex = 0;
    size_t newIndex = 0;
    bool reverse = false;
};

using CurrentCell = std::array<std::optional<Intersection>, 4>;
using LineIndices = std::vector<std::vector<std::uint32_t>>;

struct LineCache {
    size_t getNewLineIndex() {
        size_t lineIndex = 0;
        if (unusedLines.empty()) {
            lineIndex = lines.size();
            lines.emplace_back();
        } else {
            lineIndex = unusedLines.top();
            unusedLines.pop();
        }
        return lineIndex;
    }

    void updateColor(vec4 color) {
        for (size_t i = vertexOffset; i < positions.size(); ++i) {
            colors.emplace_back(color);
        }
    }

    void addIndexBuffers(Mesh& mesh) {
        for (auto& indices : lines) {
            if (!indices.empty()) {
                if (indices.back() == std::numeric_limits<std::uint32_t>::max() &&
                    indices.size() > 1) {
                    // line forms a loop
                    indices.back() = indices.front();
                    indices.emplace_back(indices[1]);
                    indices.emplace_back(indices[2]);
                } else {
                    indices.insert(indices.begin(), indices.front());
                    indices.emplace_back(indices.back());
                }
                mesh.addIndices(Mesh::MeshInfo{DrawType::Lines, ConnectivityType::StripAdjacency},
                                util::makeIndexBuffer(std::move(indices)));
            }
        }
    }

    void addBuffers(Mesh& mesh, size2_t dim) {
        if (!positions.empty()) {
            const vec2 scaling{1.0f / static_cast<float>(dim.x), 1.0f / static_cast<float>(dim.y)};
            const vec2 offset{scaling.x * 0.5f, scaling.y * 0.5f};

            std::ranges::transform(positions, positions.begin(),
                                   [&](auto& p) { return p * scaling + offset; });
            mesh.addBuffer(BufferType::PositionAttrib, util::makeBuffer(std::move(positions)));
        }
        if (!colors.empty()) {
            mesh.addBuffer(BufferType::ColorAttrib, util::makeBuffer(std::move(colors)));
        }
    }

    void reset() {
        vertexOffset = positions.size();
        unusedLines = {};
    }

    LineIndices lines;
    std::stack<size_t> unusedLines;
    std::vector<vec2> positions;
    std::vector<vec4> colors;
    size_t vertexOffset = 0;
};

struct RowCache {
    template <typename S>
    RowCache(size_t cols, S samplerFunc) : previous(cols, {}), current(cols, {}) {
        for (size_t x = 0; x < current.size(); ++x) {
            current[x].scalar = samplerFunc(x, 0);
        }
    }

    template <typename S>
    void sampleRow(size_t row, S samplerFunc) {
        current.swap(previous);
        for (size_t x = 0; x < current.size(); ++x) {
            current[x].scalar = samplerFunc(x, row + 1);
            current[x].intersect = {};
        }
    }

    void update(LineUpdate update, size_t col) {
        auto apply = [&update](Intersection& i) {
            if (i.line == update.oldIndex) {
                i.line = update.newIndex;
                if (update.reverse) {
                    if (i.type == PointType::Start) {
                        i.type = PointType::End;
                    } else if (i.type == PointType::End) {
                        i.type = PointType::Start;
                    }
                }
            }
            return i;
        };
        for (auto& elem : current | std::views::take(col)) {
            elem.intersect = elem.intersect.transform(apply);
        }
        for (auto& elem : previous | std::views::drop(col)) {
            elem.intersect = elem.intersect.transform(apply);
        }
    }

    std::vector<Cell> previous;
    std::vector<Cell> current;
};

std::pair<Intersection, Intersection> createLineSegment(vec2 startPos, vec2 endPos,
                                                        std::vector<vec2>& positions,
                                                        size_t lineIndex,
                                                        std::vector<std::uint32_t>& line) {
    const size_t posIndex = positions.size();
    positions.emplace_back(startPos);
    positions.emplace_back(endPos);

    line.emplace_back(static_cast<std::uint32_t>(posIndex));
    line.emplace_back(static_cast<std::uint32_t>(posIndex + 1));

    return {{.line = lineIndex, .index = posIndex, .type = PointType::Start},
            {.line = lineIndex, .index = posIndex + 1, .type = PointType::End}};
}

Intersection continueLine(Intersection& intersection1, vec2 pos, std::vector<vec2>& positions,
                          size_t lineIndex, std::vector<std::uint32_t>& line) {
    const size_t posIndex = positions.size();
    positions.emplace_back(pos);

    if (intersection1.type == PointType::Start) {
        line.insert(line.begin(), static_cast<std::uint32_t>(posIndex));
    } else {
        line.emplace_back(static_cast<std::uint32_t>(posIndex));
    }

    Intersection intersection2 = {.line = lineIndex, .index = posIndex, .type = intersection1.type};
    intersection1.type = PointType::Mid;
    return intersection2;
}

void closeLineLoop(Intersection& intersection1, Intersection& intersection2,
                   std::vector<std::uint32_t>& line) {
    ivwAssert((intersection1.type == PointType::Start && intersection2.type == PointType::End) ||
                  (intersection1.type == PointType::End && intersection2.type == PointType::Start),
              "Incorrect line end types");

    line.emplace_back(std::numeric_limits<std::uint32_t>::max());
    intersection1.type = PointType::Mid;
    intersection2.type = PointType::Mid;
}

LineUpdate mergeLines(CurrentCell& intersects, size_t edge1, size_t edge2,
                      std::vector<std::uint32_t>& line1, std::vector<std::uint32_t>& line2) {
    const bool reverse = intersects[edge1]->type == intersects[edge2]->type;

    if (intersects[edge1]->type == PointType::Start &&
        intersects[edge2]->type == PointType::Start) {
        // prepend reversed line of edge2
        line1.insert_range(line1.begin(), line2 | std::views::reverse);
    } else if (intersects[edge1]->type == PointType::End &&
               intersects[edge2]->type == PointType::End) {
        // append reversed line of edge 2
        line1.append_range(line2 | std::views::reverse);
    } else {
        // different start and end, append start to end
        if (intersects[edge1]->type == PointType::Start) {
            std::swap(edge1, edge2);
            line2.append_range(line1);
        } else {
            line1.append_range(line2);
        }
    }

    const size_t newLineIndex = intersects[edge1]->line;
    const size_t outdatedLine = intersects[edge2]->line;

    intersects[edge2]->line = newLineIndex;
    intersects[edge1]->type = PointType::Mid;
    intersects[edge2]->type = PointType::Mid;

    return LineUpdate{.oldIndex = outdatedLine, .newIndex = newLineIndex, .reverse = reverse};
}

size_t getNewLineIndex(LineIndices& lines, std::stack<size_t>& unusedLineIndices) {
    size_t lineIndex = 0;
    if (unusedLineIndices.empty()) {
        lineIndex = lines.size();
        lines.emplace_back();
    } else {
        lineIndex = unusedLineIndices.top();
        unusedLineIndices.pop();
    }
    return lineIndex;
}

/**
 * generate isolines in the current cell based on the Marching Square case \p theCase
 *
 * @return pair of potential line intersections on the right and top edge
 */
template <typename F, typename U>
std::pair<std::optional<Intersection>, std::optional<Intersection>> createIsoLines(
    int theCase, const std::optional<Intersection>& bottom, const std::optional<Intersection>& left,
    LineCache& cache, F calcPositionFunc, U updateCacheFunc, size_t row) {

    CurrentCell intersects{
        bottom,
        std::nullopt,
        std::nullopt,
        left,
    };

    for (auto [edge1, edge2] : marchingSquareCases[theCase]) {
        if (intersects[edge1] && intersects[edge2]) {
            // connect lines, merge if line index is different
            if (intersects[edge1]->line == intersects[edge2]->line) {
                const size_t lineIndex = intersects[edge1]->line;
                closeLineLoop(*intersects[edge1], *intersects[edge2], cache.lines[lineIndex]);
            } else {
                auto update =
                    mergeLines(intersects, edge1, edge2, cache.lines[intersects[edge1]->line],
                               cache.lines[intersects[edge2]->line]);
                cache.lines[update.oldIndex].clear();
                cache.unusedLines.push(update.oldIndex);
                updateCacheFunc(update, row);
            }
        } else if (intersects[edge1]) {
            // continue line
            const size_t lineIndex = intersects[edge1]->line;
            intersects[edge2] = continueLine(*intersects[edge1], calcPositionFunc(edge2),
                                             cache.positions, lineIndex, cache.lines[lineIndex]);
        } else if (intersects[edge2]) {
            // continue line
            const size_t lineIndex = intersects[edge2]->line;
            intersects[edge1] = continueLine(*intersects[edge2], calcPositionFunc(edge1),
                                             cache.positions, lineIndex, cache.lines[lineIndex]);
        } else {
            // create new line segment
            const size_t lineIndex = cache.getNewLineIndex();
            std::tie(intersects[edge1], intersects[edge2]) =
                createLineSegment(calcPositionFunc(edge1), calcPositionFunc(edge2), cache.positions,
                                  lineIndex, cache.lines[lineIndex]);
        }
    }
    return {intersects[1], intersects[2]};
}

template <typename T>
auto getSampleTransform(TFPrimitiveSetType type) -> double (*)(const DataMapper&, T, size_t) {
    if (type == TFPrimitiveSetType::Relative) {
        return [](const DataMapper& dm, T v, size_t ch) {
            return dm.mapFromDataToNormalized(util::glm_convert<double>(util::glmcomp(v, ch)));
        };
    } else {
        return [](const DataMapper& dm, T v, size_t ch) {
            return dm.mapFromDataToValue(util::glm_convert<double>(util::glmcomp(v, ch)));
        };
    }
}

constexpr auto dispatcher = []<typename T>(const LayerRAM* in, const IsoValueCollection& isoValues,
                                           size_t channel) {
    const auto* ram = static_cast<const LayerRAMPrecision<T>*>(in);
    const auto data = ram->getView();
    const auto dim = ram->getDimensions();

    auto sample = [data, dataMap = in->getOwner()->dataMap, channel, im = util::IndexMapper2D(dim),
                   sampleTrafo = getSampleTransform<T>(isoValues.getType())](size_t x, size_t y) {
        return sampleTrafo(dataMap, data[im(x, y)], channel);
    };

    LineCache lineCache;
    auto mesh = std::make_shared<Mesh>();

    for (const auto&& [isoValue, isoColor] :
         isoValues | std::views::transform([](auto& p) { return p.getData(); })) {

        RowCache rowCache{dim.x, sample};

        for (size_t y = 0; y < dim.y - 1; ++y) {
            std::optional<Intersection> prevCell;
            rowCache.sampleRow(y, sample);

            for (size_t x = 0; x < dim.x - 1; ++x) {
                std::array<double, 5> values = {
                    rowCache.previous[x].scalar,    rowCache.previous[x + 1].scalar,
                    rowCache.current[x + 1].scalar, rowCache.current[x].scalar,
                    rowCache.previous[x].scalar,
                };

                int theCase = 0;
                theCase += values[0] < isoValue ? 0 : 1;
                theCase += values[1] < isoValue ? 0 : 2;
                theCase += values[2] < isoValue ? 0 : 4;
                theCase += values[3] < isoValue ? 0 : 8;

                if (theCase == 0 || theCase == 15) {
                    continue;
                }

                std::array<vec2, 5> outPos{vec2{x, y}, vec2{x + 1, y}, vec2{x + 1, y + 1},
                                           vec2{x, y + 1}, vec2{x, y}};

                auto calcPosition = [&](int edge) {
                    const auto t = (isoValue - values[edge]) / (values[edge + 1] - values[edge]);
                    return Interpolation<vec2, float>::linear(outPos[edge], outPos[edge + 1],
                                                              static_cast<float>(t));
                };

                std::tie(prevCell, rowCache.current[x].intersect) = createIsoLines(
                    theCase, rowCache.previous[x].intersect, prevCell, lineCache, calcPosition,
                    [&rowCache](LineUpdate update, size_t col) { rowCache.update(update, col); },
                    x);
            }
        }

        lineCache.addIndexBuffers(*mesh);
        lineCache.updateColor(isoColor);
        lineCache.reset();
    }

    lineCache.addBuffers(*mesh, dim);
    mesh->setModelMatrix(in->getOwner()->getModelMatrix());
    return mesh;
};

}  // namespace

std::shared_ptr<Mesh> marchingSquares(const LayerRAM* layer, const IsoValueCollection& isoValues,
                                      size_t channel) {
    if (glm::compMul(layer->getDimensions()) == 0) {
        return std::make_shared<Mesh>();
    }
    if (isoValues.empty()) {
        return std::make_shared<Mesh>();
    }
    if (channel >= layer->getDataFormat()->getComponents()) {
        throw Exception(SourceContext{}, "Invalid channel {}, layer only has {} channels.",
                        channel + 1, layer->getDataFormat()->getComponents());
    }

    return dispatching::singleDispatch<std::shared_ptr<Mesh>, dispatching::filter::All>(
        layer->getDataFormatId(), dispatcher, layer, isoValues, channel);
}

}  // namespace inviwo::util
