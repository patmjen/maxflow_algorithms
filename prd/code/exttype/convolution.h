#ifndef exttype_convolution_h
#define exttype_convolution_h
#include "assert.h" 

namespace exttype{
	//_________________________semirings_______________________________________
	template<typename set> struct semiring_sum_prod{
		static set o_plus(const set & a, const set & b){return a+b;};
		static set o_times(const set & a, const set & b){return a*b;};
		static set zero(){return 0;};
	};

	template<typename set> struct semiring_max_sum{
		static set o_plus(const set & a, const set & b){return max(a,b);};
		static set o_times(const set & a, const set & b){return a+b;};
		static set zero(){return -(1<<(sizeof(set)-1)-1);};
	};

	template<typename set> struct semiring_min_sum{
		static set o_plus(const set & a, const set & b){return min(a,b);};
		static set o_times(const set & a, const set & b){return a+b;};
		static set zero(){return (1<<(sizeof(set)-1)-1);};
	};

	template<typename set> struct semiring_or_and{
		static bool o_plus(const set a, const set b){return a || b;};
		static bool o_times(const set a, const set b){return a && b;};
		static bool zero(){return 0;};
	};

	//_____________________________convolution_________________________________
	//! generic convolution<i1,i2,semiring>(a,b,c)
	/*! Operation  convolution<i1,i2,semiring>(a,b,c) evaluates a = b*c, where * is semiring a convolution in indeces i1 and i2 of b and c resp.
		\n Requires b.count(i1)==c.count(i2).
		\n Requires A::titerator, B::titerator, C::titerator defined.
		\n Requires A::field defined.
		\n Requires semiring::o_plus(x,y), semiring::o_times(x,y), semiring::zero() defined.
		\n Note: a should be distinct from b and c;
	*/
	template<int i1, int i2, template<typename> class semiring = semiring_sum_prod> class convolution{
	public:
		template<class A, class B, class C> convolution(A & a,  const B & b,  const C & c){
			assert(b.count(i1)==c.count(i2));
			assert((void*)&a!=(void*)&b && (void*)&a!=(void*)&c);
			typedef semiring<typename A::telement> my_semiring;
			typename A::titerator ai = a.get_iterator();
			typename B::titerator bi = b.get_iterator();
			typename C::titerator ci = c.get_iterator();
			do{
				typename A::field x = my_semiring::zero();
				do{
					x = my_semiring::o_plus(x,my_semiring::o_times(b[bi],c[ci]));
				}while((bi.template iterate_i<i1>()) | (ci.template iterate_i<i2>()));
				a[ai] = x;
				if(!(ci.template iterate_skip_i<i2>()))bi.template iterate_skip_i<i1>();
			}while(ai.iterate());
		};
	};
};

#endif
