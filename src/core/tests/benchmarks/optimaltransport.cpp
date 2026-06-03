/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

// =====================================================================================
// Optimal-transport TF interpolation benchmarks
// =====================================================================================
//
// Benchmarks `inviwo::algorithm::optimalTransportInterpolation` (piecewise-linear /
// secant-based) and `inviwo::algorithm::optimalTransportInterpolationClosedForm`
// (closed-form vertex opacities) across sweeps of:
//   * number of control points N           (controlPointCounts below)
//   * samplesPerSegment                     (samplesPerSegmentValues, PWL only)
//   * ClosedFormRefinementOptions           (relTolValues x maxQLValues x maxIterValues, CF only)
//
// Each benchmark instance reports the standard Google Benchmark auto-tuned wall-clock
// time and (separately, computed once outside the timed loop) the L-infinity, L1 and L2
// error of the interpolated alpha channel against the ground truth alpha_t(x).
// The oracle is `algorithm::evaluateInterpolatedAlpha`, which the header documents as
// the exact pointwise Wasserstein density expression.
//
// On shutdown the custom main writes six files into the current working directory,
// timestamped at run start:
//
//   optimaltransport_benchmark_<ts>_pwl_sweep.csv         /  .tex
//   optimaltransport_benchmark_<ts>_closedform_sweep.csv  /  .tex
//   optimaltransport_benchmark_<ts>_compare.csv           /  .tex
//
// Bolding policy in LaTeX:
//   * Sweep tables: for each N block, bold the row with the lowest
//     L-infinity error and (separately) the row with the lowest mean time.
//   * Compare table: for each N row pair, bold the smaller of the
//     two methods' L-infinity errors and the smaller of the two methods' mean times,
//     independently per column.
//
// IMPORTANT: the default cartesian product is large. Shrink the
// constexpr sweep arrays below for a quick smoke run. Each benchmark uses
// `MinTime(0.05)` to keep auto-tuning reasonable.
//
// Build with `-DIVW_TEST_BENCHMARKS=ON`. Run via the produced `bm-optimaltransport`
// executable; pass standard Google Benchmark flags (`--benchmark_filter=...`, etc).
// =====================================================================================

#include <benchmark/benchmark.h>

#include <inviwo/core/algorithm/optimaltransport.h>
#include <inviwo/core/datastructures/tfprimitive.h>
#include <inviwo/core/util/glm.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <numbers>
#include <optional>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace inviwo::bench {

