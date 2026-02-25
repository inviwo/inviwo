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

#include <inviwo/qt/editor/processorlistfilter.h>

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/stringconversion.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <algorithm>
#include <ranges>

namespace inviwo {

namespace {

constexpr auto strMatch = [](std::string_view cont, std::string_view s) {
    constexpr auto icomp = [](std::string_view::value_type l1, std::string_view::value_type r1) {
        return std::tolower(l1) == std::tolower(r1);
    };
    return !std::ranges::search(cont, s, icomp).empty();
};

constexpr auto matcher = [](auto mptr) {
    using Item = ProcessorListModel::Item;
    return [mptr](std::string_view str, const std::any&, const Item& item) -> bool {
        return strMatch(std::invoke(mptr, item.info), str);
    };
};

}  // namespace

ProcessorListFilter::ProcessorListFilter(QAbstractItemModel* model, ProcessorNetwork* net,
                                         QObject* parent)
    : QSortFilterProxyModel(parent)
    , grouping_{Grouping::Categorical}
    , dsl_{{{.name = "identifier",
             .shortcut = "i",
             .description = "processor class identifier",
             .global = true,
             .match = matcher(&ProcessorInfo::classIdentifier)},
            {.name = "name",
             .shortcut = "n",
             .description = "processor displayname",
             .global = true,
             .match = matcher(&ProcessorInfo::displayName)},
            {.name = "category",
             .shortcut = "c",
             .description = "processor category",
             .global = true,
             .match = matcher(&ProcessorInfo::category)},
            {.name = "tags",
             .shortcut = "#",
             .description = "processor tags",
             .global = true,
             .match =
                 [](std::string_view str, const std::any&, const Item& item) {
                     return std::ranges::any_of(item.info.tags.tags_, [&](const Tag& tag) {
                         return strMatch(tag.getString(), str);
                     });
                 }},
            {.name = "state",
             .shortcut = "",
             .description = "processor category",
             .global = false,
             .match =
                 [](std::string_view str, const std::any&, const Item& item) {
                     return strMatch(fmt::to_string(item.info.codeState), str);
                 }},
            {.name = "module",
             .shortcut = "m",
             .description = "processor module",
             .global = false,
             .match = [](std::string_view str, const std::any&,
                         const Item& item) { return strMatch(item.moduleId, str); }},
            {.name = "pre",
             .shortcut = "p",
             .description = "predecessor processor identifier",
             .global = false,
             .match =
                 [](std::string_view, const std::any& data, const Item& item) {
                     auto* predecessor = std::any_cast<Processor*>(data);
                     if (!predecessor) return false;
                     return std::ranges::any_of(predecessor->getOutports(), [&](Outport* p) {
                         return help::matchOutportToInports(p->getClassIdentifier(), item.help);
                     });
                 },
             .onToken = [net](std::string_view processorId) -> std::any {
                 return net->getProcessorByIdentifier(processorId);
             }}}} {
    setRecursiveFilteringEnabled(true);

    setSourceModel(model);
}

bool ProcessorListFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    auto index = sourceModel()->index(source_row, 0, source_parent);
    if (const auto* item = utilqt::getData(index, Role::Item).value<const Item*>()) {
        if (!item->info.visible) return false;
        return dsl_.match(*item);
    } else {
        return false;
    }
};

void ProcessorListFilter::setCustomFilter(const QString& filter) {
#if QT_VERSION < QT_VERSION_CHECK(6, 10, 0)
    if (dsl_.setSearchString(utilqt::fromQString(filter))) {
        invalidateFilter();
    }
#else
    const auto str = utilqt::fromQString(filter);
    if (dsl_.getSearchString() != str) {
        beginFilterChange();
        dsl_.setSearchString(str);
        endFilterChange();
    }
#endif
}

Document ProcessorListFilter::description() const {
    using P = Document::PathComponent;
    auto doc = dsl_.description();
    auto b = doc.get({P{"html"}, P{"body"}});
    auto desc = b.insert(P::first(), "div");
    desc.append("b", "Search Processors", {{"style", "color:white;"}});
    desc.append("p", "Example: name:raycaster state:stable");
    return doc;
}

std::optional<std::string_view> ProcessorListFilter::currentStr(std::string_view name) {
    return dsl_.currentStr(name);
}
void ProcessorListFilter::setGrouping(Grouping grouping) {
    if (grouping_ != grouping) {
        grouping_ = grouping;
    }
}

std::any ProcessorListFilter::currentData(std::string_view name) { return dsl_.currentData(name); }

bool ProcessorListFilter::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    const auto* a = utilqt::getData(left, Role::Item).value<const Item*>();
    const auto* b = utilqt::getData(right, Role::Item).value<const Item*>();

    if (!a || !b) {
        const auto na = utilqt::getData(left, Role::Sort);
        const auto nb = utilqt::getData(right, Role::Sort);
        return QVariant::compare(na, nb) < 0;
    }

    switch (grouping_) {
        using enum Grouping;
        case Alphabetical:
            return iCaseLess(a->info.displayName, b->info.displayName);
        case Categorical:
            if (!iCaseCmp(a->info.category, b->info.category)) {
                return iCaseLess(a->info.category, b->info.category);
            } else {
                return iCaseLess(a->info.displayName, b->info.displayName);
            }
        case CodeState:
            if (a->info.codeState != b->info.codeState) {
                return a->info.codeState < b->info.codeState;
            } else {
                return iCaseLess(a->info.displayName, b->info.displayName);
            }
        case Module:
            if (!iCaseCmp(a->moduleId, b->moduleId)) {
                return iCaseLess(a->moduleId, b->moduleId);
            } else {
                return iCaseLess(a->info.displayName, b->info.displayName);
            }
        case LastUsed:
            if (a->lastUsed != b->lastUsed) {
                return a->lastUsed > b->lastUsed;
            } else {
                return iCaseLess(a->info.displayName, b->info.displayName);
            }
        case MostUsed:
            if (a->useCount != b->useCount) {
                return a->useCount > b->useCount;
            } else {
                return iCaseLess(a->info.displayName, b->info.displayName);
            }
        case Inports:
            [[fallthrough]];
        case Outports:
            [[fallthrough]];
        default:
            return iCaseLess(a->info.displayName, b->info.displayName);
    }
}

}  // namespace inviwo
