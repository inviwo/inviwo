/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/basegl/shadercomponents/timecomponent.h>

#include <inviwo/core/properties/boolcompositeproperty.h>     // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>              // for BoolProperty
#include <inviwo/core/properties/constraintbehavior.h>        // for ConstraintBehavior, Constra...
#include <inviwo/core/properties/invalidationlevel.h>         // for InvalidationLevel, Invalida...
#include <inviwo/core/properties/ordinalproperty.h>           // for IntProperty
#include <inviwo/core/properties/propertysemantics.h>         // for PropertySemantics, Property...
#include <inviwo/core/util/timer.h>                           // for Timer
#include <modules/basegl/shadercomponents/shadercomponent.h>  // for ShaderComponent::Segment
#include <modules/opengl/shader/shader.h>                     // for Shader

#include <chrono>                                             // for duration, duration_cast
#include <functional>                                         // for __base, function
#include <ratio>                                              // for milli
#include <type_traits>                                        // for remove_reference, remove_re...
#include <utility>                                            // for move

#include <fmt/core.h>                                         // for format
#include <fmt/format.h>                                       // for compile_string_to_view, FMT...

namespace inviwo {
class Property;
class TextureUnitContainer;

TimeComponent::TimeComponent(std::string_view name,
                             std::function<void(InvalidationLevel)> invalidate)
    : ShaderComponent{}
    , name_{name}
    , enabled_{"enabled", "Enabled", true, InvalidationLevel::Valid}
    , running_{"running", "Running", false, InvalidationLevel::Valid}
    , intervalMs_{"interval",
                  "Interval (ms)",
                  33,
                  {0, ConstraintBehavior::Immutable},
                  {1000, ConstraintBehavior::Mutable},
                  1,
                  InvalidationLevel::Valid,
                  PropertySemantics::Text}
    , timer_{std::chrono::milliseconds{intervalMs_.get()}, [invalidate = std::move(invalidate)]() {
                 invalidate(InvalidationLevel::InvalidOutput);
             }} {

    running_.readonlyDependsOn(*enabled_.getBoolProperty(), [](const auto& p) { return !p.get(); });
    running_.onChange([this]() {
        if (running_ && enabled_.isChecked()) {
            if (!timer_.isRunning()) {
                timer_.start();
            }
        } else {
            timer_.stop();
        }
    });
    enabled_.getBoolProperty()->onChange([this]() {
        if (!enabled_.isChecked()) timer_.stop();
    });
    intervalMs_.onChange(
        [this]() { timer_.setInterval(std::chrono::milliseconds{intervalMs_.get()}); });

    enabled_.addProperties(running_, intervalMs_);
    enabled_.setCollapsed(true);
    enabled_.setCurrentStateAsDefault();
}

std::string_view TimeComponent::getName() const { return name_; }

void TimeComponent::process(Shader& shader, TextureUnitContainer&) {
    shader.setUniform(name_, std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(
                                 std::chrono::steady_clock::now().time_since_epoch())
                                 .count());
}

auto TimeComponent::getSegments() -> std::vector<Segment> {
    return {
        Segment{fmt::format(FMT_STRING("uniform float {};"), name_), placeholder::uniform, 600}};
}

std::vector<Property*> TimeComponent::getProperties() { return {&enabled_}; }

void TimeComponent::start() { running_.set(true); }

void TimeComponent::stop() { running_.set(false); }

void TimeComponent::setRunning(bool run) { running_.set(run); }

bool TimeComponent::getRunning() const { return timer_.isRunning(); }

}  // namespace inviwo
