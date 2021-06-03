#ifndef files_xfs_h
#define files_xfs_h

#include <string>

class xfs{
private:
	static std::string root;
protected:
	static std::string parsePath(std::string & path);
	static std::string reverseParsePath(std::string & path);
public:
	static void setRoot(const std::string & s);
	static std::string Root();
	static std::string getExt(const std::string & filename);
	static std::string getPath(const std::string & filename);// root/>bla/bla/bla/
	static std::string getName(const std::string & filename);
	static std::string trimExt(const std::string & filename);
	static std::string relName(std::string pathname, std::string absName);
	static std::string absName(std::string pathname, std::string relName);
	static std::string backslashes(std::string filename);
};

#endif
