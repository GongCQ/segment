#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include "thulac_seg.h"
using std::string;
using std::wstring;
using std::vector;
using std::list;
using std::map;
using std::set;

int Joint(const string &corpusPath, set<wstring> mfSet, const string &userDictPath, 
          const vector<double> &mergeThreshold, const unsigned int minTot, 
          vector<string> &fileNameVec, vector<wstring> &contentVec, 
          vector<list<wstring> > &segVec, vector<list<wstring> > &segMergeVec);

void FindNew(const list<wstring> &segList, const list<wstring> &segMergeList, 
             set<wstring> &newWordSet, bool print);