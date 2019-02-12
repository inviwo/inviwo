/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/networksearch.h>
#include <inviwo/qt/editor/inviwomainwindow.h>
#include <inviwo/qt/editor/networkeditor.h>
#include <inviwo/qt/editor/processorgraphicsitem.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/port.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/document.h>

#include <algorithm>

#include <warn/push>
#include <warn/ignore/all>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QEvent>
#include <QString>

#include <warn/pop>
#include "qevent.h"

namespace inviwo {

NetworkSearch::NetworkSearch(InviwoMainWindow *win)
    : QWidget(win)
    , win_{win}
    , edit_{new QLineEdit(this)}
    , items_{
          {"class", "c", "processor class identifier", true,
           [](Processor *p, const std::string &s) -> bool {
               return find(p->getClassIdentifier(), s);
           }},
          {"identifier", "i", "processor identifier", true,
           [](Processor *p, const std::string &s) -> bool { return find(p->getIdentifier(), s); }},
          {"name", "n", "processor display name", true,
           [](Processor *p, const std::string &s) -> bool { return find(p->getDisplayName(), s); }},
          {"category", "", "processor category", true,
           [](Processor *p, const std::string &s) -> bool { return find(p->getCategory(), s); }},
          {"tag", "t", "search processor tags", true,
           [](Processor *p, const std::string &s) -> bool {
               bool tag = false;
               for (const auto &t : p->getTags().tags_) {
                   tag |= find(t.getString(), s);
               }
               return tag;
           }},
          {"state", "s", "processor state", true,
           [](Processor *p, const std::string &s) -> bool {
               return find(toString(p->getCodeState()), s);
           }},
          {"inport", "", "search inport class identifiers", true,
           [](Processor *p, const std::string &s) -> bool {
               bool inport = false;
               for (const auto &pt : p->getInports()) {
                   inport |= find(pt->getClassIdentifier(), s);
               }
               return inport;
           }},
          {"outport", "", "search outport class identifiers", true,
           [](Processor *p, const std::string &s) -> bool {
               bool outport = false;
               for (const auto &pt : p->getOutports()) {
                   outport |= find(pt->getClassIdentifier(), s);
               }
               return outport;
           }},
          {"port", "", "search port class identifiers", false,
           [](Processor *p, const std::string &s) -> bool {
               bool port = false;
               for (const auto &pt : p->getOutports()) {
                   port |= find(pt->getClassIdentifier(), s);
               }
               for (const auto &pt : p->getInports()) {
                   port |= find(pt->getClassIdentifier(), s);
               }
               return port;
           }},
          {"property", "p", "search property identifiers", true,
           [](Processor *p, const std::string &s) -> bool {
               bool property = false;
               for (const auto &pr : p->getPropertiesRecursive()) {
                   property |= find(pr->getIdentifier(), s);
               }
               return property;
           }},
          {"module", "m", "processor module", true,
           [this](Processor *p, const std::string &s) -> bool {
               auto moduleMap = getModuleMap(win_->getInviwoApplication());
               bool module = false;
               auto mit = moduleMap.find(p->getClassIdentifier());
               if (mit != moduleMap.end()) {
                   module |= find(mit->second, s);
               }
               return module;
           }}} {

    setObjectName("NetworkSearch");
    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    hLayout->addWidget(edit_);
    setLayout(hLayout);
    setVisible(false);
    setMaximumWidth(400);
    setMinimumWidth(200);
    edit_->setPlaceholderText("Search Network...");
    edit_->installEventFilter(this);
    edit_->setClearButtonEnabled(true);

    Document doc;
    using P = Document::PathComponent;
    auto b = doc.append("html").append("body");
    b.append("b", "Search network", {{"style", "color:white;"}});
    b.append("p", "Example: \"tag:gl port:volume raycaster\".");
    b.append("p",
             "Terms are intersecting. Double quotes are supported. "
             "Enter will confirm selection, escape will cancel.");
    b.append("p", "Supported keys (shortcut):");
    using H = utildoc::TableBuilder::Header;
    utildoc::TableBuilder tb(b, P::end(), {{"identifier", "propertyInfo"}});

    for (auto &item : items_) {
        tb(H(item.name + (item.shortcut.empty() ? "" : " (" + item.shortcut + ")")),
           item.description);
    }
    setToolTip(utilqt::toQString(doc));

    for (auto &item : items_) {
        map_[item.name] = item.match;
        if (!item.shortcut.empty()) {
            map_[item.shortcut] = item.match;
        }
    }

    map_["*"] = [this](Processor *p, const std::string &s) -> bool {
        for (auto &item : items_) {
            if (item.global && item.match(p, s)) {
                return true;
            }
        }
        return false;
    };

    connect(edit_, &QLineEdit::textChanged, this, &NetworkSearch::updateSearch);
}

void NetworkSearch::updateSearch(const QString &str) {
    auto app = win_->getInviwoApplication();
    auto network = app->getProcessorNetwork();
    auto editor = win_->getNetworkEditor();

    if (str.isEmpty()) {
        // nothing selected
        editor->clearSelection();
        return;
    }

    auto tokens = tokenize(utilqt::fromQString(str));

    network->forEachProcessor([&](Processor *p) {
        bool match = true;
        for (const auto &token : tokens) {
            const auto &s = token.second;
            const auto &k = token.first;

            auto it = map_.find(k);
            if (it != map_.end()) {
                match &= it->second(p, s);
            }
        }
        auto pgi = editor->getProcessorGraphicsItem(p);
        pgi->setHighlight(true);
        pgi->setSelected(match);
    });
}

void NetworkSearch::focusInEvent(QFocusEvent *) {
    edit_->setFocus();
    updateSearch(edit_->text());
}

void NetworkSearch::focusOutEvent(QFocusEvent *) {}

bool NetworkSearch::eventFilter(QObject *watched, QEvent *event) {
    if (watched == edit_) {
        if (event->type() == QEvent::FocusOut) {
            auto app = win_->getInviwoApplication();
            auto editor = win_->getNetworkEditor();
            auto network = app->getProcessorNetwork();
            network->forEachProcessor(
                [&](Processor *p) { editor->getProcessorGraphicsItem(p)->setHighlight(false); });
            setVisible(false);
            return false;
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                edit_->text().clear();
                auto app = win_->getInviwoApplication();
                auto editor = win_->getNetworkEditor();
                auto network = app->getProcessorNetwork();
                network->forEachProcessor([&](Processor *p) {
                    auto pgi = editor->getProcessorGraphicsItem(p);
                    pgi->setHighlight(false);
                    pgi->setSelected(false);
                });
                setVisible(false);
                return true;
            } else if (keyEvent->key() == Qt::Key_Return) {
                edit_->text().clear();
                auto app = win_->getInviwoApplication();
                auto editor = win_->getNetworkEditor();
                auto network = app->getProcessorNetwork();
                network->forEachProcessor([&](Processor *p) {
                    auto pgi = editor->getProcessorGraphicsItem(p);
                    if (pgi->isSelected()) {
                        pgi->setSelected(false);
                        pgi->setHighlight(false);
                        pgi->setSelected(true);
                    } else {
                        pgi->setHighlight(false);
                        pgi->setSelected(false);
                    }
                });
                setVisible(false);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

std::vector<std::pair<std::string, std::string>> NetworkSearch::tokenize(const std::string &str) {
    std::vector<std::pair<std::string, std::string>> tokens;

    std::string key = "*";
    std::string curr;
    bool quote = false;
    for (auto c : str) {
        if (quote && c == '\"') {
            tokens.emplace_back(key, curr);
            curr.clear();
            key = "*";
            quote = false;
        } else if (quote) {
            curr.push_back(c);
        } else if (c == '\"') {
            quote = true;
        } else if (c == ':') {
            key = curr;
            curr.clear();
        } else if (std::isspace(c) && !curr.empty()) {
            tokens.emplace_back(key, curr);
            curr.clear();
            key = "*";
        } else if (!std::isspace(c)) {
            curr.push_back(c);
        }
    }
    if (!curr.empty()) {
        tokens.emplace_back(key, curr);
    }

    return tokens;
}

std::unordered_map<std::string, std::string> NetworkSearch::getModuleMap(InviwoApplication *app) {
    std::unordered_map<std::string, std::string> moduleMap;
    for (const auto &m : app->getModuleManager().getModules()) {
        for (const auto &p : m->getProcessors()) {
            moduleMap[p->getClassIdentifier()] = m->getIdentifier();
        }
    }
    return moduleMap;
}

bool NetworkSearch::find(const std::string &cont, const std::string &s) {
    auto icomp = [](std::string::value_type l1, std::string::value_type r1) {
        return std::tolower(l1) == std::tolower(r1);
    };
    return std::search(cont.begin(), cont.end(), s.begin(), s.end(), icomp) != cont.end();
}

}  // namespace inviwo
