#ifndef grid_topology_h
#define grid_topology_h

#include "exttype/intn.h"
#include "exttype/itern.h"

namespace geom{
	using namespace exttype;
	using exttype::mint2;

	template <int rank> class DataSetTopology{
	public:
		typedef intn<rank> tindex;
		typedef itern<rank> titerator;
		typedef range_itern<rank> trangeiterator;
	protected:
		tindex CC;
	public:
		int count(int i=0)const{return CC[i];};
		const tindex & size()const{return CC;};
		bool is_empty(){
			for(int i=0;i<rank;++i){
				if(CC[i]==0)return true;
			};
			return false;
		};
	public:
		DataSetTopology(){
			for(int i=0;i<CC.size();++i)CC[i]=0;
		};
		explicit DataSetTopology(const tindex & CC){
			setSize(CC);
		};
		void setSize(const tindex & CC){
			this->CC = CC;
		};
		const tindex & getSize()const{
			return CC;
		};

		int linsize()const{
			const tindex & cc = CC;
			int size =	cc[0];
			for(int l =1;l<rank;l++){
				size = size*cc[l];
			};
			return size;
		};

		int linsize(const tindex & cc)const{
			int size =	cc[0];
			for(int l =1;l<rank;l++){
				size = size*cc[l];
			};
			return size;
		};
	public:
		titerator getiterator()const{
			return titerator(CC);
		};
		trangeiterator getrangeiterator(const tindex& cc, const tindex & CC)const{
			tindex c0;
			c0<<0;
			return trangeiterator(tindex::max(c0,cc),tindex::min(CC,this->CC));
		};
	public:
		//__________________nb_iterator_____________________
		class nb_iterator : public intn<rank>{
		private:
			typedef intn<rank> parent;
		private:
			tindex ii0;
			const tindex & CC;
			int i;
			int l;
		private:
			bool lallowed(){
				return l<2;
			};
			bool lfindallowed(){
				while(lallowed() && !lnballowed()){
					l+=2;
				};
				return lallowed();
			};
			bool lnballowed(){
				return (ii0[i]+l>=0 && ii0[i]+l<CC[i]);
			};
			void findallowed(){
				while(allowed() && !lfindallowed()){
					++i;
					l=-1;
				};
				if(allowed()){
					(*this)[i]+=l;
				};
			};
		public:
			nb_iterator(const tindex & index, const tindex & cc):CC(cc){
				start(index);
			};

			void start(const tindex & index){
				ii0=index;
				parent::operator = (ii0);
				i = 0;
				l=-1;
				findallowed();
			};

			bool allowed()const{
				return (i<rank);
			};

			void operator ++(){
				(*this)[i]=ii0[i];
				l+=2;
				findallowed();
			};

			mint2 di()const{return mint2(i,l);};
			bool dir()const{return l>0;};

			int static all_nb_pairs_count(const tindex & CC){
				int size =0;
				for(int i=0;i<rank;++i){
					int s = (CC[i]-1)*2;
					for(int j=0;j<rank;++j){
						if(j==i)continue;
						s*=CC[j];
					};
					size+=s;
				};
				return size;
			};
		};
		//__________________dir_edge_iterator______________________
		class dir_edge_iterator{
		protected:
			titerator it;
			nb_iterator it1;
		protected:
			bool allowed2(){
				return it1.allowed();
			};
			void findallowed(){
				while(allowed() && !allowed2()){
					++it;
					if(allowed())it1.start(it);
				};
			};
		public:
			dir_edge_iterator(const tindex & cc):it(cc),it1(it,cc){
				findallowed();
			};
			bool allowed()const{
				return (bool)it.allowed();
			};
			void operator ++(){
				++it1;
				findallowed();
			};
			const tindex & i1()const{return it;};
			const tindex & i2()const{return it1;};
		};

		//______________ndir_edge_iterator___________________
		class ndir_edge_iterator : public dir_edge_iterator{
		private:
			void findallowed2(){
				while(this->allowed2() && this->it1.dir()==0)++this->it1;
			};
			void findallowed(){
				findallowed2();
				while(this->allowed() && !this->allowed2()){
					++this->it;
					if(this->allowed()){
						this->it1.start(this->it);
						findallowed2();
					};
				};
			};
		public:
			/*
			ndir_edge_iterator(const tindex & cc):dir_edge_iterator(cc){
				findallowed();
			};
			*/
			void operator ++(){
				++this->it1;
				findallowed();
			};
		};
	};
};
#endif

