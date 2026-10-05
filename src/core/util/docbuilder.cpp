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

#include <inviwo/core/util/docbuilder.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/processors/processorutils.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/util/buildinfo.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/utfutils.h>

#include <fmt/format.h>

#include <ranges>

namespace inviwo {

namespace help {

void HelpInport::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("typeName", typeName, SerializationTarget::Attribute);
    s.serialize("color", color::rgb2hex(colorCode), SerializationTarget::Attribute);
    s.serialize("dataCid", data.cid, SerializationTarget::Attribute);
    s.serialize("dataName", data.name, SerializationTarget::Attribute);
    s.serialize("dataColor", color::rgb2hex(data.color), SerializationTarget::Attribute);
    s.serialize("help", help);
}
void HelpInport::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("typeName", typeName, SerializationTarget::Attribute);
    colorCode = d.attribute("color")
                    .transform([](std::string_view s) { return color::hex2urgb(s); })
                    .value_or(uvec3{0});
    d.deserialize("dataCid", data.cid, SerializationTarget::Attribute);
    d.deserialize("dataName", data.name, SerializationTarget::Attribute);
    data.color = d.attribute("dataColor")
                     .transform([](std::string_view s) { return color::hex2urgb(s); })
                     .value_or(uvec3{0});
    d.deserialize("help", help);
}

void HelpOutport::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("typeName", typeName, SerializationTarget::Attribute);
    s.serialize("color", color::rgb2hex(colorCode), SerializationTarget::Attribute);
    s.serialize("dataCid", data.cid, SerializationTarget::Attribute);
    s.serialize("dataName", data.name, SerializationTarget::Attribute);
    s.serialize("dataColor", color::rgb2hex(data.color), SerializationTarget::Attribute);
    s.serialize("help", help);
}
void HelpOutport::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("typeName", typeName, SerializationTarget::Attribute);
    colorCode = d.attribute("color")
                    .transform([](std::string_view s) { return uvec3{color::hex2urgba(s)}; })
                    .value_or(uvec3{0});
    d.deserialize("dataCid", data.cid, SerializationTarget::Attribute);
    d.deserialize("dataName", data.name, SerializationTarget::Attribute);
    data.color = d.attribute("dataColor")
                     .transform([](std::string_view s) { return uvec3{color::hex2urgba(s)}; })
                     .value_or(uvec3{0});
    d.deserialize("help", help);
}

void HelpProperty::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("typeName", typeName, SerializationTarget::Attribute);
    s.serialize("help", help);
    s.serialize("properties", properties);
}
void HelpProperty::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("typeName", typeName, SerializationTarget::Attribute);
    d.deserialize("help", help);
    d.deserialize("properties", properties);
}

void HelpProcessor::serialize(Serializer& s) const {
    s.serialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    s.serialize("displayName", displayName, SerializationTarget::Attribute);
    s.serialize("typeName", typeName, SerializationTarget::Attribute);
    s.serialize("category", category, SerializationTarget::Attribute);
    s.serialize("codeState", codeState, SerializationTarget::Attribute);
    s.serialize("help", help);
    s.serialize("sourceFile", sourceFile);
    s.serialize("headerFile", headerFile);
    s.serialize("sourceLink", sourceLink);
    s.serialize("headerLink", headerLink);
    s.serialize("inports", inports);
    s.serialize("outports", outports);
    s.serialize("properties", properties);
    s.serialize("inviwoModule", inviwoModule);
    s.serialize("meta", meta);
}
void HelpProcessor::deserialize(Deserializer& d) {
    d.deserialize("classIdentifier", classIdentifier, SerializationTarget::Attribute);
    d.deserialize("displayName", displayName, SerializationTarget::Attribute);
    d.deserialize("typeName", typeName, SerializationTarget::Attribute);
    d.deserialize("category", category, SerializationTarget::Attribute);
    d.deserialize("codeState", codeState, SerializationTarget::Attribute);
    d.deserialize("help", help);
    d.deserialize("sourceFile", sourceFile);
    d.deserialize("headerFile", headerFile);
    d.deserialize("sourceLink", sourceLink);
    d.deserialize("headerLink", headerLink);
    d.deserialize("inports", inports);
    d.deserialize("outports", outports);
    d.deserialize("properties", properties);
    d.deserialize("inviwoModule", inviwoModule);
    d.deserialize("meta", meta);
}

}  // namespace help

