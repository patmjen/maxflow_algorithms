#ifndef xstringstream_h
#define xstringstream_h

#include <string>
#include <stdarg.h>
#include <fstream>
#include "debug/except.h"

namespace txt{

	class String : public std::string{
	protected:
		typedef std::string parent;
		typedef char * LPSTR;
		typedef const char * LPCSTR;
	public:
		String();
		String(const String & x);
		String(int x);
		String(LPCSTR s);
		String(const parent & x);
	public:
	private:
		static String Format(LPCSTR pszFormat, va_list args);
	public:
		static String Format(LPCSTR pszFormat, ... );
		int ReverseFind( char ch );
		String & TrimRight( char  pszTargets);
		operator const char *();
		void Empty();
	};

	class TextStream{
	public://interface
		int err;
		virtual TextStream & write(const char * x){return *this;};
	public:
		struct{
			std::string d;
			std::string i;
		}format;
	public:
		TextStream();
		virtual ~TextStream(){};
		TextStream & operator<<(const char * x);
		TextStream & operator<<(char x);
		TextStream & operator<<(const unsigned char x);
		TextStream & operator<<(int x);
		TextStream & operator<<(long long x);
		TextStream & operator<<(const double & x);
		TextStream & expdouble(const double & x);
		TextStream & operator<<(const void * x);
//		TextStream & operator<<(const unsigned int x);
		TextStream & operator<<(size_t x);
		TextStream & operator<<(const String & s);
	public:
		static TextStream null;
	};
	/*
	//_______________TabbedTextStream_______________________
	class TabbedTextStream : public TextStream{
	private:
	TextStream & stream;
	int newline;
	public:
	int tab;
	protected://overriding TextStream:
	virtual TextStream & write(const char * x);
	void makeTab();
	public:
	TabbedTextStream(TextStream & sm);
	};
	*/
	//________________StringStream____________________________
	class StringStream: public TextStream, public String{
		typedef String parent;
	protected://overriding TextStream:
		virtual TextStream & write(const char * x);
	public:
		StringStream();
		StringStream(LPCSTR s):parent(s){};
	};
	//________________BufferedTextStream_____________________
	class BufferedTextStream : public TextStream{
	private:
		StringStream str;
	protected://overriding TextStream:
		virtual TextStream & write(const char * x);
	protected://interface:
		virtual void flushString(const char * x)=0;
	};
	//___________________FileStream__________________________
	class FileStream: public TextStream{
	private:
		static std::ios::openmode mode1(){ return std::ios::out|std::ios::app;};
		static std::ios::openmode mode2(){ return std::ios::out|std::ios::trunc;};
		mutable std::ofstream * filestream;
	protected://overriding TextStream:
		virtual TextStream & write(const char * x);
	public:
		FileStream();
		FileStream(const char* filename, bool append=false){
			filestream = new std::ofstream(filename, append?mode1():mode2());
		};
		FileStream(const std::string & filename,bool append=false){
				filestream = new std::ofstream(filename.c_str(), append?mode1():mode2());
		};
		void clear();
		~FileStream(){
			if(filestream)delete filestream;
		};
	public:
		//! steal copy
		FileStream(const FileStream & f){
			filestream = f.filestream;
			f.filestream = 0;
		};
	};
	//____________________pTextStream_______________________
	class pTextStream : public TextStream{
	private:
		TextStream * p;
		mutable bool own;
	protected:
		virtual TextStream & write(const char * x){
			//if(!p)throw debug_exception("stream is not constructed");
			if(!p)return *this;
			p->write(x);
			return *this;
		};
	public:

		pTextStream():own(0){
			p = 0;
		};

		void attach(TextStream * s,bool Own=false){
			if(p && own)delete p;
			p = s;
			own = Own;
		};

		void detach(){
			if(p && own)delete p;
			own = false;
			p = 0;
		};

		pTextStream(const pTextStream & x){
			p = x.p;
			own = x.own;
			x.own = false;
		};

		template<class Stream> pTextStream(const Stream & s):own(true){
			p = new Stream(s);
		};

		template<class Stream> pTextStream(Stream * s, bool Own=false):own(Own){
		p = s;
		};

		~pTextStream(){
			try{
				detach();
			}catch(...){
				//try{
				//	debug::errstream<<"error detaching stream";
				//}catch(...){
					printf("error detaching stream\n");
				//};
			};
		};
	};
	//___________________EchoStream__________________________
	class EchoStream :public TextStream{
	private:
		pTextStream s1;
		pTextStream s2;
		bool on[2];
	protected:
		virtual TextStream & write(const char * x){
			if(on[0])s1<<x;
			if(on[1])s2<<x;
			return *this;
		};
	public:
		template<class Stream1, class Stream2> EchoStream(const Stream1 & S1, const Stream2 & S2):s1(S1),s2(S2){
			on[0] = 1;
			on[1] = 1;
		};
		template<class Stream1, class Stream2> EchoStream(const Stream1 & S1, Stream2 * S2):s1(S1),s2(S2){
			on[0] = 1;
			on[1] = 1;
		};
		template<class Stream1, class Stream2> EchoStream(Stream1 * S1, const Stream2 & S2):s1(S1),s2(S2){
			on[0] = 1;
			on[1] = 1;
		};
		template<class Stream1, class Stream2> EchoStream(Stream1 * S1, Stream2 * S2):s1(S1),s2(S2){
			on[0] = 1;
			on[1] = 1;
		};
		~EchoStream(){
		};
	public:
		void switch_on(int i){on[i]=1;};
		void switch_off(int i){on[i]=0;};
	};
	//_____________________TabbedTextStream____________________
	class TabbedTextStream : public pTextStream{
	private:
		typedef pTextStream parent;
	private:
		int newline;
	public:
		int tab;
	protected://overriding TextStream:
		virtual TextStream & write(const char * x);
		void makeTab();
	public:
		TabbedTextStream(const TabbedTextStream & x):parent((pTextStream&)x){
			newline = x.newline;
			tab = x.tab;
		};
		template<class Stream> TabbedTextStream(const Stream & s = Stream()):parent(s){
			tab = 0;
			newline = 0;
		};
		template<class Stream> TabbedTextStream(const Stream * s):parent(s){
			tab = 0;
			newline = 0;
		};
		~TabbedTextStream(){
		};
	};

};
#endif
