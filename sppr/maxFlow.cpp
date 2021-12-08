// config
#define STATS 0
const int statsInterval = 10000; // set to 0 to only print stats at the end
#define INITIAL_RELABEL 1
#ifndef RELABEL_TACTIC
#define RELABEL_TACTIC 3 // 1 == scan edges to push, then scan again for relabel
                                                 // 2 == scan edges to push + relabel at once
                                                 // 3 == multiple relabels per iteration
                                                 // 4 == multiple relabels per iteration, async
                                                 //             with Bo-Hong algo
#endif
#define GAP_HEURISTIC 0 // only works for RELABEL_TACTIC in {1, 2}
#define EXCESS_SCALING 0
#define P2L 0
#define INCREMENTAL_GLREL 0

#if GAP_HEURISTIC && RELABEL_TACTIC > 2
#    error "Not implemented: Gap heuristics with multi-relabel"
#endif

#include "parallel.h"

#define forSwitch(t,i,s,e,X) { \
                         intT __ss=(s), __ee=(e); \
                         if (__ee-__ss > t) sppr_parallel_for (intT i = __ss; i < __ee; ++i) X \
                         else                             for    (intT i = __ss; i < __ee; ++i) X }

#ifdef NDEBUG
#    define ass2(cond, msg)
#    define ass(cond)
#else
#    define ass2(cond, msg) if (!(cond)) \
        cerr << __FILE__ <<":" <<__LINE__<<": assertion '"    #cond << "' failed. " \
                 << msg << endl, abort()
#    define ass(cond) ass2(cond, "")
#endif

const float globUpdtFreq = 3.0;
#define ALPHA 6
#define BETA 12

#include <iostream>
#include <queue>
#include <set>
#include <array>

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#ifdef _MSC_VER
#include <intrin.h> // For _InterlockedCompareExchange
#endif

//#include <unistd.h>

using namespace std;

mutex globalMx;
#define ATOMIC(x) { unique_lock<mutex> lock(globalMx); x }

#if (CILKP && _OPENMP) || (!CILKP && !_OPENMP)
#    error "Make up your mind!"
#endif

#if _OPENMP
#    include <omp.h>
#else
#error "No OpenMP"
#endif

#include "graph.h"
#include "gettime.h"
#include "utils.h"
#include "sequence.h"
#include "parallel.h"
#include "maxFlow.h"
//#include "blockRadixSort.h"

#ifdef SPPR_HAS_TBB
    // If we have Intel TBB we use their parallel sort
#   include <tbb/parallel_sort.h>
#   define COMP_SORT tbb::parallel_sort // For now, fall back to serial sort
#elif defined(_OPENMP) && (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
    // If we're on GCC and have OpenMP we use the compiler builtin
#   include <parallel/algorithm>
#   define COMP_SORT __gnu_parallel::sort
#else
    // Otherwise, we fall back so single-threaded sort
#    define COMP_SORT sort
#endif

typedef long long ll;

template <typename T>
inline bool CAS(T* x, T y, T z) {
    if (*x != y) return 0; // double-check
#ifdef _MSC_VER
    static_assert(sizeof(T) == sizeof(long), "Size of T must be same as size of long");
    return _InterlockedCompareExchange((long *)(x), (long)(z), (long)(y)) == (long)(y);
#else
    return __sync_bool_compare_and_swap(x, y, z);
#endif
}

template <typename T, typename P, typename F>
inline bool writeAggregation(T* x, P pred, F f) {
    for(;;) {
        T old = *x;
        if (!pred(old)) return 0;
        if (CAS(x, old, f(old))) return 1;
    }
}

template <typename T>
inline bool writeMax(T* x, T y) {
    return writeAggregation(x, [=](T old) { return y > old; }, [=](T _) { return y; });
}

template <typename T>
inline bool writeMin(T* x, T y) {
    return writeAggregation(x, [=](T old) { return y < old; }, [=](T _) { return y; });
}

template <typename T, size_t N>
array<T,N> pointwiseAdd(const array<T,N>& a, const array<T,N>& b) {
    array<T,N> res;
    for (size_t i = 0; i < N; ++i)
        res[i] = a[i] + b[i];
    return res;
}

timer tGroupBy[3];
ll cntGroupBy=0;
ll sumGroupBy=0;
intT groupByMin=1<<30;
intT* tmp;
template <typename T, typename U, typename F>
intT groupBy(T* A, F f, intT* first, intT n, U maxV) {
    sumGroupBy+=n;
    cntGroupBy++;
    groupByMin = min(groupByMin, n);

    tGroupBy[0].start();
    auto cmp = [&](const T& a, const T& b) { return f(a) < f(b); };
    if (n < 500) {
        sort(A, A+n, cmp);
    } else {
        COMP_SORT(A, A+n, cmp);
        //intSort::iSort(A, n, maxV + 1, f);
    }
    tGroupBy[0].stop();

    if (n < 500) {
        int j = 0;
        for(int i = 0; i < n; ++i) {
            if (i == 0 || f(A[i]) != f(A[i-1]))
                first[j++] = i;
        }
        first[j] = n;
        return j;
    } else {
        tGroupBy[1].start();
        tmp[0] = 0;
        sppr_parallel_for (intT i = 1; i < n; ++i)
            tmp[i] = (f(A[i]) != f(A[i - 1])) ? i : -1;
        tGroupBy[1].stop();
        tGroupBy[2].start();
        intT sz = sequence::filter(tmp, first, n, [](intT x) { return x >= 0; });
        tGroupBy[2].stop();
        first[sz] = n;
        return sz;
    }
}

using Cap = int;
template<typename T> bool nonNegF(T x) { return x >= 0; }

#define PAD(x,i) char __pad##x[i];
//#define PAD(x,i)
struct Arc {
    intT to;
    intT rev;
#if RELABEL_TACTIC == 4
    atomic<Cap> resCap;
    atomic<Cap> revResCap;
#else
    Cap resCap;
    Cap revResCap;
#endif
} *arcs;
intT arcId(Arc& a) { return min((intT)(&a-arcs), a.rev); }