namespace {

std::string getTypeName(std::string_view id, auto* factory) {
    if (auto* fo = factory->getFactoryObject(id)) {
        return fo->getTypeName();
    } else {
        return "";
    }
}

std::string getTypeName(const Inport& inport, const InviwoApplication& app) {
    return getTypeName(inport.getClassIdentifier(), app.getInportFactory());
}
std::string getTypeName(const Outport& outport, const InviwoApplication& app) {
    return getTypeName(outport.getClassIdentifier(), app.getOutportFactory());
}
std::string getTypeName(const Property& property, const InviwoApplication& app) {
    return getTypeName(property.getClassIdentifier(), app.getPropertyFactory());
}
std::string getTypeName(const Processor& processor, const InviwoApplication& app) {
    return getTypeName(processor.getClassIdentifier(), app.getProcessorFactory());
}

auto headerCandidates(const std::filesystem::path& relSrc, const std::filesystem::path& base,
                      std::string_view moduleId) -> std::vector<std::filesystem::path> {
    if (*relSrc.begin() == "src") {
        std::filesystem::path header{};
        for (auto&& part : std::ranges::subrange(++relSrc.begin(), relSrc.end())) {
            header /= part;
        }
        header.replace_extension(".h");

        return {base / "include" / "modules" / toLower(moduleId) / header,
                base / "include" / "inviwo" / toLower(moduleId) / header};
    }
    return {};
}

std::optional<util::BuildInfo::ModulesDir> getModuleDir(const std::filesystem::path& path) {
    const auto& bi = util::getBuildInfo();
    if (!bi) return std::nullopt;
    for (const auto& modulesDir : bi->modulesDirs) {
        if (std::ranges::starts_with(path.native() | views::codePoints,
                                     modulesDir.repoDir.native() | views::codePoints)) {
            return modulesDir;
        }
    }
    return std::nullopt;
}

std::optional<std::string> getRepoLink(const std::filesystem::path& path) {
    return getModuleDir(path).and_then([&](const auto& modulesDir) -> std::optional<std::string> {
        if (!modulesDir.repo.contains("github.com")) return std::nullopt;

        auto file = path.generic_string();
        file.erase(0, modulesDir.repoDir.generic_string().size());
        const auto repoLink = modulesDir.repo.ends_with(".git")
                                  ? modulesDir.repo.substr(0, modulesDir.repo.size() - 4)
                                  : modulesDir.repo;
        return fmt::format("{}/blob/{}{}", repoLink, modulesDir.sha, file);
    });
}

}  // namespace

