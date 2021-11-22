#include "hi_pr.h"

/*
parse (...) :
1. Reads maximal flow problem in extended DIMACS format.
2. Prepares internal data representation.

types: 'arc' and 'node' must be predefined

type arc  must contain fields 'head', 'rev', 'resCap'

typedef 
struct arcSt
{
long             resCap;     .. residual capasity
struct nodeSt   *head;     .. head node 
struct arcSt    *rev;   .. opposite arc 
..................
}
arc;

type   node   must contain the field 'first': 

typedef
struct nodeSt
{
arcSt        *first;    ..  first outgoing arc 
....................
}
node;
*/

namespace hi_pr {

/* ----------------------------------------------------------------- */
int HiPr::parse(
	/* all parameters are output */
	unsigned long    *n_ad,                 /* address of the number of nodes */
	unsigned long    *m_ad,                 /* address of the number of arcs */
	node    **nodes_ad,            /* address of the array of nodes */
	arc     **arcs_ad,             /* address of the array of arcs */
	cType   **cap_ad,              /* address of the array of capasities */
	node    **source_ad,           /* address of the pointer to the source */
	node    **sink_ad,             /* address of the pointer to the source */
	unsigned long    *node_min_ad          /* address of the minimal node */
)
{

#define MAXLINE       100	/* max line length in the input file */
#define ARC_FIELDS      3	/* no of fields in arc line  */
#define NODE_FIELDS     2	/* no of fields in node line  */
#define P_FIELDS        3       /* no of fields in problem line */
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

	char    in_line[MAXLINE],       /* for reading input line */
		//AS: was pr_type[3] and this was causing a bug (MSVC compiler)
		pr_type[4],             /* for reading type of the problem */
		nd;                     /* source (s) or sink (t) */

	unsigned int k;                      /* temporary */
	int     err_no;                 /* no of detected error */

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

	/* The main loop:
	-  reads the line of the input,
	-  analises its type,
	-  checks correctness of parameters,
	-  puts data to the arrays,
	-  does service functions
	*/

	while (fgets(in_line, MAXLINE, stdin) != NULL) {
		no_lines++;


		switch (in_line[0]) {
		case 'c':                  /* skip lines with comments */
		case '\n':                 /* skip empty lines   */
		case '\0':                 /* skip empty lines at the end of file */
			break;

		case 'p':                  /* problem description      */
			if (no_plines > 0)
				/* more than one problem line */
			{
				err_no = EN1; goto error;
			}

			no_plines = 1;

			if (
				/* reading problem line: type of problem, no of nodes, no of arcs */
				sscanf(in_line, "%*c %3s %ld %ld", pr_type, &n, &m)
				!= P_FIELDS
				)
				/*wrong number of parameters in the problem line*/
			{
				err_no = EN2; goto error;
			}
			if (strcmp(pr_type, PROBLEM_TYPE))
				/*wrong problem type*/
			{
				err_no = EN3; goto error;
			}

			if (n <= 0 || m <= 0)
				/*wrong value of no of arcs or nodes*/
			{
				err_no = EN4; goto error;
			}

			/* allocating memory for  'nodes', 'arcs'  and internal arrays */
			nodes = (node*)calloc(n + 2, sizeof(node));//!< +1 for 0'th node, when numbering in file starts from 1, +1 for sentinelNode
			arcs = (arc*)calloc(2 * m + 1, sizeof(arc));
			arc_tail = (unsigned long*)calloc(2 * m, sizeof(unsigned long));
			arc_first = (unsigned long*)calloc(n + 2, sizeof(unsigned long));
			acap = (cType*)calloc(2 * m, sizeof(long));

			/* arc_first [ 0 .. n+1 ] = 0 - initialized by calloc */

			if (nodes == NULL || arcs == NULL ||
				arc_first == NULL || arc_tail == NULL || acap == NULL)
				/* memory is not allocated */
			{
				err_no = EN6; goto error;
			}

			/* setting pointer to the first arc */
			arc_current = arcs;

			break;

		case 'n':		         /* source(s) description */
			if (no_plines == 0)
				/* there was not problem line above */
			{
				err_no = EN8; goto error;
			}

			/* reading source  or sink */
			k = sscanf(in_line, "%*c %ld %c", &i, &nd);

			if (k < NODE_FIELDS)
				/* node line is incorrect */
			{
				err_no = EN11; goto error;
			}

			if (i < 0 || i > n)
				/* wrong value of node */
			{
				err_no = EN12; goto error;
			}

			switch (nd) {
			case 's':  /* source line */

				if (no_nslines != 0)
					/* more than one source line */
				{
					err_no = EN9; goto error;
				}

				no_nslines = 1;
				source = i;
				break;

			case 't':  /* sink line */

				if (no_nklines != 0)
					/* more than one sink line */
				{
					err_no = EN9; goto error;
				}

				no_nklines = 1;
				sink = i;
				break;

			default:
				/* wrong type of node-line */
				err_no = EN12; goto error;
				break;
			}

			node_max = 0;
			node_min = n;
			break;

		case 'a':                    /* arc description */
			if (no_nslines == 0 || no_nklines == 0)
				/* there was not source and sink description above */
			{
				err_no = EN14; goto error;
			}

			if (no_alines >= m)
				/*too many arcs on input*/
			{
				err_no = EN16; goto error;
			}

			if (
				/* reading an arc description */
				sscanf(in_line, "%*c %ld %ld %ld",
					&tail, &head, &cap)
				!= ARC_FIELDS
				)
				/* arc description is not correct */
			{
				err_no = EN15; goto error;
			}

			if (tail < 0 || tail > n ||
				head < 0 || head > n
				)
				/* wrong value of nodes */
			{
				err_no = EN17; goto error;
			}

			/* no of arcs incident to node i is stored in arc_first[i+1] */
			arc_first[tail + 1] ++;
			arc_first[head + 1] ++;

			/* storing information about the arc */
			assert(pos_current + 1 < 2 * m);
			arc_tail[pos_current] = tail;
			arc_tail[pos_current + 1] = head;
			assert(arc_current - arcs >= 0 && arc_current - arcs + 1 < 2 * m + 1);
			arc_current->head = nodes + head;
			arc_current->resCap = cap;
			arc_current->rev = arc_current + 1;
			(arc_current + 1)->head = nodes + tail;
			(arc_current + 1)->resCap = 0;
			(arc_current + 1)->rev = arc_current;

			/* searching minimumu and maximum node */
			if (head < node_min) node_min = head;
			if (tail < node_min) node_min = tail;
			if (head > node_max) node_max = head;
			if (tail > node_max) node_max = tail;

			no_alines++;
			arc_current += 2;
			pos_current += 2;
			break;

		default:
			/* unknown type of line */
			err_no = EN18; goto error;
			break;

		} /* end of switch */
	}     /* end of input loop */

	/* ----- all is red  or  error while reading ----- */

	if (feof(stdin) == 0) /* reading error */
	{
		err_no = EN21; goto error;
	}

	if (no_lines == 0) /* empty input */
	{
		err_no = EN22; goto error;
	}

	if (no_alines < m) /* not enough arcs */
	{
		err_no = EN19; goto error;
	}

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

	/* free internal memory */
	free(arc_first); free(arc_tail);

	/* Thanks God! all is done */
	return (0);

	/* ---------------------------------- */
error:  /* error found reading input */

	printf("\nline %ld of input - %s\n",
		no_lines, err_message[err_no]);

	exit(1);

}
/* --------------------   end of parser  -------------------*/

} // namespace hi_pr
