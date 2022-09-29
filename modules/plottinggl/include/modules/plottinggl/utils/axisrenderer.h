/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmoduledefine.h>          // for IVW_MODULE_PLOTTINGGL_API

#include <inviwo/core/datastructures/geometry/mesh.h>           // for Mesh
#include <inviwo/core/util/glmvec.h>                            // for vec3, ivec2, size2_t, dvec2
#include <modules/basegl/datastructures/meshshadercache.h>      // for MeshShaderCache
#include <modules/fontrendering/textrenderer.h>                 // for TextTextureObject, TextRe...
#include <modules/fontrendering/util/textureatlas.h>            // for TextureAtlas
#include <modules/opengl/rendering/texturequadrenderer.h>       // for TextureQuadRenderer
#include <modules/plotting/datastructures/axissettings.h>       // for AxisSettings
#include <modules/plotting/datastructures/majortickdata.h>      // for MajorTickData
#include <modules/plotting/datastructures/majorticksettings.h>  // for operator!=
#include <modules/plotting/datastructures/minortickdata.h>      // for MinorTickData
#include <modules/plotting/datastructures/plottextdata.h>       // for PlotTextData
#include <modules/plotting/datastructures/plottextsettings.h>   // for operator!=, PlotTextSetti...

#include <cstddef>                                              // for size_t
#include <functional>                                           // for function, reference_wrapper
#include <limits>                                               // for numeric_limits
#include <memory>                                               // for shared_ptr, unique_ptr
#include <string>                                               // for string, operator==, opera...
#include <type_traits>                                          // for is_nothrow_move_assignable_v
#include <utility>                                              // for pair
#include <vector>                                               // for operator!=, vector

#include <glm/vec2.hpp>                                         // for operator!=, vec
#include <glm/vec3.hpp>                                         // for operator!=, vec
#include <glm/vec4.hpp>                                         // for vec

namespace inviwo {

class Camera;
class ShaderType;

namespace plot {

namespace detail {

/*
 * Utilities for caching the state of the axis renderer
 * Each state is wrapped in a Guard. When the rendering is called Guard::check is called with the
 * new state. If the state is changed the guard will call reset on all its guarded objects, forcing
 * them to be updated to the new state.
 * For example the AxisCaption caches a TextTextureObject by using two guards, one for the string:
 * caption_, and one for the text settings: settings_. When ever AxisCaption::getCaption is called
 * the guards are checked, and if they are changed the TextTextureObject is reset and the texture is
 * recreated.
 */

struct Resetter {

    void operator()(TextTextureObject& text) { text.texture.reset(); }
    void operator()(util::TextureAtlas& atlas) { atlas.clear(); }
    void operator()(bool& valid) { valid = false; }
    template <typename T>
    void operator()(T& arg) {
        arg.reset();
    }
    template <typename T>
    void operator()(std::vector<T>& arg) {
        arg.clear();
    }
};

template <typename Cls, typename MP, typename... MPs>
struct GuardHelper {
    static void reset(Cls& obj) {
        Resetter{}(MP::get(obj));
        GuardHelper<Cls, MPs...>::reset(obj);
    }
};
template <typename Cls, typename MP>
struct GuardHelper<Cls, MP> {
    static void reset(Cls& obj) { Resetter{}(MP::get(obj)); }
};

template <typename C, typename T, T C::*memptr>
struct MemPtr {
    using Cls = C;
    static T& get(C& obj) { return obj.*memptr; }
};

template <typename T, typename MP, typename... MPs>
struct Guard {
    using Cls = typename MP::Cls&;
    Guard(T val = T{}) : value_{val} {}
    void check(Cls& obj, const T& value) {
        if (value_ != value) {
            GuardHelper<Cls, MP, MPs...>::reset(obj);
            value_ = value;
        }
    }

    operator const T&() const { return value_; }
    const T& get() const { return value_; }

private:
    T value_ = T{};
};

struct IVW_MODULE_PLOTTINGGL_API AxisMeshes {
    AxisMeshes();

    Mesh* getAxis(const AxisSettings& settings, const vec3& start, const vec3& end, size_t pickId);
    Mesh* getMajor(const AxisSettings& settings, const vec3& start, const vec3& end,
                   const vec3& tickDirection);
    Mesh* getMinor(const AxisSettings& settings, const vec3& start, const vec3& end,
                   const vec3& tickDirection);

private:
    std::unique_ptr<Mesh> axisMesh_;
    std::unique_ptr<Mesh> majorMesh_;
    std::unique_ptr<Mesh> minorMesh_;

