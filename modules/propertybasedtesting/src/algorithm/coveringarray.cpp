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

#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>

#include <random>

namespace inviwo {

namespace pbt {

/*
 * uses the discrete SLJ strategy, basically corresponds to
 * Algorithm 2 from
 * "Two-stage algorithms for covering array construction" by
 * K. Sarkar, C. J. Colbourn (2018)
 */
std::vector<Test> coveringArray(
    const std::vector<std::vector<std::shared_ptr<PropertyAssignment>>>& vars) {
    std::default_random_engine rng(42);  // deterministic for regression testing

    if (vars.empty()) {
        return {};
    }

    // special case
    if (vars.size() == 1) {
        // just create a test for each assignment containing (only) that one
        // assignment
        std::vector<Test> res(vars[0].size());
        for (size_t i = 0; i < vars[0].size(); i++) res[i].emplace_back(vars[0][i]);
        return res;
    }

    // maximum number of assignments in for any variable
    const size_t maxAssignments = [&]() {
        size_t res = 0;
        for (const auto& v : vars) {
            IVW_ASSERT(v.size() > 0, "coveringArray: got no assignments for some property");
            res = std::max(res, v.size());
        }
        return res;
    }();

    // create every possible interaction of two variables and
    // enumerate them. An interaction consists of the involved
    // variables (indices i and j) and one value for each (indices ii and ji)
    std::map<std::array<size_t, 4>, size_t> idx;
    // set of uncovered (i.e. not in any generated test) interactions
    std::unordered_set<size_t> uncovered;
    for (size_t i = 1; i < vars.size(); i++) {
        for (size_t j = 0; j < i; j++) {
            for (size_t ii = 0; ii < vars[i].size(); ii++) {
                for (size_t ji = 0; ji < vars[j].size(); ji++) {
                    uncovered.insert(idx.size());
                    idx[{i, j, ii, ji}] = idx.size();
                }
            }
        }
    }

    // resulting coveringarray
    std::vector<Test> result;
    while (!uncovered.empty()) {
        size_t expectedCoverage = (uncovered.size() + (maxAssignments * maxAssignments - 1)) /
                                  (maxAssignments * maxAssignments);
        // expectedCoverage > 0, since uncovered.size() > 0 and we are using the
        // ceil of the fraction (uncovered.size() / (maxAssignments*maxAssignments)

        // randomly generate a test by uniformly and independently choosing
        // an assignment for each variable, until the test covers at least as
        // many interactions as expected (i.e. coverage >= expectedCoverage)
        size_t coverage;
        std::vector<size_t> row(vars.size());  // indices of the interactions of the test
        do {
            for (size_t i = 0; i < row.size(); i++) {
                row[i] = std::uniform_int_distribution<size_t>(0, vars[i].size() - 1)(rng);
            }
            coverage = 0;  // number of uncovered interactions
            for (size_t i = 1; i < vars.size(); i++) {
                for (size_t j = 0; j < i; j++) {
                    const size_t id = idx[{i, j, row[i], row[j]}];
                    coverage += uncovered.count(id);
                }
            }
        } while (coverage < expectedCoverage);

        // remove the now covered interactions from 'uncovered'
        for (size_t i = 1; i < vars.size(); i++) {
            for (size_t j = 0; j < i; j++) {
                const size_t id = idx[{i, j, row[i], row[j]}];
                uncovered.erase(id);
            }
        }

        Test test;
        for (size_t i = 0; i < vars.size(); i++) test.emplace_back(vars[i][row[i]]);
        result.emplace_back(std::move(test));
    }

    return result;
}


constexpr double unusedWeight = 5;
/*
 * greedy heuristic
 */
std::vector<Test> optCoveringArray(
    const size_t num,
    const std::vector<
        std::pair<AssignmentComparator, std::vector<std::shared_ptr<PropertyAssignment>>>>& vars) {
    std::default_random_engine rng(42);  // deterministic for regression testing

    if (vars.empty()) {
        return {};
    }

    // special case, same as in coveringarray(..)
    if (vars.size() == 1) {
        std::vector<Test> res(vars[0].second.size());
        for (size_t i = 0; i < vars[0].second.size(); i++) {
            res[i].emplace_back(vars[0].second[i]);
        }
        return res;
    }

    // a (partial) test maps variables each to an assignment of the variable
    using TestConf = std::map<size_t, size_t>;  // {var index, assignment index}

    // generate all possible interactions, as done in coveringarray(..)
    std::vector<TestConf> unused;
    for (size_t var = 0; var < vars.size(); var++) {
        for (size_t i = 0; i < vars[var].second.size(); i++) {
            for (size_t var2 = 0; var2 < var; var2++) {
                for (size_t i2 = 0; i2 < vars[var2].second.size(); i2++) {
                    unused.emplace_back(TestConf{{{var, i}, {var2, i2}}});
                }
            }
        }
    }
    // shuffle interactions for randomization (relevant if num is small)
    std::shuffle(unused.begin(), unused.end(), rng);

    // {TestConf, current num of other generated comparable tests}
    std::vector<std::pair<TestConf, size_t>> finished;

    // returns, whether the second TestConf is comparable to the first
    // TestConf (i.e. comparing them yields not NOT_COMPARABLE)
    // note: assumes that all keys of the second argument are also present in
    // the first
    const auto comparable = [&](const TestConf& ref, const TestConf& vs) {
        PropertyEffect res = PropertyEffect::ANY;
        for (const auto& [var, i] : vs) {
            const auto& j = ref.at(var);
            const auto expect = vars[var].first(vars[var].second[i], vars[var].second[j]);
            if (expect == PropertyEffect::NOT_COMPARABLE) return false;
            res = combine(expect, res);
            if (res == PropertyEffect::NOT_COMPARABLE) return false;
        }
        return true;
    };

    // removes all TestConfs from the second argument which are not comparable
    // to the second argument
    const auto filterComparables = [&](auto& comparables, const TestConf& test) {
        for (size_t i = 0; i < comparables.size(); i++) {
            if (!comparable(finished[comparables[i]].first, test)) {
                comparables[i] = std::move(comparables.back());
                comparables.pop_back();
                i--;
            }
        }
    };
    // combined score of finished comparables when merging 'ref' into 'gen'
    // where the score is 1 + the sum of
    // 1 + (current number of tests -  current number of tests comparable to i)* 2
    // for each test i that is comparable to 'gen' merged with 'ref'
    // (i.e. the score is increased when 'gen' merged with 'ref' is comparable
    // with many tests for which are comparable to few other tests)
    // if 'disjoint' is true, the score is 0 iff the assignments in gen and ref
	// to equal properties are not identical
    const auto cmp = [&](const bool disjoint, auto comparables, const auto& gen, const auto& ref) {
        if (disjoint) {
            if (gen.size() < ref.size()) {
                for (const auto& [k, v] : gen) {
					const auto it = ref.find(k);
					if(it != ref.end() && it->second != k) {
						return static_cast<size_t>(0);
					}
                }
            } else {
                for (const auto& [k, v] : ref) {
					const auto it = gen.find(k);
					if(it != gen.end() && it->second != k) {
						return static_cast<size_t>(0);
					}
                }
            }
        }
        auto test = ref;
        test.insert(gen.begin(), gen.end());

        filterComparables(comparables, test);
        size_t res = 1;
        for (const size_t i : comparables) res += 1 + (finished.size() - finished[i].second) * 2;
        return res;
    };

    // generate tests until we achieved a 2-coverage or reached the maximum
    // number of tests
    while (!unused.empty() && finished.size() < num) {
        // use an unused interaction as a base for the new test
        TestConf gen{{unused.back()}};
        unused.pop_back();

        // create a list of all generated comparable tests
        std::vector<size_t> comparables(finished.size());
        std::iota(comparables.begin(), comparables.end(), 0);
        filterComparables(comparables, gen);

        std::cerr << unused.size() << " unused, " << finished.size() << " finished" << std::endl;
        // while the current test does not contain assignments to all variables
        while (gen.size() < vars.size()) {
            double opt = -1;
			size_t idx;
            bool is_unused = false;

            // try unused interactions
            for (size_t i = 0; i < unused.size(); i++) {
                const double val = unusedWeight * cmp(true, comparables, gen, unused[i]);
                if (val > opt) opt = val, idx = i, is_unused = true;
            }
            // try finished tests
            for (size_t i = 0; i < finished.size(); i++) {
                const double val = cmp(false, comparables, gen, finished[i].first);
                if (val > opt) opt = val, idx = i, is_unused = false;
            }

            IVW_ASSERT(opt >= 0, "Fatal error while creating covering array");

            if (is_unused) {
                gen.insert(unused[idx].begin(), unused[idx].end());
                unused[idx] = std::move(unused.back());
                unused.pop_back();
            } else {
                gen.insert(finished[idx].first.begin(), finished[idx].first.end());
            }

            filterComparables(comparables, gen);
        }
        for (const size_t i : comparables) finished[i].second++;
        finished.emplace_back(gen, comparables.size());
    }

    // build tests
    std::vector<Test> res;
    for (const auto& [test, val] : finished) {
        Test tmp;
        for (const auto& [var, i] : test) tmp.emplace_back(vars[var].second[i]);
        res.emplace_back(tmp);
    }
    return res;
}

}  // namespace pbt

}  // namespace inviwo
