/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/


#include <utility>
#include "../../include/inviwo/animationsplineinterpolation/interpolation/nurb.h"
#include "../../include/inviwo/animationsplineinterpolation/interpolation/nurbsinterpolation.h"


/**
 * To be used if the control points are already known
 * @param dim dimension of the nurb
 * @param curves vector containing the individual three dimensional nurbs
 */
Nurb::Nurb(int dim, std::vector<tinynurbs::Curve<3, float>> curves): _dim(dim), _curves(std::move(curves)) {

}

/**
 * To be used if the control points are unknown, will split the data in
 * three dimensional sets and compute the interpolation.
 * @param dim dimension of the desired nurb
 * @param points points used for the interpolation
 */
Nurb::Nurb(int dim, std::vector<std::vector<float>> points): _dim(dim){
    int nb_vecs = dim/3;
    if (dim % 3) {
        nb_vecs++;
    }
    std::vector<std::vector<glm::vec3>> vecs;

    for (int i = 0; i < nb_vecs; i++) {
        vecs.emplace_back();
        for (int j = 0; j < points.size(); j++) {
            if (!j%3) {
                vecs[i].push_back(glm::vec3());
            }
            vecs[i][j/3][j%3] = points[i][j];
        }
    }

    for (int i = 0; i < nb_vecs; i++) {
        tinynurbs::Curve<3, float> tmp;
        InterpolateCurveGlobalNoDeriv(vecs[i], tmp);
        _curves.push_back(tmp);
    }
}


/**
 * Collects the control point at the given rank from the individual 3-D nurbs
 * @param rank the rank of the control point in the nurbs
 * @return vector containing all the coordinates of the point
 */
std::vector<float> Nurb::element(int rank) {
    std::vector<float> ret;
    auto it = _curves.begin();
    auto end = _curves.end();
    while (it != end) {
        auto tmp = (*it).control_points[rank];
        ret.push_back(tmp[0]);
        ret.push_back(tmp[1]);
        ret.push_back(tmp[2]);
        it++;
    }
    while (ret.size() != _dim) {
        ret.pop_back();
    }
    return ret;
}


/**
 * Computes the point matching the given paramaeter
 * @param u
 * @return
 */
std::vector<float> Nurb::evaluate(float u) {
    std::vector<float> ret;
    auto it = _curves.begin();
    auto end = _curves.end();
    while (it != end) {
        auto tmp = tinynurbs::internal::curvePoint<3, float>((*it).degree, (*it).knots,(*it).control_points, u);
        ret.push_back(tmp[0]);
        ret.push_back(tmp[1]);
        ret.push_back(tmp[2]);
        it++;
    } 
    while (ret.size() != _dim) {
        ret.pop_back();
    }
    return ret;

}


