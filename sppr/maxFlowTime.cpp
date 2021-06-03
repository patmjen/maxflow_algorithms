// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <sstream>
#include <algorithm>
#ifdef CILKP
#include <cilk/cilk_api.h>
#endif
#include "gettime.h"
#include "utils.h"
#include "graph.h"
#include "parallel.h"
#include "IO.h"
#include "graphIO.h"
#include "parseCommandLine.h"
#include "maxFlow.h"
using namespace std;
using namespace benchIO;

int oldNWorkers;
void beforeHook() {
  cout << "Switching back to " << oldNWorkers << " threads" << endl;
  setWorkers(oldNWorkers);
  startTime();
}

void afterHook() {
  nextTimeN();
}

void timeMaxFlow(FlowGraph<intT> g, int rounds, char* outFile) {
  FlowGraph<intT> gn = g.copy();
  for (int i = 0; i < rounds; i++) {
    if (i > 0) {
      gn.del();
      gn = g.copy();
    }

    oldNWorkers = getWorkers();
    cout << "Temporarily switching from " << oldNWorkers
            << " to 32 threads" << endl;
    setWorkers(32);
    maxFlow(gn);
  }
  if (outFile) {
    timer t; t.start();
    ofstream out(outFile, ofstream::binary);
    writeFlowGraph(out, gn);
    t.stop();
    cout << "writing time: " << t.total() << endl;
  }
}

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"[-o <outFile>] [-r <rounds>] <inFile>");
  char* iFile = P.getArgument(0);
  char* oFile = P.getOptionValue("-o");
  int rounds = P.getOptionIntValue("-r",1);
  intT S, T;
  timer t;
  t.start();
  ifstream in(iFile, ifstream::binary);
  FlowGraph<intT> g = readFlowGraph<intT>(in);
  t.stop();
  cout << "reading time: " << t.total() << endl;
  timeMaxFlow(g, rounds, oFile);
}
