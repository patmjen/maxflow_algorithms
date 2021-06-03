#ifndef file_stream_h
#define file_stream_h

#include "binary_stream.h"
#include "text_stream.h"
//#include"redirect_stream.h"

#include <iostream>
#include <fstream>

namespace streaming{
	//________________bin_file_stream_____________________________
	class bin_file_stream : public binary_stream{
	public:
		std::string filename;
		std::fstream stream;
	public:
		bin_file_stream(const std::string filename,int o_mode=std::ios_base::in);
		std::string getFileName()const;
	private:
		vstream & write(const data_block & data);
		vstream & read(data_block & data);
		void flush();
	};
	//________________text_file_stream_____________________________
	class text_file_stream : public text_stream{
	public:
		std::string filename;
		std::fstream stream;
	public:
		text_file_stream(const std::string filename,int o_mode=std::ios_base::in);
		std::string getFileName()const;
	private:
		vstream & write(const std::string & s);
		vstream & read(std::string & s);
		vstream & read(std::string & s,int size);
		void flush();
	};

};

#endif
