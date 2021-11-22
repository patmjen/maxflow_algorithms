/* graph.cpp */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

/*
	special constants for node->parent. Duplicated in maxflow.cpp, both should match!
*/
#define TERMINAL ( (arc *) 1 )		/* to terminal */
#define ORPHAN   ( (arc *) 2 )		/* orphan */

template <typename captype, typename tcaptype, typename flowtype> 
	Graph<captype, tcaptype, flowtype>::Graph(size_t node_num_max, size_t edge_num_max, void (*err_function)(const char *))
	: node_num(0),
	  nodeptr_block(NULL),
	  error_function(err_function)
{
	if (node_num_max < 16) node_num_max = 16;
	if (edge_num_max < 16) edge_num_max = 16;

	nodes = (node*) malloc(node_num_max*sizeof(node));
	arcs = (arc*) malloc(2*edge_num_max*sizeof(arc));
	if (!nodes || !arcs) { if (error_function) (*error_function)("Not enough memory!"); exit(1); }

	node_last = nodes;
	node_max = nodes + node_num_max;
	arc_last = arcs;
	arc_max = arcs + 2*edge_num_max;

	maxflow_iteration = 0;
	flow = 0;
}

template <typename captype, typename tcaptype, typename flowtype>
Graph<captype, tcaptype, flowtype>::Graph(Graph&& other) noexcept :
	nodes(other.nodes),
	node_last(other.node_last),
	node_max(other.node_max),
	arcs(other.arcs),
	arc_last(other.arc_last),
	arc_max(other.arc_max),
	node_num(other.node_num),
	nodeptr_block(other.nodeptr_block),
	flow(other.flow),
	maxflow_iteration(other.maxflow_iteration),
	changed_list(other.changed_list),
	queue_first{ other.queue_first[0], other.queue_first[1] },
	queue_last{ other.queue_last[0], other.queue_last[1] },
	orphan_first(other.orphan_first),
	orphan_last(other.orphan_last),
	TIME(other.TIME)
{
	other.nodes = nullptr;
	other.node_last = nullptr;
	other.node_max = nullptr;
	other.arcs = nullptr;
	other.arc_last = nullptr;
	other.arc_max = nullptr;
	other.node_num = 0;
	other.nodeptr_block = nullptr;
	other.flow = 0;
	other.maxflow_iteration = 0;
	other.changed_list = nullptr;
	other.queue_first[0] = nullptr;
	other.queue_first[1] = nullptr;
	other.queue_last[0] = nullptr;
	other.queue_last[1] = nullptr;
	other.orphan_first = nullptr;
	other.orphan_last = nullptr;
	other.TIME = 0;
}

template <typename captype, typename tcaptype, typename flowtype> 
	Graph<captype,tcaptype,flowtype>::~Graph()
{
	if (nodeptr_block) 
	{ 
		delete nodeptr_block; 
		nodeptr_block = NULL; 
	}
	free(nodes);
	free(arcs);
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reset()
{
	node_last = nodes;
	arc_last = arcs;
	node_num = 0;

	if (nodeptr_block) 
	{ 
		delete nodeptr_block; 
		nodeptr_block = NULL; 
	}

	maxflow_iteration = 0;
	flow = 0;
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reallocate_nodes(int num)
{
	int node_num_max = (int)(node_max - nodes);
	node* nodes_old = nodes;

	node_num_max += node_num_max / 2;
	if (node_num_max < node_num + num) node_num_max = node_num + num;
	nodes = (node*) realloc(nodes_old, node_num_max*sizeof(node));
	if (!nodes) { if (error_function) (*error_function)("Not enough memory!"); exit(1); }

	node_last = nodes + node_num;
	node_max = nodes + node_num_max;

	if (nodes != nodes_old)
	{
		node* i;
		arc* a;
		for (i=nodes; i<node_last; i++)
		{
			if (i->next) i->next = (node*) ((char*)i->next + (((char*) nodes) - ((char*) nodes_old)));
		}
		for (a=arcs; a<arc_last; a++)
		{
			a->head = (node*) ((char*)a->head + (((char*) nodes) - ((char*) nodes_old)));
		}
	}
}

template <typename captype, typename tcaptype, typename flowtype> 
	void Graph<captype,tcaptype,flowtype>::reallocate_arcs()
{
	int arc_num_max = (int)(arc_max - arcs);
	int arc_num = (int)(arc_last - arcs);
	arc* arcs_old = arcs;

	arc_num_max += arc_num_max / 2; if (arc_num_max & 1) arc_num_max ++;
	arcs = (arc*) realloc(arcs_old, arc_num_max*sizeof(arc));
	if (!arcs) { if (error_function) (*error_function)("Not enough memory!"); exit(1); }

	arc_last = arcs + arc_num;
	arc_max = arcs + arc_num_max;

	if (arcs != arcs_old)
	{
		node* i;
		arc* a;
		for (i=nodes; i<node_last; i++)
		{
			if (i->first) i->first = (arc*) ((char*)i->first + (((char*) arcs) - ((char*) arcs_old)));
			if (i->parent && i->parent != ORPHAN && i->parent != TERMINAL) i->parent = (arc*) ((char*)i->parent + (((char*) arcs) - ((char*) arcs_old)));
		}
		for (a=arcs; a<arc_last; a++)
		{
			if (a->next) a->next = (arc*) ((char*)a->next + (((char*) arcs) - ((char*) arcs_old)));
			a->sister = (arc*) ((char*)a->sister + (((char*) arcs) - ((char*) arcs_old)));
		}
	}
}

#include "instances.inc"
