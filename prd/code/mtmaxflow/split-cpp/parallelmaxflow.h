//
// Parallel maximum flow computation
//
// Petter Strandmark 2009
//


/***

PROGRAM FLOW FOR ONE ITERATION
==============================

After the working threads are created:

Main   T1    T2
       |     |
       |     |   maxflow computation
       V     V
---------------barrier
  |
  |              graph updating
  V
---------------barrier 
	
repeat unless global solution found


***/
#ifndef parallelmaxflow_h
#define parallelmaxflow_h

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
using boost::thread;
using boost::ref;
using boost::shared_ptr;
using boost::barrier;
#include <vector>
#include <iostream>
using std::vector;
//BK algorithm
#include "maxflow/graph.h"

const int max_num_threads = 16;

//Error function to maxflow library
void err_func(char* err);


//
// The worker structure which represents each thread
//
template <typename captype, typename tcaptype, typename flowtype> 
struct MaxFlowWorker
{
    shared_ptr<Graph<captype,tcaptype,flowtype> > g;
    flowtype flow;
	barrier& barr;
	
    MaxFlowWorker(shared_ptr<Graph<captype,tcaptype,flowtype> > g, barrier& _barr) :
		barr(_barr)
    {
        flow = 0;
        this->g = g;
    }

	//	
	//This is executed in the worker thread
	//	
	void operator()()
    {
		bool reuse_trees = false;
		
		while(1) {
			//First maxflow without reusing trees, then enable it.
			//This is essential for good performance
			flow = g->maxflow(reuse_trees);
			reuse_trees = true;

			//Wait for everyone to finish
			barr.wait();

			// ( Now the main thread processes the graphs... )
			
			//Wait for main thread to finish processing the graphs. 
			try {
				barr.wait();
			} catch (boost::thread_interrupted&) {
				//The main thread has concluded that no further
				//processing is neccessary
				return;
			}
		} 
    }
};

//
// Main class
//
template <typename captype, typename tcaptype, typename flowtype> 
class ParallelGraph
{
public:
	//! nnE[] - exact number of edges in each subgraph, =NULL if not knowwn
	ParallelGraph(int nThreads, int nodes, int edges, int split[], int margin, int * nnE = 0);

	void add_edge(int i, int j, captype v, captype vret);
	void add_tweights(int i, tcaptype source, tcaptype sink);
	
	char what_segment(int i);
	
	void maxflow();
	
	void update_graph(int s, int gpos, int iter, signed char diff, signed char& prev_diff, captype& step, bool& has_flipped);
	
	flowtype flow;
	double time_taken;
	bool is_global;
public:
	int iter;
	
private:
	int nThreads,nSplit;
	
	int offset[max_num_threads];
	int nodes[max_num_threads];
	int margin;
	
	//To avoid horrible C++ syntax
	typedef Graph<captype,tcaptype,flowtype> GraphType;
	typedef shared_ptr<GraphType> graph_ptr;
	typedef shared_ptr<MaxFlowWorker<captype,tcaptype,flowtype> > worker_ptr;
	typedef shared_ptr<thread> thread_ptr;
	
	captype* steps[max_num_threads];
	bool *has_flipped[max_num_threads];
	signed char* prev_diff[max_num_threads];

	vector<graph_ptr> graph;
};




//
// Constructor
// 
template <typename captype, typename tcaptype, typename flowtype> 
ParallelGraph<captype,tcaptype,flowtype>::ParallelGraph(int nThreads, int n, int edges, int split[], int margin, int * nnE) :
	graph(nThreads)
{
	this->nThreads = nThreads;
	nSplit = nThreads - 1;
	this->margin = margin;
	

	offset[0] = 0;
	nodes[0]  = split[0] + margin;
	for (int i=0; i<nSplit-1; ++i) {
		offset[i+1] = split[i]; 
		nodes[i+1]  = split[i+1]-split[i] + margin;
	}
	offset[nSplit] = split[nSplit-1]; 
	nodes[nSplit]  = n-split[nSplit-1];
	offset[nSplit+1] = n;
	
	//Create the subgraphs
	for (int g = 0; g < nThreads; ++g) {
		//We know the # of nodes
		int est_nodes = nodes[g];
		//Estimated # of edges
		int est_edges;
		if(!nnE){
			est_edges = edges / nThreads + 4*margin + 10;
		}else{
			est_edges = nnE[g];
		};
		graph[g] = graph_ptr( new GraphType(est_nodes,est_edges, /*temp error handler*/err_func) );
		graph[g]->add_node(est_nodes);
	}
	
	
}

