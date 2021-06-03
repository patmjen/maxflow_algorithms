#ifndef files_advstreaming_h
#define files_advstreaming_h

#include "link/utoken.h"
#include "streamtree.h"

#include <string>
#include <stdexcept>
#include <stack>

namespace datamanage{
	using _Common::_link::Mappable;
	using _Common::_link::Method;
	//_______________________streamable__________________________
	namespace streaming{
		class vstream;
	};
/*
	template<class Target = Mappable> class streamable : public Method<Target>{
	public:
		void write(streaming::vstream & stream)const{
			stream<<vstream::data_block(*ths());
		};
		void read(streaming::vstream & stream){
			stream>>vstream::data_block(*ths());
		};
		void write_pointers(streaming::vstream & stream)const{};
		void read_pointers(streaming::vstream & stream){};
	};
*/
	class streamable{
	public:
		void write(streaming::vstream & stream)const{};
		void read(streaming::vstream & stream){};
		void write_pointers(streaming::vstream & stream)const{};
		void read_pointers(streaming::vstream & stream){};
	};

	class streamnode : public streamable{
	public:
	};

	namespace streaming{


	//_______________________vstream________________________________
		class vstream{
		private:
			Tree tree;
			std::stack<Node *> obj_stack;
			std::stack<const Node *> cobj_stack;
		public:
			vstream();
			virtual ~vstream();
			void init();
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
			virtual void flush()=0;
		public:
			template<class Streamable> vstream & operator <<(const Streamable & object){
				write(object,object);
				return *this;
			};
			template<class Streamable> vstream & operator >>(Streamable & object){
				read(object,object);
				return *this;
			};
/*
			template<template<int rank> class Streamable> vstream & operator <<(const Streamable<1> & object){
				writenode(object);
				return *this;
			};
			template<template<int rank> class Streamable> vstream & operator >>(Streamable<1> & object){
				readnode(object);
				return *this;
			};
*/
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

		public:

			template<class Streamable> void write(const Streamable & object, const streamable & xobject){
				object.write(*this);
			};

			template<class Streamable> void read(Streamable & object, streamable & xobject){
				object.read(*this);
			};

			template<class Streamable> void write(const Streamable & object, const streamnode & xobject){
				Node * node = tree.addChild(obj_stack.top(),&const_cast<Streamable&>(object));
				obj_stack.push(node);
				object.write(*this);
				obj_stack.pop();
			};

			template<class Streamable> void read(Streamable & object, streamnode & xobject){
				Node * node = tree.addChild(obj_stack.top(),&const_cast<Streamable&>(object));
				obj_stack.push(node);
				object.read(*this);
				obj_stack.pop();
			};

		protected:
			void write_pointers(const Node * node);
			void read_pointers(Node * node);
		public:

			template<class Streamable> void write_pointer(const Streamable * object){
				write_pointer(object,object);
			};

			void write_pointer(const Mappable * object, const streamnode *){
				Node * node = tree.getNode(object);
				write_index(node->index);
			};

			template<class Streamable> void read_pointer(Streamable *& object){
				treeindex index = read_index();
				Node * node = tree.getNode(index);
				Mappable * obj = tree.getObject(node);
				object = static_cast<Streamable*>(obj);
			};

		protected:
			void write_index(const treeindex & index);
			treeindex read_index();
		public:

			template<class Streamable> void save(const Streamable & x){
				init();
				write(x,x);
				write_pointers(tree.getNode(&x));
				flush();
			};
			template<class Streamable> void load(Streamable & x){
				init();
				read(x,x);
				read_pointers(tree.getNode(&x));
			};
			virtual std::string getFileName()const = 0;
			std::string relFileName(const std::string& filename)const;
			std::string absFileName(const std::string& filename)const;
		};
	};
	typedef streaming::vstream VStream;
	typedef streaming::vstream vstream;
};

#endif
