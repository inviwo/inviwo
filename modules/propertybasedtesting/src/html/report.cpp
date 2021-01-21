/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOr
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

PropertyBasedTestingReport::PropertyBasedTestingReport(std::ostream& out,
                                                       const std::vector<TestingError>& errors,
                                                       const std::vector<TestProperty*>& props) {
    HTML::Table errorTable;
    errorTable << (HTML::HeadRow() << HTML::Text("Test1") << HTML::Text("Test1 score")
                                   << HTML::Text("Expected relation") << HTML::Text("Test2 score")
                                   << HTML::Text("Test2") << HTML::Text("Equal Properties"));
    for (const auto& err : errors) {
        for (const auto& row : generateHTML(err, props)) errorTable << row;
    }

    out << "<!DOCTYPE html>\n";
    out << (HTML::HTML() << (HTML::Head() << HTML::Style(cssFile)
                                          << HTML::Meta().addAttribute(
                                                 "charset", "utf-8"))  //.stylesheet("report.css"))
                         << (HTML::Body() << errorTable));
}

std::vector<HTML::Row> PropertyBasedTestingReport::generateHTML(
    const TestingError& e, const std::vector<TestProperty*>& props) {
    const auto& [testResult1, testResult2, _expectedEffect, num1, num2] = e;
    const auto& expectedEffect = _expectedEffect;

    const std::string expectedEffectString = [expectedEffect]() {
        std::stringstream str;
        str << expectedEffect;
        return str.str();
    }();

    HTML::Row row1;
    row1 << HTML::Details(HTML::Text("Property Values"),
                          generateHTML(std::make_tuple(testResult2, testResult1, true), props));
    row1 << HTML::Text(std::to_string(num1));
    row1 << HTML::Text(expectedEffectString);
    row1 << HTML::Text(std::to_string(num2));
    row1 << HTML::Details(HTML::Text("Property Values"),
                          generateHTML(std::make_tuple(testResult1, testResult2, true), props));
    row1 << HTML::Details(HTML::Text("Equal Properties"),
                          generateHTML(std::make_tuple(testResult1, testResult2, false), props));
    HTML::Row row2;
    row2 << HTML::TableCell(
                HTML::Details(HTML::Text("Image1"), HTML::Image(testResult1->getImagePath())))
                .addAttribute("colspan", "3");
    row2 << HTML::TableCell(
                HTML::Details(HTML::Text("Image2"), HTML::Image(testResult2->getImagePath())))
                .addAttribute("colspan", "3");
    return {row1, row2};
}
HTML::BaseElement PropertyBasedTestingReport::generateHTML(
    const std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>, bool>& testResults,
    const std::vector<TestProperty*>& props) {
    const auto& [t1, t2, different] = testResults;
    HTML::Table res;
    res << (HTML::HeadRow() << HTML::Text("DisplayName") << HTML::Text("Value"));
    for (auto prop : props) {
        HTML::Row r;
        r << HTML::Text(prop->getDisplayName());
        std::map<const TestProperty*, std::vector<const TestProperty*>> tree;
        std::map<const TestProperty*, bool> isInner;
        prop->traverse([&](const TestProperty* p, const TestProperty* pa) {
            isInner[pa] = true;
            if (different == (p->getValueString(t1) != p->getValueString(t2)))
                tree[pa].emplace_back(p);
        });

        std::function<HTML::Tree(const TestProperty*)> dfs = [&](const TestProperty* p) {
            HTML::Tree res(HTML::Text(p->getDisplayName()));
            HTML::TreeChildren children;
            if (isInner[p]) {
                for (auto c : tree[p]) children << dfs(c);
            } else  // leaf
                children << HTML::Text(p->getValueString(t1));
            res << children;
            return res;
        };

        r << dfs(prop);
        res << r;
    }
    return res;
}

} // namespace pbt

}  // namespace inviwo
