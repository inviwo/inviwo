#include "TopologicalFeatures.h"

#include <fstream>
#include <iostream>
#include <cassert>

#include "constants.h"

namespace contourtree {

TopologicalFeatures::TopologicalFeatures() { }

void TopologicalFeatures::loadDataFromArrays(const ContourTreeData &input_ctdata, const std::vector<uint32_t> &input_order, const std::vector<float> &input_weights, bool partition) {
    ctdata = input_ctdata;
    order = input_order;
    weights = input_weights;

    if(partition) {
        sim.setInput(&ctdata);
        sim.simplify(order,1,0,weights);
    }
}

void TopologicalFeatures::loadDataFromFile(std::string dataLocation, bool partition) {
    ctdata = ContourTreeData();
    ctdata.loadBinFile(dataLocation);

    // read order file
    // read meta data
    std::ifstream ip(dataLocation + ".order.dat");
    int64_t orderSize;
    ip >> orderSize;
    ip.close();

    order.resize(orderSize);
    weights.resize(orderSize);

    std::string binFile = dataLocation + ".order.bin";
    std::ifstream bin(binFile, std::ios::binary);
    bin.read((char *)order.data(),order.size() * sizeof(uint32_t));
    bin.read((char *)weights.data(),weights.size() * sizeof(float));
    bin.close();

    if(partition) {
        sim.setInput(&ctdata);
        sim.simplify(order,1,0,weights);
    }
}

void TopologicalFeatures::addFeature(SimplifyCT &sim, uint32_t bno, std::vector<Feature> &features, std::set<size_t> &featureSet) {
    Branch b1 = sim.branches.at(bno);
    Feature f;
    f.from = ctdata.nodeVerts[b1.from];
    f.to = ctdata.nodeVerts[b1.from];

    std::deque<size_t> queue;
    queue.push_back(bno);
    while(queue.size() > 0) {
        size_t b = queue.front();
        queue.pop_front();
        if(b != bno && featureSet.find(b) != featureSet.end()) {
            // this cannot happen
            assert(false);
        }
        featureSet.insert(b);
        Branch br = sim.branches.at(b);
        f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
        for(int i = 0;i < br.children.size();i ++) {
            uint32_t bc = br.children.at(i);
            queue.push_back(bc);
        }
    }
    features.push_back(f);
}

std::vector<Feature> TopologicalFeatures::getPartitionedExtremaFeatures(int topk, float th) {
    std::vector<Feature> features;

    std::set<size_t> featureSet;
    if(topk == -1) {
        topk = 0;
        for(int i = order.size() - 1;i >= 0 ;i --) {
            if(weights[i] > th) {
                topk ++;
                featureSet.insert(order[i]);
            } else {
                break;
            }
        }
    }
    if(topk == 0) topk = 1;

    for(int _i = 0;_i < topk;_i ++) {
        size_t i = order.size() - _i - 1;
        Branch b1 = sim.branches.at(order[i]);
        Feature f;
        f.from = ctdata.nodeVerts[b1.from];
        f.to = ctdata.nodeVerts[b1.to];

        size_t bno = order[i];
        std::deque<size_t> queue;
        queue.push_back(bno);
        while(queue.size() > 0) {
            size_t b = queue.front();
            queue.pop_front();
            if(b != bno && featureSet.find(b) != featureSet.end()) {
                continue;
            }
            Branch br = sim.branches.at(b);
            f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
            for(int i = 0;i < br.children.size();i ++) {
                int bc = br.children.at(i);
                queue.push_back(bc);
            }
        }
        features.push_back(f);
    }
    return features;
}

std::vector<Feature> TopologicalFeatures::getArcFeatures(int topk, float th) {
    SimplifyCT sim;
    sim.setInput(&ctdata);

    sim.simplify(order,topk,th,weights);

    std::vector<Feature> features;
    std::set<size_t> featureSet;
    for(size_t _i = 0;_i < sim.branches.size();_i ++) {
        if(sim.removed[_i]) {
            continue;
        }
        featureSet.insert(_i);
    }
    for(size_t _i = 0;_i < sim.branches.size();_i ++) {
        size_t i = _i;
        if(sim.removed[i]) {
            continue;
        }
        Branch b1 = sim.branches.at(i);
        Feature f;
        f.from = ctdata.nodeVerts[b1.from];
        f.to = ctdata.nodeVerts[b1.to];

        size_t bno = i;
        std::deque<size_t> queue;
        queue.push_back(bno);
        while(queue.size() > 0) {
            size_t b = queue.front();
            queue.pop_front();
            if(b != bno && featureSet.find(b) != featureSet.end()) {
                // this cannot happen
                assert(false);
            }
            Branch br = sim.branches.at(b);
            f.arcs.insert(f.arcs.end(),br.arcs.data(), br.arcs.data()+br.arcs.size());
            for(int i = 0;i < br.children.size();i ++) {
                int bc = br.children.at(i);
                queue.push_back(bc);
            }
        }
        features.push_back(f);
    }
    return features;
}

}