namespace {

using TF = std::vector<TFPrimitiveData>;

// =====================================================================================
// Sweep parameter ranges (tweak here to shrink runtime)
// =====================================================================================

// ---- Complexity axis ----------------------------------------------------------------
//
// A single scalar: the number of control points N in each input TF. Each TF is built from N
// control points placed at independently drawn random positions x ~ U[0,1] with
// independently drawn random opacities alpha ~ U[0,1]. Every control point is a genuine feature
// of the PL function, so shape complexity and algorithmic work both scale with N.
// Both tfA and tfB are generated from the same family using distinct fixed seeds.

constexpr std::array<std::size_t, 6> controlPointCounts{4, 8, 16, 32, 64, 128};
constexpr std::uint32_t seedA = 0xC0FFEEu;
constexpr std::uint32_t seedB = 0xBADF00Du;

// PWL method samplesPerSegment sweep.
constexpr std::array<std::size_t, 9> samplesPerSegmentValues{2, 4, 8, 16, 32, 64, 128, 256};

// Closed-form refinement sweeps.
constexpr std::array<double, 5> relTolValues{1e-1, 1e-2, 1e-3, 1e-4, 1e-5};
constexpr std::array<std::size_t, 9> maxQLValues{16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
constexpr std::array<std::size_t, 9> maxIterValues{16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

// Number of dense-grid samples for accuracy evaluation.
// 100 000 points give sub-0.1% resolution at negligible cost thanks to the batch oracle.
constexpr std::size_t accuracyGrid = 100'000;

// Fixed interpolation parameter t used for all timing/accuracy comparisons.
constexpr double interpT = 0.5;

// Minimum per-benchmark wall-clock time (seconds) for Google Benchmark auto-tuning.
constexpr double benchMinTime = 0.05;

// =====================================================================================
// TF generator
// =====================================================================================
//
// Each TF is built from N control points. Positions are drawn from U[0,1] and opacities
// from U[0,1], both independently. The resulting piecewise-linear function has N genuine
// degrees of freedom — every control point is a real feature, not filler. The TF is sorted by
// position as required by the interpolation algorithms.

constexpr vec4 tfColor{1.0f, 0.0f, 0.0f, 1.0f};

TF makeRandomTF(std::size_t N, std::uint32_t seed) {
    std::mt19937 rng{seed};
    std::uniform_real_distribution<double> xDist{0.0, 1.0};
    std::uniform_real_distribution<double> aDist{0.0, 1.0};
    TF tf;
    tf.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        const double x = xDist(rng);
        const double a = aDist(rng);
        vec4 c = tfColor;
        c.a = static_cast<float>(a);
        tf.push_back(TFPrimitiveData{x, c});
    }
    std::sort(tf.begin(), tf.end(),
              [](const TFPrimitiveData& a, const TFPrimitiveData& b) { return a.pos < b.pos; });
    return tf;
}

struct TFPair {
    TF a;
    TF b;
};

TFPair makePair(std::size_t N) {
    return TFPair{makeRandomTF(N, seedA), makeRandomTF(N, seedB)};
}

// =====================================================================================
// Ground-truth oracle alpha_t(x)
// =====================================================================================
//
// `algorithm::evaluateInterpolatedAlpha` computes the exact pointwise transported
// density m_t / X_t'(q) from the control-point representations. It is independent of both
// interpolation methods under test (PWL / CF), which only produce control-point samples of the
// result. This makes it a valid reference for both.
double oracleAlpha(std::span<const TFPrimitiveData> tfA,
                   std::span<const TFPrimitiveData> tfB, double t, double x) {
    return algorithm::evaluateInterpolatedAlpha(tfA, tfB, t, x);
}

// =====================================================================================
// Sampling and accuracy helpers
// =====================================================================================

// Piecewise-linear alpha evaluation of a TF at position x. Assumes tf is sorted by pos.
double sampleTF(std::span<const TFPrimitiveData> tf, double x) {
    if (tf.empty()) return 0.0;
    if (x <= tf.front().pos) return std::max(0.0, static_cast<double>(tf.front().color.a));
    if (x >= tf.back().pos) return std::max(0.0, static_cast<double>(tf.back().color.a));
    auto it = std::lower_bound(tf.begin(), tf.end(), x,
                               [](const TFPrimitiveData& p, double v) { return p.pos < v; });
    const auto& hi = *it;
    const auto& lo = *(it - 1);
    const double dx = hi.pos - lo.pos;
    if (dx <= 0.0) return std::max(0.0, static_cast<double>(hi.color.a));
    const double f = (x - lo.pos) / dx;
    const double a0 = std::max(0.0, static_cast<double>(lo.color.a));
    const double a1 = std::max(0.0, static_cast<double>(hi.color.a));
    return (1.0 - f) * a0 + f * a1;
}

struct ErrorMetrics {
    double linf = 0.0;
    double l1 = 0.0;
    double l2 = 0.0;
    std::size_t outVertices = 0;  // number of control points in the produced TF
};

ErrorMetrics computeErrorMetrics(std::span<const TFPrimitiveData> result,
                                 std::span<const TFPrimitiveData> tfA,
                                 std::span<const TFPrimitiveData> tfB, double t,
                                 std::size_t grid = accuracyGrid) {
    ErrorMetrics m;
    if (grid < 2) return m;

    // Build a uniform x-grid once and batch-evaluate the oracle (setup O(N log N),
    // sweep O(G + N)) instead of calling evaluateInterpolatedAlpha per point (O(G * N log N)).
    std::vector<double> xs(grid);
    for (std::size_t i = 0; i < grid; ++i)
        xs[i] = static_cast<double>(i) / static_cast<double>(grid - 1);

    const auto refs = algorithm::evaluateInterpolatedAlphaGrid(tfA, tfB, t, xs);

    double sumAbs = 0.0;
    double sumSq = 0.0;
    for (std::size_t i = 0; i < grid; ++i) {
        const double got = sampleTF(result, xs[i]);
        const double e = std::abs(got - refs[i]);
        m.linf = std::max(m.linf, e);
        sumAbs += e;
        sumSq += e * e;
    }
    m.l1 = sumAbs / static_cast<double>(grid);
    m.l2 = std::sqrt(sumSq / static_cast<double>(grid));
    return m;
}

// =====================================================================================
// Time-stamp + accuracy table
// =====================================================================================

std::string formatNow() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%Y%m%d-%H%M%S");
    return os.str();
}

std::unordered_map<std::string, ErrorMetrics>& accuracyTable() {
    static std::unordered_map<std::string, ErrorMetrics> t;
    return t;
}

std::mutex& accuracyMutex() {
    static std::mutex m;
    return m;
}

void recordAccuracy(const std::string& name, const ErrorMetrics& m) {
    std::scoped_lock lock{accuracyMutex()};
    accuracyTable().emplace(name, m);  // first writer wins (call_once guarded by caller)
}

bool accuracyKnown(const std::string& name) {
    std::scoped_lock lock{accuracyMutex()};
    return accuracyTable().find(name) != accuracyTable().end();
}

// =====================================================================================
// Benchmark functions
// =====================================================================================

void BM_Pwl(benchmark::State& state, std::size_t N, std::size_t sps) {
    auto pair = makePair(N);
    const std::string name = state.name();
    if (!accuracyKnown(name)) {
        auto result = algorithm::optimalTransportInterpolation(pair.a, pair.b, interpT, sps);
        auto metrics = computeErrorMetrics(result, pair.a, pair.b, interpT);
        metrics.outVertices = result.size();
        recordAccuracy(name, metrics);
    }
    for (auto _ : state) {
        auto result = algorithm::optimalTransportInterpolation(pair.a, pair.b, interpT, sps);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

void BM_CF(benchmark::State& state, std::size_t N,
           algorithm::ClosedFormRefinementOptions opts) {
    auto pair = makePair(N);
    const std::string name = state.name();
    if (!accuracyKnown(name)) {
        auto result =
            algorithm::optimalTransportInterpolationClosedForm(pair.a, pair.b, interpT, opts);
        auto metrics = computeErrorMetrics(result, pair.a, pair.b, interpT);
        metrics.outVertices = result.size();
        recordAccuracy(name, metrics);
    }
    for (auto _ : state) {
        auto result =
            algorithm::optimalTransportInterpolationClosedForm(pair.a, pair.b, interpT, opts);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

void BM_PwlRef(benchmark::State& state, std::size_t N,
               algorithm::ClosedFormRefinementOptions opts) {
    auto pair = makePair(N);
    const std::string name = state.name();
    if (!accuracyKnown(name)) {
        auto result =
            algorithm::optimalTransportInterpolationRefined(pair.a, pair.b, interpT, opts);
        auto metrics = computeErrorMetrics(result, pair.a, pair.b, interpT);
        metrics.outVertices = result.size();
        recordAccuracy(name, metrics);
    }
    for (auto _ : state) {
        auto result =
            algorithm::optimalTransportInterpolationRefined(pair.a, pair.b, interpT, opts);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
}

// =====================================================================================
// Benchmark registration
// =====================================================================================

std::string formatRelTol(double r) {
    std::ostringstream os;
    os << std::scientific << std::setprecision(0) << r;
    return os.str();
}

std::string pwlName(std::size_t N, std::size_t sps) {
    std::ostringstream os;
    os << "OT_Pwl/N=" << N << "/sps=" << sps;
    return os.str();
}

std::string cfName(std::size_t N, double relTol, std::size_t maxQL, std::size_t maxIter) {
    std::ostringstream os;
    os << "OT_CF/N=" << N << "/relTol=" << formatRelTol(relTol)
       << "/maxQL=" << maxQL << "/maxIter=" << maxIter;
    return os.str();
}

std::string pwlRefName(std::size_t N, double relTol, std::size_t maxQL, std::size_t maxIter) {
    std::ostringstream os;
    os << "OT_PwlRef/N=" << N << "/relTol=" << formatRelTol(relTol)
       << "/maxQL=" << maxQL << "/maxIter=" << maxIter;
    return os.str();
}

// Config registry, filled by registerAll() so we can recover parameters by name later.
struct PwlCfg {
    std::size_t N = 0;
    std::size_t sps = 0;
};
struct CfCfg {
    std::size_t N = 0;
    double relTol = 0.0;
    std::size_t maxQL = 0;
    std::size_t maxIter = 0;
};
std::unordered_map<std::string, PwlCfg>& pwlRegistry() {
    static std::unordered_map<std::string, PwlCfg> r;
    return r;
}
std::unordered_map<std::string, CfCfg>& cfRegistry() {
    static std::unordered_map<std::string, CfCfg> r;
    return r;
}

// PWL-with-refinement reuses the closed-form refinement options (relTol/maxQL/maxIter).
std::unordered_map<std::string, CfCfg>& pwlRefRegistry() {
    static std::unordered_map<std::string, CfCfg> r;
    return r;
}

void registerAll() {
    using benchmark::RegisterBenchmark;

    // PWL sweep: (N, samplesPerSegment)
    for (auto N : controlPointCounts) {
        for (auto sps : samplesPerSegmentValues) {
            const std::string n = pwlName(N, sps);
            pwlRegistry().emplace(n, PwlCfg{N, sps});
            RegisterBenchmark(n.c_str(), [N, sps](benchmark::State& s) {
                BM_Pwl(s, N, sps);
            })
                ->Unit(benchmark::kMicrosecond)
                ->MinTime(benchMinTime);
        }
    }

    // Closed-form sweep: (N, relTol, maxQL, maxIter)
    for (auto N : controlPointCounts) {
        for (auto r : relTolValues) {
            for (auto q : maxQLValues) {
                for (auto it : maxIterValues) {
                    algorithm::ClosedFormRefinementOptions opts{};
                    opts.relativeTolerance = r;
                    opts.maxQuantileLevels = q;
                    opts.maxRefinementIterations = it;
                    const std::string n = cfName(N, r, q, it);
                    cfRegistry().emplace(n, CfCfg{N, r, q, it});
                    RegisterBenchmark(n.c_str(), [N, opts](benchmark::State& s) {
                        BM_CF(s, N, opts);
                    })
                        ->Unit(benchmark::kMicrosecond)
                        ->MinTime(benchMinTime);
                }
            }
        }
    }

    // PWL-with-refinement sweep: (N, relTol, maxQL, maxIter) — same refinement knobs as
    // the closed-form sweep, but secant-based opacity reconstruction.
    for (auto N : controlPointCounts) {
        for (auto r : relTolValues) {
            for (auto q : maxQLValues) {
                for (auto it : maxIterValues) {
                    algorithm::ClosedFormRefinementOptions opts{};
                    opts.relativeTolerance = r;
                    opts.maxQuantileLevels = q;
                    opts.maxRefinementIterations = it;
                    const std::string n = pwlRefName(N, r, q, it);
                    pwlRefRegistry().emplace(n, CfCfg{N, r, q, it});
                    RegisterBenchmark(n.c_str(), [N, opts](benchmark::State& s) {
                        BM_PwlRef(s, N, opts);
                    })
                        ->Unit(benchmark::kMicrosecond)
                        ->MinTime(benchMinTime);
                }
            }
        }
    }
}

// =====================================================================================
// Capturing reporter
// =====================================================================================

struct TimingRow {
    std::string name;
    double meanRealTimeNs = 0.0;  // per-iteration real time in nanoseconds
    double cpuTimeNs = 0.0;
    std::int64_t iterations = 0;
};

class CapturingReporter : public benchmark::ConsoleReporter {
public:
    void ReportRuns(const std::vector<Run>& runs) override {
        benchmark::ConsoleReporter::ReportRuns(runs);
        for (const auto& r : runs) {
            if (r.skipped) continue;
            TimingRow row;
            row.name = r.benchmark_name();
            row.meanRealTimeNs = r.GetAdjustedRealTime();
            row.cpuTimeNs = r.GetAdjustedCPUTime();
            row.iterations = r.iterations;
            rows_.push_back(std::move(row));
        }
    }
    const std::vector<TimingRow>& rows() const { return rows_; }

private:
    std::vector<TimingRow> rows_;
};

// =====================================================================================
// Joined-row infrastructure and output writers
// =====================================================================================

// Joined record for output.
struct JoinedRow {
    std::string name;
    std::size_t N = 0;
    // PWL
    std::size_t sps = 0;
    // CF
    double relTol = 0.0;
    std::size_t maxQL = 0;
    std::size_t maxIter = 0;
    // metrics
    double timeNs = 0.0;
    std::int64_t iterations = 0;
    ErrorMetrics err{};
};

std::string fmtSci(double v) {
    std::ostringstream os;
    os << std::scientific << std::setprecision(3) << v;
    return os.str();
}
std::string fmtFixed(double v, int prec = 3) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(prec) << v;
    return os.str();
}

// Escape a string for CSV (RFC 4180 minimal).
std::string csvEscape(std::string_view s) {
    if (s.find_first_of(",\"\n\r") == std::string_view::npos) return std::string{s};
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char c : s) {
        if (c == '"') out.push_back('"');
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

// Escape a string for LaTeX (basic).
std::string texEscape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '_':
            case '%':
            case '#':
            case '&':
            case '$':
            case '{':
            case '}': out.push_back('\\'); out.push_back(c); break;
            default: out.push_back(c);
        }
    }
    return out;
}

// ---- Gallery CSV (TF alpha profiles for each N, using seedA) ----------------------
//
// Writes one row per (N, sample_x) with the piecewise-linear alpha value of tfA sampled
// on a fixed grid. Python reads this to draw R7 without replicating any C++ logic.

void writeGalleryCsv(const std::filesystem::path& path, std::size_t gridSize = 256) {
    std::ofstream os{path};
    if (!os) {
        std::cerr << "Failed to open gallery CSV: " << path << "\n";
        return;
    }
    os << "N,x,alpha\n";
    os << std::scientific << std::setprecision(6);
    for (auto N : controlPointCounts) {
        const TF tfA = makeRandomTF(N, seedA);
        for (std::size_t i = 0; i < gridSize; ++i) {
            const double x = static_cast<double>(i) / static_cast<double>(gridSize - 1);
            const double a = sampleTF(tfA, x);
            os << N << "," << x << "," << a << "\n";
        }
    }
    std::cout << "  gallery CSV: " << path << "  (" << controlPointCounts.size()
              << " curves x " << gridSize << " samples)\n";
}

// ---- PWL sweep -----------------------------------------------------------------------

void writePwlSweepCsv(const std::filesystem::path& path,
                      const std::vector<JoinedRow>& rows) {
    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "N,samplesPerSegment,mean_time_ns,iterations,Linf,L1,L2,outVertices\n";
    for (const auto& r : rows) {
        os << r.N << ',' << r.sps << ','
           << fmtSci(r.timeNs) << ',' << r.iterations << ',' << fmtSci(r.err.linf) << ','
           << fmtSci(r.err.l1) << ',' << fmtSci(r.err.l2) << ',' << r.err.outVertices << '\n';
    }
}

void writePwlSweepTex(const std::filesystem::path& path,
                      const std::vector<JoinedRow>& rows) {
    // Group by N to determine bolding.
    std::map<std::size_t, std::vector<std::size_t>> groups;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        groups[rows[i].N].push_back(i);
    }

    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "% Auto-generated by optimaltransport benchmark.\n";
    os << "% Requires LaTeX packages: booktabs.\n";
    os << "\\begin{table}[ht]\n";
    os << "\\centering\n";
    os << "\\caption{Parameter sweep for the piecewise-linear (secant-based) optimal-transport "
          "interpolation \\texttt{optimalTransportInterpolation}. The complexity axis is the "
          "number of control points $N$ in each input TF. Both tfA and tfB are built from $N$ control "
          "points at random positions $x\\sim U[0,1]$ with random opacities "
          "$\\alpha\\sim U[0,1]$ (distinct fixed seeds). "
          "Each row reports one configuration of $N$ and \\texttt{samplesPerSegment} (sps). "
          "\\emph{Time} is the auto-tuned mean wall-clock per call (Google Benchmark). "
          "$L_\\infty$, $L_1$ and $L_2$ are the errors of the interpolated $\\alpha_t(x)$ on a "
          "dense uniform grid against \\texttt{evaluateInterpolatedAlpha} as reference. "
          "Within each $N$ block, the row with the lowest $L_\\infty$ and "
          "the row with the lowest time are typeset in bold; both columns are bolded independently.}\n";
    os << "\\label{tab:ot-pwl-sweep}\n";
    os << "\\begin{tabular}{rrrrrrr}\n";
    os << "\\toprule\n";
    os << "$N$ & sps & time [ns] & $L_\\infty$ & $L_1$ & $L_2$ \\\\\n";
    os << "\\midrule\n";

    for (const auto& [key, idxs] : groups) {
        // Find best (lowest) Linf and best (lowest) time in this group.
        std::size_t bestErr = idxs.front();
        std::size_t bestTime = idxs.front();
        for (std::size_t i : idxs) {
            if (rows[i].err.linf < rows[bestErr].err.linf) bestErr = i;
            if (rows[i].timeNs < rows[bestTime].timeNs) bestTime = i;
        }
        for (std::size_t i : idxs) {
            const auto& r = rows[i];
            const bool bErr = (i == bestErr);
            const bool bTime = (i == bestTime);
            auto wrap = [](const std::string& v, bool b) {
                return b ? "\\textbf{" + v + "}" : v;
            };
            os << r.N << " & "
               << r.sps << " & " << wrap(fmtSci(r.timeNs), bTime) << " & "
               << wrap(fmtSci(r.err.linf), bErr) << " & " << fmtSci(r.err.l1) << " & "
               << fmtSci(r.err.l2) << " \\\\\n";
        }
        os << "\\midrule\n";
    }
    os << "\\bottomrule\n\\end{tabular}\n";
    os << "\\end{table}\n";
}

// ---- Method comparison ---------------------------------------------------------------

struct CompareRow {
    std::size_t N = 0;
    // PWL
    std::string pwlAccCfg;
    double pwlAccLinf = 0.0;
    double pwlAccTime = 0.0;
    std::string pwlTimeCfg;
    double pwlTimeLinf = 0.0;
    double pwlTimeNs = 0.0;
    // CF
    std::string cfAccCfg;
    double cfAccLinf = 0.0;
    double cfAccTime = 0.0;
    std::string cfTimeCfg;
    double cfTimeLinf = 0.0;
    double cfTimeNs = 0.0;
    bool hasPwl = false;
    bool hasCf = false;
};

// Pick (lowest-error config, lowest-time config) over a list of rows.
std::pair<const JoinedRow*, const JoinedRow*> pickBest(const std::vector<JoinedRow>& rows,
                                                       const std::vector<std::size_t>& idxs) {
    if (idxs.empty()) return {nullptr, nullptr};
    const JoinedRow* bestErr = &rows[idxs.front()];
    const JoinedRow* bestTime = &rows[idxs.front()];
    for (std::size_t i : idxs) {
        if (rows[i].err.linf < bestErr->err.linf) bestErr = &rows[i];
        if (rows[i].timeNs < bestTime->timeNs) bestTime = &rows[i];
    }
    return {bestErr, bestTime};
}

std::string pwlCfgShort(const JoinedRow& r) {
    std::ostringstream os;
    os << "sps=" << r.sps;
    return os.str();
}

std::string cfCfgShort(const JoinedRow& r) {
    std::ostringstream os;
    os << "relTol=" << fmtSci(r.relTol) << ", maxQL=" << r.maxQL << ", maxIter=" << r.maxIter;
    return os.str();
}

std::vector<CompareRow> buildCompareRows(const std::vector<JoinedRow>& pwl,
                                         const std::vector<JoinedRow>& cf) {
    std::map<std::size_t, std::vector<std::size_t>> pwlGroups;
    std::map<std::size_t, std::vector<std::size_t>> cfGroups;
    for (std::size_t i = 0; i < pwl.size(); ++i) pwlGroups[pwl[i].N].push_back(i);
    for (std::size_t i = 0; i < cf.size(); ++i)  cfGroups[cf[i].N].push_back(i);

    std::map<std::size_t, CompareRow> merged;
    for (const auto& [N, idxs] : pwlGroups) {
        auto& cr = merged[N];
        cr.N = N;
        auto [bErr, bTime] = pickBest(pwl, idxs);
        if (bErr) {
            cr.hasPwl = true;
            cr.pwlAccCfg = pwlCfgShort(*bErr);
            cr.pwlAccLinf = bErr->err.linf;
            cr.pwlAccTime = bErr->timeNs;
            cr.pwlTimeCfg = pwlCfgShort(*bTime);
            cr.pwlTimeLinf = bTime->err.linf;
            cr.pwlTimeNs = bTime->timeNs;
        }
    }
    for (const auto& [N, idxs] : cfGroups) {
        auto& cr = merged[N];
        cr.N = N;
        auto [bErr, bTime] = pickBest(cf, idxs);
        if (bErr) {
            cr.hasCf = true;
            cr.cfAccCfg = cfCfgShort(*bErr);
            cr.cfAccLinf = bErr->err.linf;
            cr.cfAccTime = bErr->timeNs;
            cr.cfTimeCfg = cfCfgShort(*bTime);
            cr.cfTimeLinf = bTime->err.linf;
            cr.cfTimeNs = bTime->timeNs;
        }
    }
    std::vector<CompareRow> out;
    out.reserve(merged.size());
    for (auto& kv : merged) out.push_back(std::move(kv.second));
    return out;
}

void writeCompareCsv(const std::filesystem::path& path, const std::vector<CompareRow>& rows) {
    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "N,"
          "pwl_best_acc_cfg,pwl_best_acc_Linf,pwl_best_acc_time_ns,"
          "cf_best_acc_cfg,cf_best_acc_Linf,cf_best_acc_time_ns,"
          "pwl_best_time_cfg,pwl_best_time_Linf,pwl_best_time_ns,"
          "cf_best_time_cfg,cf_best_time_Linf,cf_best_time_ns\n";
    for (const auto& r : rows) {
        os << r.N << ',';
        os << csvEscape(r.pwlAccCfg) << ',' << fmtSci(r.pwlAccLinf) << ','
           << fmtSci(r.pwlAccTime) << ',';
        os << csvEscape(r.cfAccCfg) << ',' << fmtSci(r.cfAccLinf) << ','
           << fmtSci(r.cfAccTime) << ',';
        os << csvEscape(r.pwlTimeCfg) << ',' << fmtSci(r.pwlTimeLinf) << ','
           << fmtSci(r.pwlTimeNs) << ',';
        os << csvEscape(r.cfTimeCfg) << ',' << fmtSci(r.cfTimeLinf) << ','
           << fmtSci(r.cfTimeNs) << '\n';
    }
}

void writeCompareTex(const std::filesystem::path& path, const std::vector<CompareRow>& rows) {
    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "% Auto-generated by optimaltransport benchmark.\n";
    os << "% Requires LaTeX packages: booktabs.\n";
    os << "\\begin{table}[ht]\n";
    os << "\\centering\n";
    os << "\\caption{Head-to-head comparison of the piecewise-linear (PWL) and closed-form (CF) "
          "optimal-transport interpolators on matched inputs. The complexity axis is the "
          "number of control points $N$ in each input TF, with both tfA and tfB built from $N$ random "
          "control points. For every $N$ "
          "we select the configuration of each method that minimises $L_\\infty$ "
          "(\\emph{best-accuracy config}) and the configuration that minimises the auto-tuned "
          "mean wall-clock time (\\emph{best-time config}). Each pair of rows (PWL above, CF below) "
          "shares the same input. Within each row pair, the smaller of the two $L_\\infty$ values "
          "and the smaller of the two mean times are typeset in bold; the two columns are compared "
          "independently.}\n";
    os << "\\label{tab:ot-compare}\n";
    os << "% For each N we report the configuration of each method\n";
    os << "% that minimises Linf (best-accuracy) and minimises mean time (best-time).\n";
    os << "% Bolding compares the two methods within the same column.\n";
    os << "\\begin{tabular}{rllrrlrr}\n";
    os << "\\toprule\n";
    os << "$N$ & method & best-accuracy config & $L_\\infty$ & time [ns] & "
          "best-time config & $L_\\infty$ & time [ns] \\\\\n";
    os << "\\midrule\n";

    auto wrap = [](const std::string& v, bool b) {
        return b ? "\\textbf{" + v + "}" : v;
    };

    for (const auto& r : rows) {
        if (!r.hasPwl && !r.hasCf) continue;
        // Bolding: within row pair (pwl, cf), bold the lower value per column.
        const bool pwlBestAccErr =
            r.hasPwl && r.hasCf ? (r.pwlAccLinf <= r.cfAccLinf) : r.hasPwl;
        const bool cfBestAccErr =
            r.hasPwl && r.hasCf ? (r.cfAccLinf < r.pwlAccLinf) : r.hasCf;
        const bool pwlBestAccTime =
            r.hasPwl && r.hasCf ? (r.pwlAccTime <= r.cfAccTime) : r.hasPwl;
        const bool cfBestAccTime =
            r.hasPwl && r.hasCf ? (r.cfAccTime < r.pwlAccTime) : r.hasCf;
        const bool pwlBestTimeErr =
            r.hasPwl && r.hasCf ? (r.pwlTimeLinf <= r.cfTimeLinf) : r.hasPwl;
        const bool cfBestTimeErr =
            r.hasPwl && r.hasCf ? (r.cfTimeLinf < r.pwlTimeLinf) : r.hasCf;
        const bool pwlBestTimeTime =
            r.hasPwl && r.hasCf ? (r.pwlTimeNs <= r.cfTimeNs) : r.hasPwl;
        const bool cfBestTimeTime =
            r.hasPwl && r.hasCf ? (r.cfTimeNs < r.pwlTimeNs) : r.hasCf;

        if (r.hasPwl) {
            os << r.N << " & PWL & " << texEscape(r.pwlAccCfg) << " & "
               << wrap(fmtSci(r.pwlAccLinf), pwlBestAccErr) << " & "
               << wrap(fmtSci(r.pwlAccTime), pwlBestAccTime) << " & "
               << texEscape(r.pwlTimeCfg) << " & "
               << wrap(fmtSci(r.pwlTimeLinf), pwlBestTimeErr) << " & "
               << wrap(fmtSci(r.pwlTimeNs), pwlBestTimeTime) << " \\\\\n";
        }
        if (r.hasCf) {
            os << r.N << " & CF & " << texEscape(r.cfAccCfg) << " & "
               << wrap(fmtSci(r.cfAccLinf), cfBestAccErr) << " & "
               << wrap(fmtSci(r.cfAccTime), cfBestAccTime) << " & "
               << texEscape(r.cfTimeCfg) << " & "
               << wrap(fmtSci(r.cfTimeLinf), cfBestTimeErr) << " & "
               << wrap(fmtSci(r.cfTimeNs), cfBestTimeTime) << " \\\\\n";
        }
        os << "\\midrule\n";
    }
    os << "\\bottomrule\n\\end{tabular}\n";
    os << "\\end{table}\n";
}

void writeCfSweepCsv(const std::filesystem::path& path, const std::vector<JoinedRow>& rows) {
    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "N,relTol,maxQuantileLevels,maxRefinementIterations,mean_time_ns,"
          "iterations,Linf,L1,L2,outVertices\n";
    for (const auto& r : rows) {
        os << r.N << ',' << fmtSci(r.relTol)
           << ',' << r.maxQL << ',' << r.maxIter << ',' << fmtSci(r.timeNs) << ','
           << r.iterations << ',' << fmtSci(r.err.linf) << ',' << fmtSci(r.err.l1) << ','
           << fmtSci(r.err.l2) << ',' << r.err.outVertices << '\n';
    }
}

void writeCfSweepTex(const std::filesystem::path& path, const std::vector<JoinedRow>& rows) {
    std::map<std::size_t, std::vector<std::size_t>> groups;
    for (std::size_t i = 0; i < rows.size(); ++i) groups[rows[i].N].push_back(i);

    std::ofstream os{path};
    if (!os) {
        std::cerr << "[bm-optimaltransport] ERROR: cannot open " << path << " for writing\n";
        return;
    }
    os << "% Auto-generated by optimaltransport benchmark.\n";
    os << "% Requires LaTeX packages: booktabs.\n";
    os << "\\begin{table}[ht]\n";
    os << "\\centering\n";
    os << "\\caption{Parameter sweep for the closed-form optimal-transport interpolation "
          "\\texttt{optimalTransportInterpolationClosedForm}. The complexity axis is the number of "
          "control points $N$ in each input TF. Each row reports one configuration of $N$ and the adaptive "
          "refinement options \\texttt{relativeTolerance} (relTol), "
          "\\texttt{maxQuantileLevels} (maxQL) and "
          "\\texttt{maxRefinementIterations} (maxIter). \\emph{Time} is the auto-tuned mean "
          "wall-clock per call (Google Benchmark). $L_\\infty$, $L_1$ and $L_2$ are the errors of "
          "the interpolated $\\alpha_t(x)$ on a dense uniform grid against "
          "\\texttt{evaluateInterpolatedAlpha} as reference. Within each $N$ block, the row with "
          "the lowest $L_\\infty$ and the row with the lowest time are typeset in bold; both "
          "columns are bolded independently.}\n";
    os << "\\label{tab:ot-cf-sweep}\n";
    os << "\\begin{tabular}{rrrrrrrr}\n";
    os << "\\toprule\n";
    os << "$N$ & relTol & maxQL & maxIter & time [ns] & $L_\\infty$ & $L_1$ & "
          "$L_2$ \\\\\n";
    os << "\\midrule\n";

    for (const auto& [N, idxs] : groups) {
        std::size_t bestErr = idxs.front();
        std::size_t bestTime = idxs.front();
        for (std::size_t i : idxs) {
            if (rows[i].err.linf < rows[bestErr].err.linf) bestErr = i;
            if (rows[i].timeNs < rows[bestTime].timeNs) bestTime = i;
        }
        for (std::size_t i : idxs) {
            const auto& r = rows[i];
            const bool bErr = (i == bestErr);
            const bool bTime = (i == bestTime);
            auto wrap = [](const std::string& v, bool b) {
                return b ? "\\textbf{" + v + "}" : v;
            };
            os << r.N << " & "
               << fmtSci(r.relTol) << " & " << r.maxQL << " & " << r.maxIter << " & "
               << wrap(fmtSci(r.timeNs), bTime) << " & " << wrap(fmtSci(r.err.linf), bErr)
               << " & " << fmtSci(r.err.l1) << " & " << fmtSci(r.err.l2) << " \\\\\n";
        }
        os << "\\midrule\n";
    }
    os << "\\bottomrule\n\\end{tabular}\n";
    os << "\\end{table}\n";
}

}  // namespace
}  // namespace inviwo::bench