struct NodeView {
    intT first, _last;
    intT h;
    Cap e;
    pair<ll,Cap> *pushes;
#if GAP_HEURISTIC
    intT *relabels;
#endif
    intT last() const { return _last; }
};
struct Node {
    intT first;
#if P2L
    atomic<Cap> sumOutCap;
#endif
    intT h;
#if RELABEL_TACTIC == 4
    atomic<Cap> e;
#else
    intT hnew;
    Cap e;
#endif
    pair<ll,Cap> *pushes, *pushesEnd;
#if GAP_HEURISTIC
    intT *relabels, *relabelsEnd;
#endif

#if RELABEL_TACTIC != 4
    NodeView view() const {
        return { first, last(), h, e, pushes,
#if GAP_HEURISTIC
            relabels,
#endif
        };
    }
    void apply(const NodeView& v) {
        hnew = v.h;
        pushesEnd = v.pushes;
#if GAP_HEURISTIC
        relabelsEnd = v.relabels;
#endif
    }
#endif
    intT last() const { return (this + 1)->first; }
    int inWorkingSet;
} *nodes;
atomic<Cap> *enew;

template <typename N>
intT outDegree(N& n) { return n.last() - n.first; }

template <typename N>
void resetArcPointers(N& v) {
    //v.cur = v.firstAdmissible = v.first;
}

#define for_arcs(for, n, a, body) \
    {for (intT _i = (n).first; _i < (n).last(); ++_i) { Arc& a = arcs[_i]; body }}

Cap del;
intT lowestChangedLabel;
ll nm;
ll pass;
intT n, m, source, sink;
ll globalRelabels;
long double sumActive;
#if !_OPENMP
cilk::reducer_opadd<ll> otherWork, nonSatPushes, satPushes, gapNodes,
                                                gapNodeExcess, edgeScans;
#endif
ll discharges, globRelWork, globRelIters, relabels;
ll workSinceUpdate;
timer globalRelabelTime, totalTime, gapHeuristicTime, sourceSaturationTime, phaseTime[8];
intT gaps;
intT hiLabel;
pair<ll,Cap> *pushBuffer;
intT *relabelsBuffer;
intT *labelCounter;

intT *bufSize, *offset, *first;
intT wSetSize;
intT *wSet, *wSetTmp;
pair<ll,Cap> *groupedPushes;

enum Mode {
    MODE_RACE,
    MODE_SYNC,
    MODE_RANDOM,
} mode;

void increaseArcFlow(Arc& a, Cap delta) {
    ass2(a.resCap-delta >= 0,(&a-arcs)<<" "<<a.to<<" "<<a.resCap<<" "<<delta);
    a.resCap -= delta;
    ass2(a.resCap >= 0,a.resCap<<" "<<delta);
    Arc& rev = arcs[a.rev];
    rev.resCap += delta;
    ass(rev.resCap>=0);
#if RELABEL_TACTIC == 4
    rev.revResCap.store(a.resCap);
    a.revResCap.store(rev.resCap);
#else
    rev.revResCap = a.resCap;
    a.revResCap = rev.resCap;
#endif
}

//#include "dinic.C"

void init() {
    // n, m, source, sink and the graph structure stored in nodes, arcs must be
    // initialized already
    pushBuffer = new pair<ll,Cap>[m + n];
#if GAP_HEURISTIC
    labelCounter = new intT[n + 1];
    sppr_parallel_for (intT i = 0; i < n; ++i) labelCounter[i] = 0;
    labelCounter[0] = n - 1;
    relabelsBuffer = new intT[m + n];
#endif
    wSet = new intT[n+1];
    groupedPushes = new pair<ll,Cap>[m + n + 1];
    bufSize = new intT[n+1];
    offset = new intT[n+1];
    tmp = new intT[m+n+1];
    first = new intT[m+n+1];
    wSetTmp = new intT[n];
    enew = new atomic<Cap>[n];
    for (intT i = 0; i < n; ++i) {
        Node& v = nodes[i];
        v.h = 0;
        v.e = 0;
        v.inWorkingSet = 0;
        v.pushes = v.pushesEnd = pushBuffer + v.first + i;
        enew[i].store(0);
#if RELABEL_TACTIC != 4
        v.hnew = 0;
#endif
#if GAP_HEURISTIC
        v.relabels = v.relabelsEnd = relabelsBuffer + v.first + i;
#endif
        //nodes[i].buf = nodes[i].bufEnd = new intT[2*(1 + outDegree(nodes[i]))];
#if P2L
        v.sumOutCap.store(0);
#endif
        resetArcPointers(v);
        //nodes[i].buf = nodeBuffer + nodes[i].first + i;
    }
    sppr_parallel_for (intT i = 0; i < m; ++i) {
        Arc& a = arcs[i];
#if RELABEL_TACTIC == 4
        a.revResCap.store(arcs[a.rev].resCap);
#else
        a.revResCap = arcs[a.rev].resCap;
#endif
    }
}

void deinit() {
    delete[] bufSize;
    delete[] offset;
    delete[] wSet;
    delete[] pushBuffer;
    delete[] groupedPushes;
    delete[] tmp;
    delete[] first;
    delete[] wSetTmp;
    delete[] enew;
#if GAP_HEURISTIC
    delete[] relabelsBuffer;
    delete[] labelCounter;
#endif
}

void checkLabelCounter() {
    static atomic<intT> *lblCnt;
    if (!lblCnt) lblCnt = new atomic<intT>[hiLabel+1];
    sppr_parallel_for (intT i = 0; i < hiLabel; ++i) lblCnt[i].store(labelCounter[i]);
    sppr_parallel_for (intT i = 0; i < n; ++i) lblCnt[nodes[i].h]--;
    sppr_parallel_for (intT i = 0; i < hiLabel; ++i) ass2(!lblCnt[i], i);
}

