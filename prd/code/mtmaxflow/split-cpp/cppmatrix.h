//
// CPPMATRIX
//
// C++ wrapper for Matlab matrices 
//
// Petter Strandmark 2010
//


#ifndef CPPMATRIX_MATLAB_HEADER
#define CPPMATRIX_MATLAB_HEADER

#include <sstream>
#include <algorithm>
#include <stdexcept>

#include "mex.h"
#include "mexutils.h"

template<typename T>
class matrix
{
public:
    
	T* data;
    mxArray* array;
	mwSize M,N,O;
	
	matrix(const mxArray* array)
	{
		this->array = (mxArray*)array; //Hack to allow const mxArrays
		ASSERT(!mxIsSparse(array));		
		ASSERT(mxGetClassID(array) == typeToID());

		int ndim = mxGetNumberOfDimensions(array);
		ASSERT(ndim<=3);
		if (ndim <= 2) {
			M = mxGetM(array);
			N = mxGetN(array);	
			O = 1;
		}
		else {
			const mwSize* dims = mxGetDimensions(array);
			M = dims[0];
			N = dims[1];
			O = dims[2];
		}
		data = (T*)mxGetPr(array);
		
		shouldDestroy = false;
	}

	matrix(int M, int N=1, int O=1)
	{
		this->M = M;
		this->N = N;
		this->O = O;
		if (O==1) {
			mwSize size[] = {M,N};
			this->array = mxCreateNumericArray(2,size,typeToID(),mxREAL);
		}
		else {
			mwSize size[] = {M,N,O};
			this->array = mxCreateNumericArray(3,size,typeToID(),mxREAL);
		}
		data = (T*)mxGetPr(array);
		
		shouldDestroy = true;
	}
	
	matrix(const matrix& org) 
	{
		*this = org;
	}
	
	matrix()
	{
		M = N = O = 0;
		data = 0;
		array = 0;
	}
	
	~matrix()
	{
		if (shouldDestroy) {
			mxDestroyArray(array);
		}
	}
	
	mwSize numel() 
	{
		return M*N*O;
	}
	
	void operator=(const matrix& org) 
	{
		if (org.shouldDestroy) {
			throw std::runtime_error("matrix() : Cannot copy a managed matrix");
		}
		shouldDestroy = false;
		M = org.M;
		N = org.N;
		O = org.O;
		data = org.data;
		array = org.array;
	}
	
	T& operator[](mwSize i)
	{
		return data[i];
	}
	T& operator()(mwSize i)
	{
		return data[i];
	}
	T& operator()(mwSize i, mwSize j)
	{
		return data[i + M*j];
	}
	T& operator()(mwSize i, mwSize j, mwSize k)
	{
		return data[i + M*j + M*N*k];
	}
	
	operator mxArray*()
	{
		shouldDestroy = false;
		return array;
	}
    
    T min() 
    {
        return *std::min_element(data, data+numel());
    }
    
    T max()
    {
        return *std::max_element(data, data+numel());
    }
	
private:
	
	bool shouldDestroy;
	
	mxClassID typeToID()
	{
		mexErrMsgTxt("Unknown type!");
		return 0;
	}
};

template<>
mxClassID matrix<double>::typeToID()
{
	return mxDOUBLE_CLASS;
}
template<>
mxClassID matrix<float>::typeToID()
{
	return mxSINGLE_CLASS;
}
template<>
mxClassID matrix<unsigned char>::typeToID()
{
	return mxUINT8_CLASS;
}
template<>
mxClassID matrix<signed char>::typeToID()
{
	return mxINT8_CLASS;
}
template<>
mxClassID matrix<unsigned int>::typeToID()
{
	ASSERT(sizeof(unsigned int)==4);
	return mxUINT32_CLASS;
}
template<>
mxClassID matrix<signed int>::typeToID()
{
	ASSERT(sizeof(signed int)==4);
	return mxINT32_CLASS;
}
template<>
mxClassID matrix<unsigned short>::typeToID()
{
	ASSERT(sizeof(unsigned short)==4);
	return mxUINT16_CLASS;
}
template<>
mxClassID matrix<signed short>::typeToID()
{
	ASSERT(sizeof(signed short)==4);
	return mxINT16_CLASS;
}

#endif
