#include <set>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include "public.h"
#include "joint.h"
#include "corpus.h"
using std::set;
using std::cout;
using std::endl;
using boost::filesystem::ifstream;
using boost::filesystem::ofstream;
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::remove;
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;

int Joint(const string &corpusPath, set<wstring> mfSet, const string &userDictPath, 
          const vector<double> &mergeThreshold, const unsigned int minTot, 
          vector<string> &fileNameVec, vector<wstring> &contentVec, 
          vector<list<wstring> > &segVec, vector<list<wstring> > &segMergeVec){
    // read corpus text
    const wstring docSeparator = L"\f";
    set<string> fileNameSet;
    GetAllFileName(corpusPath, fileNameSet);
    fileNameVec.reserve(fileNameSet.size());
    for(set<string>::const_iterator name = fileNameSet.begin(); name != fileNameSet.end(); name++){
        fileNameVec.push_back(*name);
    }
    contentVec.resize(fileNameSet.size());
    list<wstring> charList;
    for(unsigned int n = 0; n < fileNameVec.size(); n++){
        const string &fileName = fileNameVec.at(n);
        string fullPath = corpusPath + fileName;
        ifstream fileReader;
        fileReader.open(fullPath);
        wstring &docStr = contentVec.at(n);
        docStr.reserve(200);
        while(!fileReader.eof()){
            string line;
            getline(fileReader, line);
            wstring wline = to_utf<wchar_t>(line, "utf-8");
            for(int i = 0; i < wline.size(); i++){
                wstring currentChar = wline.substr(i, 1);
                if(currentChar != docSeparator){
                    charList.push_back(currentChar);
                    docStr.append(currentChar);
                }
            }
            charList.push_back(L"\n");
        }
        charList.push_back(docSeparator);
    }

    // merge simply and then find merged fraction
    Corpus corpusSimple(charList, mfSet, 12);
    corpusSimple.MergeSimple();
    list<wstring> simpleMergeList;
    corpusSimple.GetLastMerge(simpleMergeList);
    string tempDictFileName = "__temp_dict_file";
    path tempDictFilePath(tempDictFileName);
    if(exists(tempDictFilePath)){
        remove(tempDictFilePath);
    }
    ofstream tempDictFile(tempDictFileName);
    set<wstring> tempDictSet;
    for(list<wstring>::const_iterator frac = simpleMergeList.begin(); frac != simpleMergeList.end(); frac++){
        if(frac->size() > 1 && tempDictSet.find(*frac) == tempDictSet.end()){
            tempDictFile<<from_utf(*frac, "utf-8")<<std::endl;
            tempDictSet.insert(*frac);
        }
    }
    charList.clear();
    tempDictSet.clear();
    corpusSimple.Clear();

    // user dictionary
    ifstream userDictFile;
    userDictFile.open(userDictPath);
    string userWord;
    while(!userDictFile.eof()){
        getline(userDictFile, userWord);
        tempDictFile<<userWord<<endl;
    }

    // segment by thulac
    segVec.resize(fileNameSet.size());
    list<wstring> allSegList;
    for(unsigned int n = 0; n < contentVec.size(); n++){
        list<wstring> &segList = segVec.at(n);
        ThulacSeg(contentVec.at(n), segList, const_cast<char*>(tempDictFileName.c_str()), "./thulac/models/");
        for(list<wstring>::const_iterator word = segList.begin(); word != segList.end(); word++){
            allSegList.push_back(*word);
        }
        allSegList.push_back(docSeparator);
    }

    // merge
    segMergeVec.resize(fileNameSet.size());
    Corpus corpusMerge(allSegList, mfSet, 12);
    for(unsigned int t = 0; t < mergeThreshold.size(); t++){
        corpusMerge.MergeOnce(mergeThreshold.at(t), 'e', minTot);
    }
    list<wstring> allMergeList;
    corpusMerge.GetLastMerge(allMergeList);

    list<wstring> mergeList;
    unsigned int docCount = 0;
    for(list<wstring>::const_iterator word = allMergeList.begin(); word != allMergeList.end(); word++){
        if(*word == docSeparator){
            segMergeVec.at(docCount) = mergeList;
            mergeList.clear();
            docCount++;
        }
        else{
            mergeList.push_back(*word);
        }
    }

    return docCount == fileNameSet.size() ? 0 : 1;
}

void FindNew(const list<wstring> &segList, const list<wstring> &segMergeList, 
             set<wstring> &newWordSet, bool print){
    set<wstring> segSet;
    for(list<wstring>::const_iterator word = segList.begin(); word != segList.end(); word++){
        segSet.insert(*word);
    }
    set<wstring> segMergeSet;
    for(list<wstring>::const_iterator word = segMergeList.begin(); word != segMergeList.end(); word++){
        segMergeSet.insert(*word);
    }

    for(set<wstring>::const_iterator word = segMergeSet.begin(); word != segMergeSet.end(); word++){
        if(segSet.find(*word) == segSet.end()){
            newWordSet.insert(*word);
            if(print){
                cout<<from_utf(*word, "utf-8")<<" ";
            }
        }
    }
}