    using MPAxis = MemPtr<AxisMeshes, std::unique_ptr<Mesh>, &AxisMeshes::axisMesh_>;
    using MPMajor = MemPtr<AxisMeshes, std::unique_ptr<Mesh>, &AxisMeshes::majorMesh_>;
    using MPMinor = MemPtr<AxisMeshes, std::unique_ptr<Mesh>, &AxisMeshes::minorMesh_>;

    Guard<vec3, MPAxis, MPMajor, MPMinor> startPos_;
    Guard<vec3, MPAxis, MPMajor, MPMinor> endPos_;
    Guard<vec4, MPAxis> color_;
    Guard<size_t, MPAxis> pickId_;
    Guard<dvec2, MPMajor, MPMinor> range_;
    Guard<bool, MPMajor, MPMinor> flip_;
    Guard<MajorTickData, MPMajor, MPMinor> major_;
    Guard<MinorTickData, MPMinor> minor_;
    Guard<vec3, MPMajor, MPMinor> tickDirection_;
};

template <typename P>
struct AxisLabels {
    using LabelPos = std::vector<P>;
    using Updater = std::function<void(LabelPos&, util::TextureAtlas&, const AxisSettings&,
                                       const vec3&, const vec3&, const vec3&)>;

    AxisLabels(Updater updatePos) : updatePos_{updatePos} {}
    AxisLabels(const AxisLabels&) = default;
    AxisLabels(AxisLabels&&) noexcept = default;
    AxisLabels& operator=(const AxisLabels&) = default;
    AxisLabels& operator=(AxisLabels&&) noexcept = default;

    util::TextureAtlas& getAtlas(const AxisSettings& settings, const vec3& start, const vec3& end,
                                 TextRenderer& renderer) {
        startPos_.check(*this, start);
        endPos_.check(*this, end);
        range_.check(*this, settings.getRange());
        labelsSettings_.check(*this, settings.getLabelSettings());
        major_.check(*this, settings.getMajorTicks());
        labels_.check(*this, settings.getLabels());

        if (!validAtlas_) {
            renderer.configure(labelsSettings_.get().getFont());
            atlas_.fillAtlas(renderer, labels_, labelsSettings_.get().getColor());
            validAtlas_ = true;
        }
        return atlas_;
    }

    const util::TextureAtlas& getCurrentAtlas() const { return atlas_; }

    const LabelPos& getLabelPos(const AxisSettings& settings, const vec3& start, const vec3& end,
                                TextRenderer& renderer, const vec3& tickDirection) {

        startPos_.check(*this, start);
        endPos_.check(*this, end);
        range_.check(*this, settings.getRange());
        labelsSettings_.check(*this, settings.getLabelSettings());
        major_.check(*this, settings.getMajorTicks());
        tickDirection_.check(*this, tickDirection);
        flipped_.check(*this, settings.getFlipped());

        if (positions_.empty()) {
            auto& atlas = getAtlas(settings, start, end, renderer);
            updatePos_(positions_, atlas, settings, start, end, tickDirection);
        }
        return positions_;
    }

protected:
    Updater updatePos_;
    util::TextureAtlas atlas_;
    bool validAtlas_ = false;
    LabelPos positions_;

    using MPAtlas = MemPtr<AxisLabels, bool, &AxisLabels::validAtlas_>;
    using MPLabel = MemPtr<AxisLabels, LabelPos, &AxisLabels::positions_>;

    Guard<vec3, MPAtlas, MPLabel> startPos_;
    Guard<vec3, MPAtlas, MPLabel> endPos_;
    Guard<dvec2, MPAtlas, MPLabel> range_;
    Guard<PlotTextData, MPAtlas, MPLabel> labelsSettings_;
    Guard<std::vector<std::string>, MPAtlas> labels_;
    Guard<MajorTickData, MPAtlas, MPLabel> major_;
    Guard<vec3, MPLabel> tickDirection_;
    Guard<bool, MPLabel> flipped_;
};

struct IVW_MODULE_PLOTTINGGL_API AxisCaption {
    AxisCaption() = default;
    TextTextureObject& getCaption(const std::string& caption, const PlotTextSettings& settings,
                                  TextRenderer& renderer) {
        caption_.check(*this, caption);
        settings_.check(*this, settings);
        if (!axisCaption_.texture) {
            renderer.configure(settings_.get().getFont());
            axisCaption_ =
                util::createTextTextureObject(renderer, caption_, settings_.get().getColor());
        }
        return axisCaption_;
    }

private:
    TextTextureObject axisCaption_;
    using MPCap = MemPtr<AxisCaption, TextTextureObject, &AxisCaption::axisCaption_>;
    Guard<std::string, MPCap> caption_;
    Guard<PlotTextData, MPCap> settings_;

