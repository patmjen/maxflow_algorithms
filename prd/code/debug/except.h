#ifndef debug_except_h
#define debug_except_h

#undef _SCL_SECURE_NO_WARNINGS
#undef _CRT_SECURE_NO_WARNINGS

#define _SCL_SECURE_NO_WARNINGS 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "assert.h"
//#include "dynamic/mallocator.h"

//typedef std::basic_string<char,std::char_traits<char>,mallocator<char> > dstring;
typedef std::string dstring;
class debug_exception : public std::exception{
private:
	//const char * pmsg;
	dstring smsg;
	void halt(){
		fprintf(stderr,"Error: %s\n",smsg.c_str());
		exit(1);
	};
public:
	debug_exception():smsg("unspecified"){
		assert(false);
	};

	debug_exception(const char * msg):smsg(msg){
		halt();
	};
	debug_exception(const dstring & msg):smsg(msg){
		halt();
	};
	debug_exception(const debug_exception & x):smsg(x.smsg){
		halt();
	};
	virtual ~debug_exception() throw (){};
public:
	virtual const char *what( ) const throw( ){
		return smsg.c_str();
	};
};

#endif
