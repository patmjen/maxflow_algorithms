#include "graph.h"

typedef wghVertex<intT> FlowVertex;

// provided by maxFlowTime.C, should be called right before and right after
// the time-intensive flow computation
void beforeHook();
void afterHook();


// g will be modified by the algorithm to contain the flow values
// on each edge
void prepareMaxFlow(FlowGraph<intT>& g);

// The return value will be the value of the
// maximum flow / minimum cut
intT maxFlow();

// g will be modified by the algorithm to contain the flow values
// on each edge, the return value will be the value of the
// maximum flow / minimum cut
intT maxFlow(FlowGraph<intT>& g);
