#ifndef GRID3D_H
#define GRID3D_H

#include "ScalarFunction.h"
#include <stdint.h>
#include <vector>
#include <set>
#include <string>
#include <cassert>
#include <fstream>

namespace contourtree {

struct Tet {
    int64_t v[4];
};

template <class T>
class Grid3D : public ScalarFunction
{
public:
    Grid3D(int resx, int resy, int resz);

public:
    int getMaxDegree();
    int getVertexCount();
    int getStar(int64_t v, std::vector<int64_t> &star);
    bool lessThan(int64_t v1, int64_t v2);
    scalar_t getFunctionValue(int64_t v);

public:
    void loadGrid(std::string fileName);

protected:
    void updateStars();

public:
    int dimx, dimy, dimz;
    int nv;
    std::vector<Tet> tets;
    int starin[14][3];
    int64_t star[14];
    std::vector<T> fnVals;

protected:
    inline int64_t index(int64_t x, int64_t y, int64_t z) {
        return (x + y * dimx + z * dimx * dimy);
    }
};

template <class T>
Grid3D<T>::Grid3D(int resx, int resy, int resz) :
    dimx(resx), dimy(resy), dimz(resz)
{
    nv = dimx * dimy * dimz;
    this->updateStars();
}

template <class T>
int Grid3D<T>::getMaxDegree() {
    return 14;
}

template <class T>
int Grid3D<T>::getVertexCount() {
    return nv;
}

template <class T>
int Grid3D<T>::getStar(int64_t v, std::vector<int64_t> &star) {
    int z = v / (dimx * dimy);
    int rem = v % (dimx * dimy);
    int y = rem / dimx;
    int x = rem % dimx;

    int ct = 0;
    for(int i = 0;i < 14;i ++) {
        int _x = x + starin[i][0];
        int _y = y + starin[i][1];
        int _z = z + starin[i][2];
        if(_x < 0 || _x >= dimx ||
           _y < 0 || _y >= dimy ||
           _z < 0 || _z >= dimz) {
            continue;
        }
        star[ct ++] = (v + this->star[i]);
    }
    return ct;
}

template <class T>
bool Grid3D<T>::lessThan(int64_t v1, int64_t v2) {
    if(fnVals[v1] < fnVals[v2]) {
        return true;
    } else if(fnVals[v1] == fnVals[v2]) {
        return (v1 < v2);
    }
    return false;
}

template <class T>
scalar_t Grid3D<T>::getFunctionValue(int64_t v) {
    return (scalar_t) this->fnVals[v];
}

template <class T>
void Grid3D<T>::loadGrid(std::string fileName) {
    std::ifstream ip(fileName, std::ios::binary);
    this->fnVals.resize(nv);
    ip.read((char *)fnVals.data(),nv * sizeof(T));
    ip.close();
}

template <class T>
void Grid3D<T>::updateStars() {
    int ordering [][3] = {
        {0,1,2},
        {0,2,1},
        {1,0,2},
        {1,2,0},
        {2,0,1},
        {2,1,0}
    };

    Tet tet;
    tet.v[0] = index(0,0,0);
    for(int i = 0;i < 6;i ++) {
        int64_t loc[] = {0,0,0};
        for(int j = 0;j < 3;j ++) {
            loc[ordering[i][j]] ++;
            tet.v[j + 1] = index(loc[0],loc[1],loc[2]);
        }
        tets.push_back(tet);
    }


    ////
    int x = 1;
    int y = 1;
    int z = 1;
    int v = index(1,1,1);

    std::set<int64_t> unique;
    for(int xx = -1;xx < 1;xx ++) {
        int _x = x + xx;
        for(int yy = -1;yy < 1;yy ++) {
            int _y = y + yy;
            for(int zz = -1;zz < 1;zz ++) {
                int _z = z + zz;
                int vv = index(_x,_y,_z);
                for(int i = 0;i < 6;i ++) {
                    int in = -1;
                    for(int j = 0;j < 4;j ++) {
                        if(tets[i].v[j] + vv == v) {
                            in = j;
                            break;
                        }
                    }
                    if(in != -1)
                    for(int j = 0;j < 4;j ++) {
                        if(j != in) {
                            unique.insert(tets[i].v[j] + vv);
                        }
                    }
                }
            }
        }
    }
    assert(unique.size() == 14);

    int sct = 0;
    for(int64_t adj: unique) {
        z = adj / (dimx * dimy);
        int rem = adj % (dimx * dimy);
        y = rem / dimx;
        x = rem % dimx;

        starin[sct][0] = x - 1;
        starin[sct][1] = y - 1;
        starin[sct][2] = z - 1;
        star[sct] = adj - v;
        sct ++;
    }
}

} // namespace 

#endif // GRID3D_H
