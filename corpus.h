#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using std::string;
using std::wstring;
using std::vector;
using std::list;
using std::map; 
using std::set;
using std::size_t;

typedef size_t Index;

class Indexer{
protected:
    map<wstring, Index> strToSeq;
    vector<wstring> seqToStr;

public: 
    Indexer();
    Index Add(const wstring &wstr);
    void Clear();
    void Reserve(Index capacity);
    const wstring &operator[](const Index &seq) const;
    const Index &operator[](const wstring &wstr) const;
};

class Entropy{
protected:
    map<Index, map<Index, Index> > counters;
    map<Index, double> entMap;
    map<Index, Index> totMap;
    map<Index, map<Index, double> > probMap;
    void Summary(const map<Index, Index> &counter, double &ent, Index &tot);

public:
    void Add(const Index &first, const Index &second);
    void Clear();
    void Complete();
    double GetEnt(Index index);
    double GetProb(Index cond, Index follow);
    Index GetTot(Index index);
    void PrintInfo();
};

class Corpus{
protected: 
    wchar_t* corChars;
    Index maxLen;
    Index corSize;
    Indexer indexer;
    set<wstring> mfSet;
    map<wstring, Index> fracFreq;
    Entropy fracEntNext;
    Entropy fracEntPrior;
    list<vector<Index> > fracMerge;
    list<vector<bool> > mfLabel;

    // convert a fraction to wstring
    // begin: the begin of fraction
    // len  : then length of fraction
    wstring FracToWStr(Index begin, Index len);

    // convert a fraction to string
    // begin: the begin of fraction
    // len  : then length of fraction
    string FracToStr(Index begin, Index len);

    // evaluate frequency of each fraction, 
    // and save them into this->fracFreq.
    void FracFreq();

    // evaluate entropy and conditional probability for fractions,
    // and save them into this->fracEntNext and this->fracEntPrior. 
    void EvalEnt();

public: 
    // constructor
    // path: the path of corpus file
    // mergeForbidPath: the path of a file which contain some string line-by-line which are forbade to be merged, 
    // maxLen: max length of a word
    Corpus(const string path, const string mergeForbidPath, unsigned int maxLen);
    ~Corpus();

    // merge some simple pattern such as date and number
    void MergeSimple();

    // merge fractions once
    // threshold: threshold for mergence parameter
    // option: method for mergence, 'e' for entropy, 'p' for probability.
    void MergeOnce(double threshold, const char option, const unsigned int minTot);

    // get the frequency of a special fraction
    // frac: a fraction
    Index GetFracFreq(const wstring &frac);

    // print the result of last mergence
    // index: whether print the index of fraction or not
    void PrintLastMerge(bool index, const string separator);

    // print the strings which are forbade to be merged
    void PrintMergeForbid();

    // get the size of corpus.
    Index GetSize();
};