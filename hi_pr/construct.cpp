#include "hi_pr.h"
#include <stdio.h>

//AS: added initialization from data structures, rather than a file
//AS: added destructor for HiPr

namespace hi_pr {

template<typename tcap> void HiPr::construct(unsigned int _nV, unsigned int _nE, const int * _E, const tcap * _cap, const tcap * _excess)
{

	globUpdtFreq = GLOB_UPDT_FREQ;
	//printf("c\nc HiPr version 3.6\n");
	//printf("c Copyright C by IG Systems, igsys@eclipse.net\nc\n");


	/* all parameters are output */
	unsigned long    *n_ad = &n;            /* address of the number of nodes */
	unsigned long    *m_ad = &m;            /* address of the number of arcs */
	node    **nodes_ad = &nodes;   /* address of the array of nodes */
	arc     **arcs_ad = &arcs;       /* address of the array of arcs */
	cType   **cap_ad = &cap;         /* address of the array of capasities */
	node    **source_ad = &source;   /* address of the pointer to the source */
	node    **sink_ad = &sink;       /* address of the pointer to the source */
	unsigned long    *node_min_ad = &nMin;    /* address of the minimal node */

#define PROBLEM_TYPE "max"      /* name of problem type*/


	unsigned long    n,                      /* internal number of nodes */
		node_min = 0,             /* minimal no of node  */
		node_max = 0,             /* maximal no of nodes */
		*arc_first = NULL,         /* internal array for holding
								 - node degree
								 - position of the first outgoing arc */
		*arc_tail = NULL,          /* internal array: tails of the arcs */
		source = 0,               /* no of the source */
		sink = 0,                 /* no of the sink */
		/* temporary variables carrying no of nodes */
		head, tail, i;

	unsigned long    m,                      /* internal number of arcs */
		/* temporary variables carrying no of arcs */
		last, arc_num, arc_new_num;

	node    *nodes = NULL,            /* pointer to the node structure */
		*head_p,
		*ndp;

	arc     *arcs = NULL,             /* pointer to the arc structure */
		*arc_current = NULL,
		*arc_new,
		*arc_tmp;

	cType   *acap = NULL,             /* array of capasities */
		cap;                    /* capasity of the current arc */

	unsigned long    no_lines = 0,             /* no of current input line */
		no_plines = 0,            /* no of problem-lines */
		no_nslines = 0,           /* no of node-source-lines */
		no_nklines = 0,           /* no of node-source-lines */
		no_alines = 0,            /* no of arc-lines */
		pos_current = 0;          /* 2*no_alines */

	int  err_no;                 /* no of detected error */

	/* -------------- error numbers & error messages ---------------- */
#define EN1   0
#define EN2   1
#define EN3   2
#define EN4   3
#define EN6   4
#define EN10  5
#define EN7   6
#define EN8   7
#define EN9   8
#define EN11  9
#define EN12 10
#define EN13 11
#define EN14 12
#define EN16 13
#define EN15 14
#define EN17 15
#define EN18 16
#define EN21 17
#define EN19 18
#define EN20 19
#define EN22 20

	static char *err_message[] =
	{
		/* 0*/    "more than one problem line.",
		/* 1*/    "wrong number of parameters in the problem line.",
		/* 2*/    "it is not a Max Flow problem line.",
		/* 3*/    "bad value of a parameter in the problem line.",
		/* 4*/    "can't obtain enough memory to solve this problem.",
		/* 5*/    "more than one line with the problem name.",
		/* 6*/    "can't read problem name.",
		/* 7*/    "problem description must be before node description.",
		/* 8*/    "this parser doesn't support multiply sources and sinks.",
		/* 9*/    "wrong number of parameters in the node line.",
		/*10*/    "wrong value of parameters in the node line.",
		/*11*/    " ",
		/*12*/    "source and sink descriptions must be before arc descriptions.",
		/*13*/    "too many arcs in the input.",
		/*14*/    "wrong number of parameters in the arc line.",
		/*15*/    "wrong value of parameters in the arc line.",
		/*16*/    "unknown line type in the input.",
		/*17*/    "reading error.",
		/*18*/    "not enough arcs in the input.",
		/*19*/    "source or sink doesn't have incident arcs.",
		/*20*/    "can't read anything from the input file."
	};
	/* --------------------------------------------------------------- */
	/* allocating memory for  'nodes', 'arcs'  and internal arrays */
	n = _nV + 2; //regular nodes and two explicit nodes for source and sink
	source = n - 2;
	sink = n - 1;
	m = _nE + _nV;//add s->v links or v->t links

	nodes = (node*)calloc(n + 2, sizeof(node));
	arcs = (arc*)calloc(2 * m + 1, sizeof(arc));
	arc_tail = (unsigned long*)calloc(2 * m, sizeof(unsigned long));
	arc_first = (unsigned long*)calloc(n + 2, sizeof(unsigned long));
	acap = (cType*)calloc(2 * m, sizeof(long));

	if (nodes == NULL || arcs == NULL ||
		arc_first == NULL || arc_tail == NULL)
		/* memory is not allocated */
	{
		err_no = EN6; goto error;
	}

	/* setting pointer to the first arc */
	arc_current = arcs;
	//copy data

	node_max = 0;
	node_min = n;

	for (unsigned int e = 0; e < _nE; ++e) {
		tail = _E[2 * e];
		head = _E[2 * e + 1];
		cType cap1 = _cap[2 * e];
		cType cap2 = _cap[2 * e + 1];
		add_arc(tail, head, cap1, cap2, arc_first, arc_tail, pos_current, arc_current, nodes, node_min, node_max, no_alines);
	};

	this->flow0 = 0;
	for (unsigned int v = 0; v < _nV; ++v) {
		tcap cap = _excess[v];// dont access nodes beyond nV - Ok
		if (cap == 0) {
			--m;
			continue;
		};
		if (cap > 0) {
			add_arc(source, v, cap, 0, arc_first, arc_tail, pos_current, arc_current, nodes, node_min, node_max, no_alines);
		} else {
			add_arc(v, sink, -cap, 0, arc_first, arc_tail, pos_current, arc_current, nodes, node_min, node_max, no_alines);
			this->flow0 += cap;
		};
	};

	//AS: this is a fix for sink, having no incoming arcs
	node_max = n - 1 + node_min;

	/********** ordering arcs - linear time algorithm ***********/

	/* first arc from the first node */
	(nodes + node_min)->first = arcs;

	/* before below loop arc_first[i+1] is the number of arcs outgoing from i;
	after this loop arc_first[i] is the position of the first
	outgoing from node i arcs after they would be ordered;
	this value is transformed to pointer and written to node.first[i]
	*/

	for (i = node_min + 1; i <= node_max + 1; i++) {
		arc_first[i] += arc_first[i - 1];
		(nodes + i)->first = arcs + arc_first[i];
	}


	for (i = node_min; i < node_max; i++) /* scanning all the nodes
											 exept the last*/
	{

		last = ((nodes + i + 1)->first) - arcs;
		/* arcs outgoing from i must be cited
		from position arc_first[i] to the position
		equal to initial value of arc_first[i+1]-1  */

		for (arc_num = arc_first[i]; arc_num < last; arc_num++) {
			tail = arc_tail[arc_num];

			while (tail != i)
				/* the arc no  arc_num  is not in place because arc cited here
				must go out from i;
				we'll put it to its place and continue this process
				until an arc in this position would go out from i */

			{
				arc_new_num = arc_first[tail];
				arc_current = arcs + arc_num;
				arc_new = arcs + arc_new_num;

				/* arc_current must be cited in the position arc_new
				swapping these arcs:                                 */

				head_p = arc_new->head;
				arc_new->head = arc_current->head;
				arc_current->head = head_p;

				cap = arc_new->resCap;
				arc_new->resCap = arc_current->resCap;
				arc_current->resCap = cap;

				if (arc_new != arc_current->rev) {
					arc_tmp = arc_new->rev;
					arc_new->rev = arc_current->rev;
					arc_current->rev = arc_tmp;

					(arc_current->rev)->rev = arc_current;
					(arc_new->rev)->rev = arc_new;
				}

				arc_tail[arc_num] = arc_tail[arc_new_num];
				arc_tail[arc_new_num] = tail;

				/* we increase arc_first[tail]  */
				arc_first[tail] ++;

				tail = arc_tail[arc_num];
			}
		}
		/* all arcs outgoing from  i  are in place */
	}

	/* -----------------------  arcs are ordered  ------------------------- */

	/*----------- constructing lists ---------------*/


	for (ndp = nodes + node_min; ndp <= nodes + node_max; ndp++)
		ndp->first = (arc*)NULL;

	for (arc_current = arcs + (2 * m - 1); arc_current >= arcs; arc_current--) {
		arc_num = arc_current - arcs;
		tail = arc_tail[arc_num];
		ndp = nodes + tail;
		/* avg
		arc_current -> next = ndp -> first;
		*/
		ndp->first = arc_current;
	}

	//AS: this is a fix for nodes, not having outcoming arcs
	for (ndp = nodes + node_max; ndp >= nodes + node_min; --ndp) {
		if (!ndp->first) {
			ndp->first = (ndp + 1)->first;
		};
	};

	/* ----------- assigning output values ------------*/
	*m_ad = m;
	*n_ad = node_max - node_min + 1;
	*source_ad = nodes + source;
	*sink_ad = nodes + sink;
	*node_min_ad = node_min;
	*nodes_ad = nodes + node_min;
	*arcs_ad = arcs;
	*cap_ad = acap;

	for (arc_current = arcs, arc_num = 0;
		arc_num < 2 * m;
		arc_current++, arc_num++
		)
		acap[arc_num] = arc_current->resCap;

	if (source < node_min || source > node_max)
		/* bad value of the source */
	{
		err_no = EN20; goto error;
	}

	if ((*source_ad)->first == (arc*)NULL ||
		(*sink_ad)->first == (arc*)NULL)
		/* no arc goes out of the source */
	{
		err_no = EN20; goto error;
	}

	/* ---------------------------------- */

	/* free internal memory */
	free(arc_first);
	free(arc_tail);

	//////////////////////////////////////////////////////////////////////////////////////////
	//printf("c nodes:       %10ld\nc arcs:        %10ld\nc\n", n, m);
	{
		int cc = allocDS();
		if (cc) {
			fprintf(stderr, "Allocation error\n"); exit(1);
		};
		init();

		return;
	};
error:  /* error found reading input */

	printf("%s\n", err_message[err_no]);

};

template void HiPr::construct<int>(unsigned int, unsigned int, const int *, const int *, const int *);


void HiPr::destroy()
{
	free(nodes - nMin);
	free(arcs);
	free(cap);
	free(buckets);
};

void inline HiPr::add_arc(unsigned int tail, unsigned int head, cType cap1, cType cap2, unsigned long *& arc_first, unsigned long *& arc_tail, unsigned long & pos_current, arc *& arc_current, node *& nodes, unsigned long & node_min, unsigned long & node_max, unsigned long & no_alines)
{
	/* no of arcs incident to node i is stored in arc_first[i+1] */
	arc_first[tail + 1] ++;
	arc_first[head + 1] ++;

	/* storing information about the arc */
	arc_tail[pos_current] = tail;
	arc_tail[pos_current + 1] = head;
	arc_current->head = nodes + head;
	arc_current->resCap = cap1;
	arc_current->rev = arc_current + 1;
	(arc_current + 1)->head = nodes + tail;
	(arc_current + 1)->resCap = cap2;
	(arc_current + 1)->rev = arc_current;

	/* searching minimumu and maximum node */
	if (head < node_min) node_min = head;
	if (tail < node_min) node_min = tail;
	if (head > node_max) node_max = head;
	if (tail > node_max) node_max = tail;

	no_alines++;
	arc_current += 2;
	pos_current += 2;
};

void HiPr::construct(const char * filename)
{
	freopen(filename, "r", stdin);
	this->flow0 = 0;
	parse(&n, &m, &nodes, &arcs, &cap, &source, &sink, &nMin);
	fclose(stdin);
	int cc = allocDS();
	if (cc) {
		fprintf(stderr, "Allocation error\n"); exit(1);
	};
	init();
};

} // namespace hi_pr
