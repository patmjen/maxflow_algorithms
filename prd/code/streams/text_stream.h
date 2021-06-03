#ifndef text_stream_h
#define text_stream_h

//#include "advstreaming.h"
#include "virtual_stream.h"

namespace streaming{
	//_______________________vstream______________________
	class text_stream : public vstream{
	public:
		using vstream::operator <<;
		using vstream::operator >>;
	public:
		class data_block{
		public:
			std::string data;
			int size;
		public:
			~data_block();
			data_block();
			template<class type> data_block(type & Data, const std::string & format){
				char s[100];
				sprintf(s,format.c_str(),Data);
				data = s;
				size = (int)strlen(s);
			};
			data_block(const char * s);
			data_block(const std::string & s);
			data_block(const std::wstring & s);
		};
	public:
		struct{
			std::string i;
			std::string d;
		}format;
	public:
		text_stream();
		~text_stream();
	public:
		virtual vstream & write(const std::string & s)=0;
		virtual vstream & read(std::string & s)=0;
		virtual vstream & read(std::string & s,int size)=0;
	protected:
		vstream & operator << (const data_block & data);
		vstream & operator >> (data_block & data);
	public:
		virtual vstream & operator << (const int & x);
		virtual vstream & operator >> (int & x);
		virtual vstream & operator << (const unsigned int & x);
		virtual vstream & operator >> (unsigned int & x);
		virtual vstream & operator << (const char& x);
		virtual vstream & operator >> (char & x);
		virtual vstream & operator << (const unsigned char & x);
		virtual vstream & operator >> (unsigned char & x);
		virtual vstream & operator << (const double& x);
		virtual vstream & operator >> (double & x);
		virtual vstream & operator << (const float& x);
		virtual vstream & operator >> (float & x);
		virtual vstream & operator << (const short int& x);
		virtual vstream & operator >> (short int & x);
		virtual vstream & operator << (const unsigned short int& x);
		virtual vstream & operator >> (unsigned short int & x);
	public:
		virtual vstream & operator << (const std::string & x);
		virtual vstream & operator >> (std::string & x);
	public:
		virtual vstream & operator << (const std::wstring & x);
		virtual vstream & operator >> (std::wstring & x);
	};
};

#endif
