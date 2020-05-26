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

PropertyBasedTestingReport::PropertyBasedTestingReport
		( const std::vector< TestingError >& errors
		, const std::vector<std::shared_ptr<TestProperty>>& props) {
	std::ostream& out = std::cerr;

	HTML::Table errorTable;
	errorTable << (HTML::HeadRow()
			<< HTML::Text("Test1")
			<< HTML::Text("Test1 background pixels") 
			<< HTML::Text("Expected relation")
			<< HTML::Text("Test2 background pixels")
			<< HTML::Text("Test2")
			<< HTML::Text("Equal Properties"));
	for(const auto& err : errors)
		errorTable << generateHTML(err, props);

	out << "<!DOCTYPE html>\n"
		<< "<meta charset=\"utf-8\"/>\n";
	out << (HTML::HTML()
			<< (HTML::Head().stylesheet("report.css"))
			<< (HTML::Body() << errorTable));
}

HTML::Row PropertyBasedTestingReport::generateHTML
		( const TestingError& e
		, const std::vector<std::shared_ptr<TestProperty>>& props) {
	const auto&[testResult1, testResult2, expectedEffect, num1, num2] = e;

	const std::string expectedEffectString = [expectedEffect]() {
			std::stringstream str;
			str << expectedEffect;
			return str.str();
		}();

	std::vector<std::shared_ptr<TestProperty>> sameProperties, differentProperties;
	// find properties which are (not) equal in the test results
	for(auto prop : props) {
		if(prop->getValueString(testResult1) == prop->getValueString(testResult2))
			sameProperties.emplace_back(prop);
		else
			differentProperties.emplace_back(prop);
	}

	HTML::Row res;
	res << HTML::Details(HTML::Text("Image1"), HTML::Image(testResult1->getImagePath())),
	res << HTML::Details(HTML::Text("Property Values"), generateHTML(testResult1, differentProperties));
	res << HTML::Text(std::to_string(num1));
	res << HTML::Text(expectedEffectString);
	res << HTML::Text(std::to_string(num2));
	res << HTML::Details(HTML::Text("Property Values"), generateHTML(testResult2, differentProperties));
	res << HTML::Details(HTML::Text("Image2"), HTML::Image(testResult2->getImagePath())),
	res << HTML::Details(HTML::Text("Equal Properties"), generateHTML(testResult1, sameProperties));
	return res;
}
HTML::Element PropertyBasedTestingReport::generateHTML
		( std::shared_ptr<TestResult> testResult
		, const std::vector<std::shared_ptr<TestProperty>>& props) {
	HTML::Table res;
	res << (HTML::HeadRow() << HTML::Text("DisplayName") << HTML::Text("Identifier") << HTML::Text("Value"));
	for(auto prop : props) {
		res << (HTML::Row()
				<< HTML::Text(prop->getProperty()->getDisplayName())
				<< HTML::Text(prop->getProperty()->getIdentifier())
				<< HTML::Text(prop->getValueString(testResult)));
	}
	return res;
}

}  // namespace inviwo
