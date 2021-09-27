#include "TriMesh.h"

#include <fstream>
#include <iostream>
#include <set>
#include <algorithm>
#include <cassert>

namespace contourtree {

TriMesh::TriMesh()
{

}

int TriMesh::getMaxDegree() {
    return maxStar;
}

int TriMesh::getVertexCount() {
    return nv;
}

int TriMesh::getStar(int64_t v, std::vector<int64_t> &star) {
    int ct = 0;
    for(uint32_t vv: vertices[v].adj) {
        star[ct] = vv;
        ct ++;
    }
    return ct;
}

bool TriMesh::lessThan(int64_t v1, int64_t v2) {
    if(fnVals[v1] < fnVals[v2]) {
        return true;
    } else if(fnVals[v1] == fnVals[v2]) {
        return (v1 < v2);
    }
    return false;
}

scalar_t TriMesh::getFunctionValue(int64_t v) {
    return (scalar_t) this->fnVals[v];
}

void TriMesh::loadData(std::string fileName)
{
    std::ifstream ip(fileName);
    std::string s;
    ip >> s;
    int nt;
    ip >> nv >> nt;

    vertices.resize(nv);
    fnVals.resize(nv);
    for(int i = 0;i < nv;i ++) {
        float x,y,z,fn;
        ip >> x >> y >> z >> fn;
        fnVals[i] = fn;
    }
    for(int i = 0;i < nt;i ++) {
        int ps,v1,v2,v3;
        ip >> ps >> v1 >> v2 >> v3;
        assert(ps == 3);

        vertices[v1].adj.insert(v2);
        vertices[v1].adj.insert(v3);
        vertices[v2].adj.insert(v1);
        vertices[v2].adj.insert(v3);
        vertices[v3].adj.insert(v2);
        vertices[v3].adj.insert(v1);
    }

    maxStar = 0;
    for(int i = 0;i < nv;i ++) {
        maxStar = std::max(maxStar,(int)vertices[i].adj.size());
    }
    std::cout << maxStar << std::endl;
}

}
