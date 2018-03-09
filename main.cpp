#include "corpus.h"
#include "public.h"
#include "joint.h"
#include "thulac_seg.h"
#include <iostream>
#include <list>
#include <map>
#include <boost/locale.hpp>
using namespace std;
using boost::locale::conv::from_utf;
using boost::locale::conv::to_utf;

int main(){  
    vector<string> fileNameVec;
    vector<wstring> contentVec;
    vector<list<wstring> > segVec;
    vector<list<wstring> > segMergeVec;
    vector<double> mergeThreshold(2);
    mergeThreshold.at(0) = 0.6;
    mergeThreshold.at(1) = 0.5;
    const unsigned int minTot = 4;
    set<wstring> mfSet;
    ReadMergeForbid("./corpus/merge_forbid.txt", mfSet);
    const string corpusPath = "./corpus/for_each_day/sample/";
    const string userDictPath = "./user_dict";
    int status = Joint(corpusPath, mfSet, userDictPath, 
                       mergeThreshold, minTot, 
                       fileNameVec, contentVec,
                       segVec, segMergeVec);
    cout<<"status "<<status<<endl;
    cout<<fileNameVec.size()<<" "<<contentVec.size()<<" "<<segVec.size()<<" "<<segMergeVec.size()<<endl;
    // for(unsigned int n = 0; n < fileNameVec.size(); n++){
    //     cout<<endl<<".    "<<fileNameVec.at(n)<<endl;
    //     set<wstring> newWordSet;
    //     FindNew(segVec.at(n), segMergeVec.at(n), newWordSet, true); 
    // }
    for(unsigned int n = 0; n < fileNameVec.size(); n++){
        cout<<endl<<fileNameVec.at(n)<<" =================";
        cout<<endl<<"[S]     ";
        list<wstring> &segList = segVec.at(n);
        for(list<wstring>::const_iterator word = segList.begin(); word != segList.end(); word++){
            cout<<from_utf(*word, "utf-8")<<" ";
        }
        cout<<endl<<"[M]     ";
        list<wstring> &segMergeList = segMergeVec.at(n);
        for(list<wstring>::const_iterator word = segMergeList.begin(); word != segMergeList.end(); word++){
            cout<<from_utf(*word, "utf-8")<<" ";
        }
        cout<<endl;
    }
}