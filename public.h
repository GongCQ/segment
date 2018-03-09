#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <utility>
using std::size_t;
using std::string;
using std::wstring;
using std::vector;
using std::list;
using std::map;
using std::set;

void isort(vector<double> &vec, vector<size_t> &ivec);

vector<size_t> isort(vector<double> &vec);

void GetAllFileName(const string &pathStr, set<string> &fileNameSet);

void ReadMergeForbid(const string &pathStr, set<wstring> &mfSet);;