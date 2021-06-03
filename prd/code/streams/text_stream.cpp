#include "text_stream.h"
#include <stdio.h>
#include <stdlib.h>

namespace streaming{
	//___________________text_stream::data_block_________________
	text_stream::data_block::data_block(){
		size = 0;
	};

	text_stream::data_block::~data_block(){
	};

	text_stream::data_block::data_block(const char * s){
		data = s;
		size = (int)strlen(s);
	};

	text_stream::data_block::data_block(const std::string & s){
		data = s;
		size = (int)s.length();
	};

	text_stream::data_block::data_block(const std::wstring & s){
		data.resize(size = (int)s.length()*2);
		memcpy(&data[0],s.c_str(),size);
	};

	//___________________text_stream_____________________________
	text_stream::text_stream(){
		format.d = "%4e";
		format.i = "%i";
	};
	text_stream::~text_stream(){
	};

	vstream & text_stream::operator << (const data_block & data){
		write(data.data);
		return *this;
	};

	vstream & text_stream::operator >> (data_block & data){
		read(data.data);
		return *this;
	};

	vstream & text_stream::operator << (const int & x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (int & x){
		data_block s;
		(*this)>>s;
		x = atoi(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const unsigned int & x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (unsigned int & x){
		data_block s;
		(*this)>>s;
		x = (unsigned int)atol(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const char& x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (char & x){
		data_block s;
		(*this)>>s;
		x = (char)atoi(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const unsigned char & x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (unsigned char & x){
		data_block s;
		(*this)>>s;
		x = (unsigned char)atoi(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const double& x){
		return (*this)<<data_block(x,format.d);
	};

	vstream & text_stream::operator >> (double & x){
		data_block s;
		(*this)>>s;
		x = atof(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const float& x){
		return (*this)<<data_block(x,format.d);
	};

	vstream & text_stream::operator >> (float & x){
		data_block s;
		(*this)>>s;
		x = (float)atof(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const short int& x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (short int & x){
		data_block s;
		(*this)>>s;
		x = (short int)atoi(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const unsigned short int& x){
		return (*this)<<data_block(x,format.i);
	};

	vstream & text_stream::operator >> (unsigned short int & x){
		data_block s;
		(*this)>>s;
		x = (unsigned short int)atoi(s.data.c_str());
		return *this;
	};

	vstream & text_stream::operator << (const std::string & x){
		(*this)<<(int)x.size();
		(*this)<<data_block(x);
		return *this;
	};

	vstream & text_stream::operator >> (std::string & x){
		int size;
		(*this)>>size;
		read(x,size);
		return *this;
	};

	vstream & text_stream::operator << (const std::wstring & x){
		(*this)<<(int)x.size();
		(*this)<<data_block(x);
		return *this;
	};

	vstream & text_stream::operator >> (std::wstring & x){
		int size;
		(*this)>>size;
		std::string s;
		read(s,size*2);
		x.resize(size);
		memcpy(&x[0],s.c_str(),size*2);
		return *this;
	};

};