//
// add_edge
// 
template <typename captype, typename tcaptype, typename flowtype> 
void ParallelGraph<captype,tcaptype,flowtype>::add_edge(int i, int j, captype v, captype vret)
{
	//Not the most efficient solution
	for (int g = 0; g < nThreads; ++g) {	
		if ( i < offset[g+1]+margin && 
			 j < offset[g+1]+margin && 
			 i >= offset[g] &&
			 j >= offset[g]) {
			//Within this graph
				 
			if (g>0 && 
				i < offset[g]+margin &&
				j < offset[g]+margin) {
				//Should be added to both this graph and 
				//the previous one, but half each
				graph[g]  ->add_edge(i-offset[g]  , j-offset[g]  , v/2, vret/2);
				graph[g-1]->add_edge(i-offset[g-1], j-offset[g-1], v/2, vret/2);	
			}
			else if (g<nThreads-1 &&
					 i >= offset[g+1] &&
					 j >= offset[g+1] ) {
				//Should be added to both this graph and 
				//the next one, but half each
				graph[g]  ->add_edge(i-offset[g]  , j-offset[g]  , v/2, vret/2);
				graph[g+1]->add_edge(i-offset[g+1], j-offset[g+1], v/2, vret/2);
			}
			else {
				//Only within this graph
				graph[g]->add_edge(i-offset[g], j-offset[g], v, vret);		
			}
			
			break;
		}
	}
}

//
// add_tweights
// 
template <typename captype, typename tcaptype, typename flowtype> 
void ParallelGraph<captype,tcaptype,flowtype>::add_tweights(int i, tcaptype source, tcaptype sink)
{
	//Not the most efficient solution
	for (int g = 0; g < nThreads; ++g) {
		if ( i < offset[g+1]+margin && 
			 i >= offset[g]) {
			//Within this graph
				 
			if (g>0 && i < offset[g]+margin) {
				//Should be added to both this graph and 
				//the previous one, but half each
				graph[g]  ->add_tweights(i-offset[g],   source/2, sink/2);
				graph[g-1]->add_tweights(i-offset[g-1], source/2, sink/2);	
			}
			else if (g<nThreads-1 && i >= offset[g+1]) {
				//Should be added to both this graph and 
				//the next one, but half each
				graph[g]  ->add_tweights(i-offset[g],   source/2, sink/2);
				graph[g+1]->add_tweights(i-offset[g+1], source/2, sink/2);
			}
			else {
				//Only within this graph	
				graph[g]->add_tweights(i-offset[g], source, sink);
			}
			
			break;
		}
	}
}

//
// what_segment
// 
template <typename captype, typename tcaptype, typename flowtype> 
char ParallelGraph<captype,tcaptype,flowtype>::what_segment(int i)
{
	for (int g = 0; g < nThreads; ++g) {
		if ( i < offset[g+1]+margin && 
			 i >= offset[g]) {
			//Within this graph
			return graph[g]->what_segment(i - offset[g]);
		}
	}
}


