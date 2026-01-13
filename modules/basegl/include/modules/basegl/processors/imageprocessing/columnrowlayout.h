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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>               // for ImageMultiInport, ImageOutport
#include <inviwo/core/processors/processor.h>          // for Processor
#include <inviwo/core/processors/processorinfo.h>      // for ProcessorInfo
#include <inviwo/core/properties/compositeproperty.h>  // for CompositeProperty
#include <inviwo/core/properties/ordinalproperty.h>    // for IntProperty
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/util/glmvec.h>                         // for ivec2
#include <modules/basegl/datastructures/splittersettings.h>  // for Direction
#include <modules/basegl/properties/splitterproperty.h>      // for SplitterProperty
#include <modules/basegl/rendering/splitterrenderer.h>       // for SplitterRenderer
#include <modules/basegl/viewmanager.h>                      // for ViewManager
#include <modules/opengl/shader/shader.h>                    // for Shader

#include <variant>

namespace inviwo {
class Deserializer;
class Event;

namespace layout {
enum class InputMode : std::uint8_t { Multi, Sequence };

struct IVW_MODULE_BASEGL_API MultiInput {
    explicit MultiInput(std::function<void(bool)> update);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateEvent(ResizeEvent* event, ViewManager& vm);
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    size_t indexOf(Outport* to) const;

private:
    ImageMultiInport inport;
    std::vector<std::shared_ptr<const Image>> data;
};

struct IVW_MODULE_BASEGL_API SequenceInput {
    explicit SequenceInput(std::function<void(bool)> update);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateEvent(ResizeEvent* event, ViewManager& vm);
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    size_t indexOf(Outport* to) const;

private:
    DataInport<DataSequence<Image>> inport;
    std::vector<std::shared_ptr<const Image>> data;
};

struct IVW_MODULE_BASEGL_API Input {
    explicit Input(std::function<void(bool)> update);

    void addPorts(Processor* p);
    void removePorts(Processor* p);
    size_t size() const;
    const std::vector<std::shared_ptr<const Image>>& getData();
    void propagateEvent(ResizeEvent* event, ViewManager& vm);
    void propagateEvent(Event* event, size_t index);
    void propagateEvent(Event* event, Processor* p, Outport* source);
    size_t indexOf(Outport* to) const;

    void setMode(Processor* p, InputMode mode, std::function<void(bool)> update);

private:
    std::variant<MultiInput, SequenceInput> input_;
};

struct IVW_MODULE_BASEGL_API SplitterPositions {
    SplitterPositions(std::string_view identifier, std::string_view displayName,
                      std::function<float()> minSpacing);

    auto splits() const {
        return splitters_ | std::views::take(nSplitters_) | std::views::transform(toFloat) |
               std::views::transform(getter);
    }

    float position(size_t i) const { return i >= size() ? 1.0f : get(i)->get(); }
    FloatProperty* get(size_t i) { return static_cast<FloatProperty*>(splitters_[i]); }
    const FloatProperty* get(size_t i) const {
        return static_cast<const FloatProperty*>(splitters_[i]);
    }
    void set(size_t i, float pos) {
        if (i < size()) get(i)->set(std::clamp(pos, 0.0f, 1.0f));
    }
    size_t size() const { return nSplitters_; }

    void enforceOrder(size_t fixedSliderIndex);
    bool updateSize(size_t newSize);
    void spaceEvenly();
    void deserialized();

    CompositeProperty splitters_;

private:
    size_t nSplitters_;
    std::function<float()> minSpacing_;
    bool isEnforcing_;

    static constexpr auto toFloat = [](const Property* prop) {
        return static_cast<const FloatProperty*>(prop);
    };
    static constexpr auto visible = [](const Property* prop) { return prop->getVisible(); };
    static constexpr auto getter = [](const FloatProperty* prop) { return prop->get(); };
};

}  // namespace layout

class IVW_MODULE_BASEGL_API Layout : public Processor {
public:
    explicit Layout();
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

    layout::Input input_;
    ImageOutport outport_;

    ViewManager viewManager_;

    OptionProperty<layout::InputMode> inputMode_;
    SplitterProperty splitterSettings_;
    IntProperty minWidth_;
    layout::SplitterPositions horizontalSplitters_;
    layout::SplitterPositions verticalSplitters_;
    SplitterRenderer horizontalRenderer_;
    SplitterRenderer verticalRenderer_;
    ButtonProperty splitEvenly_;
    ivec2 currentDim_;
    Shader shader_;
    bool deserialized_;
    std::vector<float> splits_;
};

class IVW_MODULE_BASEGL_API ColumnLayout : public Layout {
public:
    ColumnLayout();
    virtual ~ColumnLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

class IVW_MODULE_BASEGL_API RowLayout : public Layout {
public:
    RowLayout();
    virtual ~RowLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

class IVW_MODULE_BASEGL_API GridLayout : public Layout {
public:
    GridLayout();
    virtual ~GridLayout() = default;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    virtual ivec2 getGrid(size_t inputs) const override;
};

}  // namespace inviwo
