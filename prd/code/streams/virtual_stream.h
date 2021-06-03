#ifndef vstream_h
#define vstream_h

#include <vector>
#include <string>
#include <string.h>

namespace std{
	typedef basic_string<wchar_t> wstring;
};

namespace streaming{
	//_______________________vstream______________________
	class vstream{
	public:
		vstream(){};
		virtual ~vstream(){};
	public:
		virtual vstream & operator << (const int & x)=0;
		virtual vstream & operator >> (int & x)=0;
		virtual vstream & operator << (const unsigned int & x)=0;
		virtual vstream & operator >> (unsigned int & x)=0;
		virtual vstream & operator << (const char& x)=0;
		virtual vstream & operator >> (char & x)=0;
		virtual vstream & operator << (const unsigned char & x)=0;
		virtual vstream & operator >> (unsigned char & x)=0;
		virtual vstream & operator << (const double& x)=0;
		virtual vstream & operator >> (double & x)=0;
		virtual vstream & operator << (const float& x)=0;
		virtual vstream & operator >> (float & x)=0;
		virtual vstream & operator << (const short int& x)=0;
		virtual vstream & operator >> (short int & x)=0;
		virtual vstream & operator << (const unsigned short int& x)=0;
		virtual vstream & operator >> (unsigned short int & x)=0;
	public:
		template<class type> vstream & operator << (const std::vector<type> & x){
			int size = (int)x.size();
			(*this)<<size;
			for(int i=0;i<size;++i)(*this)<<x[i];
			return *this;
		};
		template<class type> vstream & operator >> (std::vector<type> & x){
			int size;
			(*this)>>size;
			x.resize(size);
			for(int i=0;i<size;++i)(*this)>>x[i];
			return *this;
		};
	public:
		virtual vstream & operator << (const std::string & x)=0;
		virtual vstream & operator >> (std::string & x)=0;
	public:
		virtual vstream & operator << (const std::wstring & x)=0;
		virtual vstream & operator >> (std::wstring & x)=0;
	};
};

#endif
