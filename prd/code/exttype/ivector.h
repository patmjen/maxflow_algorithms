#ifndef ivector_h
#define ivector_h

#include "arrayn.h" 
#include "assert.h"
//#include <utility>
#include <algorithm>
#include "geom/math.h"

//!Vector extention of basic types
namespace exttype{
	//______________array_entry_______________
	//! Pair (value, index)
	/*! Returned by ivector::min(), ivector::max() methodth
	*/
	template<typename type> struct array_entry{
		type value;
		int arg;
		array_entry(const type value_, int arg_):value(value_),arg(arg_){};
		operator std::pair<type,int>(){return std::pair<type,int>(value,arg);};
	};

	//________________________ivector________________________________________________________
	//! Linear vector traits.
	/*!
		requires: base::count(), base::operator[], base::telement;
	*/
	template <class target, class base> class ivector : public base{
	private:
		target * ths(){return static_cast<target*>(this);};
		const target * ths()const{return static_cast<const target*>(this);};
	public:
		typedef typename base::telement field;
		typedef base container;
		typedef target tvector;
		typedef ivector<tvector,container> tivector;
	public:
	protected:
		ivector(){};
	public:
		//default copy constructor
		//default operator =
	private:
		tvector cpy()const{return tvector(*ths());};
	public:
		template<class vector_2, class vector2_container>
		tvector& operator +=(const ivector<vector_2, vector2_container> & x){
			for(int i=0;i<this->count();i++){
				(*this)[i] += x[i];
			};
			return *ths();
		};
		tvector& operator +=(const field & k){
			for(int i=0;i<this->count();i++)(*ths())[i]+=k;
			return *ths();
		};
		template<class vector_2, class vector2_container>
		inline tvector& operator -=(const ivector<vector_2, vector2_container> & x){
			for(int i=0;i<this->count();i++){
				(*this)[i] -= x[i];
			};
			return *ths();
		};
		tvector& operator -=(const field & k){
			for(int i=0;i<this->count();i++)(*ths())[i]-=k;
			return *ths();
		};
		tvector& operator *=(const field k){
			for(int i=0;i<this->count();i++)(*ths())[i]*=k;
			return *ths();
		};
		tvector& operator /=(const field k){
			for(int i=0;i<this->count();i++)(*ths())[i]/=k;
			return *ths();
		};
		tvector operator *(const field k)const{return cpy()*=k;};
		tvector operator /(const field k)const{return cpy()/=k;};
		tvector & neg(){
			for(int i=0;i<this->count();i++)(*ths())[i]=-(*ths())[i];
			return *ths();
		};

		tvector operator -()const{
			return cpy().neg();
		};

		tvector operator +(const tvector& x)const{
			return cpy()+=x;
		};

		tvector operator -(const tvector& x)const{
			return cpy()-=x;
		};

		tvector operator +(const field & k)const{
			return cpy()+=k;
		};

		tvector operator -(const field & k)const{
			return cpy()-=k;
		};

		field operator *(const tvector& x)const{
			field s = 0;
			for(int i=0;i<this->count();i++){
				s+=(*this)[i]*x[i];
			};
			return s;
		};

		int find(field & val)const{
			for(int i=0;i<this->count();++i){
				if((*this)[i]==val){
					return i;
				};
			};
			return -1;
		};
		
		std::pair<field,int> min()const{
			if(this->count()==0)throw debug_exception("Empty vector, min is undefined");
			array_entry<field> r((*this)[0],0);
			for(int i=1;i<this->count();i++){
				if((*this)[i]<r.value){
					r.value = (*this)[i];
					r.arg = i;
				};
			};
			return r;
		};

		std::pair<field,int> max()const{
			if(this->count()==0)throw debug_exception("Empty vector, max is undefined");
			array_entry<field> r((*this)[0],0);
			for(int i=1;i<this->count();i++){
				if((*this)[i]>r.value){
					r.value = (*this)[i];
					r.arg = i;
				};
			};
			return r;
		};
		std::pair<field,int> maxabs()const{
			if(this->count()==0)throw debug_exception("Empty vector, max is undefined");
			array_entry<field> r(math::abs((*this)[0]),0);
			for(int i=1;i<this->count();i++){
				field v = math::abs((*this)[i]);
				if(v>r.value){
					r.value = v;
					r.arg = i;
				};
			};
			return r;
		};
		std::pair<field,int> minabs()const{
			if(this->count()==0)throw debug_exception("Empty vector, min is undefined");
			array_entry<field> r(math::abs((*this)[0]),0);
			for(int i=1;i<this->count();i++){
				field v = math::abs((*this)[i]);
				if(v<r.value){
					r.value = v;
					r.arg = i;
				};
			};
			return r;
		};
	public:
		bool operator == (const tvector & x)const{
			if(this->count()!=x.count()){
				return false;
			};
			for(int i=0;i<this->count();++i){
				if((*this)[i]!=x[i])return false;
			};
			return true;
		};
		bool operator != (const tvector & x)const{
			if(this->count()!=x.count()){
				return true;
			};
			for(int i=0;i<this->count();++i){
				if((*this)[i]!=x[i])return true;
			};
			return false;
		};
		bool operator < (const tvector & x)const{
			for(int i=0;i<this->count();++i){
				if((*this)[i]>=x[i])return false;
			};
			return true;
		};

		bool operator >= (const tvector & x)const{
			for(int i=0;i<this->count();++i){
				if((*this)[i]<x[i])return false;
			};
			return true;
		};

		bool operator > (const tvector & x)const{
			for(int i=0;i<this->count();++i){
				if((*this)[i]<=x[i])return false;
			};
			return true;
		};

		bool operator <= (const tvector & x)const{
			for(int i=0;i<this->count();++i){
				if((*this)[i]>x[i])return false;
			};
			return true;
		};

	public:
		static tvector max(const tvector&a, const tvector&b){
			tvector r(a);
			for(int i=0;i<r.count();++i){
				r[i] = std::max(a[i],b[i]);
			};
			return r;
		};
		static tvector min(const tvector&a, const tvector&b){
			tvector r(a);
			for(int i=0;i<r.count();++i){
				r[i] = std::min(a[i],b[i]);
			};
			return r;
		};
		static tvector abs(const tvector&a){
			tvector r(a);
			for(int i=0;i<r.count();++i){
				r[i] = math::abs(a[i]);
			};
			return r;
		};
	public:
		field sum()const{
			field r=0;
			for(int i=0;i<this->count();++i){
				r+=(*this)[i];
			};
			return r;
		};
		field prod()const{
			field r=1;
			for(int i=0;i<this->count();++i){
				r*=(*this)[i];
			};
			return r;
		};
		field mean()const{
			if(this->count()<1){
				throw debug_exception("mean is undefined");
			};
			return sum()/this->count();
		};
		field l2()const{
			return (*ths())*(*ths());
		};
		double norm()const{
			return sqrt(double(l2()));
		};
		tvector normalized()const{
			return (*this)/norm();
		};

	public:
		//! assignment
		template <class target2, class base2> inline tvector& operator << (const ivector<target2, base2> & y){
			int sz = std::min(this->count(),y.count());
			for(int i=0;i<sz;++i){
				(*this)[i] = field(y[i]);
			};
			return *ths();
		};
		//! assignment
		inline tvector& operator << (const field & c){
			for(int i=0;i<this->count();++i){
				(*this)[i] = c;
			};
			return *ths();
		};

//		static tvector zero;
	};

	//___________________ivector_n__________________________
	//! Fixed-length vector template.
	template<class target, typename type, int n> class ivector_n : public ivector<target, array_tn<type,n> >{
	};
};
#endif
