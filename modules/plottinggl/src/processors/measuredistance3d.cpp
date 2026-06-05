/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <modules/plottinggl/processors/measuredistance3d.h>

#include <inviwo/core/network/networklock.h>
#include <inviwo/core/ports/inport.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/util/exception.h>
#include <modules/opengl/texture/textureutils.h>
#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/basegl/datastructures/linedata.h>
#include <modules/plotting/utils/labelscaling.h>

#include <boost/sml.hpp>

namespace sml = boost::sml;

namespace inviwo::plot {

namespace {

struct Invalid {};
struct PlacingFirstLocator {};
struct PlacingSecondLocator {};
struct PlacedFirst {};
struct Valid {};

struct MoveLocator {
    dvec3 pos{0.0};
};
struct PlaceLocator {
    dvec3 pos{0.0};
};
struct PlaceFirst {};
struct FinalizeFirst {};
struct PlaceSecond {};
struct MarkAsValid {};
struct Reset {};

struct Fsm {
    auto operator()() const noexcept {
        using namespace sml;  // NOLINT(google-build-using-namespace)

        const auto invalid = sml::state<Invalid>;
        const auto placingFirst = sml::state<PlacingFirstLocator>;
        const auto placedFirst = sml::state<PlacedFirst>;
        const auto placingSecond = sml::state<PlacingSecondLocator>;
        const auto valid = sml::state<Valid>;

        const auto setFirst = [](const auto& e, MeasureDistance3D& processor) {
            processor.setLocator(0, e.pos);
        };
        const auto setSecond = [](const auto& e, MeasureDistance3D& processor) {
            processor.setLocator(1, e.pos);
        };
        const auto markValid = [](MeasureDistance3D& processor) {
            processor.invalidate(InvalidationLevel::InvalidOutput);
        };
        const auto reset = [](MeasureDistance3D& processor) { processor.reset(); };

        // clang-format off
        return sml::make_transition_table(
           *invalid + event<MoveLocator> / (setFirst) = placingFirst,
            invalid + event<PlaceLocator> / (setFirst) = placedFirst,
            invalid + event<Reset> / (reset) = invalid,
            invalid + event<MarkAsValid> = valid,
            invalid + event<PlaceFirst> = placingFirst,
            invalid + event<FinalizeFirst> = placedFirst,

            placingFirst + event<MoveLocator> / (setFirst) = placingFirst,
            placingFirst + event<PlaceLocator> / (setFirst) = placedFirst,
            placingFirst + event<Reset> / (reset) = invalid,
            placingFirst + event<MarkAsValid> = valid,
            placingFirst + event<PlaceSecond> = placedFirst,
            placingFirst + event<FinalizeFirst> = placedFirst,

            placedFirst + event<MoveLocator> / (setSecond) = placingSecond,
            placedFirst + event<PlaceLocator> / (setSecond, markValid) = valid,
            placedFirst + event<Reset> / (reset) = invalid,
            placedFirst + event<MarkAsValid> = valid,
            placedFirst + event<PlaceFirst> = placingFirst,

            placingSecond + event<MoveLocator> / (setSecond) = placingSecond,
            placingSecond + event<PlaceLocator> / (setSecond, markValid) = valid,
            placingSecond + event<Reset> / (reset) = invalid,
            placingSecond + event<MarkAsValid> = valid,
            placingSecond + event<PlaceFirst> = placingFirst,
            placingSecond + event<FinalizeFirst> = placedFirst,


            valid + event<MoveLocator> / (reset, setFirst) = placingFirst,
            valid + event<PlaceLocator> / (setFirst) = placedFirst,
            valid + event<Reset> / (reset) = invalid,
            valid + event<PlaceFirst> = placingFirst,
            valid + event<PlaceSecond> = placingSecond,
            valid + event<FinalizeFirst> = placedFirst
        );
        // clang-format on
    }
};

}  // namespace

struct MeasurementSM {
    explicit MeasurementSM(MeasureDistance3D* processor) : processor{processor} {}
    MeasureDistance3D* processor;
    sml::sm<Fsm> sm{*processor};
};

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MeasureDistance3D::processorInfo_{
    "org.inviwo.MeasureDistance3D",  // Class identifier
    "Measure Distance 3D",           // Display name
    "Plotting",                      // Category
    CodeState::Experimental,         // Code state
    "GL, Plotting",                  // Tags
    R"(Measures and shows the distance between two points in world space. The coordinates can be
       set via the corresponding properties or placing two locators in the 3D scene.

       One of the Mesh, Layer, or Volume inports can be connected for obtaining the world space
       transformation.
)"_unindentHelp,
};

