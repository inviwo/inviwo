#ifndef SIMPLIFYCT_HPP
#define SIMPLIFYCT_HPP

#include "ContourTreeData.h"
#include "SimFunction.h"
#include <queue>
#include <vector>

#if defined (WIN32)
#include <functional>
#endif

namespace contourtree {

class SimplifyCT;

struct BranchCompare {
    BranchCompare(){}
    BranchCompare(const SimplifyCT * simct): sim(simct) {}
    bool operator () (uint32_t v1, uint32_t v2);

    const SimplifyCT *sim;
};

class SimplifyCT
{
public:
    SimplifyCT();

    void setInput(ContourTreeData *data);
    void simplify(SimFunction *simFn);
    void simplify(const std::vector<uint32_t> &order, int topk = -1, float th = 0, const std::vector<float> &wts = std::vector<float>());
    void computeWeights();
    void writeToFile(const std::string fileName);

protected:
    void initSimplification(SimFunction *f);
    void addToQueue(uint32_t ano);
    bool isCandidate(const Branch &br);
    void removeArc(uint32_t ano);
    void mergeVertex(uint32_t v);

public:
    bool compare(uint32_t b1, uint32_t b2) const;

public:
    const ContourTreeData *data;
    std::vector<Branch> branches;
    std::vector<Node> nodes;

    std::vector<float> fn;
    std::vector<float> fnv;
    std::vector<bool> invalid;
    std::vector<bool> removed;
    std::vector<bool> inq;
    SimFunction *simFn;

    std::priority_queue<uint32_t,std::vector<uint32_t>,BranchCompare> queue;
    std::vector<uint32_t> order;
    std::vector<float> weights;
    std::vector<std::vector<uint32_t>> vArray;
};

}

#endif // SIMPLIFYCT_HPP