void checkDistanceLabels() {
    sppr_parallel_for (intT i = 0; i < n; ++i) {
        if (nodes[i].h == hiLabel) continue;
        for_arcs(sppr_parallel_for, nodes[i], a, {
            if (a.resCap > 0) {
                ass2(nodes[i].h <= nodes[a.to].h + 1,
                             i << " " << a.to << " " << nodes[i].h << " " << nodes[a.to].h);
            }
        })
    }
}

void checkSanity() {
    cout << "Sanity check" << endl;
    ass(nodes[source].h == n);
    ass(nodes[sink].h == 0);
    checkDistanceLabels();
#if GAP_HEURISTIC
    checkLabelCounter();
#endif
    sppr_parallel_for (intT i = 0; i < n; ++i) {
        Node& v = nodes[i];
        for_arcs(sppr_parallel_for, v, a, {
            ass(a.resCap == arcs[a.rev].revResCap);
            ass(a.revResCap == arcs[a.rev].resCap);
            ass2(a.resCap >= 0, a.resCap);
            ass(arcs[a.rev].resCap >= 0);
        })
    }
}

void revBfs(intT *frontier, intT sz, intT d, intT exclude) {
    //cout << "BFS start" << endl;
    timer t;
    t.clear();
    t.start();
    intT *degs = new intT[n], *offset = new intT[n];
    while (sz) {
        //cout << "level " << d << " " << sz << endl;
#if GAP_HEURISTIC
        labelCounter[d] = sz;
#endif
        sppr_parallel_for (intT i = 0; i < sz; ++i)
            degs[i] = outDegree(nodes[frontier[i]]);
        intT maxNewSz = sequence::plusScan(degs, offset, sz);
        globRelWork += maxNewSz;
        globRelIters++;
        intT *nxt = new intT[maxNewSz];
        forSwitch(400, i, 0, sz, {
            intT o = offset[i];
            Node& v = nodes[frontier[i]];
            forSwitch(1000, j, 0, degs[i], {
                Arc& ar = arcs[v.first + j];
                nxt[o + j] = -1;
                if (!ar.revResCap) continue;
                intT wi = ar.to;
                if (wi == exclude) continue;
                Node& w = nodes[wi];
                if (CAS(&w.h, hiLabel, v.h + 1)) {
//#if RELABEL_TACTIC != 4
                    w.hnew = w.h;
//#endif
                    nxt[o + j] = wi;
                }
            })
        })
        sz = sequence::filter(nxt, frontier, maxNewSz, [](intT x) { return x >= 0; });
        delete[] nxt;
        d++;
    }
    delete[] degs;
    delete[] offset;
    t.stop();
    //cout << "BFS end time " << t.total() << " diameter " << d << endl;
}

void globalRelabel() {
    //cout << "global relabeling (low = " << lowestChangedLabel << ") ... " << flush;
    //cout << "hiLabel=" << hiLabel <<    endl;
    workSinceUpdate = 0;
    globalRelabelTime.start();
    globalRelabels++;

    sppr_parallel_for (intT i = 0; i < n; ++i) {
        Node& v = nodes[i];
        if (v.h > lowestChangedLabel || (lowestChangedLabel == 0 && i != sink))
            v.h = hiLabel;
#if GAP_HEURISTIC
        if (i > lowestChangedLabel)
            labelCounter[i] = 0;
#endif
    }

    intT *frontier = new intT[n];
    intT sz = 0;
    if (lowestChangedLabel == 0)
        frontier[sz++] = sink;
    else {
        intT *tmp = new intT[n];
        sppr_parallel_for (intT vi = 0; vi < n; ++vi) {
            Node& v = nodes[vi];
            if (v.h == lowestChangedLabel)
                tmp[vi] = vi;
            else
                tmp[vi] = -1;
        }
        sz = sequence::filter(tmp, frontier, n, [](intT x) { return x >= 0; });
        delete[] tmp;
    }
    revBfs(frontier, sz, lowestChangedLabel, source);
    delete[] frontier;
    globalRelabelTime.stop();
#if INCREMENTAL_GLREL
    lowestChangedLabel = numeric_limits<intT>::max();
#endif
    //cout << "done" << endl;
}

void printStats() {
    cout << setprecision(4) << fixed;
#define FIELD(name, value) cout << "    " << left << setw(30) << (name) \
                 << right << setw(20) << fixed << setprecision(2) << (value) << endl
    cout << "after " << pass << " iterations:" << endl;
    FIELD("nodes", n);
    FIELD("edges", m/2);
    FIELD("sink excess", nodes[sink].e);
    cout << endl;
    FIELD("active nodes", wSetSize);
    //FIELD("average active", pass > 0 ? to_string(sumActive / pass) : "n/a");
    cout << endl;
    FIELD("global relabels", globalRelabels);
    FIELD(GAP_HEURISTIC ? "gaps (enabled)" : "gaps (disabled)", gaps);
#if !_OPENMP && GAP_HEURISTIC && STATS
    FIELD("* gap nodes", gapNodes.get_value());
    FIELD("* gap excess", gapNodeExcess.get_value());
#endif
    cout << endl;
    FIELD("total time", totalTime.total());
    FIELD("* source time", sourceSaturationTime.total());
    FIELD("* glrel time", globalRelabelTime.total());
    FIELD("* phase1 time", phaseTime[0].total());
    FIELD("* phase2 time", phaseTime[1].total());
    FIELD("* phase3 time", phaseTime[2].total());
    FIELD("* phase4 time", phaseTime[3].total());
    FIELD("* phase5 time", phaseTime[4].total());
    FIELD("* phase6 time", phaseTime[5].total());
    FIELD("* phase7 time", phaseTime[6].total());
    FIELD("* phase8 time", phaseTime[7].total());
    FIELD("* groupBy time 1", tGroupBy[0].total());
    FIELD("* groupBy time 2", tGroupBy[1].total());
    FIELD("* groupBy time 3", tGroupBy[2].total());
    FIELD("* gap heuristic time", gapHeuristicTime.total());
    FIELD("groupBy avg", (double)sumGroupBy / cntGroupBy);
    FIELD("groupBy min", groupByMin);
    FIELD("glrel work / iters", (double)globRelWork / globRelIters);
    FIELD("discharges", discharges);
    FIELD("relabels", relabels);
    FIELD("relabels / discharge", (double)relabels / discharges);
#if !_OPENMP && STATS
    FIELD("edge scans", edgeScans.get_value());
    FIELD("edge scans / discharge", 1.0 * edgeScans.get_value() / discharges);
    //FIELD("* phase3 time", phase3Time.total());
    cout << endl;
    FIELD("pushes + relabels", satPushes.get_value() + nonSatPushes.get_value() + relabels);
    FIELD("pushes", satPushes.get_value() + nonSatPushes.get_value());
    FIELD("    -> saturating", satPushes.get_value());
    FIELD("    -> non-saturating", nonSatPushes.get_value());
    cout << endl;
    FIELD("total work", globRelWork + otherWork.get_value());
    FIELD("* work glrel", globRelWork);
    FIELD("* other work", otherWork.get_value());
#endif
    cout << endl;
}

