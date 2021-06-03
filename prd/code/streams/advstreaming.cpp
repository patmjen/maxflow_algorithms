#include "advstreaming.h"

#include "files/xfs.h"

namespace datamanage{
	namespace streaming{

		//_____________________vstream____________________________

		vstream::vstream(){
			obj_stack.push(tree.root());
			cobj_stack.push(tree.root());
		};

		vstream::~vstream(){
		};

		void vstream::init(){
			tree.clear();
			while(obj_stack.size()>1)obj_stack.pop();
			while(cobj_stack.size()>1)cobj_stack.pop();
		};

		void vstream::write_index(const treeindex & index){
			(*this)<<index;
		};

		treeindex vstream::read_index(){
			treeindex index;
			(*this)>>index;
			return index;
		};

		std::string vstream::relFileName(const std::string & filename)const{
			return xfs::relName(xfs::getPath(getFileName()),filename);
		};

		std::string vstream::absFileName(const std::string & filename)const{
			return xfs::absName(xfs::getPath(getFileName()),filename);
		};

		void vstream::write_pointers(const Node * node){
			cobj_stack.push(node);
			node->write_pointers(*this);
			for(int i=0;i<(int)node->subnodes.size();++i){
				write_pointers(node->subnodes[i]);
			};
			cobj_stack.pop();
		};

		void vstream::read_pointers(Node * node){
			cobj_stack.push(node);
			node->read_pointers(*this);
			for(int i=0;i<(int)node->subnodes.size();++i){
				read_pointers(node->subnodes[i]);
			};
			cobj_stack.pop();
		};
	};
};