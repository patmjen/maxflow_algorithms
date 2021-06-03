#ifndef imatrix_h
#define imatrix_h

#include "exttype/ivector.h"
#include "exttype/itern.h"
#include "exttype/convolution.h"
#include "debug/except.h"

//#include "intmatrix.h"

namespace geom{
	//___________________Imatrix_________________________________________________________
	/*!
		Requires:
			typename base::tvect;
			typename base::field;
			typename base::tindex;
			typename base::titerator;
			mint2 base::size();
			int base::count(int);
			field base::operator[](mint2);
			tvect base::operator[](int);
			----
			field operator = (0);
			field operator = (1);
			----
			target::target(mint2);
			titerator target::get_iterator()
	*/
	template <class target, class base> class imatrix : public ivector<target, base >{
	public:
		using base::size;
		using base::count;
		//mint2 size()const{return base::size();};

		typedef typename base::tvect tvect;
		typedef typename base::telement field;
		typedef typename base::tindex tindex;
		typedef typename base::titerator titerator;
		typedef target tmatrix;
	protected:
//		matrix_ rankminor(const intmatrix& ss)const;
	private:
		target* ths(){return static_cast<target*>(this);};
		const target* ths()const{return static_cast<const target*>(this);};
	private:
		target cpy()const{return target(*ths());};
	public:
		target operator *(const field & k)const{
			return cpy()*=k;
		};
		target operator *(const tmatrix& x)const{
			tmatrix r; 
			convolution<1,0>(r,(*this),x); 
			return r;
		};
		template<class vector, class vector_base>  vector operator *(const ivector<vector, vector_base> & x)const{
			vector r; 
			convolution<1,0>(r,*ths(),static_cast<const vector&>(x));
			return r;
		};
	public:
		//! assignment
		template <class target2, class base2> target& operator << (const imatrix<target2, base2> & y){
			int sz = std::min(count(),y.count());
			for(int i=0;i<sz;++i){
				(*this)[i] << y[i];
			};
			return *ths();
		};
		//! assignment
		tmatrix& operator << (const field & c){
			for(iter2 ii(size());ii.allowed();++ii){
				(*this)[ii] = c;
			};
			return *ths();
		};

		void fill(const field & c){
			(*this)<<c;
		};

		static target eye(int n){
			target A(mint2(n,n));
			A.fill(0);
			for(int i=0;i<n;++i){
				A[mint2(i,i)]=1;
			};
			return A;
		};

		static target zeros(int m,int n){
			target A(m,n);
			A.fill(0);
			return A;
		};

		target transp()const{
			target A(size().reverse());
			for(titerator ii(size());ii.allowed();++ii){
				A[ii]=(*this)[ii.reverse()];
			};
			return A;
		};


		void swap_rows(int i,int j){
			for(int k=0;k<count(1);++k){
				std::swap((*this)[mint2(i,k)],(*this)[mint2(j,k)]);
			};
		};

		target invert()const;	};

	//____________________________implementation____________________________________________

	template<class target, class base> target imatrix<target, base>::invert()const{
		field s=1;
		int i,j,k;
		if(count(0)!=count(1))throw debug_exception("can not invert nonsquare matrix");
		int n = count(0);
		target A = cpy();
		target I = eye(n);
		for(i=0;i<n;i++){
			j=i;
			double max=math::abs(A[j][i]);
			for(int j1=i+1;j1<n;j1++)if(math::abs(A[j1][i])>max){
				max = math::abs(A[j1][i]);
				j = j1;
			};
			if(max==0.0)assert(0 && "invert failed: det=0");
			if((j-i)%2)s=-s;
			if(j!=i){A.swap_rows(i,j);I.swap_rows(i,j);};
			field A_ii = A[i][i];
			s*=A_ii;
			I[i]/=A_ii;
			A[i]/=A_ii;
			for(k=0;k<n;k++)if(k!=i){
				field A_ki = A[mint2(k,i)];
				I[k]-=(I[i]*A_ki);
				A[k]-=(A[i]*A_ki);
			};
		};
		return I;
	};
};
#endif