help::HelpProcessor help::buildProcessorHelp(Processor& processor, InviwoApplication& app) {
    std::vector<HelpInport> inports;
    for (auto* inport : processor.getInports()) {
        inports.push_back(HelpInport{
            .classIdentifier = std::string{inport->getClassIdentifier()},
            .displayName = inport->getIdentifier(),
            .typeName = getTypeName(*inport, app),
            .colorCode = inport->getColorCode(),
            .data = inport->getDataInfo(),
            .help = inport->getHelp(),
        });
    }

    std::vector<HelpOutport> outports;
    for (auto* outport : processor.getOutports()) {
        outports.push_back(HelpOutport{
            .classIdentifier = std::string{outport->getClassIdentifier()},
            .displayName = outport->getIdentifier(),
            .typeName = getTypeName(*outport, app),
            .colorCode = outport->getColorCode(),
            .data = outport->getDataInfo(),
            .help = outport->getHelp(),
        });
    }

    std::vector<HelpProperty> properties;
    std::vector<std::vector<HelpProperty>*> stack{&properties};

    LambdaNetworkVisitor visitor{
        [&](const Property& property) {
            stack.back()->push_back(HelpProperty{
                .classIdentifier = std::string{property.getClassIdentifier()},
                .displayName = std::string{property.getDisplayName()},
                .typeName = getTypeName(property, app),
                .help = property.getHelp(),
                .properties = {},
            });
        },
        [&](const CompositeProperty& property, NetworkVisitorEnter) {
            stack.back()->push_back(HelpProperty{
                .classIdentifier = std::string{property.getClassIdentifier()},
                .displayName = std::string{property.getDisplayName()},
                .typeName = getTypeName(property, app),
                .help = property.getHelp(),
                .properties = {},
            });
            stack.push_back(&stack.back()->back().properties);

            return true;
        },
        [&](const CompositeProperty&, NetworkVisitorExit) { stack.pop_back(); }};

    processor.accept(visitor);

    const auto& info = processor.getProcessorInfo();
    InviwoModule* m = util::getProcessorModule(processor.getClassIdentifier(), app);
    auto pfo = app.getProcessorFactory()->getFactoryObject(processor.getClassIdentifier());

    const auto sourceFile = std::filesystem::path{info.file};
    const auto candidates = m ? headerCandidates(sourceFile.lexically_relative(m->getPath()),
                                                 m->getPath(), m->getIdentifier())
                              : std::vector<std::filesystem::path>{};

    const auto headerFile = candidates.empty() ? std::filesystem::path{} : candidates.front();
    const auto sourceLink = getRepoLink(sourceFile);
    const auto headerLink = headerFile.empty() ? "" : getRepoLink(headerFile);

    return HelpProcessor{
        .classIdentifier = processor.getClassIdentifier(),
        .displayName = std::string{processor.getDisplayName()},
        .typeName = getTypeName(processor, app),
        .category = info.category,
        .tags = info.tags,
        .help = info.help,
        .sourceFile = info.file,
        .headerFile = headerFile.generic_string(),
        .sourceLink = sourceLink.value_or(""),
        .headerLink = headerLink.value_or(""),
        .inports = std::move(inports),
        .outports = std::move(outports),
        .properties = std::move(properties),
        .inviwoModule = m ? m->getIdentifier() : "<unknown>",
        .meta = pfo->getMetaInformation(),
    };
}

namespace {
void link(std::string_view typeName, Document::DocumentHandle& handle) {
    constexpr std::string_view base = "https://inviwo.org/inviwo/cpp-api/class";
    if (!typeName.empty()) {
        std::string name{typeName.substr(0, typeName.find_first_of('<'))};
        std::string doxyName{name};
        replaceInString(doxyName, ":", "_1");
        handle.append("a", "", {{"href", fmt::format("{}{}", base, doxyName)}})
            .append("span", name, {{"class", "id"}});
    }
}
}  // namespace

Document help::toDocument(const HelpProperty& property, std::string_view path) {
    Document doc;

    auto spaced = [](auto str) { return fmt::format(" {}", str); };

    auto html = doc.append("html");
    auto body = html.append("body");
    body.append("h3", property.displayName);
    link(property.typeName, body);
    body.append("div", property.classIdentifier);
    body.append("div", "", {{"class", "help"}}).append(property.help);

    if (!property.properties.empty()) {
        body.append("h4", "Properties");
        auto properties = body.append("ul", "", {{"class", "list"}});
        for (auto&& [idx, subProperty] : util::enumerate(property.properties)) {
            auto li = properties.append("li", "", {{"class", "item"}});
            if (!subProperty.properties.empty()) {
                li.append("a", "",
                          {{"href", fmt::format("file://{}/{}?type=processor", path, idx)}});
                li += " ";
                li.append("span", spaced(subProperty.displayName), {{"class", "name"}});
            } else {
                li.append("span", spaced(subProperty.displayName), {{"class", "name"}});
            }
            li += " ";
            link(subProperty.typeName, li);
            li.append("span", spaced(subProperty.classIdentifier), {{"class", "id"}});
            if (!subProperty.help.empty()) {
                li += " ";
                li.append("div", "", {{"class", "help"}}).append(subProperty.help);
            }
        }
    }

    return doc;
}

