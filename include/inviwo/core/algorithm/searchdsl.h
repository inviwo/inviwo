/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/document.h>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>

#include <fmt/format.h>

namespace inviwo {

template <typename... Ts>
class SearchDSL {
public:
    struct Item {
        std::string name;
        std::string shortcut;
        std::string description;
        bool global;
        std::function<bool(std::string_view, const Ts&...)> match;
    };

    SearchDSL(std::vector<Item> items)
        : global_{"global", "*", "", true,
                  [this](std::string_view str, const Ts&... things) -> bool {
                      for (const auto& item : items_) {
                          if (item.global && item.match(str, things...)) {
                              return true;
                          }
                      }
                      return false;
                  }}
        , items_{std::move(items)} {

        for (const auto& item : items_) {
            map_[item.name] = &item;
            if (!item.shortcut.empty()) {
                map_[item.shortcut] = &item;
            }
        }
        map_[global_.shortcut] = &global_;
    }

    SearchDSL(const SearchDSL&) = delete;
    SearchDSL(SearchDSL&&) = delete;
    SearchDSL& operator=(const SearchDSL&) = delete;
    SearchDSL& operator=(SearchDSL&&) = delete;

    Document description() const {
        Document doc;
        using P = Document::PathComponent;
        auto b = doc.append("html").append("body");
        b.append("p", "Supported keys (shortcut):");
        using H = utildoc::TableBuilder::Header;
        utildoc::TableBuilder tb(b, P::end());

        for (auto& item : items_) {
            if (item.shortcut.empty()) {
                tb(H(item.name), "", item.global ? "global" : "", item.description);
            } else {
                tb(H(item.name), fmt::format("({})", item.shortcut), item.global ? "global" : "",
                   item.description);
            }
        }
        b.append("p",
                 "An unqualified string will search all 'global' keys. "
                 "Terms are intersecting. Double quotes are supported.");

        return doc;
    }

    bool setSearchString(std::string_view str) {
        if (searchStr_ == str) return false;
        searchStr_ = str;
        current_.clear();
        tokenize();

        return true;
    }

    bool match(const Ts&... things) const {
        for (const auto& [item, str] : current_) {
            if (!item->match(str, things...)) return false;
        }
        return true;
    }

private:
    void addToken(std::string_view key, std::string_view str) {
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            str = str.substr(1, str.size() - 2);
        } else if (str.size() >= 1 && str.front() == '"') {
            str = str.substr(1);
        }
        if (auto it = map_.find(key); it != map_.end()) {
            current_.emplace_back(it->second, str);
        }
    }

    void tokenize() {
        auto str = std::string_view(searchStr_);
        bool quote = false;
        size_t tokenStart = 0;
        std::string_view key = global_.shortcut;

        for (size_t i = 0; i < str.size(); ++i) {
            auto c = str[i];

            if (quote && c == '\"') {
                quote = false;
            } else if (quote) {
                // do nothing
            } else if (c == '\"') {
                quote = true;
            } else if (c == ':') {
                key = str.substr(tokenStart, i - tokenStart);
                tokenStart = i + 1;
            } else if (std::isspace(c) && tokenStart < i) {
                addToken(key, str.substr(tokenStart, i - tokenStart));
                key = global_.shortcut;
                tokenStart = i + 1;
            } else if (std::isspace(c)) {
                tokenStart = i + 1;
            }
        }
        if (tokenStart < str.size()) {
            addToken(key, str.substr(tokenStart, str.size() - tokenStart));
        }
    }

    Item global_;

    std::vector<Item> items_;
    std::unordered_map<std::string_view, const Item*> map_;

    std::string searchStr_;
    std::vector<std::pair<const Item*, std::string_view>> current_;
};

}  // namespace inviwo