    static_assert(std::is_nothrow_move_assignable_v<Guard<std::string, MPCap>>);
    static_assert(std::is_nothrow_move_assignable_v<Guard<PlotTextData, MPCap>>);
};

}  // namespace detail

/*
 *\brief Renders AxisProperty and CategoricalAxisProperty
 */
class IVW_MODULE_PLOTTINGGL_API AxisRendererBase {
public:
    AxisRendererBase(const AxisSettings& settings);
    AxisRendererBase(const AxisRendererBase& rhs);
    AxisRendererBase(AxisRendererBase&& rhs) noexcept = default;
    AxisRendererBase& operator=(const AxisRendererBase& rhs) = delete;
    AxisRendererBase& operator=(AxisRendererBase&& rhs) noexcept = default;
    virtual ~AxisRendererBase() = default;

    void setAxisPickingId(size_t id) { axisPickingId_ = id; }
    size_t getAxisPickingId() const { return axisPickingId_; }

protected:
    void renderAxis(Camera* camera, const vec3& start, const vec3& end, const vec3& tickdir,
                    const size2_t& outputDims, bool antialiasing);

    std::reference_wrapper<const AxisSettings> settings_;

    TextRenderer textRenderer_;
    TextureQuadRenderer quadRenderer_;

    size_t axisPickingId_ = std::numeric_limits<size_t>::max();  // max == unused

    detail::AxisMeshes meshes_;
    detail::AxisCaption caption_;

    static std::shared_ptr<MeshShaderCache> getShaders();
    std::shared_ptr<MeshShaderCache> shaders_;

private:
    static std::vector<std::pair<ShaderType, std::string>> shaderItems_;
    static std::vector<MeshShaderCache::Requirement> shaderRequirements_;
};

class IVW_MODULE_PLOTTINGGL_API AxisRenderer : public AxisRendererBase {
public:
    using Labels = detail::AxisLabels<ivec2>;
    AxisRenderer(const AxisSettings& settings);
    AxisRenderer(const AxisRenderer& rhs) = default;
    AxisRenderer(AxisRenderer&& rhs) noexcept = default;
    AxisRenderer& operator=(const AxisRenderer& rhs) = delete;
    AxisRenderer& operator=(AxisRenderer&& rhs) noexcept = default;
    virtual ~AxisRenderer() = default;

    void render(const size2_t& outputDims, const ivec2& startPos, const ivec2& endPos,
                bool antialiasing = true);

    /**
     * Returns the bounding rect (lower left, upper right) of the axis in pixels.
     */

    std::pair<vec2, vec2> boundingRect(const ivec2& startPos, const ivec2& endPos);

private:
    void renderText(const size2_t& outputDims, const ivec2& startPos, const ivec2& endPos);

    Labels labels_;
};

class IVW_MODULE_PLOTTINGGL_API AxisRenderer3D : public AxisRendererBase {
public:
    using Labels = detail::AxisLabels<vec3>;
    AxisRenderer3D(const AxisSettings& property);
    AxisRenderer3D(const AxisRenderer3D& rhs) = default;
    AxisRenderer3D(AxisRenderer3D&& rhs) noexcept = default;
    AxisRenderer3D& operator=(const AxisRenderer3D& rhs) = delete;
    AxisRenderer3D& operator=(AxisRenderer3D&& rhs) noexcept = default;
    virtual ~AxisRenderer3D() = default;

    void render(Camera* camera, const size2_t& outputDims, const vec3& startPos, const vec3& endPos,
                const vec3& tickDirection, bool antialiasing = true);

private:
    void renderText(Camera* camera, const size2_t& outputDims, const vec3& startPos,
                    const vec3& endPos, const vec3& tickDirection);

    Labels labels_;
};

}  // namespace plot

}  // namespace inviwo