//
// maxflow
// 
template <typename captype, typename tcaptype, typename flowtype> 
void ParallelGraph<captype,tcaptype,flowtype>::maxflow()
{
	time_taken = 0;
	startTime();
	
	//Parameters for the algorithm
	captype stepsize = 10;
	
	
	
	//
	// Allocate memory for the steplengths and previous difference
	//
	for (int s = 0; s < nSplit; ++s) {
		steps[s]     = new captype[margin];
		has_flipped[s]  = new bool[margin];
		prev_diff[s] = new signed char[margin]; 

		for (int i = 0; i < margin; ++i) {
			steps[s][i] = stepsize;
			has_flipped[s][i] = false;
			prev_diff[s][i] = 0;
		}
	}
	
	
	//
	// Create and start the maxflow computations
	// for each subgraph
	//
	barrier barr(nThreads + 1); //number of computation threads + main thread
		
	vector<worker_ptr> workers(nThreads);
	vector<thread_ptr> threads(nThreads);
	
	for (int g=0; g<nThreads; ++g) {
		workers[g] = worker_ptr( new MaxFlowWorker<captype,tcaptype,flowtype>(graph[g], barr) );
		threads[g] = thread_ptr( new thread( ref(*workers[g]) ) );
	}
	
	is_global = false;
	for (iter=1; iter<=1000; ++iter) {	
		//
		// Wait for every computation to finish
		// 
		barr.wait();	
		
		//
		// Go through the common nodes and look at the
		// residual capacity
		//
		int nDiff = 0;
		for (int s = 0; s < nSplit; ++s) {
			
			for (int i=offset[s+1];i<offset[s+1]+margin; ++i) {
				signed char l1 = graph[s]->what_segment(i-offset[s]);
				signed char l2 = graph[s+1]->what_segment(i-offset[s+1]);
				//Are the solutions different?
				if (l1 != l2) {
					signed char diff = l1 - l2;
					update_graph(s,i,iter,diff,prev_diff[s][i-offset[s+1]], steps[s][i-offset[s+1]],has_flipped[s][i-offset[s+1]]);
					nDiff++;
				}	
			}
			
		}
		

		if (nDiff == 0) {
			// We are done and we have the maximum flow
			is_global = true;
			break;
		}
        
		//Finished processing the graph data: ready for another computation
		//The other threads are already waiting at this barrier
		barr.wait();
		
		time_taken += endTime("Max flow iteration %3d, ndiff : %4d",iter,nDiff);
    }
	
	//Notify threads that no more processing needs to be done.
	for (int g=0; g<nThreads; ++g) {
		threads[g]->interrupt();
	}
    
	flow = 0;
	for (int g=0;g<nThreads;++g) {
		flow += workers[g]->flow;
	}
	
	for (int s = 0; s < nSplit; ++s) {
		delete[] steps[s];
		delete[] prev_diff[s];
		delete[] has_flipped[s];
	}
    
	//Wait for the threads to completely finish
	for (int g=0; g<nThreads; ++g) {
		threads[g]->join();
	}
	std::cout<<"iterations: "<<iter<<"\n";
	time_taken += endTime("Kolmogorov");
}


template <typename captype, typename tcaptype, typename flowtype> 
void ParallelGraph<captype,tcaptype,flowtype>::update_graph(int s, int i, int iter, signed char diff, signed char& prev_diff, captype& step, bool& has_flipped)
{
	captype damping  = 0.5;
	
	//If previous diff was 0 and diff nonzero, maybe
	//reset the step length?	
	if (prev_diff * diff == -1) { 
		//Both labels flipped during last maxflow computation
		step = step * damping;
		has_flipped = true;		
	}
	else if (prev_diff * diff == 1  && !has_flipped) {	
		step = step / damping;
	}
	//Update last known difference
	prev_diff = diff;
	
	captype change = diff * step;

	//Change first graph
	captype cap = graph[s]->get_trcap(i-offset[s]);
	graph[s]->set_trcap(i-offset[s], cap + change);
	//Change second graph
	cap = graph[s+1]->get_trcap(i-offset[s+1]);
	graph[s+1]->set_trcap(i-offset[s+1], cap - change);
	
	//Mark the nodes as possibly changed
	graph[s]  ->mark_node(i-offset[s]);
	graph[s+1]->mark_node(i-offset[s+1]);
}

template<> void ParallelGraph<int,int,int>::update_graph(int s, int i, int iter,  signed char diff, signed char& prev_diff, int& step, bool& has_flipped);

#endif