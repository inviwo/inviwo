#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>

namespace inviwo {

namespace util {

	// 2-coverage, randomized discrete SLJ strategy
std::vector<Test> coveringArray(const Test& init, const std::vector<std::vector< std::shared_ptr<PropertyAssignment> >>& vars) {
	std::cerr << "coveringArray: vars.size() = " << vars.size() << std::endl;
	assert(vars.size() > 0);

	// special case
	if(vars.size() == 1) {
		std::vector<Test> res(vars[0].size(), init);
		for(size_t i = 0; i < vars[0].size(); i++) {
			res[i].emplace_back(vars[0][i]);
		}
		std::cerr << "special case : res.size() = " << res.size() << std::endl;
		return res;
	}

	const size_t v = [&](){
			size_t res = 0;
			for(const auto& v : vars) res = std::max(res, v.size());
			return res;
		}();

	std::unordered_set<size_t> uncovered;
	std::map< std::array<size_t,4>, size_t > idx;
	for(size_t i = 1; i < vars.size(); i++) {
		for(size_t j = 0; j < i; j++) {
			for(size_t ii = 0; ii < vars[i].size(); ii++) {
				for(size_t ji = 0; ji < vars[j].size(); ji++) {
					uncovered.insert(idx.size());
					idx[{i,j,ii,ji}] = idx.size();
				}
			}
		}
	}

	std::vector<std::vector<size_t>> coveringArray;
	while(!uncovered.empty()) {
		size_t expectedCoverage = (uncovered.size() + (v*v-1)) / (v*v); // expectedCoverage > 0
		size_t coverage;
		std::vector<size_t> row(vars.size());
		do {
			for(size_t i = 0; i < row.size(); i++)
				row[i] = rand() % vars[i].size();
			coverage = 0; // number of uncovered interactions
			for(size_t i = 1; i < vars.size(); i++) {
				for(size_t j = 0; j < i; j++) {
					size_t id = idx[{i,j,row[i],row[j]}];
					coverage += uncovered.count(id);
				}
			}
		} while(coverage < expectedCoverage);
		for(size_t i = 1; i < vars.size(); i++) {
			for(size_t j = 0; j < i; j++) {
				size_t id = idx[{i,j,row[i],row[j]}];
				uncovered.erase(id);
			}
		}
		coveringArray.emplace_back(row);
	}

	// contruct result
	std::vector<Test> res(coveringArray.size(), init);
	for(size_t c = 0; c < coveringArray.size(); c++) {
		for(size_t i = 0; i < vars.size(); i++) {
			res[c].emplace_back( vars[i][coveringArray[c][i]] );
		}
	}

	size_t naive = 0;
	for(size_t i = 1; i < vars.size(); i++)
		for(size_t j = 0; j < i; j++)
			naive += vars[i].size() * vars[j].size();
	std::cerr << "size reduction: " << naive << " => " << res.size() << std::endl;

	return res;
}

} // namespace util

} // namespace inviwo
