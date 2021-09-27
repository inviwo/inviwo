#ifndef CONTOURTREEDATA_HPP
#define CONTOURTREEDATA_HPP

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <constants.h>
#include "MergeTree.h"

namespace contourtree {

struct Node {
    std::vector<uint32_t> next;
    std::vector<uint32_t> prev;
};

struct Arc {
    uint32_t from;
    uint32_t to;
    uint32_t id;
};

class ContourTreeData
{
public:
    ContourTreeData();
    ContourTreeData(const ContourTree &CT);
    ContourTreeData(const MergeTree &CT);

    void loadBinFile(std::string fileName);
    void loadTxtFile(std::string fileName);

protected:
    void loadData(const std::vector<int64_t>& nodeids, const std::vector<scalar_t>& nodefns, const std::vector<char>& nodeTypes, const std::vector<int64_t>& iarcs);

public:
    uint32_t noNodes;
    uint32_t noArcs;

    std::vector<Node> nodes;
    std::vector<Arc> arcs;
    std::vector<float> fnVals;
    std::vector<char> type;
    std::vector<int64_t> nodeVerts;

    std::unordered_map<int64_t, uint32_t> nodeMap;
};

} // namespace contourtree

#endif // CONTOURTREEDATA_HPP
