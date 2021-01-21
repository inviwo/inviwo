#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>

#include <random>

namespace inviwo {

namespace pbt {

// 2-coverage, randomized discrete SLJ strategy
std::vector<Test> coveringArray(
		const std::vector<std::vector<std::shared_ptr<PropertyAssignment>>>& vars) {
    srand(42);  // deterministic for regression testing

    std::cerr << "coveringArray: vars.size() = " << vars.size() << std::endl;
    IVW_ASSERT(vars.size() > 0, "coveringArray: passed 0 variables");

    // special case
    if (vars.size() == 1) {
        std::vector<Test> res(vars[0].size());
        for (size_t i = 0; i < vars[0].size(); i++) res[i].emplace_back(vars[0][i]);
        std::cerr << "special case : res.size() = " << res.size() << std::endl;
        return res;
    }

    const size_t v = [&]() {
        size_t res = 0;
        for (const auto& v : vars) res = std::max(res, v.size());
        return res;
    }();

    std::unordered_set<size_t> uncovered;
    std::map<std::array<size_t, 4>, size_t> idx;
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

    std::vector<std::vector<size_t>> coveringArray;
    while (!uncovered.empty()) {
        size_t expectedCoverage =
            (uncovered.size() + (v * v - 1)) / (v * v);  // expectedCoverage > 0
        size_t coverage;
        std::vector<size_t> row(vars.size());
        do {
            for (size_t i = 0; i < row.size(); i++) row[i] = rand() % vars[i].size();
            coverage = 0;  // number of uncovered interactions
            for (size_t i = 1; i < vars.size(); i++) {
                for (size_t j = 0; j < i; j++) {
                    size_t id = idx[{i, j, row[i], row[j]}];
                    coverage += uncovered.count(id);
                }
            }
        } while (coverage < expectedCoverage);
        for (size_t i = 1; i < vars.size(); i++) {
            for (size_t j = 0; j < i; j++) {
                size_t id = idx[{i, j, row[i], row[j]}];
                uncovered.erase(id);
            }
        }
        coveringArray.emplace_back(row);
    }

    // contruct result
    std::vector<Test> res(coveringArray.size());
    for (size_t c = 0; c < coveringArray.size(); c++) {
        for (size_t i = 0; i < vars.size(); i++) res[c].emplace_back(vars[i][coveringArray[c][i]]);
    }

    size_t naive = 0;
    for (size_t i = 1; i < vars.size(); i++) {
        for (size_t j = 0; j < i; j++) naive += vars[i].size() * vars[j].size();
    }
    std::cerr << "size reduction: " << naive << " => " << res.size() << std::endl;

    return res;
}

std::vector<Test> optCoveringArray(
    const size_t num,
    const std::vector<std::pair<std::function<std::optional<PropertyEffect>(
                                    const std::shared_ptr<PropertyAssignment>& oldVal,
                                    const std::shared_ptr<PropertyAssignment>& newVal)>,
                                std::vector<std::shared_ptr<PropertyAssignment>>>>& vars) {
    srand(42);  // deterministic for regression testing

    IVW_ASSERT(vars.size() > 0, "optCoveringArray: passed 0 variables");

    // special case
    if (vars.size() == 1) {
        std::vector<Test> res(vars[0].second.size());
        for (size_t i = 0; i < vars[0].second.size(); i++) res[i].emplace_back(vars[0].second[i]);
        return res;
    }

    using TestConf = std::map<size_t, size_t>;  // {var, var_idx}, vars[var][var_idx]
    std::vector<TestConf> unused;

    // init
    for (size_t var = 0; var < vars.size(); var++) {
        for (size_t i = 0; i < vars[var].second.size(); i++) {
            for (size_t var2 = 0; var2 < var; var2++) {
                for (size_t i2 = 0; i2 < vars[var2].second.size(); i2++) {
                    unused.emplace_back(std::map<size_t, size_t>{{{var, i}, {var2, i2}}});
				}
            }
		}
    }
    std::shuffle(unused.begin(), unused.end(), std::default_random_engine(rand()));

    std::vector<std::pair<TestConf, size_t>>
        finished;  // {TestConf, current num of other finished comparables}

    size_t comparisons = 0;  // just for debugging

    // note: assumes that all keys of the second argument are also present in
    // the first
    const std::function<bool(const TestConf&, const TestConf&)> comparable = [&](const auto& ref,
                                                                                 const auto& vs) {
        std::optional<PropertyEffect> res = {PropertyEffect::ANY};
        for (const auto& [var, i] : vs) {
            const auto& j = ref.at(var);
            const auto& expect = vars[var].first(vars[var].second[i], vars[var].second[j]);
            comparisons++;
            if (!expect) return false;
            res = combine(*expect, *res);
            if (!res) return false;
        }
        return true;
    };

    const std::function<std::vector<size_t>(std::vector<size_t>, const TestConf&)>
        filterComparables = [&](auto comparables, const auto& test) {
            for (size_t i = 0; i < comparables.size(); i++) {
                if (!comparable(finished[comparables[i]].first, test)) {
                    comparables[i] = comparables.back();
                    comparables.pop_back();
                    i--;
                }
            }
            return comparables;
        };
    // combined score of finished comparables when merging ref into gen
    const std::function<size_t(const bool, const std::vector<size_t>&, const TestConf&,
                               const TestConf&)>
        cmp = [&](const bool disjoint, const auto& comparables, const auto& gen, const auto& ref) {
            if (disjoint) {
                for (const auto& [k, v] : gen) {
                    if (ref.count(k) > 0) return static_cast<size_t>(0);
                }
            }
            auto test = ref;
            test.insert(gen.begin(), gen.end());

            size_t res = 1;
            for (const size_t i : filterComparables(comparables, test))
                res += 1 + (finished.size() - finished[i].second) * 2;
            return res;
        };

    while (!unused.empty() && finished.size() < num) {
        TestConf gen{{unused.back()}};
        unused.pop_back();
        std::vector<size_t> comparables(finished.size());
        std::iota(comparables.begin(), comparables.end(), 0);

        std::cerr << unused.size() << std::endl;

        while (gen.size() < vars.size()) {
            int opt = -1, idx;
            bool is_unused = false;

            for (size_t i = 0; i < unused.size(); i++) {
                const int val = cmp(true, comparables, gen, unused[i]);
                if (val > opt) opt = val, idx = i, is_unused = true;
            }
            for (size_t i = 0; i < finished.size(); i++) {
                const int val = cmp(false, comparables, gen, finished[i].first);
                if (val > opt) opt = val, idx = i, is_unused = false;
            }

            IVW_ASSERT(opt != -1, "Fatal error while creating covering array");

            if (is_unused) {
                gen.insert(unused[idx].begin(), unused[idx].end());
                swap(unused[idx], unused.back());
                unused.pop_back();
            } else {
                gen.insert(finished[idx].first.begin(), finished[idx].first.end());
            }

            comparables = filterComparables(comparables, gen);
        }
        for (const size_t i : comparables) finished[i].second++;
        finished.emplace_back(gen, comparables.size());
    }

    std::cerr << comparisons << " comparisons" << std::endl;

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
