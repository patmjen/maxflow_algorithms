#include "xfs.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

std::string xfs::getPath(const std::string & filename){
	size_t x = filename.find_last_of("/\\");
	if(x==std::string::npos)return "";
	std::string r = filename.substr(0,int(x+1));
	return r;
};
std::string xfs::getName(const std::string & filename){
	size_t x = filename.find_last_of("/\\");
	if(x==std::string::npos)return filename;
	return filename.substr(int(x+1),filename.length()-int(x+1));
};

std::string xfs::root = "./";

void xfs::setRoot(const std::string & s){
	root = getPath(s);
};

std::string xfs::Root(){
	return root;
};

std::string xfs::getExt(const std::string & filename){
	int pos = (int)filename.rfind('.');
	std::string ext="";
	if(pos>0){
		ext = filename.substr((pos+1),filename.length()-(pos+1));
	};
	return ext;
};

std::string xfs::trimExt(const std::string & filename){
	int pos = (int)filename.rfind('.');
	std::string r = filename;
	if(pos>0){
		r = filename.substr(0,pos);
	};
	return r;
};

std::string xfs::parsePath(std::string & path){
	int x1 = (int)path.find('\\');
	int x2 = (int)path.find('/');
	if((x2<x1 || x1<0)&& x2>0)x1=x2;
	if(x1<0)return "";
	std::string r = path.substr(0,(x1+1));
	path.erase(0,x1+1);
	return r;
};
std::string xfs::reverseParsePath(std::string & path){
	while(!path.empty() && std::string("\\/").find(path[path.size()-1])!=std::string::npos)path.erase(--path.end());
	int x1 = (int)path.rfind('\\');
	int x2 = (int)path.rfind('/');
	if(x2>x1)x1=x2;
	if(x1<0)return "";
	std::string r = path.substr((x1+1),path.length()-(x1+1))+"/";
	path = path.substr(0,x1+1);
	return r;
};
std::string xfs::relName(std::string pathname, std::string absName){
	if(absName=="")return absName;
	std::string pathname0 = pathname;
	std::string absname0 = absName;
	std::string r1;
	std::string r2;
	int x;
	int coincide = 0;
	do{
		r1 = parsePath(pathname);
		r2 = parsePath(absName);
#ifdef WIN32
		x = _stricmp(r1.substr(0,r1.size()-1).c_str(),r2.substr(0,r2.size()-1).c_str());
#else
		x = r1.substr(0,r1.size()-1).compare(r2.substr(0,r2.size()-1));
#endif
		coincide++;
	}while(r1.size()>0 && r2.size()>0 && x == 0);
	if(coincide==1 && x!=0){
		std::cout<<"cant compute relative path to "<<absname0<<"\n"<<"from "<<pathname0<<"\n";
		std::cout<<"parse state:\n";
		std::cout<<"pathname="<<pathname.c_str()<<"\n";
		std::cout<<"absname="<<absName.c_str()<<"\n";
		throw std::runtime_error("cannot compute relative path");
	};
	absName = r2+absName;
	while(!r1.empty()){
		absName = std::string("../")+absName;			 
		r1 = parsePath(pathname);
	};
	x = (int)absName.find(':');
	if(x>=0){
		std::cout<<"cant compute relative path to "<<absname0<<"\n"<<"from "<<pathname0<<"\n";
		std::cout<<"parse state:\n";
		std::cout<<"pathname="<<pathname.c_str()<<"\n";
		std::cout<<"absname="<<absName.c_str()<<"\n";
		throw std::runtime_error("pathe parse error");
		return absName;
	};
	return absName;
};
std::string xfs::absName(std::string pathname, std::string relName){
	std::string relName0 = relName;
	std::string pathname0 = pathname;
	if(relName.empty())return relName;
	pathname = getPath(pathname);
	int x = (int)relName.find(':');
	if(x>=0)return relName;
	std::string r1 = parsePath(relName);
	while(r1.compare("..\\")==0 || r1.compare("../")==0){
		reverseParsePath(pathname);
		r1 = parsePath(relName);
	};
	x = (int)relName.find("..");
	if(x>=0){
		std::cout<<"cant compute abs path to "<<relName0.c_str()<<"\n"<<"from "<<pathname0.c_str()<<"\n";
		std::cout<<"to "<<relName0<<"\n";
		std::cout<<"pathname= "<<pathname<<"\n";
		std::cout<<"r1= "<<r1.c_str()<<"\n";
		std::cout<<"relName= "<<relName<<"\n";
		throw std::runtime_error("path parse error");
	};
	return pathname+r1+relName;
};

std::string xfs::backslashes(std::string filename){
	replace(filename.begin(),filename.end(),'\\','/');
	return filename;
};
