#include "xstringstream.h"
//#include <iostream>
#include <string.h>

namespace txt{
	//_____________________String_____________________________
	String::String(){
	};

	String::String(const String & x):parent(x){
	};

	String::String(const parent & x):parent(x){
	};

	String::String(LPCSTR s):parent(s){
	};

	String::String(int x){
		*this = Format("%i",x);
	};

	String String::Format(LPCSTR pszFormat, va_list args ){
		char s[1024];
		vsprintf(s, pszFormat, args);
		return String(s);
	};

	String String::Format(LPCSTR pszFormat, ... ){
		String r;
		va_list argList;
		va_start( argList, pszFormat );
		r += Format( pszFormat, argList );
		va_end( argList );
		return r;
	};


	int String::ReverseFind( char ch ){
		return (int)parent::rfind(ch);
	};

	String & String::TrimRight( char  pszTargets){
		while(length() && *end()==pszTargets)erase(end());
		return *this;
	};

	String::operator const char *(){
		return c_str();
	};

	void String::Empty(){
		empty();
	};

	//________________TextStream_______________________________
	TextStream TextStream::null;
	TextStream::TextStream(){
		err = 0;
		//format.d = "%4e";
		format.d = "%g";
		format.i = "%i";
	};
	TextStream & TextStream::operator<<(const char * x){
		return write(x);
	};
	TextStream & TextStream::operator<<(char x){
		return *this;
	};
	TextStream & TextStream::operator<<(const unsigned char x){
		return *this;
	};

	TextStream & TextStream::operator<<(const String & s){
		(*this)<<s.c_str();
		return *this;
	};

	TextStream & TextStream::operator<<(int x){
		String s;
		s = String::Format("%i",x);
		(*this)<<s;
		return *this;
	};
	TextStream & TextStream::operator<<(long long x){
		String s;
		s = String::Format("%lli",x);
		(*this)<<s;
		return *this;
	};
	TextStream & TextStream::operator<<(const double & x){
		String s;
		s = String::Format(format.d.c_str(),x);
		(*this)<<s;
		return *this;
	};
	TextStream & TextStream::expdouble(const double & x){
		String s;
		s = String::Format(format.d.c_str(),x);
		(*this)<<s;
		return *this;
	};
	TextStream & TextStream::operator<<(const void * x){
		String s;
		s = String::Format("0x%X",x);
		(*this)<<s;
		return *this;
	};
/*
	TextStream & TextStream::operator<<(const unsigned int x){
		String s;
		s = String::Format("0x%X",x);
		(*this)<<s;
		return *this;
	};
*/
	TextStream & TextStream::operator<<(size_t x){
		String s;
		s = String::Format("%i",x);
		(*this)<<s;
		return *this;
	};
	//________________StringStream________________________________
	StringStream::StringStream(){
	};
	TextStream & StringStream::write(const char * x){
		String::operator +=(String(x));
		return *this;
	};
	//________________BufferedTextStream__________________________
	TextStream & BufferedTextStream::write(const char * x){
		str<<x;
		String s = x;
		int p = (int)s.rfind('\n');
		if(p>=0){
			str.TrimRight('\n');
			flushString(str);
			str.Empty();
		};
		return *this;
	};
	//__________________FileStream________________________________
	TextStream & FileStream::write(const char * x){
		(*filestream)<<x;
		(*filestream).flush();
		return *this;
	};
	FileStream::FileStream(){
	};

	/*
	FileStream::FileStream(const char* filename):filestream(filename){
	};

	FileStream::FileStream(const std::string & filename):filestream(filename.c_str()){
	};
	*/

	void FileStream::clear(){
		(*filestream).seekp(0);
	};

	//_______________TabbedTextStream_______________________
	void TabbedTextStream::makeTab(){
		for(int i=0;i<tab;i++)parent::write("\t");
	};
	TextStream & TabbedTextStream::write(const char * x){
		if(!strlen(x))return *this;
		if(newline){
			makeTab();
			newline = 0;
		};
		std::string s(x);
		int p = (int)s.find("\n");
		if(p>=0){
			parent::write(s.substr(0,p+1).c_str());
			newline = 1;
			write(s.substr(p+1,s.length()-p-1).c_str());
		}else parent::write(x);
		return *this;
	};
};
