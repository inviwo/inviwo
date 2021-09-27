#include <iostream>
#include <chrono>
#include <fstream>
#include <cmath>

#include <MergeTree.h>
#include <Grid3D.h>
#include "ContourTreeData.h"
#include "SimplifyCT.h"
#include "Persistence.h"
#include "TriMesh.h"
#include "TopologicalFeatures.h"
#include "HyperVolume.h"


using namespace contourtree;

/**
 * Example function that computes the contour tree of a 3D structured float grid of size dimx x dimy x dimz (stored row major, i.e., first along the x-direction, then y-, and finally z-direction).
 * @param dataName gives the path to the input grid, without the extension. (The extension is assumed to be .raw)
 *
 * This function also computes and stores the branch decomposition, with respect to either the persistence measure (default) or approximate hyper volume (when persistence is set to false)
 * The outputs are the following files:
 *      dataName.rg.dat: metadata about the contour tree (count of nodes and arcs)
 *      dataName.rg.bin: the contour tree itself (in binary format)
 *      dataName.part.raw: mapping of each vertex in the input to an arc in the contour tree
 *      dataName.order.dat: metadata about the branch decomposition
 *      dataName.order.bin: simplification order to generate the branch decomposition
 */
template <class T>
void exampleProcessing(std::string dataName, int dimx, int dimy, int dimz, bool persistence = true) {
    std::chrono::time_point<std::chrono::system_clock> start, end;
    Grid3D<T> grid(dimx,dimy,dimz);

    std::string data = dataName;

    start = std::chrono::system_clock::now();
    grid.loadGrid(data + ".raw");
    MergeTree ct;

    // Change to TypeJoinTree or TypeSplitTree for join and split tree computation respectively
    contourtree::TreeType tree = TypeContourTree;
    std::cout << "computing contour tree" << std::endl;
    ct.computeTree(&grid,tree);
    end = std::chrono::system_clock::now();
    std::cout << "Time to compute contour tree: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";
    ct.output(data, tree);


    std::cout << "creating hierarchical segmentation" << std::endl;
    // now simplify and store simplification hierarchy
    start = std::chrono::system_clock::now();
    ContourTreeData ctdata;
    ctdata.loadBinFile(data);

    SimplifyCT sim;
    sim.setInput(&ctdata);
    SimFunction *simFn;
    if(persistence) {
        simFn = new Persistence(ctdata);
    } else {
        simFn = new HyperVolume(ctdata,data + ".part.raw");
    }
    sim.simplify(simFn);
    end = std::chrono::system_clock::now();
    std::cout << "Time to simplify: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << "ms\n";

    sim.outputOrder(data);
    std::cout << "done" << std::endl;
}

/**
 * Example function demonstrating how to obtain features corresponding to a given simplification criteria
 * The simplification criteria is either provided as the top k features, or a threshold (which is used if topk = -1)
 *
 * Each Feature object, returned by the function, stores the contour tree arcs that correspond to the simplified feature.
 * The required vertices in the input corresponding to these features is obtained by the union of vertices in the
 * <dataName>.part.raw file having arc ids corresponding to the controu tree arcs from the Feature object.
 */
std::vector<Feature> exampleQuerying(std::string dataName, bool useArcs, int topk = -1, float threshold = 0) {
    TopologicalFeatures topoFeatures;

    std::vector<Feature> features;
    if(useArcs) {
        // get features corresponding to extremum-saddle pairs, i.e., the input is partitioned based on leaf arcs of the contour tree after simplification
        topoFeatures.loadData(dataName,false);
        features = topoFeatures.getArcFeatures(topk,threshold);
    } else {
        // get features corresponding to branches, i.e., the input is partitioned based on the branch decomposition after simplification
        topoFeatures.loadData(dataName,true);
        features = topoFeatures.getPartitionedExtremaFeatures(topk,threshold);
    }
    return features;
}

int main(int argc, char *argv[]) {
    return 0;
}
