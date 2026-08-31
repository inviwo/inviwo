/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2026 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <inviwo/core/interaction/eventtransformer.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processorinfo.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/glmvec.h>
#include <modules/basegl/datastructures/splittersettings.h>
#include <modules/basegl/properties/splitterproperty.h>
#include <modules/basegl/rendering/splitterrenderer.h>
#include <modules/fontrendering/properties/fontproperty.h>
#include <modules/fontrendering/textrenderer.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/rendering/texturequadrenderer.h>

#include <variant>

namespace inviwo {
class Deserializer;
class Event;

namespace layout {
enum class InputMode : std::uint8_t { Multi, Sequence };

enum class Sorting : std::uint8_t {
    ConnectionOrder,
    ProcessorIdentifier,
    ProcessorDisplayName,
    ProcessorXPosition
};

struct IVW_MODULE_BASEGL_API View {
    ivec2 pos;
    ivec2 size;
    bool empty() const { return glm::any(glm::lessThanEqual(size, ivec2(0))); }
};

struct IVW_MODULE_BASEGL_API MultiInput : ProcessorMetaDataObserver {
    explicit MultiInput(std::function<void(bool)> update,
                        std::function<const std::vector<View>&()> views);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateSizes();
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    size_t indexOf(Outport* to) const;
    bool updateLabels(std::vector<std::string>& labels, std::string_view format) const;
    void setSorting(Sorting sortOrder);

private:
    virtual void onProcessorMetaDataPositionChange() override;

    void sortInputChanged();

    void sort();

    std::function<void(bool)> update;
    std::function<const std::vector<View>&()> views;

    Sorting sortOrder;
    ImageMultiInport inport;
    std::vector<Outport*> outports;
    std::vector<std::shared_ptr<const Image>> data;
    std::vector<Processor::NameDispatcherHandle> callbacks;
};

struct IVW_MODULE_BASEGL_API SequenceInput {
    explicit SequenceInput(std::function<void(bool)> update,
                           std::function<const std::vector<View>&()> views);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateSizes();
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    static size_t indexOf(Outport*);
    bool updateLabels(std::vector<std::string>& labels, std::string_view format) const;
    void setSorting(Sorting sortOrder);

private:
    std::function<void(bool)> update;
    std::function<const std::vector<View>&()> views;

    DataInport<DataSequence<Image>> inport;
    std::vector<std::shared_ptr<const Image>> data;
};

struct IVW_MODULE_BASEGL_API Input {
    explicit Input(const std::function<void(bool)>& update,
                   const std::function<const std::vector<View>&()>& views);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateSizes();
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    size_t indexOf(Outport* to) const;
    bool updateLabels(std::vector<std::string>& labels, std::string_view format) const;
    void setSorting(Sorting sortOrder);

    void setMode(Processor* p, InputMode mode, const std::function<void(bool)>& update,
                 const std::function<const std::vector<View>&()>& views);

private:
    std::variant<MultiInput, SequenceInput> input_;
};

struct IVW_MODULE_BASEGL_API SplitterPositions {
private:
    static constexpr auto toFloat = [](const Property* prop) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return static_cast<const DoubleProperty*>(prop);
    };
    static constexpr auto visible = [](const Property* prop) { return prop->getVisible(); };
    static constexpr auto getter = [](const DoubleProperty* prop) { return prop->get(); };

    using SplitsView = decltype(std::declval<const CompositeProperty&>() |
                                std::views::take(std::declval<size_t>()) |
                                std::views::transform(toFloat) | std::views::transform(getter));

public:
    SplitterPositions(std::string_view identifier, std::string_view displayName,
                      std::function<void()> onChange, std::function<double()> minSize);

    SplitsView splits() const {
        return splitters_ | std::views::take(nSplitters_) | std::views::transform(toFloat) |
               std::views::transform(getter);
    }

    double position(size_t i) const { return i >= size() ? 1.0 : get(i)->get(); }
    DoubleProperty* get(size_t i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return static_cast<DoubleProperty*>(splitters_[i]);
    }
    const DoubleProperty* get(size_t i) const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return static_cast<const DoubleProperty*>(splitters_[i]);
    }
    void set(size_t i, double pos) {
        if (i < size()) get(i)->set(std::clamp(pos, 0.0, 1.0));
    }
    size_t size() const { return nSplitters_; }

    void enforceOrder(size_t changedIndex);
    bool updateSize(size_t newSize);
    void spaceEvenly();
    void deserialized();

    CompositeProperty splitters_;

private:
    size_t nSplitters_;
    std::function<void()> onChange_;
    std::function<double()> minSize_;
    bool isEnforcing_;
};

}  // namespace layout

class IVW_MODULE_BASEGL_API Layout : public Processor {
public:
    explicit Layout();
    Layout(const Layout&) = delete;
    Layout(Layout&&) = delete;
    Layout& operator=(const Layout&) = delete;
    Layout& operator=(Layout&&) = delete;

    virtual ~Layout() = default;

    virtual void process() override;
    virtual void propagateEvent(Event* event, Outport* source) override;
    virtual void deserialize(Deserializer& d) override;
    virtual bool isConnectionActive([[maybe_unused]] Inport* from, Outport* to) const override;

protected:
    virtual ivec2 getGrid(size_t inputs) const = 0;

    void updateSplitters(bool connect);
    void calculateViews(ivec2 imgSize);
    void splittersChanged();
    void updateLabelTextures();

    layout::Input input_;
    ImageOutport outport_;

    EventTransformer eventTransformer_;
    std::vector<layout::View> views_;

    OptionProperty<layout::InputMode> inputMode_;
    OptionProperty<layout::Sorting> sorting_;
    SplitterProperty splitterSettings_;
    IntProperty minWidth_;
    layout::SplitterPositions horizontalSplitters_;
    layout::SplitterPositions verticalSplitters_;
    SplitterRenderer horizontalRenderer_;
    SplitterRenderer verticalRenderer_;
    ButtonProperty splitEvenly_;

    BoolCompositeProperty labels_;
    StringProperty format_;
    FontProperty font_;
    FloatVec4Property color_;
    FloatVec2Property position_;
    IntVec2Property offset_;

    ivec2 currentDim_;
    Shader shader_;
    bool deserialized_;
    std::vector<float> splits_;

    TextRenderer textRenderer_;
    std::vector<std::string> textLabels_;
    std::vector<TextTextureObject> textObjects_;
    TextureQuadRenderer textureRenderer_;
};

class IVW_MODULE_BASEGL_API ColumnLayout : public Layout {
public:
    ColumnLayout();
    ColumnLayout(const ColumnLayout&) = delete;
    ColumnLayout(ColumnLayout&&) = delete;
    ColumnLayout& operator=(const ColumnLayout&) = delete;
    ColumnLayout& operator=(ColumnLayout&&) = delete;

    virtual ~ColumnLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

class IVW_MODULE_BASEGL_API RowLayout : public Layout {
public:
    RowLayout();
    RowLayout(const RowLayout&) = delete;
    RowLayout(RowLayout&&) = delete;
    RowLayout& operator=(const RowLayout&) = delete;
    RowLayout& operator=(RowLayout&&) = delete;
    virtual ~RowLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

class IVW_MODULE_BASEGL_API GridLayout : public Layout {
public:
    GridLayout();
    GridLayout(const GridLayout&) = delete;
    GridLayout(GridLayout&&) = delete;
    GridLayout& operator=(const GridLayout&) = delete;
    GridLayout& operator=(GridLayout&&) = delete;
    virtual ~GridLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

}  // namespace inviwo