bool needGlobalUpdate() {
    return workSinceUpdate * globUpdtFreq > nm;
}

#if RELABEL_TACTIC == 4
array<ll, 2> processNode(intT i, intT ui, bool dbg=0) {
    ll addRelabels = 0, addSatPushes = 0, addNonSatPushes = 0,
         addOtherWork = 0, addWorkSinceUpdate = 0, addEdgeScans = 0;
    ass(ui != source && ui != sink);
    Node& u = nodes[ui];
    pair<ll,Cap> *buf = u.pushes;
    if (CAS(&u.inWorkingSet, 0, 1))
        *buf++ = { ui, 0 };
    while (u.e > 0 && u.h < n) {
        Cap ee = u.e;
        intT hh = n - 1;
        intT ai = -1;
        for_arcs(for, u, a, {
            if (!a.resCap) continue;
            Node& v = nodes[a.to];
            intT vh = v.h;
            if (vh < hh) {
                ai = &a - arcs;
                hh = vh;
            }
        });
        addWorkSinceUpdate += outDegree(u);
        if (u.h > hh && u.h < n && ai != -1) {
            Arc& a = arcs[ai];
            intT vi = a.to;
            Node& v = nodes[vi];
            Cap delta = min(ee, a.resCap.load());
            if (delta && u.h > v.h) {
                increaseArcFlow(a, delta);
                u.e -= delta;
                v.e += delta;
                if (vi != sink && vi != source && CAS(&v.inWorkingSet, 0, 1))
                    *buf++ = make_pair(vi, 0);
                addWorkSinceUpdate += BETA;
                addRelabels++;
            }
        } else if (u.h < hh + 1) {
            u.h = hh + 1;
        }
    }
    u.pushesEnd = buf;
    return { addWorkSinceUpdate, addRelabels };

    /*
    ass(vi != source && vi != sink);
    Node& v = nodes[vi];
    pair<ll,Cap> *buf = v.pushes;
    if (CAS(&v.inWorkingSet, 0, 1))
        *buf++ = { vi, 0 };
    while (v.e > 0 && v.h < hiLabel) {
        for_arcs(for, v, a, {
            if (STATS) addEdgeScans++;
            if (!v.e) break;
            intT wi = a.to;
            Node& w = nodes[wi];
            if (v.h != w.h + 1 || !a.resCap) continue;
            Cap delta;
            {
                tbb::spin_mutex::scoped_lock lock(arcMutex(a));
                delta = min(a.resCap, v.e.load());
                increaseArcFlow(a, delta);
            }
            if (!delta) continue;
            if (STATS) {
                if (delta == a.resCap) addSatPushes++;
                else addNonSatPushes++;
            }
            v.e -= delta;
            w.e += delta;
            if (wi != sink && CAS(&w.inWorkingSet, 0, 1))
                *buf++ = make_pair(wi, 0);
            //if (a.resCap && w.h >= v.h)
                //newLabel = min(newLabel, w.h + 1);
        })
        if (!v.e) break;
        {
            tbb::spin_mutex::scoped_lock lock(v.mx);
            addWorkSinceUpdate += BETA + outDegree(v);
            intT newLabel = hiLabel;
            for_arcs(for, v, a, {
                    if (a.resCap)
                        newLabel = min(newLabel, nodes[a.to].h + 1);
            })
            if (newLabel > v.h) {
                addRelabels++;
                //if (dbg) ATOMIC(cout << "RELABEL" << endl;)
                //if (dbg) ATOMIC(cout<<"relabel "<<vi << " " << v.h << " -> " << newLabel << endl;)
                v.h = newLabel;
            } else {
                break;
            }
        }
    }
    v.pushesEnd = buf;
#if !_OPENMP && STATS
    satPushes += addSatPushes;
    nonSatPushes += addNonSatPushes;
    otherWork += addOtherWork;
    edgeScans += addEdgeScans;
#endif
    return { addWorkSinceUpdate, addRelabels };
    */
}
#else
array<ll, 2> processNode(intT i, intT vi) {
    ass(vi != source && vi != sink);

    //cout << "Processing " << vi << endl;
    bool dbg=0;
    //if (dbg) cout << "(1)processing " << vi << " del=" << del << endl;
    ll addRelabels = 0, addSatPushes = 0, addNonSatPushes = 0,
         addOtherWork = 0, addWorkSinceUpdate = 0,
         addEdgeScans = 0;
    Node& vorig = nodes[vi];
    ass(vorig.e > 0);
    if (vorig.h == hiLabel) {
        vorig.pushesEnd = vorig.pushes;
        vorig.hnew = vorig.h;
        vorig.inWorkingSet = 0;
#if GAP_HEURISTIC
        // TODO fix the bug in groupBy when this happens
        vorig.relabelsEnd = vorig.relabels;
#endif
        return {0, 0};
    }
    //if (dbg) cout << "processing " << vi << " (excess " << vorig.e << ")"    << endl;
    NodeView v = vorig.view();
    //if (dbg) cout << "    v.e = " << v.e << endl;
    ass(v.e > 0);
    int relabel = 0;
#if INCREMENTAL_GLREL
    intT lowestChangedL = numeric_limits<intT>::max();
#endif

#if RELABEL_TACTIC == 3
    while (v.e > 0) {
#endif
        intT newh = hiLabel;
        bool skipped = 0;
        //for_arcs(for, v, cur, {
        for (intT _i = v.first; _i < v.last(); ++_i) {
            Arc& cur = arcs[_i];
            if (STATS) addOtherWork++;
            if (!v.e) break;
            addEdgeScans++;
            intT wi = cur.to;
            Node& w = nodes[wi];
            //if (dbg) cout << "inspecting " << vi <<"->" << wi << " (c=" << c << ", dc=" << dc << ")" << endl;
            //if (w.e && w.h == vorig.h + 1) {
            bool admissible = v.h == w.h + 1;
//#if RELABEL_TACTIC == 3
            if (w.e) {
                bool win = vorig.h == w.h + 1 || vorig.h < w.h - 1 || (vorig.h == w.h && vi < wi);
                // TODO what if vorig.h == w.h? In that case nobody wins
                //if (!win) {
                    //// in this scenario, w could be relabeled above v and manipulate the
                    //// edge capacity, leading to non-determinism depending on whether
                    //// v gets relabeled first or w pushes first to v.
                    //// In that case, make sure that we don't relabel v invalidly
                    //newh = min(newh, hiLabel - 1);
                //}
                if (!win && admissible) {
                    skipped = 1;
                    continue;
                }
            }
//#endif

            // TODO if c && admissible, the relabel step should be after the push
            Cap c = cur.resCap;
//#if RELABEL_TACTIC != 1
#//if P2L
//            if (min(c, w.sumOutCap - enew[wi]- w.e - c) > 0 && w.h >= v.h)
//#else
            if (c && w.h >= v.h)
//#endif
            {
                newh = min(newh, w.h + 1);
            }
//#endif

            if (!c || !admissible) continue;

            Cap delta = min(c, v.e);
            if (wi != sink) {
/*#if P2L
                Cap p2lCap;
                if (mode == MODE_RACE)
                    p2lCap = max((Cap)0, w.sumOutCap - enew[wi] - w.e - cur.revResCap);
                else
                    p2lCap = max((Cap)0, w.sumOutCap - w.e - cur.revResCap);
                delta = min(delta, p2lCap);
#endif*/
/*#if EXCESS_SCALING
                Cap dc;
                if (mode == MODE_RACE)
                    dc = max((Cap)0, del - enew[wi] - w.e);
                else
                    dc = max((Cap)0, del - w.e);
                if (dc < delta) {
                    delta = dc;
                    skipped = 1;
                }
#endif*/
            }
            if (!delta) continue;
            ass(delta > 0);
            if (dbg) ATOMIC(cout << "push " << vi << " (" << v.e << ") -> " << wi << ": " << delta << endl;)
            if (STATS) {
                if (delta == c) ++addSatPushes;
                else                        ++addNonSatPushes;
            }
            v.e -= delta;
            ass(v.e >= 0);
            increaseArcFlow(cur, delta);
//#if INCREMENTAL_GLREL
//            lowestChangedL = min(lowestChangedL, w.h);
//#endif
            if (mode == MODE_RACE) {
                enew[wi] += delta;
/*#if P2L
                vorig.sumOutCap -= delta;
                w.sumOutCap += delta;
#endif*/
                if ((!w.e || wi == sink) && CAS(&w.inWorkingSet,0,1))
                    *v.pushes++ = make_pair(wi, 0);
            } else {
                *v.pushes++ = make_pair(wi, delta);
            }
        }
        if (!v.e || skipped) goto done;
        {
            // relabel
            addRelabels++;
            addWorkSinceUpdate += BETA + outDegree(v);
            if (STATS) addOtherWork += outDegree(v);

#if RELABEL_TACTIC == 1
            for_arcs(for, v, a, {
                Node& w = nodes[a.to];
#if P2L
                if (min(a.resCap, w.sumOutCap - enew[a.to]- w.e - a.revResCap) > 0 && w.h >= v.h)
#else
                if (a.resCap && w.h >= v.h)
#endif
                    newh = min(newh, w.h + 1);
            });
#endif

#if GAP_HEURISTIC
            if (newh < hiLabel) {
                *v.relabels++ = v.h;
                *v.relabels++ = hiLabel + newh;
            }
#endif
            if (dbg) {
                ATOMIC(cout << "relabel " << vi << " from " << v.h << " to "
                                << newh << " (excess left=" << v.e << ", relabel="
                                << ++relabel << ", skipped=" << skipped
                                //<< " maxLabel=" << maxLabel
                                << ")" << endl;)
            }
            ass(newh > v.h);
            v.h = newh;
            resetArcPointers(v);
#if RELABEL_TACTIC == 3
            if (v.h == hiLabel) break;
        }
#endif
    }

done:
#if INCREMENTAL_GLREL
    writeMin(&lowestChangedLabel, lowestChangedL);
#endif
    if (mode == MODE_RACE) {
        enew[vi] += v.e - vorig.e;
        *v.pushes++ = make_pair(vi, 0);
    } else {
        *v.pushes++ = make_pair(vi, v.e - vorig.e);
    }
    nodes[vi].apply(v);
#if !_OPENMP && STATS
    satPushes += addSatPushes;
    nonSatPushes += addNonSatPushes;
    otherWork += addOtherWork;
    edgeScans += addEdgeScans;
#endif
    return { addWorkSinceUpdate, addRelabels };
}
#endif

