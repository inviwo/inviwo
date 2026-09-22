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

#include <modules/basegl/shadercomponents/alternativeshadercomponent.h>

#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/ports/inport.h>

namespace inviwo {

AlternativeShaderComponent::AlternativeShaderComponent(std::string_view aName,
                                                       std::vector<ShaderComponent*> someComponents)

    : ShaderComponent{}, name{aName}, components{std::move(someComponents)}, active{0} {

    if (components.empty()) {
        throw Exception(SourceContext{}, "Has to have at least 1 component");
    }

    auto makeAltCallback = [&, updating = std::make_shared<bool>(false)](BoolCompositeProperty* cp,
                                                                         size_t index) {
        return [updating, cp, index, this]() {
            if (*updating) return;
            util::KeepTrueWhileInScope guard{updating.get()};

            if (cp->isChecked()) {
                active = index;
            } else {
                active = (index + 1) % opts.size();
            }
            for (auto&& [i, p] : std::views::zip(std::views::iota(0uz), opts)) {
                p->setChecked(index == i);
            }

            for (auto&& [i, c] : std::views::zip(std::views::iota(0uz), components)) {
                for (auto&& [p, s] : c->getInports()) {
                    p->setOptional(i != index);
                }
            }
        };
    };

    for (auto&& [i, comp] : std::views::zip(std::views::iota(0uz), components)) {
        auto& cp = opts.emplace_back(std::make_unique<BoolCompositeProperty>(
            util::stripIdentifier(comp->getName()), comp->getName(), i == active));
        for (auto* p : comp->getProperties()) {
            cp->addProperty(p, false);
        }
        cp->getBoolProperty()->setInvalidationLevel(InvalidationLevel::InvalidResources);
        cp->onChange(makeAltCallback(cp.get(), i));
    }
}

std::string_view AlternativeShaderComponent::getName() const { return name; }
void AlternativeShaderComponent::initializeResources(Shader& shader) {
    components[active]->initializeResources(shader);
}
void AlternativeShaderComponent::process(Shader& shader, TextureUnitContainer& container) {
    components[active]->process(shader, container);
}
std::vector<std::tuple<Inport*, std::string>> AlternativeShaderComponent::getInports() {
    std::vector<std::tuple<Inport*, std::string>> inports;
    for (auto& comp : components) {
        inports.append_range(comp->getInports());
    }
    return inports;
}
std::vector<Property*> AlternativeShaderComponent::getProperties() {
    return opts | std::views::transform([](auto& p) { return p.get(); }) |
           std::ranges::to<std::vector<Property*>>();
}
auto AlternativeShaderComponent::getSegments() -> std::vector<Segment> {
    return components[active]->getSegments();
}

ShaderComponent* AlternativeShaderComponent::activeComponent() const {
    return components[active];
}

}  // namespace inviwo
