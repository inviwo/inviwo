#include "MergeTree.h"

#include <chrono>
#include <fstream>
#include <iostream>

#if !defined (WIN32)
#include <parallel/algorithm>
#else
#include <algorithm>
#endif

namespace contourtree {

MergeTree::MergeTree()
{
    newRoot = 0;
}

void MergeTree::computeTree(ScalarFunction* data, TreeType type) {
    this->data = data;
    std::chrono::time_point<std::chrono::system_clock> ct, en;
    ct = std::chrono::system_clock::now();
    setupData();
    orderVertices();
    switch(type) {
    case TypeContourTree:
        computeJoinTree();
        nodes = DisjointSets<int64_t>(noVertices);
        computeSplitTree();
        ctree.setup(this);
        ctree.computeCT();
        break;

    case TypeSplitTree:
        computeSplitTree();
        break;

    case TypeJoinTree:
        computeJoinTree();
        break;

    default:
        std::cout << "Invalid tree type" << std::endl;
        assert(false);
    }

    en = std::chrono::system_clock::now();
    long time = std::chrono::duration_cast<std::chrono::milliseconds>(en-ct).count();

    std::cout << "Time taken to compute tree : " << time << "ms" << std::endl;
}

void MergeTree::setupData() {
    std::cout << "setting up data" << std::endl;
    maxStar = data->getMaxDegree();
    star.resize(maxStar);

    noVertices = data->getVertexCount();
    newVertex = false;

    criticalPts.resize(noVertices, REGULAR);
    sv.resize(noVertices);
    prev.resize(noVertices,-1);
    next.resize(noVertices,-1);
    cpMap.resize(noVertices + 1);
    for(int64_t i = 0;i < noVertices;i ++) {
        sv[i] = i;
    }
    nodes = DisjointSets<int64_t>(noVertices);
}

void MergeTree::orderVertices() {
    std::cout << "ordering vertices" << std::endl;
#if defined (WIN32)
    std::sort(sv.begin(),sv.end(),Compare(data));
#else
    __gnu_parallel::sort(sv.begin(),sv.end(),Compare(data));
#endif
}

void MergeTree::computeJoinTree() {
    std::cout << "computing join tree" << std::endl;
    int64_t ct = 0;
    for(int64_t i = noVertices - 1;i >= 0; i --) {
        if(ct % 1000000 == 0) {
            std::cout << "processing vertex " <<  ct << " of " << noVertices << std::endl;
        }
        ct ++;

        int64_t v = sv[i];
        processVertex(v);
    }
    int64_t in = 0;
    if(criticalPts[sv[in]] == SADDLE) {
        // add a new vertex
        newVertex = true;
    } else {
        criticalPts[sv[in]] = MINIMUM;
    }
    newRoot = in;
}


void MergeTree::computeSplitTree() {
    std::cout << "computing split tree" << std::endl;
    int64_t ct = 0;
    for(int64_t i = 0;i < noVertices; i ++) {
        if(ct % 1000000 == 0) {
            std::cout << "processing vertex " <<  ct << " of " << noVertices << std::endl;
        }
        ct ++;

        int64_t v = sv[i];
        processVertexSplit(v);
    }
    int64_t in = noVertices - 1;
    if(criticalPts[sv[in]] == SADDLE) {
        // add a new vertex
        newVertex = true;
    } else {
        criticalPts[sv[in]] = MAXIMUM;
    }
    newRoot = in;
}

void MergeTree::generateArrays(TreeType tree)
{
    if(tree == TypeContourTree) {
        ctree.generateArrays();
        return;
    }
    // assume size of contour tree fits within 4 bytes
    std::vector<uint32_t> arcMap;
    if(newVertex) {
        arcMap.resize(noVertices + 1,-1);
    } else {
        arcMap .resize(noVertices,-1);
    }
    noArcs = 0;
    noNodes = 0;
    for(int64_t i = 0;i < noVertices;i ++) {
        if(criticalPts[i] != REGULAR) {
            noNodes ++;
        }
    }
    if(newVertex) {
        noNodes ++;
    }
    noArcs = noNodes - 1;

    std::cout << ("Creating required memory!") << std::endl;
    nodeids.resize(noNodes);
    nodefns.resize(noNodes);
    nodeTypes.resize(noNodes);
    arcs.resize(noArcs * 2);

    std::vector<int64_t> arcFrom(noNodes);
    std::vector<int64_t> arcTo(noNodes);

    std::cout << "Generating tree" << std::endl;
    int nct = 0;
    scalar_t minf = data->getFunctionValue(sv[0]);
    scalar_t maxf = data->getFunctionValue(sv[noVertices-1]);
    if(newVertex) {
        if(tree == TypeJoinTree){
            nodeids[nct] = noVertices;
            nodefns[nct] = minf;
            nodeTypes[nct] = MINIMUM;
            nct ++;
        }
    }
    for(int i = 0;i < noVertices;i ++) {
        if(criticalPts[sv[i]] != REGULAR) {
            nodeids[nct] = sv[i];
            nodefns[nct] = data->getFunctionValue(sv[i]);
            nodeTypes[nct] = criticalPts[sv[i]];
            nct ++;
        }
    }
    if(newVertex) {
        if(tree != TypeJoinTree){
            nodeids[nct] = noVertices;
            nodefns[nct] = maxf;
            nodeTypes[nct] = MAXIMUM;
            nct ++;
        }
    }

    if(tree == TypeJoinTree) {
        uint32_t arcNo = 0;
        for(int64_t i = 0;i < noVertices;i ++) {
            if((criticalPts[i] == MAXIMUM || criticalPts[i] == SADDLE) && i != sv[0]) {
                arcMap[i] = arcNo;

                int64_t to = i;
                int64_t from = prev[to];

                while(criticalPts[from] == REGULAR) {
                    arcMap[from] = arcNo;
                    from = prev[from];
                }
                arcMap[from] = arcNo;

                arcs[arcNo * 2 + 0] = from;
                arcs[arcNo * 2 + 1] = to;
                if(criticalPts[i] == SADDLE) {
                    arcFrom[arcNo] = from;
                    arcTo[arcNo] = to;
                }
                arcNo ++;
            }
        }
        if(newVertex) {
            int to = sv[0];
            int from = noVertices;
            arcs[arcNo * 2 + 0] = from;
            arcs[arcNo * 2 + 1] = to;
            arcMap[to] = arcMap[from] = arcNo ++;
        }
        assert(arcNo == noArcs);
    } else {
        uint32_t arcNo = 0;
        for(int64_t i = 0;i < noVertices;i ++) {
            if((criticalPts[i] == MINIMUM || criticalPts[i] == SADDLE) && i != sv[sv.size() - 1]) {
                arcMap[i] = arcNo;

                int64_t from = i;
                int64_t to = next[from];

                while(criticalPts[to] == REGULAR) {
                    arcMap[to] = arcNo;
                    to = next[to];
                }
                arcMap[to] = arcNo;

                arcs[arcNo * 2 + 0] = from;
                arcs[arcNo * 2 + 1] = to;
                if(criticalPts[i] == SADDLE) {
                    arcFrom[arcNo] = from;
                    arcTo[arcNo] = to;
                }
                arcNo ++;
            }
        }
        if(newVertex) {
            int from = sv[sv.size() - 1];
            int to = noVertices;
            arcs[arcNo * 2 + 0] = from;
            arcs[arcNo * 2 + 1] = to;
            arcMap[to] = arcMap[from] = arcNo ++;
        }
        assert(arcNo == noArcs);
    }
}

void MergeTree::writeToFile(const std::string fileName, TreeType tree){
    if(tree == TypeContourTree) {
        ctree.writeToFile(fileName);
        return;
    }
    // write meta data
    std::cout << "Writing meta data" << std::endl;
    {
        std::ofstream pr(fileName + ".rg.dat");
        pr << noNodes << "\n";
        pr << noArcs << "\n";
        pr.close();
    }
    std::cout << "writing tree output" << std::endl;
    std::string rgFile = fileName + ".rg.bin";
    std::ofstream of(rgFile,std::ios::binary);
    of.write((char *)nodeids.data(),nodeids.size() * sizeof(int64_t));
    of.write((char *)nodefns.data(),nodeids.size() * sizeof(scalar_t));
    of.write((char *)nodeTypes.data(),nodeids.size());
    of.write((char *)arcs.data(),arcs.size() * sizeof(int64_t));
    of.close();

    std::cout << "writing partition" << std::endl;
    std::string rawFile = fileName + ".part.raw";
    of.open(rawFile, std::ios::binary);
    of.write((char *)arcMap.data(), arcMap.size() * sizeof(uint32_t));
    of.close();
}

void MergeTree::processVertex(int64_t v) {
    int starct = data->getStar(v, star);
    if(starct == 0) {
        return;
    }
    set.clear();
    for(int x = 0;x < starct; x++) {
        int64_t tin = star[x];
        if(data->lessThan(v,tin)) {
            // upperLink
            int64_t comp = nodes.find(tin);
            set.insert(comp);
        }
    }
    if(set.size() == 0) {
        // Maximum
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
        criticalPts[v] = MAXIMUM;
    } else {
        if(set.size() > 1) {
            criticalPts[v] = SADDLE;
        }
        for(int64_t comp: set) {
            int64_t to = cpMap[comp];
            int64_t from = v;
            prev[to] = from;
            nodes.merge(comp, v);
        }
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
    }
}

void MergeTree::processVertexSplit(int64_t v) {
    int starct = data->getStar(v, star);
    if(starct == 0) {
        return;
    }
    set.clear();
    for(int x = 0;x < starct; x++) {
        int64_t tin = star[x];
        if(!(data->lessThan(v,tin))) {
            // lowerLink
            int64_t comp = nodes.find(tin);
            set.insert(comp);
        }
    }
    if(set.size() == 0) {
        // Minimum
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
        criticalPts[v] = MINIMUM;
    } else {
        if(set.size() > 1) {
            criticalPts[v] = SADDLE;
        }
        for(int64_t comp: set) {
            int64_t from = cpMap[comp];
            int64_t to = v;
            next[from] = to;
            nodes.merge(comp, v);
        }
        int64_t comp = nodes.find(v);
        cpMap[comp] = v;
    }
}

}
