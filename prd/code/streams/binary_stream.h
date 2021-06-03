#ifndef binary_stream_h
#define binary_stream_h

#include "virtual_stream.h"

namespace streaming{
	//_______________________vstream______________________
	class binary_stream : public vstream{
	public:
		class data_block{
		public:
			int size;
			char *data;
		public:
			template<class type> data_block(type *Data,int Size):data((char*)Data),size(Size*sizeof(type)){};
			template<class type> data_block(type & Data):data((char*)&Data),size(sizeof(type)){};
			operator data_block & ()const{return const_cast<data_block&>(*this);};
		};
	public:
		virtual vstream & write(const data_block & data)=0;
		virtual vstream & read(data_block & data)=0;
	public:
		vstream & operator << (const int & x);
		vstream & operator >> (int & x);
		vstream & operator << (const unsigned int & x);
		vstream & operator >> (unsigned int & x);
		vstream & operator << (const char& x);
		vstream & operator >> (char & x);
		vstream & operator << (const unsigned char & x);
		vstream & operator >> (unsigned char & x);
		vstream & operator << (const double& x);
		vstream & operator >> (double & x);
		vstream & operator << (const float& x);
		vstream & operator >> (float & x);
		vstream & operator << (const short int& x);
		vstream & operator >> (short int & x);
		vstream & operator << (const unsigned short int& x);
		vstream & operator >> (unsigned short int & x);
	public:
		vstream & operator << (const std::string & x);
		vstream & operator >> (std::string & x);
	public:
		vstream & operator << (const std::wstring & x);
		vstream & operator >> (std::wstring & x);
	};
};

#endif

