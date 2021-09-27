#ifndef TOPOLOGICALFEATURES_HPP
#define TOPOLOGICALFEATURES_HPP

#include "SimplifyCT.h"
#include <string>
#include <set>

namespace contourtree {

struct Feature {
    std::vector<uint32_t> arcs;
    uint32_t from, to;
};

class TopologicalFeatures
{
public:
    TopologicalFeatures();

    void loadDataFromArrays(const ContourTreeData &input_ctdata, const std::vector<uint32_t> &input_order, const std::vector<float> &input_weights, bool partition = false);
    void loadDataFromFile(std::string dataLocation, bool partition = false);
    std::vector<Feature> getArcFeatures(int topk = -1, float th = 0);
    std::vector<Feature> getPartitionedExtremaFeatures(int topk = -1, float th = 0);

public:
    ContourTreeData ctdata;
    std::vector<uint32_t> order;
    std::vector<float> weights;

    // when completely partitioning branch decomposition
    std::vector<std::vector<uint32_t> > featureArcs;
    SimplifyCT sim;

private:
    void addFeature(SimplifyCT &sim, uint32_t bno, std::vector<Feature> &features, std::set<size_t> &featureSet);

};

}

#endif // TOPOLOGICALFEATURES_HPP
