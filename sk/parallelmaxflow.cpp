#include "parallelmaxflow.h"
#include <stdexcept>

namespace sk {

//Error function to maxflow library
void err_func(const char* err)
{
	throw std::runtime_error(err);
}

template<> void ParallelGraph<int, int, int>::update_graph(
	int s, int i, int iter, signed char diff, signed char& prev_diff, int& step, bool& has_flipped)
{
	//without this some algorithm does not terminate on some trivial examples	
	if (iter > 15 && rand() % 2 == 0) {
		//Do nothing for possibly faster convergence
		return;
	}


	//If previous diff was 0 and diff nonzero, maybe
	//reset the step length?	
	if (prev_diff * diff == -1) {
		//Both labels flipped during last maxflow computation
		step = step >> 1;
		if (step < 1) {
			step = 1;
		}
		has_flipped = true;
	} else if (prev_diff * diff == 1 && !has_flipped) {
		step = step << 1;
	}

	//Update last known difference
	prev_diff = diff;

	int change = diff * step;

	//Change first graph
	//int cap = graph[s]->get_trcap(i-offset[s]);
	//graph[s]->set_trcap(i-offset[s], cap + change);
	graph[s]->add_tweights(i - offset[s], change, 0);
	//Change second graph
	//cap = graph[s+1]->get_trcap(i-offset[s+1]);
	//graph[s+1]->set_trcap(i-offset[s+1], cap - change);
	graph[s + 1]->add_tweights(i - offset[s + 1], -change, 0);

	//Mark the nodes as possibly changed
	graph[s]->mark_node(i - offset[s]);
	graph[s + 1]->mark_node(i - offset[s + 1]);
}

} // namespace sk