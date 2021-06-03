#include "binary_stream.h"

namespace streaming{
	//______________________vstream___________________
	vstream & binary_stream::operator << (const int & x){
		return write(data_block(x));
	};
	vstream & binary_stream::operator >> (int & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const unsigned int & x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (unsigned int & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const char& x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (char & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const unsigned char & x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (unsigned char & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const double& x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (double & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const float& x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (float & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const short int& x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (short int & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const unsigned short int& x){
		return write(data_block(x));
	};

	vstream & binary_stream::operator >> (unsigned short int & x){
		data_block d(x);
		return read(d);
	};

	vstream & binary_stream::operator << (const std::string & x){
		int size = (int)x.size();
		(*this)<<(int)x.size();
		write(data_block(&x[0],size));
		return *this;
	};

	vstream & binary_stream::operator >> (std::string & x){
		int size;
		data_block d(&size,1);
		read(d);
		x.resize(size);
		data_block xd(&x[0],size);
		read(xd);
		return *this;
	};

	vstream & binary_stream::operator << (const std::wstring & x){
		int size = (int)x.size();
		write(data_block(&size,1));
		write(data_block(&x[0],size));
		return *this;
	};

	vstream & binary_stream::operator >> (std::wstring & x){
		int size;
		data_block d(&size,1);
		read(d);
		x.resize(size);
		data_block xd(&x[0],size);
		read(xd);
		return *this;
	};
};