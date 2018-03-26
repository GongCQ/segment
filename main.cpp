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

// ./build -cp ./corpus/for_each_day/latest/ -mf ./corpus/merge_forbid.txt  -ud ./user_dict -mc 4 -mt 0.6 -mt 0.5 -tp ./corpus/result/ -print 0
int main(int argc, char **argv){  
    // read parameters
    string corpusPath = "./corpus/for_each_day/latest/";
    string mfSetPath = "./corpus/merge_forbid.txt";
    string userDictPath = "./user_dict";
    vector<double> mergeThreshold;
    unsigned int minCount = 4;
    string targetPath = "./corpus/result/";
    string seperator = " ";
    bool print = true;
    for(int p = 1; p < argc; p += 2){
        if(strcmp(argv[p], "-cp") == 0){
            corpusPath = argv[p + 1];
        }
        else if(strcmp(argv[p], "-mf") == 0){
            mfSetPath = argv[p + 1];
        }
        else if(strcmp(argv[p], "-ud") == 0){
            userDictPath = argv[p + 1];
        }
        else if(strcmp(argv[p], "-mc") == 0){
            minCount = atoi(argv[p + 1]); 
        }
        else if(strcmp(argv[p], "-mt") == 0){
            mergeThreshold.push_back(atof(argv[p + 1]));
        }
        else if(strcmp(argv[p], "-tp") == 0){
            targetPath = argv[p + 1];
        }
        else if(strcmp(argv[p], "-sp") == 0){
            seperator = argv[p + 1];
        }
        else if(strcmp(argv[p], "-print") == 0){
            print = strcmp(argv[p + 1], "0") == 0 ? false : true;
        }
    }
    cout<<"**************** parameters ************************"<<endl;
    cout<<"corpusPath     : " << corpusPath<<endl;
    cout<<"mfSetPath      : " << mfSetPath<<endl;
    cout<<"userDictPath   : " << userDictPath<<endl;
    cout<<"mergeThreshold : ";
    for(int i = 0; i < mergeThreshold.size(); i++){
        cout<<mergeThreshold.at(i)<<" ";
    }
    cout<<endl;
    cout<<"minCount       : "<<minCount<<endl;
    cout<<"targetPath     : "<<targetPath<<endl;
    cout<<"seperator      : "<<seperator<<endl;
    cout<<"print          : "<<print<<endl;
    cout<<"****************************************************"<<endl;

    // segment and merge
    vector<string> fileNameVec;
    vector<wstring> contentVec;
    vector<list<wstring> > segVec;
    vector<list<wstring> > segMergeVec;
    set<wstring> mfSet;
    ReadMergeForbid(mfSetPath, mfSet);
    int status = Joint(corpusPath, mfSet, userDictPath, 
                       mergeThreshold, minCount, 
                       fileNameVec, contentVec,
                       segVec, segMergeVec);
    cout<<"completed! status "<<status<<" ... "<<(status == 0 ? "OK" : "ERROR")<<endl;
    cout<<"please turn to " << targetPath <<endl;

    ToFile(targetPath, seperator, fileNameVec, contentVec, segVec, segMergeVec);

    // print
    if(print){
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
            cout<<endl<<"[N]     ";
            set<wstring> newWordSet;
            FindNew(segVec.at(n), segMergeVec.at(n), newWordSet, true); 
            cout<<endl;
        }
    }

    return status;
}