void run() {
#if !_OPENMP
    otherWork.set_value(0);
    satPushes.set_value(0);
    nonSatPushes.set_value(0);
    gapNodeExcess.set_value(0);
    gapNodes.set_value(0);
    edgeScans.set_value(0);
#endif
    relabels = 0;
    discharges = 0;

    globalRelabels = 0;
    gaps = 0;
    globRelWork = 0;
    globRelIters = 0;

    totalTime.clear();
    globalRelabelTime.clear();
    sourceSaturationTime.clear();
    gapHeuristicTime.clear();
    for (int i = 0; i < 8; ++i) phaseTime[i].clear();

    sumActive = 0;
    hiLabel = n;
    nm = ALPHA * (ll)n + (ll)m/2;
    pass = 0;

#ifdef INITIAL_RELABEL
    workSinceUpdate = numeric_limits<ll>::max();
#else
    workSinceUpdate = 0;
#endif

    totalTime.start();
    sourceSaturationTime.start();

    int oldNumThreads = getWorkers();
    int numThreads = getWorkers();
    bool sequential = 0;
    Node& S = nodes[source];
    S.h = n;
    Cap maxExcess = 0;
    // TODO allow multi-edges
    sppr_parallel_for (intT i = 0; i < outDegree(S); ++i) {
        Arc& a = arcs[S.first + i];
        ass(a.to != source);
        wSetTmp[i] = a.to;
        Cap delta = a.resCap;
        if (!delta) continue;
        ass(delta > 0);
        increaseArcFlow(a, delta);
        nodes[a.to].e += delta;
#if EXCESS_SCALING
        writeMax(&maxExcess, nodes[a.to].e);
#endif
    }
    wSetSize = sequence::filter(wSetTmp, wSet, outDegree(S),
                                                            [](intT vi) { return vi != sink && vi >= 0 && nodes[vi].e > 0; });
    wSetSize = unique(wSet, wSet + wSetSize) - wSet;
    lowestChangedLabel = 0;
#if EXCESS_SCALING
    cout << "initial max excess: " << maxExcess << endl;
    del = 1;
    while (del < maxExcess)
        del <<= 1;
#endif
#if !_OPENMP
    satPushes += outDegree(S);
#endif
    sourceSaturationTime.stop();

#if P2L
    sppr_parallel_for (intT vi = 0; vi < n; ++vi) {
        Node& v = nodes[vi];
        for_arcs(for, v, cur, {
                v.sumOutCap += cur.resCap;
        });
    }
#endif
    mode = MODE_RACE;
    while (wSetSize) {
        //cout << "iteration " << pass << " active " << wSetSize << endl;
        //cout << "    active="; for (intT i= 0; i < wSetSize; ++i) cout << setw(3) << wSet[i] << " ";cout << endl;
        //cout << "    e=         "; for (intT vi = 0; vi < n; ++vi) cout << setw(3) << nodes[vi].e << " ";cout << endl;
        //cout << "    h=         "; for (intT vi = 0; vi < n; ++vi) cout << setw(3) << nodes[vi].h << " ";cout << endl;
        //if (statsInterval > 0 && pass % statsInterval == 0) printStats();
        sumActive += wSetSize;

        if (wSetSize < 50 && !sequential) {
            sequential = 1;
            //cout<< "Falling back to sequential after " << totalTime.total() << endl;
            //mode = MODE_RACE;
            setWorkers(1);
            //cout << "Falling back to augmenting paths" << endl;
            //timer dinicTime; dinicTime.start();
            //dinic::run();
            //dinicTime.stop();
            //cout << "Dinic: " << dinicTime.total() << endl;
            //break;
        } else if (wSetSize > 50 && sequential) {
            sequential = 0;
            //cout<< "Back to parallel after " << totalTime.total() << endl;
            setWorkers(oldNumThreads);
        }

        //checkSanity();
        if (needGlobalUpdate())
            globalRelabel();
        //cout << "active = "; s.process([](intT i, intT vi) { ATOMIC(cout << vi << " ";); }); cout << endl;

        phaseTime[7].start();
        phaseTime[0].start();
        //cout << "ITER" << endl;
        discharges += wSetSize;
        array<ll,2> *infos = new array<ll,2>[wSetSize];
        sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
            intT vi = wSet[i];
            infos[i] = processNode(i, vi);
            bufSize[i] = nodes[vi].pushesEnd - nodes[vi].pushes;
        }
        phaseTime[0].stop();
        phaseTime[7].stop();

#if GAP_HEURISTIC
        gapHeuristicTime.start();
        intT *relSizes = new intT[wSetSize], *relOffset = new intT[wSetSize];
        sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
            Node& v = nodes[wSet[i]];
            relSizes[i] = v.relabelsEnd - v.relabels;
        }
        intT k = sequence::plusScan(relSizes, relOffset, wSetSize);
        intT *allRelabels = new intT[k], *relFirst = new intT[k + 1];
        allRelabels[k] = hiLabel;
        sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
                Node& v = nodes[wSet[i]];
                copy(v.relabels, v.relabelsEnd, allRelabels + relOffset[i]);
        }
        k = groupBy(allRelabels, [](intT x) { return x; }, relFirst, k, hiLabel);
        intT mid = 0;
        sppr_parallel_for (intT i = 0; i < k; ++i) {
            intT label1 = allRelabels[relFirst[i]];
            intT label2 = allRelabels[relFirst[i+1]];
            if (label1 < hiLabel && label2 >= hiLabel) mid = i+1;
        }
        ass(mid >= 0);
        intT gap = hiLabel;
        sppr_parallel_for (intT i = 0; i < mid; ++i) {
            intT label = allRelabels[relFirst[i]];
            intT count = relFirst[i+1] - relFirst[i];
            labelCounter[label] -= count;
            if (labelCounter[label] == 0)
                writeMin(&gap, label);
        }
        sppr_parallel_for (intT i = mid; i < k; ++i) {
            intT label = allRelabels[relFirst[i]];
            intT count = relFirst[i+1] - relFirst[i];
            labelCounter[label-hiLabel] += count;
        }
        delete[] relSizes;
        delete[] relOffset;
        delete[] allRelabels;
        delete[] relFirst;

        if (gap < hiLabel) {
            gaps++;
            cout << "GAP at " << gap << "!" << endl;
            sppr_parallel_for(intT vi = 0; vi < n; ++vi) {
                Node& v = nodes[vi];
                //ass(v.h != gap); // needn't be the case
                if (v.h > gap) {
                    v.h = hiLabel;
#if !_OPENMP && STATS // uses Cilk reducer
                    gapNodes++;
                    gapNodeExcess += v.e + v.enew;
#endif
                }
            }
        }
        gapHeuristicTime.stop();