const ProcessorInfo& MeasureDistance3D::getProcessorInfo() const { return processorInfo_; }

MeasureDistance3D::MeasureDistance3D()
    : Processor{}
    , inport_{"inport", "<description of the inport data and any requirements on the data>"_help}
    , mesh_{"mesh", "Mesh providing the World to Model matrix and axis information"_help}
    , layer_{"layer", "Layer providing the World to Model matrix and axis information"_help}
    , volume_{"volume", "Volume providing World to Model matrix and axis information"_help}
    , outport_{"outport", "<description of the generated outport data>"_help}
    , enabled_{"enabled", "Enabled", true}
    , positions_{{{"startPosition", "Start Position",
                   util::ordinalSymmetricVector(dvec3{0.0})
                       .set("First location of the measurement"_help)},
                  {"endPosition", "End Position",
                   util::ordinalSymmetricVector(dvec3{0.0})
                       .set("Second location of the measurement"_help)}}}
    , distance_{"distance", "Distance",
                util::ordinalLength(0.0)
                    .setInc(0.0001)
                    .set(InvalidationLevel::Valid)
                    .set(ReadOnly::Yes)}
    , clearMeasurement_{"clearMeasurement", "Clear Measurement",
                        [this]() { sm->sm.process_event(Reset{}); }}
    , locatorColor_{"locatorColor", "Locator Color",
                    util::ordinalColor(vec4{0.114f, 0.99f, 0.57f, 1.0f})}
    , locatorSize_{"locatorSize", "Locator Size (pixel)", util::ordinalLength(30)}
    , locatorLineWidth_{"locatorLineWidth", "Locator Line Width (pixel)",
                        util::ordinalLength(1.5f, 50.0f)
                            .set("Width of the locator lines (in pixel)"_help)}
    , renderOnTop_{"renderOnTop", "Render Measurements on Top",
                   "Renders the distance measurement and axis on top by disabling the "
                   "depth test."_help,
                   true}
    , style_{"style", "Axis Style"}
    , measurementAxis_{"measurementAxis_", "Axis"}
    , camera_{"camera", "Camera"}
    , placeLocatorEvent_{"placeLocator",
                         "Place Locator",
                         "Shortcut to perform a measurement"_help,
                         [](Event*) { /* dummy, handled in invokeEvent() */ },
                         MouseButton::Left,
                         MouseStates{flags::any},
                         KeyModifier::Control}
    , cancelMeasurementEvent_{"cancel",
                              "Cancel Measurement",
                              "Shortcut to cancel a measurement"_help,
                              [](Event*) { /* dummy, handled in invokeEvent() */ },
                              IvwKey::Escape,
                              KeyState::Press,
                              KeyModifiers{flags::none}}
    , axisRenderer_{AxisData{}}
    , sm{std::make_unique<MeasurementSM>(this)}
    , locatorMesh_{DrawType::Lines,
                   ConnectivityType::None,
                   {
                       {{-1.0f, 0.0f, 0.0f}},
                       {{1.0f, 0.0f, 0.0f}},
                       {{0.0f, -1.0f, 0.0f}},
                       {{0.0f, 1.0f, 0.0f}},
                       {{0.0f, 0.0f, -1.0f}},
                       {{0.0f, 0.0f, 1.0f}},
                   },
                   {0, 1, 2, 3, 4, 5}} {

    inport_.setOptional(true);
    mesh_.setOptional(true);
    layer_.setOptional(true);
    volume_.setOptional(true);

    isReady_.setUpdate([this]() -> ProcessorStatus {
        if (auto connectedPorts = std::ranges::fold_left(
                std::to_array<const Inport*>({&mesh_, &layer_, &volume_}) |
                    std::views::transform([](auto* p) { return p->isConnected() ? 1 : 0; }),
                0, std::plus<>{});
            connectedPorts > 1) {
            return {ProcessorStatus::Error,
                    "Excactly one of the Mesh, Layer, or Volume inports can be connected"};
        }
        return allInportsAreReady();
    });

    addPorts(inport_, mesh_, layer_, volume_, outport_);

    auto updateDistance = [this]() {
        const auto dist{glm::distance(positions_[0].get(), positions_[1].get())};
        distance_.set(dist);
        measurementAxis_.setRange({0.0, dist});
    };

    addProperty(enabled_);
    for (auto& prop : positions_) {
        addProperty(prop);
        prop.onChange(updateDistance);
    }
    addProperties(distance_, clearMeasurement_, locatorColor_, locatorSize_, locatorLineWidth_,
                  renderOnTop_, style_, measurementAxis_, camera_, placeLocatorEvent_,
                  cancelMeasurementEvent_);

    distance_.setSerializationMode(PropertySerializationMode::None);
    style_.setCollapsed(true);
    style_.registerProperties(measurementAxis_);

    measurementAxis_.setCollapsed(true);
    measurementAxis_.labelingAlgorithm_.set(LabelingAlgorithm::Limits);
    measurementAxis_.minorTicks_.style.set(TickData::Style::None);
    measurementAxis_.labelSettings_.setChecked(false);
    measurementAxis_.setCurrentStateAsDefault();
}

