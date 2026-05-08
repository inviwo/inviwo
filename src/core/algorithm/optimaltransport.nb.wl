(* ::Package:: *)
(* Optimal Transport Interpolation for Piecewise-Linear Transfer Functions *)
(* Mathematica translation of optimaltransport.cpp *)

(* A TF point is {pos, {r, g, b, a}} *)

eps = 10^-12;

(* --- Sanitize: sort by position, remove duplicates --- *)
sanitize[tf_List] := Module[{sorted, result},
  sorted = SortBy[tf, First];
  result = {First[sorted]};
  Do[
    If[Abs[pt[[1]] - Last[result][[1]]] > eps,
      AppendTo[result, pt],
      (* Duplicate position: keep the last one *)
      result[[-1]] = pt
    ],
    {pt, Rest[sorted]}
  ];
  result
]

(* --- Evaluate piecewise-linear TF at position x --- *)
evaluateTF[tf_List, x_] := Module[{},
  If[tf === {}, Return[{0, 0, 0, 0}]];
  If[x <= tf[[1, 1]], Return[tf[[1, 2]]]];
  If[x >= tf[[-1, 1]], Return[tf[[-1, 2]]]];
  (* Find interval *)
  Module[{i, p0, p1, dx, u},
    i = LengthWhile[tf, #[[1]] <= x &];
    If[i < 1, i = 1];
    If[i >= Length[tf], i = Length[tf] - 1];
    p0 = tf[[i]];
    p1 = tf[[i + 1]];
    dx = p1[[1]] - p0[[1]];
    If[Abs[dx] < eps, Return[(p0[[2]] + p1[[2]])/2]];
    u = (x - p0[[1]])/dx;
    (1 - u) p0[[2]] + u p1[[2]]
  ]
]

(* --- Linear blend of two TFs --- *)
linearBlend[tfA_List, tfB_List, t_] := Module[{positions, result},
  positions = Union[tfA[[All, 1]], tfB[[All, 1]], 
    SameTest -> (Abs[#1 - #2] < eps &)];
  Table[{x, (1 - t) evaluateTF[tfA, x] + t evaluateTF[tfB, x]}, {x, positions}]
]

(* --- Compute CDF: list of {pos, cumulativeMass, alpha, {r,g,b}} --- *)
computeCdf[tf_List] := Module[{points, cumulative = 0, totalMass},
  If[tf === {}, Return[<|"points" -> {}, "totalMass" -> 0|>]];
  points = {{tf[[1, 1]], 0, Max[0, tf[[1, 2, 4]]], tf[[1, 2, 1 ;; 3]]}};
  Do[
    Module[{x0, x1, dx, a0, a1, area},
      x0 = tf[[i - 1, 1]];
      x1 = tf[[i, 1]];
      dx = x1 - x0;
      If[dx <= eps, Continue[]];
      a0 = Max[0, tf[[i - 1, 2, 4]]];
      a1 = Max[0, tf[[i, 2, 4]]];
      area = 0.5 (a0 + a1) dx;
      cumulative += area;
      AppendTo[points, {tf[[i, 1]], cumulative, a1, tf[[i, 2, 1 ;; 3]]}];
    ],
    {i, 2, Length[tf]}
  ];
  totalMass = cumulative;
  <|"points" -> points, "totalMass" -> totalMass|>
]

(* --- Support boundaries --- *)
supportMin[cdf_] := Module[{pts = cdf["points"]},
  If[pts === {}, Return[0]];
  Do[
    If[pts[[i, 2]] - pts[[i - 1, 2]] > eps, Return[pts[[i - 1, 1]]]],
    {i, 2, Length[pts]}
  ];
  pts[[1, 1]]
]

supportMax[cdf_] := Module[{pts = cdf["points"]},
  If[pts === {}, Return[0]];
  Do[
    If[pts[[i, 2]] - pts[[i - 1, 2]] > eps, Return[pts[[i, 1]]]],
    {i, Length[pts], 2, -1}
  ];
  pts[[-1, 1]]
]

(* --- Quantile at a CDF point --- *)
quantileAtPoint[cdf_, pt_] := If[cdf["totalMass"] <= eps, 0,
  Clip[pt[[2]]/cdf["totalMass"], {0, 1}]
]

(* --- Add quantile levels from a CDF --- *)
addQuantileLevels[cdf_, samplesPerSegment_] := Module[{pts, levels = {}},
  pts = cdf["points"];
  If[pts === {} || cdf["totalMass"] <= eps, Return[{0, 1}]];
  Do[
    Module[{q0, q1},
      q0 = quantileAtPoint[cdf, pts[[i - 1]]];
      q1 = quantileAtPoint[cdf, pts[[i]]];
      AppendTo[levels, q0];
      If[q1 - q0 > eps,
        Do[
          Module[{u = s/samplesPerSegment},
            AppendTo[levels, (1 - u) q0 + u q1]
          ],
          {s, 1, samplesPerSegment - 1}
        ]
      ];
      AppendTo[levels, q1];
    ],
    {i, 2, Length[pts]}
  ];
  AppendTo[levels, 0];
  AppendTo[levels, 1];
  levels
]

(* --- Merged quantile levels from both CDFs --- *)
mergedQuantileLevels[cdfA_, cdfB_, samplesPerSegment_] := Module[{levels},
  levels = Join[
    addQuantileLevels[cdfA, samplesPerSegment],
    addQuantileLevels[cdfB, samplesPerSegment]
  ];
  levels = Union[levels, SameTest -> (Abs[#1 - #2] < eps &)];
  If[levels === {} || First[levels] > 0, PrependTo[levels, 0]];
  If[Last[levels] < 1, AppendTo[levels, 1]];
  levels
]

(* --- Solve quadratic for CDF inversion within a segment --- *)
solveSegmentInverse[a0_, a1_, dx_, localMass_] := Module[{slope, bigA, bigB, bigC, disc, r0, r1},
  If[localMass <= eps, Return[0]];
  slope = (a1 - a0)/dx;
  If[Abs[slope] < eps,
    If[a0 <= eps, Return[0]];
    Return[Clip[localMass/a0, {0, dx}]]
  ];
  bigA = 0.5 slope;
  bigB = a0;
  bigC = -localMass;
  disc = Max[0, bigB^2 - 4 bigA bigC];
  r0 = (-bigB + Sqrt[disc])/(2 bigA);
  r1 = (-bigB - Sqrt[disc])/(2 bigA);
  If[-eps <= r0 <= dx + eps, Return[Clip[r0, {0, dx}]]];
  If[-eps <= r1 <= dx + eps, Return[Clip[r1, {0, dx}]]];
  Clip[r0, {0, dx}]
]

(* --- Invert CDF: given quantile q, find position x --- *)
invertCdf[cdf_, q_] := Module[{pts, qc, targetMass, idx, p0, p1, segMass, dx, localMass, u},
  pts = cdf["points"];
  If[pts === {}, Return[0]];
  If[cdf["totalMass"] <= eps, Return[pts[[1, 1]]]];
  qc = Clip[q, {0, 1}];
  If[qc <= 0, Return[supportMin[cdf]]];
  If[qc >= 1, Return[supportMax[cdf]]];
  targetMass = qc * cdf["totalMass"];
  (* Binary search for the segment containing targetMass *)
  idx = LengthWhile[pts, #[[2]] < targetMass &] + 1;
  idx = Clip[idx, {2, Length[pts]}];
  p1 = pts[[idx]];
  p0 = pts[[idx - 1]];
  segMass = p1[[2]] - p0[[2]];
  If[segMass <= eps, Return[p0[[1]]]];
  dx = p1[[1]] - p0[[1]];
  If[dx <= eps, Return[p0[[1]]]];
  localMass = targetMass - p0[[2]];
  u = solveSegmentInverse[p0[[3]], p1[[3]], dx, localMass];
  p0[[1]] + u
]

(* --- Quantile point: position + color at quantile q --- *)
quantilePointFn[tf_, cdf_, q_] := Module[{x, color},
  x = invertCdf[cdf, q];
  color = evaluateTF[tf, x];
  {x, q, color[[1 ;; 3]]}  (* {pos, quantile, {r,g,b}} *)
]

(* ============================================================ *)
(* MAIN FUNCTION: Optimal Transport Interpolation               *)
(* ============================================================ *)
optimalTransportInterpolation[tfA_List, tfB_List, t_, samplesPerSegment_: 16] := 
Module[{tc, a, b, cdfA, cdfB, massA, massB, targetMass, levels, interpolatedCdf,
        vertices, n, density, alpha, maxDensity, gapThreshold,
        alphaAtStartA, alphaAtStartB, zeroAtStart,
        alphaAtEndA, alphaAtEndB, zeroAtEnd,
        actualMass, scale, domainMin, domainMax, result},
  
  tc = Clip[t, {0, 1}];
  a = sanitize[tfA];
  b = sanitize[tfB];
  
  If[a === {}, Return[b]];
  If[b === {}, Return[a]];
  If[tc <= 0, Return[a]];
  If[tc >= 1, Return[b]];
  
  cdfA = computeCdf[a];
  cdfB = computeCdf[b];
  
  massA = cdfA["totalMass"];
  massB = cdfB["totalMass"];
  targetMass = (1 - tc) massA + tc massB;
  
  (* Fall back to linear blend if mass is zero *)
  If[massA <= eps || massB <= eps || targetMass <= eps,
    Return[linearBlend[a, b, tc]]
  ];
  
  levels = mergedQuantileLevels[cdfA, cdfB, samplesPerSegment];
  If[Length[levels] < 2, Return[linearBlend[a, b, tc]]];
  
  (* Interpolate quantile functions *)
  interpolatedCdf = Table[
    Module[{qa, qb, x, color},
      qa = quantilePointFn[a, cdfA, q];
      qb = quantilePointFn[b, cdfB, q];
      x = (1 - tc) qa[[1]] + tc qb[[1]];
      color = (1 - tc) qa[[3]] + tc qb[[3]];
      {x, q, color}  (* {pos, quantile, {r,g,b}} *)
    ],
    {q, levels}
  ];
  
  (* Remove degenerate vertices (same position) *)
  vertices = {First[interpolatedCdf]};
  Do[
    If[Abs[pt[[1]] - Last[vertices][[1]]] <= eps,
      vertices[[-1]] = pt,
      AppendTo[vertices, pt]
    ],
    {pt, Rest[interpolatedCdf]}
  ];
  
  n = Length[vertices];
  If[n < 2, Return[linearBlend[a, b, tc]]];
  
  (* Compute interval densities *)
  density = Table[
    Module[{dx, dq},
      dx = vertices[[i + 1, 1]] - vertices[[i, 1]];
      dq = vertices[[i + 1, 2]] - vertices[[i, 2]];
      If[dx > eps && dq > eps, targetMass dq/dx, 0]
    ],
    {i, 1, n - 1}
  ];
  
  (* Gap threshold *)
  maxDensity = Max[density];
  gapThreshold = maxDensity * 10^-6;
  
  (* Assign vertex alphas *)
  alpha = ConstantArray[0., n];
  
  (* Interior vertices *)
  Do[
    Module[{leftIsGap, rightIsGap},
      leftIsGap = density[[i - 1]] <= gapThreshold;
      rightIsGap = density[[i]] <= gapThreshold;
      alpha[[i]] = Which[
        leftIsGap && rightIsGap, 0,
        leftIsGap, 0,  (* gap -> peak transition: ramp from zero *)
        rightIsGap, 0, (* peak -> gap transition: ramp to zero *)
        True, 0.5 (density[[i - 1]] + density[[i]])
      ]
    ],
    {i, 2, n - 1}
  ];
  
  (* Boundary conditions *)
  alphaAtStartA = Max[0, evaluateTF[a, supportMin[cdfA]][[4]]];
  alphaAtStartB = Max[0, evaluateTF[b, supportMin[cdfB]][[4]]];
  zeroAtStart = (alphaAtStartA <= eps) || (alphaAtStartB <= eps);
  
  alphaAtEndA = Max[0, evaluateTF[a, supportMax[cdfA]][[4]]];
  alphaAtEndB = Max[0, evaluateTF[b, supportMax[cdfB]][[4]]];
  zeroAtEnd = (alphaAtEndA <= eps) || (alphaAtEndB <= eps);
  
  If[! zeroAtStart, alpha[[1]] = density[[1]]];
  If[! zeroAtEnd, alpha[[n]] = density[[n - 1]]];
  
  (* Rescale to preserve total mass *)
  actualMass = Sum[
    Module[{dx = vertices[[i + 1, 1]] - vertices[[i, 1]]},
      If[dx <= eps, 0, 0.5 (alpha[[i]] + alpha[[i + 1]]) dx]
    ],
    {i, 1, n - 1}
  ];
  If[actualMass > eps,
    scale = targetMass/actualMass;
    alpha = alpha * scale;
  ];
  
  (* Clamp negatives *)
  alpha = Max[0, #] & /@ alpha;
  
  (* Build output TF *)
  domainMin = (1 - tc) a[[1, 1]] + tc b[[1, 1]];
  domainMax = (1 - tc) a[[-1, 1]] + tc b[[-1, 1]];
  
  result = {};
  
  If[vertices[[1, 1]] - domainMin > eps,
    Module[{color = (1 - tc) evaluateTF[a, domainMin][[1 ;; 3]] + tc evaluateTF[b, domainMin][[1 ;; 3]]},
      AppendTo[result, {domainMin, Append[color, 0]}]
    ]
  ];
  
  Do[
    AppendTo[result, {vertices[[i, 1]], Append[vertices[[i, 3]], alpha[[i]]]}],
    {i, 1, n}
  ];
  
  If[domainMax - vertices[[n, 1]] > eps,
    Module[{color = (1 - tc) evaluateTF[a, domainMax][[1 ;; 3]] + tc evaluateTF[b, domainMax][[1 ;; 3]]},
      AppendTo[result, {domainMax, Append[color, 0]}]
    ]
  ];
  
  If[result === {}, Return[linearBlend[a, b, tc]]];
  
  result
]

(* ============================================================ *)
(* Earth Mover's Distance                                       *)
(* ============================================================ *)
earthMoversDistance[tfA_List, tfB_List, samplesPerSegment_: 16] := 
Module[{a, b, cdfA, cdfB, levels, distance, prevQ, prevDiff},
  a = sanitize[tfA];
  b = sanitize[tfB];
  If[a === {} || b === {}, Return[0]];
  cdfA = computeCdf[a];
  cdfB = computeCdf[b];
  If[cdfA["totalMass"] <= eps || cdfB["totalMass"] <= eps, Return[0]];
  levels = mergedQuantileLevels[cdfA, cdfB, samplesPerSegment];
  If[Length[levels] < 2, Return[0]];
  
  distance = 0;
  prevQ = levels[[1]];
  prevDiff = Abs[invertCdf[cdfA, prevQ] - invertCdf[cdfB, prevQ]];
  Do[
    Module[{diff, dq},
      diff = Abs[invertCdf[cdfA, q] - invertCdf[cdfB, q]];
      dq = q - prevQ;
      distance += 0.5 (prevDiff + diff) dq;
      prevQ = q;
      prevDiff = diff;
    ],
    {q, Rest[levels]}
  ];
  distance
]

(* ============================================================ *)
(* Visualization helpers                                        *)
(* ============================================================ *)

(* Convert TF to a piecewise-linear alpha function for plotting *)
tfToAlphaFunction[tf_List] := Interpolation[
  {#[[1]], #[[2, 4]]} & /@ tf,
  InterpolationOrder -> 1
]

(* Plot the interpolation at various t values *)
plotInterpolation[tfA_List, tfB_List, steps_: 10, samplesPerSegment_: 16] := Module[{plots},
  plots = Table[
    Module[{result, alphaFn},
      result = optimalTransportInterpolation[tfA, tfB, t, samplesPerSegment];
      ListLinePlot[
        {#[[1]], #[[2, 4]]} & /@ result,
        PlotRange -> {{0, 1}, {0, All}},
        PlotLabel -> StringForm["t = ``", t],
        Filling -> Axis
      ]
    ],
    {t, 0, 1, 1/steps}
  ];
  GraphicsGrid[Partition[plots, UpTo[3]], ImageSize -> 900]
]

(* Example usage: *)
(* 
tfA = {{0.1, {1, 0, 0, 0}}, {0.2, {1, 0, 0, 1}}, {0.3, {1, 0, 0, 0}}};
tfB = {{0.6, {0, 0, 1, 0}}, {0.7, {0, 0, 1, 1}}, {0.8, {0, 0, 1, 0}}};
plotInterpolation[tfA, tfB, 10, 16]
*)