#endif

        phaseTime[7].start();
        phaseTime[1].start();
        array<ll,2> info = sequence::mapReduce<array<ll,2>>(infos, wSetSize,
                                                             pointwiseAdd<ll,2>,
                                                             utils::identityF<array<ll,2>>());
        workSinceUpdate += info[0];
        relabels += info[1];
        delete[] infos;
        offset[wSetSize] = sequence::plusScan(bufSize, offset, wSetSize);
        phaseTime[1].stop();
        if (mode == MODE_SYNC) {
            ass(!EXCESS_SCALING && !P2L); // TODO implement
            phaseTime[2].start();
            if (wSetSize > 300) {
                sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
                    Node& v = nodes[wSet[i]];
#if RELABEL_TACTIC != 4
                    v.h = v.hnew;
#endif
                    copy(v.pushes, v.pushesEnd, groupedPushes + offset[i]);
                }
            } else {
                for (intT i = 0; i < wSetSize; ++i) {
                    Node& v = nodes[wSet[i]];
#if RELABEL_TACTIC != 4
                    v.h = v.hnew;
#endif
                    copy(v.pushes, v.pushesEnd, groupedPushes + offset[i]);
                }
            }
            phaseTime[2].stop();
            phaseTime[3].start();
            wSetSize = groupBy(groupedPushes, [](const pair<ll,Cap>& x) { return x.first; },
                                                first, offset[wSetSize], n);
            phaseTime[3].stop();
            phaseTime[4].start();
            if (wSetSize > 300) {
                sppr_parallel_for(intT i = 0; i < wSetSize; i++) {
                    intT vi = groupedPushes[first[i]].first;
                    Node& v = nodes[vi];
                    for (intT j = first[i]; j < first[i+1]; ++j)
                        v.e += groupedPushes[j].second;
                    wSetTmp[i] = vi;
                }
            } else {
                for (intT i = 0; i < wSetSize; i++) {
                    intT vi = groupedPushes[first[i]].first;
                    Node& v = nodes[vi];
                    intT sz = first[i+1]-first[i];
                    if (sz > 300) {
                        v.e += sequence::mapReduce<Cap>(groupedPushes + first[i], sz, utils::addF<Cap>(),
                                                                                        [](const pair<ll, Cap>& x) { return x.second; });
                    } else {
                        for (intT j = first[i]; j < first[i+1]; ++j)
                            v.e += groupedPushes[j].second;
                    }
                    wSetTmp[i] = vi;
                }
            }
            phaseTime[4].stop();
        } else { // MODE_RACE
            phaseTime[2].start();
            if (wSetSize > 300) {
                sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
                    intT vi = wSet[i];
                    Node& v = nodes[vi];
#if RELABEL_TACTIC != 4
                    v.h = v.hnew;
#endif
                    intT sz = v.pushesEnd - v.pushes;
                    intT o = offset[i];
                    for (intT j = 0; j < sz; ++j)
                        wSetTmp[o++] = v.pushes[j].first;
                }
            } else {
                for (intT i = 0; i < wSetSize; ++i) {
                    intT vi = wSet[i];
                    Node& v = nodes[vi];
#if RELABEL_TACTIC != 4
                    v.h = v.hnew;
#endif
                    intT sz = v.pushesEnd - v.pushes;
                    intT o = offset[i];
                    for (intT j = 0; j < sz; ++j)
                        wSetTmp[o++] = v.pushes[j].first;
                }
            }
            phaseTime[2].stop();
            phaseTime[3].start();
            wSetSize = offset[wSetSize];
            Cap maxExcess = 0;
            sppr_parallel_for (intT i = 0; i < wSetSize; ++i) {
                intT vi = wSetTmp[i];
                Node& v = nodes[vi];
                v.e += enew[vi];
                enew[vi] = 0;
                v.inWorkingSet = 0;
#if EXCESS_SCALING
                if (vi != sink && v.h < hiLabel)
                    writeMax(&maxExcess, v.e);
#endif
            }
