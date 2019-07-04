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

#ifndef IVW_NURB_UTILITIES_H
#define IVW_NURB_UTILITIES_H

#include "../../../../ext/tinynurbs/tinynurbs/tinynurbs.h"
//#include <glm/gtc/constants.hpp>
//#include <cmath>
#include <glm/glm.hpp>

/* Solves a triangular system of linear equations using the Thomas Algorithm.

    Vectors a,b,c represent the lower, main, and upper diagonal, respectively.
    Vector x represents the right-hand side and will be overwritten with the solution.
*/
template <typename T>
void SolveTridiagonalSystem(const std::vector<double>& a, const std::vector<double>& b, const std::vector<double>& c,
                            std::vector<T>& x)
{
    //Allocate scratch space
    const int N = a.size();
    std::vector<double> cprime(N);

    cprime[0] = c[0] / b[0];
    x[0] = x[0] / b[0];

    // loop from 1 to N - 1 inclusive
    for (int ix = 1; ix < N; ix++)
    {
        const double m = 1.0f / (b[ix] - a[ix] * cprime[ix - 1]);
        cprime[ix] = c[ix] * m;
        x[ix] = (x[ix] - a[ix] * x[ix - 1]) * m;
    }

    // loop from N - 2 to 0 inclusive
    for (int ix = N - 2; ix >= 0; ix--)
    {
        x[ix] -= cprime[ix] * x[ix + 1];
    }
}


template<typename value_type, int dim>
void GetInterpolatingNaturalCubicSpline(const std::vector<glm::vec<dim, value_type>>& InPoints,
                                        const std::vector<double>& CurveParameter,
                                        tinynurbs::Curve<dim, value_type>& ResCurve)
{
    //Types
    using VecType = typename glm::vec<dim, value_type>;

    //Let us use the classic variables, so that our math conforms to the book
    const int n = (int)InPoints.size() - 1;
    const int degree = 3; //We choose a piecewise cubic b-spline.
    const int p = degree;

    if (n < 1) return;

    //Shorthand for the parametrization
    const std::vector<double>& u = CurveParameter;

    //Knot Vector U: interpolation of input points occurs at the knots
    const int m = n+p+3; //Two additional knots compared to the spline interpolation without derivatives
    std::vector<double> Knots(m+1);
    for(int i(0);i<=p;i++)
    {
        Knots[i] = 0;
        Knots[m-i] = 1;
    }
    for(int j(1);j<=n-1;j++)
    {
        Knots[j+p] = u[j];
    }
    // - shorthand
    const std::vector<double>& U = Knots;

    /////////////////////////////////////////////
    //Coefficient matrix with n+3 equations.
    //It is a tridiagonal system:
    // a represents the diagonal below b.
    // b represents the main diagonal.
    // c represents the diagonal above b.
    std::vector<double> a(n+3, 0), b(n+3, 0), c(n+3, 0);

    //Interpolate the endpoints
    b[0] = 1;
    b[n+2] = 1;
    //Each inner input point is constructed from 3 control points.
    // Only 3 points, since we evaluate directly at an interior knot.
    // The fourth basis function is always zero then.
    for(int i(1);i<n;i++)
    {
        const int idxSpan = tinynurbs::findSpan<double>(degree, U, u[i]);
        auto Basis = tinynurbs::bsplineBasis<double>(degree, idxSpan, U, u[i]);
        a[i + 1] = Basis[0];
        b[i + 1] = Basis[1];
        c[i + 1] = Basis[2];
    }

    if (n >= 2)
    {
        //Natural end condition: second derivative is zero at the endpoints, meaning the curve becomes a straight line there.
        // That is perfect for continuing any animation with a linear interpolation.
        a[1] = u[2] - u[0];
        b[1] = -(u[2] - u[0]) - (u[1] - u[0]);
        c[1] = u[1] - u[0];
        a[n+1] = u[n] - u[n-1];
        b[n+1] = -(u[n] - u[n-1]) - (u[n] - u[n-2]);
        c[n+1] = u[n] - u[n-2];
    }
    else
    {
        //We have only two input points. Keep it a straight line.
        a[1] = -1;
        b[1] = 1;
        b[n+1] = -1;
        c[n+1] = 1;
    }


    /////////////////////////////////////////////
    //Right-hand side
    ResCurve.control_points.resize(n+3);
    std::vector<VecType>& Solution = ResCurve.control_points;
    //Solution vector contains RightHandSide at first and will be overwritten by SolveTridiagonalSystem() 
    int RowOffset(0);
    for(int i(0);i<=n;i++)
    {
        if (i == 1) RowOffset++;
        if (i == n) RowOffset++;
        Solution[i + RowOffset] = InPoints[i];
    }
    if (n >= 2)
    {
        //Natural end condition
        Solution[1] = VecType(0);
        Solution[n+1] = VecType(0);
    }
    else
    {
        //Keep it a straight line with equidistant sampling.
        Solution[1] = (U[p+1] / p) * (InPoints[1] - InPoints[0]);
        Solution[n+1] = (1 - U[m-p-1]) / p * (InPoints[1] - InPoints[0]);
    }

    //Solve!
    SolveTridiagonalSystem(a, b, c, Solution);

    //Set up the actual curve
    ResCurve.knots = U;
    ResCurve.degree = degree;
}

#endif //IVW_NURB_UTILITIES_H
