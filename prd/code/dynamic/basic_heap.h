#ifndef basic_heap_h
#define basic_heap_h

namespace dynamic{

	template<class node, class comp, bool forward> class basic_heap{
	protected:
		node * _beg;
		node * _end;
	protected:
		__forceinline int child(int i){
			return 2*i+1;
		};
		__forceinline int parent(int i){
			return (i-1)/2;
		};
		__forceinline node* undiff(node * p, int k)const{
			if(forward){
				return p+k;
			}else{
				return p-k;
			};
		};
		__forceinline const node* undiff(const node * p, int k)const{
			if(forward){
				return p+k;
			}else{
				return p-k;
			};
		};

		__forceinline int diff(const node * p, const node * beg)const{
			if(forward){
				return int(p-beg);
			}else{
				return int(beg-p);
			};
		};
		node * get_node(int i){
			return undiff(_beg,i);
		};
	public:
		int size()const{return diff(_end,_beg);};
		bool is_empty()const{return (_end==_beg);};
		node & operator [] (int i){return *undiff(_beg,i);};
		const node & operator [] (int i)const{return *undiff(_beg,i);};
	public:
		void init(node* be){
			_beg = be;
			_end = be;
		};
		void clear(){
			_end = _beg;
		};
		__forceinline void expand(){
			_end = undiff(_end,1);
		};
		__forceinline void shrink(){
			_end = undiff(_end,-1);
		};
		//_______up_heap_______
		__forceinline int up_heap(int i, const node & val){
			for(;i>0;){
				int j = parent(i);
				if (comp::compare(*get_node(j), val)){
					*get_node(i) = *get_node(j);
					i = j;
				}else{
					break;
				};
			};
			return i;
		};
		//_______down_heap_______
		__forceinline int down_heap(int i,const node & val){
			int bottom = diff(_end,_beg);
			int j = child(i)+1;
			for(;j<bottom;j = child(i)+1){	
				//select larger child
				if(comp::compare(*get_node(j), *get_node(j-1))){
					--j;
				};
				if(comp::compare(*get_node(j),val)){
					return i;
				};
				*get_node(i) = *get_node(j);
				i = j;				
			};
			if(j==bottom){
				j = bottom-1;
				if(comp::compare(*get_node(j),val)){
					return i;
				};
				*get_node(i) = *get_node(j);
				i = j;
			};
			return i;
		};


		__forceinline node * push_heap(){
			expand();
			node * p = undiff(_end,-1);
			node val = *p;
			int k = up_heap(diff(p,_beg),val);
			p = undiff(_beg,k);
			*p = val;
			return p;
		};

		__forceinline node * push_heap(node & val){
			*_end = val;
			return push_heap();
		};

		__forceinline const node & top()const{
			return *_beg;
		};

		__forceinline node & top(){
			return *_beg;
		};

		const node * end()const{return _end;};
		node * end(){return _end;};
		const node * beg()const{return _beg;};
		node * beg(){return _beg;};

		__forceinline void move_down_heap(node * p){
			node val_p = *p;
			shrink();
			node val_e = *_end;
			*_end = val_p;
			int k0 = diff(p,_beg);
			int	k1 = up_heap(k0,val_e);
			if(k1==k0){
				k1 = down_heap(k0,val_e);
			};
			*undiff(_beg,k1) = val_e;
		};

		__forceinline node * delete_heap(node * p){
			move_down_heap(p);
			return _end;
		};

		__forceinline node * modify(node * p, const node & q){
			int k0 = diff(p,_beg);
			int	k1 = up_heap(k0,q);
			if(k1==k0){
				k1 = down_heap(k0,q);
			};
			p = undiff(_beg,k1);
			*p = q;
			return p;
		};

		__forceinline node* pop_heap(){
			return delete_heap(_beg);
		};
		void verify(){
			for(int i=1;i<diff(_end,_beg);++i){
				int j = parent(i);
				assert(!comp::compare(*get_node(j), *get_node(i)));
			};
		};
	};

	template<class type> class lt_comp{
	public:
		static bool compare( const type & a, const type & b){
			return a<b;
		};
		bool operator ()( const type & a, const type & b){
			return compare(a,b);
		};
	};

	template<class type> class gt_comp{
	public:
		static bool compare(const type & a, const type & b){
			return a>b;
		};
		bool operator ()( const type & a, const type & b){
			return compare(a,b);
		};
	};
};

#endif