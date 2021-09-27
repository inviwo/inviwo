#include "ContourTreeData.h"

#include <fstream>
#include <iostream>
#include <cassert>
#include "constants.h"

namespace contourtree {

ContourTreeData::ContourTreeData() {

}

ContourTreeData::ContourTreeData(const ContourTree &CT) {
    noNodes = CT.nodeids.size();
    noArcs = CT.arcNo;
    this->loadData(CT.nodeids, CT.nodefns, CT.nodeTypes, CT.arcs);  
}

ContourTreeData::ContourTreeData(const MergeTree &MT) {
    noNodes = MT.noNodes;
    noArcs = MT.noArcs;
    this->loadData(MT.nodeids, MT.nodefns, MT.nodeTypes, MT.arcs);
}

void ContourTreeData::loadBinFile(std::string fileName) {
    // read meta data
    {
        std::ifstream ip(fileName + ".rg.dat");
        ip >> noNodes;
        ip >> noArcs;
        assert(noNodes == noArcs + 1);
        ip.close();
    }
    std::cout << noNodes << " " << noArcs << std::endl;

    std::vector<int64_t> nodeids(noNodes);
    std::vector<scalar_t> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    // read the tree
    std::string rgFile = fileName + ".rg.bin";
    std::ifstream ip(rgFile, std::ios::binary);
    ip.read((char *)nodeids.data(),nodeids.size() * sizeof(int64_t));
    ip.read((char *)nodefns.data(),nodeids.size() * sizeof(scalar_t));
    ip.read((char *)nodeTypes.data(),nodeids.size());
    ip.read((char *)arcs.data(),arcs.size() * sizeof(int64_t));
    ip.close();

    std::cout << "finished reading data" << std::endl;
    this->loadData(nodeids,nodefns,nodeTypes,arcs);
}

void ContourTreeData::loadTxtFile(std::string fileName) {
    std::ifstream ip(fileName);
    ip >> noNodes;
    ip >> noArcs;

    std::vector<int64_t> nodeids(noNodes);
    std::vector<scalar_t> nodefns(noNodes);
    std::vector<char> nodeTypes(noNodes);
    std::vector<int64_t> arcs(noArcs * 2);

    for(size_t i = 0;i < noNodes;i ++) {
        int64_t v;
        float fn;
        ip >> v;
        ip >> fn;
        char t;
        std::string type;
        ip >> type;
        if(type.compare("MINIMA") == 0) {
            t = MINIMUM;
        } else if(type.compare("MAXIMA") == 0) {
            t = MAXIMUM;
        } else if(type.compare("SADDLE") == 0) {
            t = SADDLE;
        } else {
            t = REGULAR;
        }
        nodeids[i] = v;
        nodefns[i] = (scalar_t)(fn);
        nodeTypes[i] = t;
    }
    for(size_t i = 0;i < noArcs;i ++) {
        int v1, v2;
        ip >> v1 >> v2;
        arcs[i * 2 + 0] = v1;
        arcs[i * 2 + 1] = v2;
    }
    ip.close();
    std::cout << "finished reading data" << std::endl;
    this->loadData(nodeids,nodefns, nodeTypes,arcs);
}

void ContourTreeData::loadData(const std::vector<int64_t> &nodeids, const std::vector<scalar_t> &nodefns, const std::vector<char> &nodeTypes, const std::vector<int64_t> &iarcs) {
    nodes.resize(noNodes);
    nodeVerts.resize(noNodes);
    fnVals.resize(noNodes);
    type.resize(noNodes);
    arcs.resize(noArcs);

    scalar_t minf = nodefns[0];
    scalar_t maxf = nodefns[noNodes - 1];
    for(uint32_t i = 0;i < noNodes;i ++) {
        assert(nodefns[i] >= minf && nodefns[i] <= maxf);
        nodeVerts[i] = nodeids[i];
        fnVals[i] = (float)(nodefns[i] - minf) / (maxf - minf);
        type[i] = nodeTypes[i];
        nodeMap[nodeVerts[i]] = i;
    }

    for(uint32_t i = 0;i < noArcs;i ++) {
        arcs[i].from = nodeMap[iarcs[i * 2 + 0]];
        arcs[i].to = nodeMap[iarcs[i * 2 + 1]];
        arcs[i].id = i;
        nodes[arcs[i].from].next.push_back(i);
        nodes[arcs[i].to].prev.push_back(i);
    }
}

} // namespace
