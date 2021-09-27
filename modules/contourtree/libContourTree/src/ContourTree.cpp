#include "ContourTree.h"
#include "MergeTree.h"
#include <vector>
#include <deque>
#include <cassert>
#include <fstream>
#include <iostream>

namespace contourtree {

ContourTree::ContourTree() {}

void ContourTree::setup(const MergeTree *tree) {
    std::cout << "setting up merge process" << std::endl;;
    this->tree = tree;
    nv = tree->data->getVertexCount();
    nodesJoin.resize(nv);
    nodesSplit.resize(nv);
    ctNodes.resize(nv);
    for(int64_t i = 0;i < nv;i ++) {
        // init
        nodesJoin[i].v = i;
        nodesSplit[i].v = i;
        ctNodes[i].v = i;

        // add join arcs
        int64_t to = i;
        int64_t from = tree->prev[to];
        if(from != -1) {
            nodesSplit[from].next.push_back(to);
            nodesSplit[to].prev.push_back(from);
        }

        // add split arcs
        to = tree->next[i];
        from = i;
        if(to != -1) {
            nodesJoin[from].next.push_back(to);
            nodesJoin[to].prev.push_back(from);
        }
    }
}

void ContourTree::computeCT() {
    std::cout << "merging join and split trees" << std::endl;;
    std::deque<int64_t> q;
    for(int64_t v = 0;v < nv;v ++) {
        Node &jn = nodesJoin[v];
        Node &sn = nodesSplit[v];
        if(sn.next.size() + jn.prev.size() == 1) {
            q.push_back(v);
        }
    }

    while(q.size() > 0) {
        int xi = q.front();
        q.pop_front();
        Node &jn = nodesJoin[xi];
        Node &sn = nodesSplit[xi];
        if(sn.next.size() == 0 && sn.prev.size() == 0) {
            assert((jn.next.size() == 0 && jn.prev.size() == 0));
            continue;
        }

        if(sn.next.size() == 0) {
            if(sn.prev.size() > 1) {
                std::cout << "Can this happen too???" << std::endl;
                assert(false);
            }
            int xj = sn.prev[0];
            remove(xi, nodesJoin);
            remove(xi, nodesSplit);

            int fr = xj;
            int to = xi;
            assert(fr < nv && to < nv);
            addArc(fr, to);
            if(nodesSplit[xj].next.size() + nodesJoin[xj].prev.size() == 1) {
                q.push_back(xj);
            }
        } else {
            if(jn.next.size() > 1) {
                std::cout << "Can this happen too???" << std::endl;
                assert(false);
            }
            int xj = jn.next[0];
            remove(xi, nodesJoin);
            remove(xi, nodesSplit);

            int fr = xi;
            int to = xj;
            assert(fr < nv && to < nv);
            addArc(fr, to);

            if(nodesSplit[xj].next.size() + nodesJoin[xj].prev.size() == 1) {
                q.push_back(xj);
            }
        }
    }
    
    // saving some memory
    nodesJoin.clear();
    nodesJoin.shrink_to_fit();
    nodesSplit.clear();
    nodesSplit.shrink_to_fit();
}

void ContourTree::generateArrays() {
    std::cout << "removing deg-2 nodes and computing segmentation" << std::endl;

    nodeids.clear();
    nodefns.clear();
    nodeTypes.clear();
    arcs.clear();
    arcMap.clear();

    arcMap.resize(nv, -1);

    arcNo = 0;
    for(int64_t i = 0;i < nv;i ++) {
        // go in sorted order
        int64_t v = tree->sv[i];
        // process only regular vertices
        if(ctNodes[v].prev.size() == 1 && ctNodes[v].next.size() == 1) {
            continue;
        }
        nodeids.push_back(v);
        nodefns.push_back(tree->data->getFunctionValue(v));
        nodeTypes.push_back(tree->criticalPts[v]);

        // create an arc for which this critical point is the source of the arc
        // traverse up for each of its arcs
        int64_t from = v;
        for(int i = 0;i < ctNodes[v].next.size();i ++) {
            int64_t vv = ctNodes[v].next[i];
            while(ctNodes[vv].prev.size() == 1 && ctNodes[vv].next.size() == 1) {
                // regular
                arcMap[vv] = arcNo;
                vv = ctNodes[vv].next[0];
            }
            arcMap[v] = arcNo;
            arcMap[vv] = arcNo;
            int64_t to = vv;
            // create arc (from, to)
            arcs.push_back(from);
            arcs.push_back(to);
            arcNo ++;
        }
    }
}

void ContourTree::writeToFile(const std::string fileName) { 
    // write meta data
    std::cout << "Writing meta data" << std::endl;
    {
        std::ofstream pr(fileName + ".rg.dat");
        pr << nodeids.size() << "\n";
        pr << arcNo << "\n";
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

void ContourTree::remove(int64_t xi, std::vector<ContourTree::Node> &nodeArray) {
    Node &jn = nodeArray[xi];

    if(jn.prev.size() == 1 && jn.next.size() == 1) {
        int64_t p = jn.prev[0];
        int64_t n = jn.next[0];
        Node &pn = nodeArray[p];
        Node &nn = nodeArray[n];

        removeAndAdd(pn.next, xi, nn.v);
        removeAndAdd(nn.prev, xi, pn.v);
    } else if(jn.prev.size() == 0 && jn.next.size() == 1) {
        int n = jn.next[0];
        Node &nn = nodeArray[n];
        remove(nn.prev, xi);
    } else if(jn.prev.size() == 1 && jn.next.size() == 0) {
        int p = jn.prev[0];
        Node &pn = nodeArray[p];
        remove(pn.next, xi);
    } else {
        assert(false);
    }
}

void ContourTree::removeAndAdd(std::vector<int64_t> &arr, int64_t rem, int64_t add) {
    for(int i = 0;i < arr.size();i ++) {
        if(arr[i] == rem) {
            arr[i] = add;
            return;
        }
    }
    assert(false);
}

void ContourTree::remove(std::vector<int64_t> &arr, int64_t xi) {
    for(int i = 0;i < arr.size();i ++) {
        if(arr[i] == xi) {
            if(i != arr.size() - 1) {
                arr[i] = arr[arr.size() - 1];
            }
            arr.pop_back();
            return;
        }
    }
    assert(false);
}

void ContourTree::addArc(int64_t from, int64_t to) {
    ctNodes[from].next.push_back(to);
    ctNodes[to].prev.push_back(from);
}


}
