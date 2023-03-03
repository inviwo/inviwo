/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2023 Inviwo Foundation
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

#include <inviwo/core/algorithm/markdown.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/unindent.h>

#include <warn/push>
#include <warn/ignore/all>
#include <md4c/md4c.h>
#include <warn/pop>

#include <utf8/checked.h>

namespace inviwo {

namespace {
void renderAttribute(const MD_ATTRIBUTE& attr, StrBuffer& strBuffer,
                     void (*appendTo)(std::string_view, StrBuffer&)) {
    for (int i = 0; attr.substr_offsets[i] < attr.size; i++) {
        MD_TEXTTYPE type = attr.substr_types[i];
        MD_OFFSET off = attr.substr_offsets[i];
        MD_SIZE size = attr.substr_offsets[i + 1] - off;
        const MD_CHAR* text = attr.text + off;

        switch (type) {
            case MD_TEXT_NULLCHAR:
                utf8::append(0x0000, std::back_inserter(strBuffer.buff));
                break;
            case MD_TEXT_ENTITY:
                appendTo(std::string_view(text, size), strBuffer);
                break;
            default:
                appendTo(std::string_view(text, size), strBuffer);
                break;
        }
    }
}

}  // namespace

Document util::md2doc(std::string_view markdown) {
    using State = std::vector<Document::DocumentHandle>;

    auto enter_block_callback = [](MD_BLOCKTYPE type, void* detail, void* userdata) -> int {
        auto* state = static_cast<State*>(userdata);

        constexpr std::array<std::string_view, 4> align{{"", "left", "center", "right"}};

        switch (type) {
            case MD_BLOCK_DOC:
                state->push_back(state->back().append("div"));
                break;
            case MD_BLOCK_QUOTE:
                state->push_back(state->back().append("blockquote"));
                break;
            case MD_BLOCK_UL:
                state->push_back(state->back().append("ul"));
                break;
            case MD_BLOCK_OL: {
                auto* blockDetail = static_cast<MD_BLOCK_OL_DETAIL*>(detail);

                if (blockDetail->start == 1) {
                    state->push_back(state->back().append("ol"));
                } else {
                    state->push_back(state->back().append(
                        "ol", "", {{"start", fmt::format("{}", blockDetail->start)}}));
                }
                break;
            }
            case MD_BLOCK_LI:
                state->push_back(state->back().append("li"));
                break;
            case MD_BLOCK_HR:
                state->push_back(state->back().append("hr"));
                break;
            case MD_BLOCK_H: {
                auto* blockDetail = static_cast<MD_BLOCK_H_DETAIL*>(detail);
                state->push_back(state->back().append(fmt::format("h{}", blockDetail->level)));
                break;
            }
            case MD_BLOCK_CODE: {
                auto* blockDetail = static_cast<MD_BLOCK_CODE_DETAIL*>(detail);
                if (blockDetail->lang.text != nullptr) {
                    StrBuffer buff;
                    renderAttribute(blockDetail->lang, buff, util::htmlEncodeTo);
                    state->push_back(
                        state->back()
                            .append("code", "",
                                    {{"class", fmt::format("language-{}", buff.view())}})
                            .append("pre"));
                } else {
                    state->push_back(state->back().append("code").append("pre"));
                }
                break;
            }
            case MD_BLOCK_HTML:
                state->push_back(state->back().append("div"));
                break;
            case MD_BLOCK_P:
                state->push_back(state->back().append("p"));
                break;
            case MD_BLOCK_TABLE:
                state->push_back(state->back().append("table"));
                break;
            case MD_BLOCK_THEAD:
                state->push_back(state->back().append("thead"));
                break;
            case MD_BLOCK_TBODY:
                state->push_back(state->back().append("tbody"));
                break;
            case MD_BLOCK_TR:
                state->push_back(state->back().append("tr"));
                break;
            case MD_BLOCK_TH: {
                auto* blockDetail = static_cast<MD_BLOCK_TD_DETAIL*>(detail);
                if (blockDetail->align == MD_ALIGN_DEFAULT) {
                    state->push_back(state->back().append("th"));

                } else {
                    state->push_back(state->back().append(
                        "th", "",
                        {{"align", std::string{align[static_cast<int>(blockDetail->align)]}}}));
                }
                break;
            }
            case MD_BLOCK_TD: {
                auto* blockDetail = static_cast<MD_BLOCK_TD_DETAIL*>(detail);
                if (blockDetail->align == MD_ALIGN_DEFAULT) {
                    state->push_back(state->back().append("td"));

                } else {
                    state->push_back(state->back().append(
                        "td", "",
                        {{"align", std::string{align[static_cast<int>(blockDetail->align)]}}}));
                }
                break;
            }
        }

        return 0;
    };
    auto leave_block_callback = [](MD_BLOCKTYPE, void*, void* userdata) -> int {
        auto* state = static_cast<State*>(userdata);
        state->pop_back();
        return 0;
    };
    auto enter_span_callback = [](MD_SPANTYPE type, void* detail, void* userdata) -> int {
        auto* state = static_cast<State*>(userdata);

        switch (type) {
            case MD_SPAN_EM:
                state->push_back(state->back().append("em"));
                break;
            case MD_SPAN_STRONG:
                state->push_back(state->back().append("strong"));
                break;
            case MD_SPAN_U:
                state->push_back(state->back().append("u"));
                break;
            case MD_SPAN_A: {
                auto* blockDetail = static_cast<MD_SPAN_A_DETAIL*>(detail);

                StrBuffer buff;
                renderAttribute(blockDetail->href, buff, util::urlEncodeTo);
                std::unordered_map<std::string, std::string> attr = {
                    {"href", std::string{buff.view()}}};
                if (blockDetail->title.text) {
                    buff.clear();
                    renderAttribute(blockDetail->title, buff, util::htmlEncodeTo);
                    attr.emplace("title", buff.view());
                }

                state->push_back(state->back().append("a", "", attr));
                break;
            }
            case MD_SPAN_IMG: {
                auto* blockDetail = static_cast<MD_SPAN_IMG_DETAIL*>(detail);

                StrBuffer buff;
                renderAttribute(blockDetail->src, buff, util::urlEncodeTo);
                std::unordered_map<std::string, std::string> attr = {
                    {"src", std::string{buff.view()}}};
                if (blockDetail->title.text) {
                    buff.clear();
                    renderAttribute(blockDetail->title, buff, util::htmlEncodeTo);
                    attr.emplace("title", buff.view());
                }
                state->push_back(state->back().append("img", "", attr));
                break;
            }
            case MD_SPAN_CODE:
                state->push_back(state->back().append("code"));
                break;
            case MD_SPAN_DEL:
                state->push_back(state->back().append("del"));
                break;
            case MD_SPAN_LATEXMATH:
                state->push_back(state->back().append("x-equation"));
                break;
            case MD_SPAN_LATEXMATH_DISPLAY:
                state->push_back(state->back().append("x-equation", "", {{"type", "display"}}));
                break;
            case MD_SPAN_WIKILINK:  // dummy
                state->push_back(state->back().append("div"));
                break;
        }

        return 0;
    };
    auto leave_span_callback = [](MD_SPANTYPE, void*, void* userdata) -> int {
        auto* state = static_cast<State*>(userdata);
        state->pop_back();
        return 0;
    };
    auto text_callback = [](MD_TEXTTYPE, const MD_CHAR* text, MD_SIZE size, void* userdata) -> int {
        auto* state = static_cast<State*>(userdata);
        if (state->back().element().name() == "img") {
            state->back().element().attributes().emplace("alt", std::string_view(text, size));
        } else {
            state->back() += std::string_view(text, size);
        }
        return 0;
    };
    auto debug_log_callback = [](const char* msg, void*) -> void {
        LogInfoCustom("Markdown", "Error: " << msg);
    };

    MD_PARSER parser = {
        0,
        MD_FLAG_NOHTML | MD_FLAG_PERMISSIVEAUTOLINKS | MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH,
        enter_block_callback,
        leave_block_callback,
        enter_span_callback,
        leave_span_callback,
        text_callback,
        debug_log_callback,
        nullptr};

    Document doc;
    State state{doc.handle()};
    md_parse(markdown.data(), static_cast<MD_SIZE>(markdown.size()), &parser,
             static_cast<void*>(&state));

    return doc;
};

Document util::unindentMd2doc(std::string_view markdown) {
    return util::md2doc(indent::unindent(markdown));
}

}  // namespace inviwo