MeasureDistance3D::~MeasureDistance3D() = default;

void MeasureDistance3D::process() {
    if (!enabled_ || sm->sm.is(sml::state<Invalid>)) {
        outport_.setData(inport_.getData());
        return;
    }

    const auto dims{outport_.getDimensions()};

    if (measurementAxis_.isModified()) {
        measurementAxis_.update(axisRenderer_.getData());
    }

    const auto startPosition{positions_[0].get()};
    const auto endPosition{positions_[1].get()};

    // interpret tick lengths and offsets given in pixel and transform them to world space
    const SpatialIdentity spatialEntity{};
    const auto& coordTransform{spatialEntity.getCoordinateTransformer(camera_)};

    const auto center{(startPosition + endPosition) * 0.5};
    const auto centerClip = util::transformPos(coordTransform.getWorldToClipMatrix(), center);
    const auto clipToWorld{coordTransform.getClipToWorldMatrix()};

    auto pixelDistToWorld = [&](double len) {
        return static_cast<float>(glm::distance(
            center,
            util::transformPos(clipToWorld,
                               centerClip + dvec3{0.0, len / static_cast<double>(dims.y), 0.0})));
    };

    auto& axisData = axisRenderer_.getData();
    axisData.major.length = pixelDistToWorld(measurementAxis_.majorTicks_.length);
    axisData.minor.length = pixelDistToWorld(measurementAxis_.minorTicks_.length);
    axisData.captionSettings.offset.x = pixelDistToWorld(measurementAxis_.captionSettings_.offset_);

    // use unit of first data axis to annotate the distance label
    auto [_, dataAxis] = [this]() -> std::pair<dmat4, const Axis*> {
        if (mesh_.hasData()) {
            const auto data = mesh_.getData();
            return {data->getCoordinateTransformer().getWorldToModelMatrix(), data->getAxis(0)};
        } else if (layer_.hasData()) {
            const auto data = layer_.getData();
            return {data->getCoordinateTransformer().getWorldToModelMatrix(), data->getAxis(0)};
        } else if (volume_.hasData()) {
            const auto data = volume_.getData();
            return {data->getCoordinateTransformer().getWorldToModelMatrix(), data->getAxis(0)};
        } else {
            return {dmat4{1.0}, nullptr};
        }
    }();
    if (dataAxis) {
        auto [range, exponent] = scaleRange(dvec2{0.0, distance_.get()}, 3);

        const Axis axis{.name = fmt::format("{:.4g}", range.y), .unit = dataAxis->unit};
        axisData.caption = formatAxisCaption(axis, CaptionType::Custom, LabelScale::Thousands,
                                             "{n}{su: }", exponent, axis.name);
    } else {
        axisData.caption = fmt::format("{:.4g}", distance_.get());
    }

    utilgl::activateTargetAndClearOrCopySource(outport_, inport_, ImageType::ColorDepthPicking);

    auto locatorTransform{glm::scale(dvec3{pixelDistToWorld(locatorSize_)})};

    locatorTransform[3] = dvec4{startPosition, 1.0};
    locatorMesh_.setWorldMatrix(locatorTransform);
    lineRenderer_.render(locatorMesh_, camera_.get(), dims,
                         LineData{.lineWidth = locatorLineWidth_, .defaultColor = locatorColor_});

    if (sm->sm.is(sml::state<Valid>) || sm->sm.is(sml::state<PlacingSecondLocator>)) {
        locatorTransform[3] = dvec4{endPosition, 1.0};
        locatorMesh_.setWorldMatrix(locatorTransform);
        lineRenderer_.render(
            locatorMesh_, camera_.get(), dims,
            LineData{.lineWidth = locatorLineWidth_, .defaultColor = locatorColor_});

        const utilgl::DepthFuncState depthFunc{renderOnTop_ ? gl::GL_ALWAYS : gl::GL_LESS};
        // align tick direction with the up vector of the camera
        const auto axisDir{endPosition - startPosition};
        auto tickDirection{glm::cross(axisDir, glm::cross(axisDir, camera_.getLookUp()))};
        if (glm::dot(camera_.getLookUp(), tickDirection) > 0.0) {
            tickDirection *= -1.0;
        }
        axisRenderer_.render(&camera_.get(), dims, vec3{startPosition}, vec3{endPosition},
                             tickDirection);
    }
}

