/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/html/report.h>

namespace inviwo {

namespace pbt {

const std::string& cssFile =
    R""""(
td, th {
  border: 1px solid #999;
  text-align: left;
}
th {
  background: lightblue;
  border-color: white;
}
body {
  padding: 1rem;
}
)"""";

void reportValues(Document::DocumentHandle h,
    const std::shared_ptr<TestResult> t1, const std::shared_ptr<TestResult> t2, const bool different,
    const std::vector<const TestProperty*>& props) {
        
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto tb = h.append("table");

    auto headRow = tb.append("tr");
    headRow.append("th", "DisplayName");
    headRow.append("th", "Value");
    for (auto prop : props) {
        std::map<const TestProperty*, std::vector<const TestProperty*>> tree;
        std::map<const TestProperty*, bool> isInner;
        prop->traverse([&](const TestProperty* p, const TestProperty* pa) {
            isInner[pa] = true;
            if (different == (p->getValueString(t1) != p->getValueString(t2)))
                tree[pa].emplace_back(p);
        });

        const std::function<void(Document::DocumentHandle,const TestProperty*)> dfs =
            [&](Document::DocumentHandle cell, const TestProperty* p) {
                auto resTree = cell.append("ul");
                resTree.append("span", p->getDisplayName());
                auto children = resTree.append("ul");
                if (isInner[p]) {
                    for (auto c : tree[p]) dfs(children, c);
                } else  // leaf
                    children.append("div", p->getValueString(t1));
            };
        auto row = tb.append("tr");
        row.append("td", prop->getDisplayName());
        dfs(row.append("td"), prop);
    }
}

template<typename F>
void details(Document::DocumentHandle h, const std::string& s, F f) {
    auto t = h.append("details");
    t.append("summary", s);
    f(t);
}

void reportError(Document::DocumentHandle tb, const TestingError& e, const std::vector<const TestProperty*>& props) {
    using namespace std::placeholders; // for _1
    const auto& [testResult1, testResult2, _expectedEffect, num1, num2] = e;
    const auto& expectedEffect = _expectedEffect;

    const std::string expectedEffectString = [expectedEffect]() {
        std::stringstream str;
        str << expectedEffect;
        return str.str();
    }();


    auto row1 =    tb.append("tr");
    details(row1.append("td"), "Property Values", std::bind(reportValues, _1, testResult1, testResult2, true, props));
    row1.append("td", std::to_string(num1));
    row1.append("td", expectedEffectString);
    row1.append("td", std::to_string(num2));
    details(row1.append("td"), "Property Values", std::bind(reportValues, _1, testResult2, testResult1, true, props));
    details(row1.append("td"), "Equal Values", std::bind(reportValues, _1, testResult1, testResult2, false, props));

    auto row2 = tb.append("tr");
    row2.append("td","",{{"colspan","3"}}).append("img","",{{"src", testResult1->getImagePath()}});
    row2.append("td","",{{"colspan","3"}}).append("img","",{{"src", testResult2->getImagePath()}});
}

void propertyBasedTestingReport(
        std::ostream& out, const std::vector<TestingError>& errors,
         const std::vector<const TestProperty*>& props) {

    Document doc;
    using P = Document::PathComponent;
    using H = utildoc::TableBuilder::Header;
    auto html = doc.append("html");
    html.append("head").append("style", cssFile);
    auto body = html.append("body");
    auto tb = body.append("table");
    for(const auto& err : errors)
        reportError(tb, err, props);

    out << "<!DOCTYPE html>\n" << doc << std::endl;
}

} // namespace pbt

}  // namespace inviwo
