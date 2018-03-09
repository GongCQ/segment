#include <string>
#include <list>
#include <map>
using std::string;
using std::wstring;
using std::list;
using std::map;

void ThulacSeg(const wstring &input, list<wstring> &segList, char *userDictPath, const char *modelPath); 
void ThulacSeg(const string &input, list<wstring> &segList, char *userDictPath, const char *modelPath);  