Document help::toDocument(const HelpProcessor& processor) {
    auto spaced = [](auto str) { return fmt::format(" {}", str); };

    Document doc;

    auto html = doc.append("html");
    auto body = html.append("body");
    body.append("h3", fmt::format("{}", processor.displayName));

    auto tr = body.append("table").append("tr");
    tr.append("td").append("img", "",
                           {{"src", fmt::format("{0}.png?type=preview&classIdentifier={0}",
                                                processor.classIdentifier)}});
    auto td = tr.append("td");
    link(processor.typeName, td);
    td.append("i", spaced(processor.classIdentifier));

    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    utildoc::TableBuilder tb(td, P::end());
    tb(H("Module"), processor.inviwoModule);
    tb(H("Category"), processor.category);
    tb(H("State"), processor.codeState);
    tb(H("Tags"), processor.tags);

    body.append("div", "", {{"class", "help"}}).append(processor.help);

    if (!processor.inports.empty()) {
        body.append("h4", "Inports");
        auto inports = body.append("ol", "", {{"class", "list"}});
        for (const auto& inport : processor.inports) {
            auto li = inports.append("li", "", {{"class", "item"}});
            li += " ";
            li.append("span", spaced(inport.displayName), {{"class", "name"}});
            li += " ";
            link(inport.typeName, li);
            li.append("span", spaced(inport.classIdentifier), {{"class", "id"}});
            if (!inport.help.empty()) {
                li += " ";
                li.append("div", "", {{"class", "help"}}).append(inport.help);
            }
        }
    }

    if (!processor.outports.empty()) {
        body.append("h4", "Outports");
        auto outports = body.append("ol", "", {{"class", "list"}});
        for (const auto& outport : processor.outports) {
            auto li = outports.append("li", "", {{"class", "item"}});
            li += " ";
            li.append("span", outport.displayName, {{"class", "name"}});
            li += " ";
            link(outport.typeName, li);
            li.append("span", spaced(outport.classIdentifier), {{"class", "id"}});
            if (!outport.help.empty()) {
                li += " ";
                li.append("div", "", {{"class", "help"}}).append(outport.help);
            }
        }
    }

    if (!processor.properties.empty()) {
        body.append("h4", "Properties");
        auto properties = body.append("ul", "", {{"class", "list"}});
        for (auto&& [idx, property] : util::enumerate(processor.properties)) {
            auto li = properties.append("li", "", {{"class", "item"}});
            li += " ";
            if (!property.properties.empty()) {
                li.append("a", "",
                          {{"href", fmt::format("file:///{}/{}?type=processor",
                                                processor.classIdentifier, idx)}})
                    .append("span", spaced(property.displayName), {{"class", "name"}});
            } else {
                li.append("span", property.displayName, {{"class", "name"}});
            }
            li += " ";
            link(property.typeName, li);
            li.append("span", spaced(property.classIdentifier), {{"class", "id"}});

            if (!property.help.empty()) {
                li += " ";
                li.append("div", "", {{"class", "help"}}).append(property.help);
            }
        }
    }

    if (auto md = getModuleDir(processor.sourceFile)) {
        const auto repoDirSize = md->repoDir.generic_string().size();
        auto sourceName = std::string_view{processor.sourceFile};
        sourceName.remove_prefix(repoDirSize);

        body.append("h4", "Files");
        auto repo = body.append("div");
        repo.appendText(md->name);
        repo.appendText(" ");
        repo.append("a", md->repo, {{"href", md->repo}});
        auto links = body.append("ul");
        auto li1 = links.append("li", "", {{"class", "item"}});
        li1.appendText("Source ");
        li1.appendText(sourceName);
        li1.appendText(" ");
        li1.append("a", "file", {{"href", fmt::format("file:///{}", processor.sourceFile)}});
        if (!processor.sourceLink.empty()) {
            li1.appendText(" ");
            li1.append("a", "github", {{"href", processor.sourceLink}});
        }
        if (processor.headerFile.size() > repoDirSize) {
            auto headerName = std::string_view{processor.headerFile};
            headerName.remove_prefix(repoDirSize);

            auto li2 = links.append("li", "", {{"class", "item"}});
            li2.appendText("Header ");
            li2.appendText(headerName);
            li2.appendText(" ");
            li2.append("a", "file", {{"href", fmt::format("file:///{}", processor.headerFile)}});
            if (!processor.headerLink.empty()) {
                li2.appendText(" ");
                li2.append("a", "github", {{"href", processor.headerLink}});
            }
        }
    }

    if (!processor.meta.empty()) {
        body.append("h4", "Meta");
        body.append("div").append(processor.meta);
    }

    return doc;
}

}  // namespace inviwo
