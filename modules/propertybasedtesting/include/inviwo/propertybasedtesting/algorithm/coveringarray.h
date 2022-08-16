/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/propertybasedtesting/algorithm/generatingassignments.h>
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>

namespace inviwo {

namespace pbt {

/*
 * 2-coverage, randomized discrete SLJ strategy
 * - vars should contain a set of assignments for each variable
 *   and no variable should be represented more than once
 */
std::vector<Test> IVW_MODULE_PROPERTYBASEDTESTING_API
coveringArray(const std::vector<std::vector<std::shared_ptr<PropertyAssignment>>>& vars);

/*
 * 2-coverage, but using a greedy randomized strategy for optimizing for comparability.
 * Much slower than the procedure above, but also yields tests that are much
 * more comparable.
 * - num is the maximum number of tests to be generated; useful for preventing
 *   unnecessary calculations
 * - vars should contain a single pair for each variable, where the first component is
 *   an AssignmentComparator and the second contains the actual assignments
 */
std::vector<Test> IVW_MODULE_PROPERTYBASEDTESTING_API optCoveringArray(
    const size_t num,
    const std::vector<
        std::pair<AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>& vars);

}  // namespace pbt

}  // namespace inviwo
