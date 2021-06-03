#ifndef geom_algebra_Zn_h
#define geom_algebra_Zn_h

namespace geom{
	template<int n> class Zn{
	private:
		typedef Zn<n> tZn;
		int v;
	public:
		static const int N=n;
	private:
		void reduce(){
			while(v>=N)v-=N;
			while(v<0)v+=N;
		};
	public:
		operator const int &()const{return v;};
		Zn(const int x):v(x){
			reduce();
		};
		template<typename other> tZn & operator+=(const other & x){
			v+=x;if(v>=N)v-=N;
			return *this;
		};
		tZn & operator-=(const tZn & x){
			v-=x.v;if(v<0)v+=N;
			return *this;
		};
		tZn & operator*=(const tZn & x){
			v =(v*x.v)%N;
			return *this;
		};
		tZn & operator/=(const tZn & x){
			v = v/x.v;
			return *this;
		};
		template<typename other> tZn operator + (const other & x)const{
			return tZn(*this)+=x;
		};
		tZn operator - (const tZn & x)const{
			return tZn(*this)-=x;
		};
		tZn operator * (const tZn & x)const{
			return tZn(*this)*=x;
		};
		tZn operator / (const tZn & x)const{
			return tZn(*this)/=x;
		};
		tZn & operator ++(){
			++v;if(v==N)v=0;
			return *this;
		};
		tZn & operator ++(int){
			tZn r = *this;
			++(*this);
			return r;
		};
		tZn & operator --(){
			--v;if(v<0)v=N;
			return *this;
		};
		tZn & operator --(int){
			tZn r = *this;
			--(*this);
			return r;
		};
		template<typename other> int operator == (const other & x){
			return v==x;
		};
		template<typename other> int operator != (const other & x){
			return !v==x;
		};
		template<typename other> int operator < (const other & x){
			return v<x;
		};
		template<typename other> int operator > (const other & x){
			return v>x;
		};
	};
	template <int n> class Zniterator: public Zn<n>{
	private:
		typedef Zn<n> parent;
		int _allowed;
		Zn<n> start;
	public:
		Zniterator():parent(0),start(0){_allowed = 1;};
		Zniterator(const int i):parent(i),start(i){_allowed=1;};
		int allowed(){
			return _allowed;
		};
		void operator ++(){
			parent::operator++();
			if((*this)==start)_allowed=0;
		};
	};
};

#endif
