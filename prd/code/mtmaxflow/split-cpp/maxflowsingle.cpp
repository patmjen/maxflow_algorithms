//
// Petter Strandmark 2009--2010
// petter@maths.lth.se
//

#include <stdexcept>
#include <new>
using namespace std;	

#define TIMING
//Prints lots of timing information if defined
#define PRINT_TIMING
#include "mextiming.h"
#include "cppmatrix.h"

#include "maxflow-v3.01/graph.h"

//Error function to maxflow library
void err_func(char* err)
{
	throw runtime_error(err);
}


template<typename Type>
void mexFunctionReal(int			nlhs, 		/* number of expected outputs */
				     mxArray		*plhs[],	/* mxArray output pointer array */
				     int			nrhs, 		/* number of inputs */
				     const mxArray	*prhs[]		/* mxArray input pointer array */)
{

    startTime();
	// input checks
	if (nrhs < 2 || nlhs <= 0 || !mxIsSparse(prhs[0]) || !mxIsSparse(prhs[1]))
	{
		mexErrMsgTxt ("USAGE: [flow,labels] = maxflowdual(A,T,[split, margin, bidirectional])");
	}
	const mxArray *A = prhs[0];
	const mxArray *T = prhs[1];
	if (mxIsComplex(A) || mxIsComplex(T))
	{
		mexErrMsgTxt ("Complex entries are not supported!");
	}
	// fetch its dimensions
	// actually, we must have m=n
	unsigned m = mxGetM(A);
	unsigned n = mxGetN(A);
	unsigned nzmax = mxGetNzmax(A);
	if (m != n)
	{
		mexErrMsgTxt ("Matrix A should be square!");
	}
	if (n != mxGetM(T) || mxGetN(T) != 2)
	{
		mexErrMsgTxt ("T should be of size Nx2");
	}
    
	
	bool bidirectional = 0;
	if (nrhs >= 3) {
        const mxArray* mxbidir = prhs[2];

		if ( !mxIsDouble(mxbidir) || mxGetNumberOfElements(mxbidir) != 1 ) {
			mexErrMsgTxt("Bidirectional flag must be a double scalar");
		}
		bidirectional = (*(double*)mxGetPr(mxbidir)) != 0;
	}
    
    endTime("Input check");
	


	//
	// Create graph
	//
	double *pr = mxGetPr(A);
	mwIndex* ir = mxGetIr(A);
	mwIndex* jc = mxGetJc(A);
	
	Graph<Type,Type,Type> pgraph( n, /*guess*/ 5*n );
	pgraph.add_node(n);
	
	// traverse the adjacency matrix and add n-links
	unsigned int i, j, k;
	float v,vret;
	
	
	for (j = 0; j < n; j++)
	{
		// this is a simple check whether there are non zero
		// entries in the j'th column
		if (jc[j] == jc[j+1])
		{
			continue; // nothing in this column
		}
		for (k = jc[j]; k <= jc[j+1]-1; k++)
		{
			i = ir[k];
			v = (float)pr[k];
			vret = 0.0f;
			if (bidirectional) {
				vret = v;
			}
			
			pgraph.add_edge(i,j,v,vret);
		}
	}


 
	// traverse the terminal matrix and add t-links
	pr = mxGetPr(T);
	ir = mxGetIr(T);
	jc = mxGetJc(T);
	for (j = 0; j <= 1; j++)
	{
		if (jc[j] == jc[j+1])
		{
			continue;
		}
		for (k = jc[j]; k <= jc[j+1]-1; k++)
		{
			i = ir[k];
			v = (float)pr[k];

			if (j == 0) 
			{
				// source weight
				pgraph.add_tweights(i, v, 0.0f);
			}
			else if (j == 1) 
			{
				// sink weight
				pgraph.add_tweights(i, 0.0f, v);			
			}
		}
	}

    endTime("Building graph");
    


	pgraph.maxflow();
	
	//Maxflow computation done.
	double time_taken = endTime("Maximum flow");
	double is_global = 1;

	//
	// Figure out segmentation
	//
	if (nlhs > 0) {
		matrix<signed char> labels(n,1);
		plhs[0] = labels;
		for (i=0;i<n;++i) {
			labels[i] = pgraph.what_segment(i);
		}
	}
	
    endTime("Building labels");
    
    
    if (nlhs > 1) {
		matrix<double> mxis_global(1,1);
        plhs[1] = mxis_global;
        mxis_global(0) = is_global;
    }
    
    if (nlhs > 2) {
		matrix<double> mx_time_taken(1,1);
        plhs[2] = mx_time_taken;
        mx_time_taken(0) = time_taken;
    }
    
    
    endTime("Clean-up");
}




void mexFunction(int			nlhs, 		/* number of expected outputs */
				 mxArray		*plhs[],	/* mxArray output pointer array */
				 int			nrhs, 		/* number of inputs */
				 const mxArray	*prhs[]		/* mxArray input pointer array */)
{
	try {
		#ifdef USE_INTEGER
			mexFunctionReal<int>(nlhs,plhs,nrhs,prhs);
		#else
			mexFunctionReal<float>(nlhs,plhs,nrhs,prhs);
		#endif
	}
	catch (bad_alloc& ) {
		mexErrMsgTxt("Out of memory");
	}
	catch (exception& e) {
		mexErrMsgTxt(e.what());
	}
	catch (...) {
		mexErrMsgTxt("Unknown exception");
	}
}