#if EXCESS_SCALING
            if (maxExcess <= del/2) {
                del /= 2;
                cout << "new del = " << del << " (time " << totalTime.total() << ")" << endl;
            }
#endif
            phaseTime[3].stop();
        }
        phaseTime[5].start();
        if (wSetSize > 1000) {
            wSetSize = sequence::filter(wSetTmp, wSet, wSetSize,
                    [](intT vi) {
                        return nodes[vi].e > 0 &&
                                     nodes[vi].h < hiLabel &&
                                     vi != sink;
                    });
        } else {
            int j = 0;
            for (intT i = 0; i < wSetSize; ++i) {
                intT vi = wSetTmp[i];
                if (nodes[vi].e > 0 && nodes[vi].h < hiLabel && vi != sink)
                    wSet[j++] = vi;
            }
            wSetSize = j;
        }
        phaseTime[5].stop();
        phaseTime[7].stop();

        //t.stop(); cout << " t3 = " << t.total(); t.clear(); t.start();
        //cout << endl;
        ++pass;
    }
    totalTime.stop();
    //printStats();
    //cout << "Resetting number of threads to " << oldNumThreads << endl;
    setWorkers(oldNumThreads);
}

void prepareMaxFlow(FlowGraph<intT>& g) {
    //for (intT i = 0; i < g.g.n; ++i) {
        //FlowVertex& v = g.g.V[i];
        //for (intT j = 0; j < v.degree; ++j) {
            //intT to = v.Neighbors[j];
            //intT c = v.nghWeights[j];
            //cout << i << " -> " << to << " (" << c << ")" << endl;
        //}
    //}
    sppr_parallel_for (intT i = 1; i < n; ++i) {
        ass(g.g.V[i].Neighbors == g.g.V[i - 1].Neighbors + g.g.V[i - 1].degree);
        ass(g.g.V[i].nghWeights == g.g.V[i - 1].nghWeights + g.g.V[i - 1].degree);
    }
    intT *adj = g.g.V[0].Neighbors, *weights = g.g.V[0].nghWeights;

    //timer timeInit; timeInit.start();
    n = g.g.n, m = 2 * g.g.m;
    nodes = new Node[n + 1];
    source = g.source;
    sink = g.sink;
    //cout << "source=" << source << " sink=" << sink << endl;

    //timer t; t.start();
    nodes[n].first = m;
    Cap *cap = new Cap[m / 2];
    intT *edges = new intT[m];
    sppr_parallel_for (intT i = 0; i < n; ++i) {
        nodes[i].first = -1;
        FlowVertex& v = g.g.V[i];
        intT offset = v.Neighbors - adj;
        sppr_parallel_for (intT j = 0; j < v.degree; ++j) {
            intT pairId = offset + j;
            cap[pairId] = v.nghWeights[j];
            v.nghWeights[j] = i;
        }
    }
    sppr_parallel_for (intT i = 0; i < m; ++i) edges[i] = i;
    auto toEdge = [&] (intT idx) {
        intT i = weights[idx / 2];
        intT j = adj[idx / 2];
        if (idx & 1) swap(i, j);
        return make_pair(i, j);
    };
    //t.stop(); cout << "t1 = " << t.total() << endl; t.clear(); t.start();
    //static_assert(sizeof(__int128) >= 2 * sizeof(intT), "Need to be able to square");
    COMP_SORT(edges, edges + m, [&] (intT x, intT y) { return toEdge(x) < toEdge(y); });

    intT *firstTmp = new intT[m], *first = new intT[m];
    firstTmp[0] = 0;
    sppr_parallel_for (intT i = 1; i < m; ++i)
        firstTmp[i] = (toEdge(edges[i]).first != toEdge(edges[i - 1]).first) ? i : -1;
    intT sz = sequence::filter(firstTmp, first, m, nonNegF<intT>);
    sppr_parallel_for (intT i = 0; i < sz; ++i) {
        intT ni = toEdge(edges[first[i]]).first;
        if (ni >= 0)
            nodes[ni].first = first[i];
    }
    //t.stop(); cout << "t2 = " << t.total() << endl; t.clear(); t.start();
    delete[] firstTmp;
    delete[] first;

    for (intT i = n - 1; i >= 0; --i)
        if (nodes[i].first < 0) nodes[i].first = nodes[i + 1].first;
    //t.stop(); cout << "t3 = " << t.total() << endl; t.clear(); t.start();

    intT* pos = new intT[m];
    sppr_parallel_for (intT i = 0; i < m; ++i)
        pos[edges[i]] = i;
    arcs = new Arc[m];
    //t.stop(); cout << "t4 = " << t.total() << endl; t.clear(); t.start();
    sppr_parallel_for (intT i = 0; i < m; ++i) {
        intT idx = edges[i];
        pair<intT, intT> fromTo = toEdge(idx);
        //arcs[i].isorig = !(idx & 1);
        arcs[i].rev = pos[idx ^ 1];
        arcs[i].to = fromTo.second;
        arcs[i].resCap = (idx & 1) ? 0 : cap[idx / 2];
    }
    delete[] pos;
    delete[] cap;
    //t.stop(); cout << "t5 = " << t.total() << endl; t.clear(); t.start();

    sppr_parallel_for (intT i = 0; i < m; ++i) {
        intT pairId = edges[i] / 2;
        if (edges[i] & 1) weights[pairId] = i;
    }
    delete[] edges;

    init();
    //t.stop(); cout << "t6 = " << t.total() << endl; t.clear(); t.start();
    //timeInit.stop();
    //cout << "init time: " << timeInit.total() << endl;
}

intT maxFlow() {
    //beforeHook();
    run();
    //afterHook();
    //timer timeDeinit; timeDeinit.start();
    deinit();

    //atomic<Cap> flow(0);
    //sppr_parallel_for (intT i = 0; i < n; ++i) {
        //FlowVertex& v = g.g.V[i];
        //sppr_parallel_for (intT j = 0; j < v.degree; ++j) {
            //v.nghWeights[j] = arcs[v.nghWeights[j]].resCap;
            //if (v.Neighbors[j] == sink) flow += v.nghWeights[j];
        //}
    //}
    //ass(flow == nodes[sink].e);
    Cap flow = nodes[sink].e;
    //cout << "flow=" << flow << endl;
    delete[] nodes;
    delete[] arcs;
    //timeDeinit.stop();
    //cout << "deinit time: " << timeDeinit.total() << endl;
    return flow;
}

intT maxFlow(FlowGraph<intT>& g) {
    prepareMaxFlow(g);
    return maxFlow();
}
                 
