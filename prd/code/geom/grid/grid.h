#ifndef geom_grid_grid_h
#define geom_grid_grid_h

#include "data/grid_topology.h"
#include "data/dataset.h"
#include "geom/vectn.h"
#include <vector>

namespace geom{
	using namespace exttype;
	namespace grid{
		//_______________________GridTopology____________________________________
		template<int rank> class GridTopology : public DataSetTopology<rank>{
		private:
			typedef DataSetTopology<rank> parent;
		public:
			typedef typename parent::tindex tindex;
			typedef typename parent::titerator titerator;
			typedef typename parent::trangeiterator trangeiterator;
			typedef typename std::vector<tindex> GridIndexList;
			typedef GridTopology topology;
		public:
			//_________________tlineiterator___________________
			class tlineiterator:public titerator{
			private:
				typedef titerator parent;
			private:
				int _allowed;
				//double d;
				vectn<rank> dl;
				intn<rank> ss;
				double l;
				vectn<rank> dd;
				vectn<rank>	p;
			public:
				vectn<rank>	v;
			private:
				int indexallowed(int i){
					return ((*this)[i]>=0 && (*this)[i]<this->count(i));
				};
				int indexallowed(){
					for(int i=0;i<rank;i++)if(!indexallowed(i))return 0;
					return 1;
				};
			public:
				double getl(){return l;};
				tlineiterator():_allowed(1){
				};

				tlineiterator(const tindex & CC, const vectn<rank> &P, const vectn<rank> & V, const double D):_allowed(1){
					init(CC,P,V,D);
				};

				void init(const tindex & CC, const vectn<rank> &P, const vectn<rank> & V, const double D){
					parent::setcount(CC);
					//d = D;
					p = P;
					v = V;
					l = 0;
					for(int i=0;i<rank;i++){
						if(math::abs(v[i])>1e-5)dl[i] = (v/v[i]).l();
						else dl[i] = 1e5;
						ss[i] = math::sgn(v[i]);
						dd[i] = dl[i]*(ss[i]>0?(::ceil(p[i])-p[i]):(p[i]-::floor(p[i])));
						if(dd[i]==0.0)dd[i] = dl[i];
						(*this)[i] = (int)::floor(p[i]);
						if((*this)[i]==p[i])if(ss[i]<0)(*this)[i]--;
					};
					//if(!indexallowed())++(*this);
					while(l<D && !indexallowed())inc();
					if(l>=D)_allowed = 0;
				};

				int allowed(){
					//return (l<d);
					return _allowed;
				};
			private:
				int inc(){
					int i = dd.findmin();
					(*this)[i]+=ss[i];
					double mind = dd[i];
					l+=mind;
					for(int k=0;k<rank;k++)dd[k]-=mind;
					dd[i]=dl[i];
					return i;
				};
			public:
				void operator ++(){
					int i = inc();
					if(!indexallowed(i))_allowed = 0;
				};
				void stepby(double L){
					if(allowed()){
						for(int i=0;i<rank;i++){
							int nhits = (int)::floor((L-dd[i])/dl[i])+1;
							(*this)[i]+=ss[i]*nhits;
							dd[i] = dd[i]+nhits*dl[i]-L;
						};
						l+=L;
						if(!indexallowed())_allowed = 0;
					};
				};
			};

			class tsphereiterator: public trangeiterator{
			private:
				typedef trangeiterator parent;
			private:
				double r2;
				const vectn<rank> x;
				vectn<rank> p;
				int covered(const intn<rank> & ii){
					p<<(*this);
					return ((p-x).l2()<r2+math::sqrt(rank));
				};
			public:
				tsphereiterator(const trangeiterator & II,const vectn<rank> &X, const double R):parent(II),x(X){
					r2 = math::sqr(R);
					if(!covered(*this))++(*this);
				};
				int allowed(){
					return parent::allowed();
				};
				void operator ++(){
					do{
						parent::operator ++();
					}while(parent::allowed() && !covered(*this));
				};
			};
		protected:
			double CellSize;
		public:
			GridTopology(const tindex & size, double CS = 1.0):parent(size),CellSize(CS){
			};
			void setCellSize(double cellsize){
				CellSize = cellsize;
			};
			double getCellSize()const{
				return CellSize;
			};
			tlineiterator getlineiterator(const vectn<rank> &p, const vectn<rank> & v, const double d)const{
				return tlineiterator(this->CC,p,v,d);
			};
			tsphereiterator getsphereiterator(const vectn<rank> &x, double r)const{
				tindex c0 = tindex::floor((x-(vectn<rank>()<<r)));
				tindex c1 = tindex::ceil((x+(vectn<rank>()<<r)));
				return tsphereiterator(getrangeiterator(c0,c1),x,r);
			};
		};
		//______________________________ObjectGrid_____________________________
		template <class type, int rank, class topology = GridTopology<rank> > class ObjectGrid : public dynamic::DataSet<type,rank>{
		private:
			typedef dynamic::DataSet<type,rank> parent;
			typedef typename topology::GridIndexList GridIndexList;
		public:
		//	typedef _list::objectlist<type> ObjectArray;
			typedef std::vector<type> ObjectArray;
		public:
			ObjectArray getObjects(const GridIndexList & l)const{
				ObjectArray R;
				for(int i=0;i<l.count();i++){
					const type & x = (*this)[l[i]];
					R.add(&x);
				};
				return R;
			};
		public:
			ObjectGrid(const intn<rank> & size,double CellSize = 1.0):parent(size){
				setCellSize(CellSize);
			};

			ObjectArray coverage(const vectn<rank> & x, double r)const{
				return getObjects(topology::coverage(x,r));
			};
		};
	};
};
#endif
