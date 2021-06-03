#ifndef key_less_h
#define key_less_h

template<class key_type>
struct key_less{
	key_less(key_type* _key_beg):key_beg(_key_beg){};
	key_type* key_beg;
	bool operator()(
		const key_type & _Left, 
		const key_type & _Right
		)const{
			return key_beg[_Left]<key_beg[_Right];
	};
};

template<class key_type>
struct key_gt{
	key_gt(key_type* _key_beg):key_beg(_key_beg){};
	key_type* key_beg;
	bool operator()(
		const key_type & _Left, 
		const key_type & _Right
		)const{
			return key_beg[_Left]>key_beg[_Right];
	};
};

#endif