namespace {

template <typename T, typename... Types>
constexpr size_t index_of() {
    size_t i = 0;
    const bool found = ((++i && std::is_same_v<T, Types>) || ...);
    return i - found;
}

constexpr std::string_view keySMState = "StateMachineState";

}  // namespace

void MeasureDistance3D::serialize(Serializer& s) const {
    Processor::serialize(s);

    using states =
        std::tuple<Invalid, PlacingFirstLocator, PlacedFirst, PlacingSecondLocator, Valid>;
    size_t stateIndex = 0;
    util::for_each_type<states>{}([&]<typename T>() {
        if (sm->sm.is(sml::state<T>)) {
            stateIndex = index_of<T, Invalid, PlacingFirstLocator, PlacedFirst,
                                  PlacingSecondLocator, Valid>();
        }
    });

    s.serialize(keySMState, fmt::format("{}", stateIndex));
}

void MeasureDistance3D::deserialize(Deserializer& d) {
    Processor::deserialize(d);

    std::unique_ptr<size_t> stateIndex;
    d.deserialize(keySMState, stateIndex);
    if (stateIndex) {
        switch (*stateIndex) {
            case 0:  // Invalid
            default:
                sm->sm.process_event(Reset{});
                break;
            case 1:  // PlacingFirstLocator
                sm->sm.process_event(PlaceFirst{});
                break;
            case 2:  // PlacedFirst
                sm->sm.process_event(FinalizeFirst{});
                break;
            case 3:  // PlacingSecondLocator
                sm->sm.process_event(PlaceSecond{});
                break;
            case 4:  // Valid
                sm->sm.process_event(MarkAsValid{});
                break;
        }

    } else {
        sm->sm.process_event(Reset{});
    }
}

void MeasureDistance3D::invokeEvent(Event* event) {
    if ((*cancelMeasurementEvent_.getEventMatcher())(event)) {
        event->markAsUsed();
        sm->sm.process_event(Reset{});
        reset();
    } else if ((*placeLocatorEvent_.getEventMatcher())(event)) {
        if (auto* mouseEvent = event->getAs<MouseEvent>()) {
            auto mousePosNdc{mouseEvent->ndc()};
            if (mouseEvent->depth() >= 1.0) {
                // no depth information/far clip plane, replace with distance to lookTo
                mousePosNdc.z =
                    camera_.getNormalizedDeviceFromNormalizedScreenAtFocusPointDepth({0.0, 0.0}).z;
            }
            const auto mousePos{camera_.getWorldPosFromNormalizedDeviceCoords(mousePosNdc)};

            switch (mouseEvent->state()) {
                case MouseState::Press:
                    break;
                case MouseState::Move:
                    sm->sm.process_event(MoveLocator{.pos = mousePos});
                    break;
                case MouseState::Release:
                    mouseEvent->ndc();
                    sm->sm.process_event(PlaceLocator{.pos = mousePos});
                    break;
                case MouseState::DoubleClick:
                    break;
            }
            event->markAsUsed();
        }
    }

    if (!event->hasBeenUsed()) {
        Processor::invokeEvent(event);
    }
}

void MeasureDistance3D::reset() {
    const NetworkLock lock{this};
    for (auto& prop : positions_) {
        prop.resetToDefaultState();
    }
    distance_.resetToDefaultState();
}

void MeasureDistance3D::setLocator(size_t index, dvec3 position) {
    if (index >= positions_.size()) {
        throw RangeException{SourceContext{}, "index out or range {}", index};
    }
    positions_[index].set(position);
}

}  // namespace inviwo::plot
