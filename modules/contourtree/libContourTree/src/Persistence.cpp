#include "Persistence.h"

namespace contourtree {

Persistence::Persistence(const ContourTreeData &ctData) {
    fnVals = ctData.fnVals.data();
}

void Persistence::init(std::vector<float> &fn, std::vector<Branch> &br) {
    this->fn = fn.data();
    for(int i = 0;i < fn.size();i ++) {
        this->fn[i] = fnVals[br[i].to] - fnVals[br[i].from];
    }
}

void Persistence::update(const std::vector<Branch> &br, uint32_t brNo) {
    fn[brNo] = fnVals[br[brNo].to] - fnVals[br[brNo].from];
}

void Persistence::branchRemoved(std::vector<Branch>&, uint32_t, std::vector<bool>&) {
    // not required for persistence
}

float Persistence::getBranchWeight(uint32_t brNo) {
    return fn[brNo];
}

}
