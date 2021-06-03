#include "file_stream.h"
#include <stdexcept>

namespace streaming{
	//_____________________bin_file_stream_________________________
	bin_file_stream::bin_file_stream(const std::string filename, int o_mode){
		this->filename = filename;
		stream.open(filename.c_str(),std::ios_base::openmode(o_mode | std::ios_base::binary));
		if(!stream.is_open())throw std::runtime_error(std::string("unable to open file ")+filename);
	};

	std::string bin_file_stream::getFileName()const{return filename;};

	vstream & bin_file_stream::write(const data_block & data){
		stream.write(data.data,data.size);
		return *this;
	};

	vstream & bin_file_stream::read(data_block & data){
		stream.read(data.data,data.size);
		return *this;
	};

	void bin_file_stream::flush(){
		stream.flush();
	};

	//____________________text_file_stream________________________
	text_file_stream::text_file_stream(const std::string filename,int o_mode){
		this->filename = filename;
		stream.open(filename.c_str(),std::ios_base::openmode(o_mode | std::ios_base::binary));
		if(!stream.is_open())throw std::runtime_error("unable to open file "+filename);
	};

	std::string text_file_stream::getFileName()const{return filename;};

	vstream & text_file_stream::write(const std::string & s){
		stream<<s.c_str()<<" ";
		return *this;
	};

	vstream & text_file_stream::read(std::string & s){
		char c[1024];
		stream>>c;
		s = c;
		stream.get(c[0]);
		return *this;
	};

	vstream & text_file_stream::read(std::string & s,int size){
		s.resize(size);
		stream.read(&s[0],size);
		return *this;
	};

	void text_file_stream::flush(){
		stream.flush();
	};
};