// =====================================================================================
// main
// =====================================================================================

int main(int argc, char** argv) {
    using namespace inviwo::bench;

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;

    registerAll();

    CapturingReporter reporter;
    const std::size_t n = benchmark::RunSpecifiedBenchmarks(&reporter);
    if (n == 0) {
        std::cerr << "[bm-optimaltransport] WARNING: no benchmarks ran (registered: "
                  << (pwlRegistry().size() + cfRegistry().size()) << ").\n";
    }
    std::cerr << "[bm-optimaltransport] reporter captured " << reporter.rows().size()
              << " runs (registered: pwl=" << pwlRegistry().size()
              << ", cf=" << cfRegistry().size() << ").\n";

    // Index timing rows by name for O(1) lookup. Use exact-match first; if that
    // matches nothing, fall back to suffix-prefix matching (e.g. "_mean" suffixes
    // for aggregate runs).
    std::unordered_map<std::string, const TimingRow*> timingByName;
    timingByName.reserve(reporter.rows().size());
    for (const auto& tr : reporter.rows()) {
        timingByName.emplace(tr.name, &tr);
    }
    auto findTiming = [&](const std::string& key) -> const TimingRow* {
        if (auto it = timingByName.find(key); it != timingByName.end()) return it->second;
        // Fallback: any name starting with key (handles "<name>_mean" etc.)
        for (const auto& tr : reporter.rows()) {
            if (tr.name.size() >= key.size() &&
                tr.name.compare(0, key.size(), key) == 0) {
                return &tr;
            }
        }
        return nullptr;
    };

    // Drive output from the registry so every registered benchmark contributes a row
    // (timing/accuracy may be NaN if a particular run produced nothing).
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<JoinedRow> pwlRows;
    pwlRows.reserve(pwlRegistry().size());
    std::size_t pwlJoined = 0;
    {
        std::scoped_lock lock{accuracyMutex()};
        const auto& acc = accuracyTable();
        for (const auto& [name, cfg] : pwlRegistry()) {
            JoinedRow jr;
            jr.name = name;
            jr.N = cfg.N;
            jr.sps = cfg.sps;
            if (auto it = acc.find(name); it != acc.end()) {
                jr.err = it->second;
            } else {
                jr.err = ErrorMetrics{nan, nan, nan};
            }
            if (auto* tr = findTiming(name)) {
                jr.timeNs = tr->meanRealTimeNs;
                jr.iterations = tr->iterations;
                ++pwlJoined;
            } else {
                jr.timeNs = nan;
                jr.iterations = 0;
            }
            pwlRows.push_back(std::move(jr));
        }
    }

    std::vector<JoinedRow> cfRows;
    cfRows.reserve(cfRegistry().size());
    std::size_t cfJoined = 0;
    {
        std::scoped_lock lock{accuracyMutex()};
        const auto& acc = accuracyTable();
        for (const auto& [name, cfg] : cfRegistry()) {
            JoinedRow jr;
            jr.name = name;
            jr.N = cfg.N;
            jr.relTol = cfg.relTol;
            jr.maxQL = cfg.maxQL;
            jr.maxIter = cfg.maxIter;
            if (auto it = acc.find(name); it != acc.end()) {
                jr.err = it->second;
            } else {
                jr.err = ErrorMetrics{nan, nan, nan};
            }
            if (auto* tr = findTiming(name)) {
                jr.timeNs = tr->meanRealTimeNs;
                jr.iterations = tr->iterations;
                ++cfJoined;
            } else {
                jr.timeNs = nan;
                jr.iterations = 0;
            }
            cfRows.push_back(std::move(jr));
        }
    }

    std::vector<JoinedRow> pwlRefRows;
    pwlRefRows.reserve(pwlRefRegistry().size());
    std::size_t pwlRefJoined = 0;
    {
        std::scoped_lock lock{accuracyMutex()};
        const auto& acc = accuracyTable();
        for (const auto& [name, cfg] : pwlRefRegistry()) {
            JoinedRow jr;
            jr.name = name;
            jr.N = cfg.N;
            jr.relTol = cfg.relTol;
            jr.maxQL = cfg.maxQL;
            jr.maxIter = cfg.maxIter;
            if (auto it = acc.find(name); it != acc.end()) {
                jr.err = it->second;
            } else {
                jr.err = ErrorMetrics{nan, nan, nan};
            }
            if (auto* tr = findTiming(name)) {
                jr.timeNs = tr->meanRealTimeNs;
                jr.iterations = tr->iterations;
                ++pwlRefJoined;
            } else {
                jr.timeNs = nan;
                jr.iterations = 0;
            }
            pwlRefRows.push_back(std::move(jr));
        }
    }
    std::cerr << "[bm-optimaltransport] joined timing: pwl=" << pwlJoined << "/"
              << pwlRows.size() << ", cf=" << cfJoined << "/" << cfRows.size()
              << ", pwlref=" << pwlRefJoined << "/" << pwlRefRows.size() << "\n";

    // Sort for stable, readable output.
    auto pwlLess = [](const JoinedRow& a, const JoinedRow& b) {
        return std::tie(a.N, a.sps) < std::tie(b.N, b.sps);
    };
    auto cfLess = [](const JoinedRow& a, const JoinedRow& b) {
        return std::tie(a.N, a.relTol, a.maxQL, a.maxIter) <
               std::tie(b.N, b.relTol, b.maxQL, b.maxIter);
    };
    std::sort(pwlRows.begin(), pwlRows.end(), pwlLess);
    std::sort(cfRows.begin(), cfRows.end(), cfLess);
    std::sort(pwlRefRows.begin(), pwlRefRows.end(), cfLess);

    const std::string ts = formatNow();
    const std::filesystem::path cwd = std::filesystem::current_path();
    const std::filesystem::path pwlCsv =
        cwd / ("optimaltransport_benchmark_" + ts + "_pwl_sweep.csv");
    const std::filesystem::path pwlTex =
        cwd / ("optimaltransport_benchmark_" + ts + "_pwl_sweep.tex");
    const std::filesystem::path cfCsv =
        cwd / ("optimaltransport_benchmark_" + ts + "_closedform_sweep.csv");
    const std::filesystem::path cfTex =
        cwd / ("optimaltransport_benchmark_" + ts + "_closedform_sweep.tex");
    const std::filesystem::path cmpCsv =
        cwd / ("optimaltransport_benchmark_" + ts + "_compare.csv");
    const std::filesystem::path cmpTex =
        cwd / ("optimaltransport_benchmark_" + ts + "_compare.tex");

    const std::filesystem::path pwlRefCsv =
        cwd / ("optimaltransport_benchmark_" + ts + "_pwlref_sweep.csv");
    const std::filesystem::path pwlRefTex =
        cwd / ("optimaltransport_benchmark_" + ts + "_pwlref_sweep.tex");

    const std::filesystem::path galleryCsv =
        cwd / ("optimaltransport_benchmark_" + ts + "_gallery.csv");

    writeGalleryCsv(galleryCsv);
    writePwlSweepCsv(pwlCsv, pwlRows);
    writePwlSweepTex(pwlTex, pwlRows);
    writeCfSweepCsv(cfCsv, cfRows);
    writeCfSweepTex(cfTex, cfRows);

    // PWL-with-refinement shares the closed-form CSV/TeX schema (relTol/maxQL/maxIter).
    writeCfSweepCsv(pwlRefCsv, pwlRefRows);
    writeCfSweepTex(pwlRefTex, pwlRefRows);

    auto cmpRows = buildCompareRows(pwlRows, cfRows);
    writeCompareCsv(cmpCsv, cmpRows);
    writeCompareTex(cmpTex, cmpRows);

    std::cout << "\nBenchmark output files written to:\n"
              << "  " << galleryCsv.string() << "\n"
              << "  " << pwlCsv.string() << "  (" << pwlRows.size() << " rows)\n"
              << "  " << pwlTex.string() << "\n"
              << "  " << cfCsv.string() << "  (" << cfRows.size() << " rows)\n"
              << "  " << cfTex.string() << "\n"
              << "  " << pwlRefCsv.string() << "  (" << pwlRefRows.size() << " rows)\n"
              << "  " << pwlRefTex.string() << "\n"
              << "  " << cmpCsv.string() << "  (" << cmpRows.size() << " rows)\n"
              << "  " << cmpTex.string() << "\n";

    benchmark::Shutdown();
    return 0;
}
