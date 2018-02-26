#include "public.h"
#include "corpus.h"
#include "iostream"
#include <cmath>
#include <exception>
#include <list>
#include <float.h>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
using std::log2;
using std::cout;
using std::endl;
using std::list;
using std::pair;
using std::out_of_range;
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;

// Index --------------------------------------------------
Indexer::Indexer(){
    seqToStr.reserve(8);
}

Index Indexer::Add(const wstring &wstr){
    map<wstring, Index>::const_iterator ite = strToSeq.find(wstr);
    if(ite != strToSeq.end()){
        return ite->second;
    }
    else{
        Index seq = strToSeq.size();
        strToSeq.insert(pair<wstring, Index>(wstr, seq));
        seqToStr.push_back(wstr);
        return seq;
    }
}

void Indexer::Clear(){
    strToSeq.clear();
    seqToStr.clear();
}

void Indexer::Reserve(Index capacity){
    seqToStr.reserve(capacity);
}

const wstring& Indexer::operator[](const Index &seq) const{
    return seqToStr.at(seq);
}

const Index& Indexer::operator[](const wstring &wstr) const{
    map<wstring, Index>::const_iterator ite = strToSeq.find(wstr);
    if(ite == strToSeq.end()){
        throw out_of_range("query wstr does not exist in map");
    }
    return ite->second;
}

// Entropy ------------------------------------------------
void Entropy::Summary(const map<Index, Index> &counter, double &ent, Index &tot){ 
    tot = 0;
    for(map<Index, Index>::const_iterator it = counter.begin(); it != counter.end(); it++){
        tot += it->second;
    }
    ent = 0;
    double dtot = (double)tot;
    for(map<Index, Index>::const_iterator it = counter.begin(); it != counter.end(); it++){
        ent += (it->second / dtot) * log2(dtot / it->second);
    }
}

void Entropy::Add(const Index &first, const Index &second){
    map<Index, map<Index, Index> >::iterator it = this->counters.find(first);
    if(it == this->counters.end()){
        this->counters.insert(pair<Index, map<Index, Index> >(first, map<Index, Index>()));
        it = this->counters.find(first);
    }
    map<Index, Index>::iterator itc = it->second.find(second);
    if(itc == it->second.end()){
        it->second.insert(pair<Index, Index>(second, 1));
    }
    else{
        itc->second += 1;
    }
}

void Entropy::Clear(){
    this->counters.clear();
    this->entMap.clear();
    this->totMap.clear();
    this->probMap.clear();
}

void Entropy::Complete(){
    this->entMap.clear();
    this->totMap.clear();
    this->probMap.clear();
    for(map<Index, map<Index, Index> >::const_iterator ite = this->counters.begin(); ite != this->counters.end(); ite++){
        double ent;
        Index tot;
        Summary(ite->second, ent, tot);
        this->entMap.insert(pair<Index, double>(ite->first, ent));
        this->totMap.insert(pair<Index, Index>(ite->first, tot));
        this->probMap.insert(pair<Index, map<Index, double> >(ite->first, map<Index, double>()));
        map<Index, double> &prob = this->probMap.find(ite->first)->second;
        const map<Index, Index> &counter = ite->second;
        for(map<Index, Index>::const_iterator i = counter.begin(); i != counter.end(); i++){
            prob.insert(pair<Index, double>(i->first, double(i->second) / tot));
        }
    }
}

double Entropy::GetEnt(Index index){
    map<Index, double>::const_iterator ite = this->entMap.find(index);
    return ite == this->entMap.end() ? -1 : ite->second;
}

double Entropy::GetProb(Index cond, Index follow){
    map<Index, map<Index, double> >::const_iterator itec = this->probMap.find(cond);
    if(itec == this->probMap.end()){
        return -1;
    }
    else{
        map<Index, double>::const_iterator itef = itec->second.find(follow);
        return itef == itec->second.end() ? -1 : itef->second;
    }
}

Index Entropy::GetTot(Index index){
    map<Index, Index>::const_iterator ite = this->totMap.find(index);
    return ite == this->totMap.end() ? -1 : ite->second;
}

void Entropy::PrintInfo(){
    cout<<"size of counters: "<<counters.size()<<endl;
    cout<<"size of entMap: "<<entMap.size()<<endl;
    cout<<"size of totMap: "<<totMap.size()<<endl;
    cout<<"size of probMap: "<<probMap.size()<<endl;
}

// Corpus -------------------------------------------------
wstring Corpus::FracToWStr(Index begin, Index len){
    if(begin + len > this->corSize){
        throw out_of_range("begin+len excessed the size of corpus");
    }
    return wstring(corChars + begin, len);
}

string Corpus::FracToStr(Index begin, Index len){
    return from_utf(FracToWStr(begin, len), "utf-8");
}

void Corpus::FracFreq(){
    vector<Index> &lastMerge = fracMerge.back();
    this->fracFreq.clear();
    Index begin = 0;
    while(begin < this->corSize){
        if(lastMerge.at(begin) == 0){
            begin++;
            continue;
        }
        wstring frac = FracToWStr(begin, lastMerge.at(begin) - begin);
        map<wstring, Index>::iterator it = this->fracFreq.find(frac);
        if(it == this->fracFreq.end()){
            this->fracFreq.insert(pair<wstring, Index>(frac, 1));
        }
        else{
            it->second += 1;
        }
        begin = lastMerge.at(begin);
    }  
}

void Corpus::EvalEnt(){
    this->fracEntNext.Clear();
    this->fracEntPrior.Clear();
    const vector<Index> &lastMerge = this->fracMerge.back();
    Index size = lastMerge.size();
    for(Index left = 0; left < size; left++){
        if(lastMerge.at(left) != 0 && lastMerge.at(left) != size - 1){
            Index middle = lastMerge.at(left);
            Index right = lastMerge.at(middle);
            Index first = this->indexer[FracToWStr(left, middle - left)];
            Index second = this->indexer[FracToWStr(middle, right - middle)];
            this->fracEntNext.Add(first, second);
            this->fracEntPrior.Add(second, first);
        }
    }
    this->fracEntNext.Complete();
    this->fracEntPrior.Complete();
}

Corpus::Corpus(const string path, const string mergeForbidPath, unsigned int maxLen){
    setlocale(LC_ALL, "");
    this->corChars = nullptr;
    this->maxLen = maxLen;
    this->fracMerge = list<vector<Index> >();
    this->mfLabel = list<vector<bool> >();

    // get each character
    list<wchar_t> tempCharList;
    boost::filesystem::ifstream corpusFile;
    corpusFile.open(path);
    string line;
    while(!corpusFile.eof()){
        getline(corpusFile, line);
        wstring wline = to_utf<wchar_t>(line, "utf-8");
        for(int c = 0; c < wline.size(); c++){
            tempCharList.push_back(wline[c]);
        }
        tempCharList.push_back('\n');
    }
    this->indexer.Reserve(tempCharList.size());
    
    // put characters into array
    this->corSize = tempCharList.size();
    this->corChars = new wchar_t[tempCharList.size() + 1];
    int i = 0;
    for(list<wchar_t>::iterator ite = tempCharList.begin(); ite != tempCharList.end(); ite++){
        this->corChars[i] = *ite;
        const wstring ws(this->corChars + i, 1);
        Index seq = this->indexer.Add(ws);
        i++;
    }
    this->corChars[i] = '\0';

    // merge forbid characters
    boost::filesystem::ifstream mfFile;
    mfFile.open(mergeForbidPath);
    string mfLine;
    while(!mfFile.eof()){
        getline(mfFile, mfLine);
        wstring mfWline = to_utf<wchar_t>(mfLine, "utf-8");
        mfSet.insert(mfWline);
    }
}

Corpus::~Corpus(){
    delete[] corChars;
}

void Corpus::MergeSimple(){
    this->fracMerge.push_back(vector<Index>(this->corSize + 1));
    this->mfLabel.push_back(vector<bool>(this->corSize + 1));
    vector<Index> &lastMerge = this->fracMerge.back();
    vector<bool> &lastLabel = this->mfLabel.back();
    for(Index b = 0; b < this->corSize + 1; b++){
        lastMerge.at(b) = 0;
        lastLabel.at(b) = true;
    }

    Index b = 0;
    while(b < this->corSize){
        const wchar_t beginChar = this->corChars[b];
        if(mfSet.find(FracToWStr(b, 1)) != mfSet.end()){
            lastMerge.at(b) = b + 1;
            lastLabel.at(b) = false;
            b++;
            continue;
        }

        // it's may be a date
        bool isDate = false;
        if(beginChar >= L'0' && beginChar <= L'9'){ 
            for(Index l = 1; b + l <= this->corSize; l++){
                const wchar_t end = this->corChars[b + l];
                if(!((end >= L'0' && end <= L'9') || end == L'年' || end == L'月' || end == L'日')){
                    for(Index il = l; il >= 1; il--){ // search the end of the possible date inversely
                        const wchar_t priorEnd = this->corChars[b + il - 1];
                        if(priorEnd == L'年' || priorEnd == L'月' || priorEnd == L'日'){
                            lastMerge.at(b) = b + il;
                            lastLabel.at(b) = false;
                            this->indexer.Add(FracToWStr(b, il));
                            b = b + il;
                            isDate = true;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        if(isDate){
            continue;
        }

        // it's may be a number
        bool isNum = false;
        if(beginChar >= L'0' && beginChar <= L'9'){  
            for(Index l = 1; b + l <= this->corSize; l++){
                const wchar_t end = this->corChars[b + l];
                if(!((end >= L'0' && end <= L'9') || end == L'%' || end == L'.' || end == ',')){
                    lastMerge.at(b) = b + l;
                    lastLabel.at(b) = false;
                    this->indexer.Add(FracToWStr(b, l));
                    b = b + l;
                    isNum = true;
                    break;
                }
            }
        }
        if(isNum){
            continue;
        }

        // it's not a special fraction
        lastMerge.at(b) = b + 1;
        lastLabel.at(b) = (mfSet.find(FracToWStr(b, lastMerge.at(b) - b)) == mfSet.end());
        b++;
    }

    FracFreq();
    EvalEnt();
}

void Corpus::MergeOnce(double threshold, const char option, const unsigned int minTot){
    // fraction frequency after merge once completely.
    vector<Index> &lastMerge = fracMerge.back();
    map<wstring, Index> fracFreq2;;
    Index begin = 0;
    while(begin < this->corSize){
        Index end = lastMerge.at(begin);
        if(end == 0){
            begin++;
            continue;
        }
        Index end2 = lastMerge.at(lastMerge.at(begin));
        if(end2 == 0){
            break;
        }
        wstring frac = FracToWStr(begin, end2 - begin);
        map<wstring, Index>::iterator it = fracFreq2.find(frac);
        if(it == fracFreq2.end()){
            fracFreq2.insert(pair<wstring, Index>(frac, 1));
        }
        else{
            it->second += 1;
        }
        begin = lastMerge.at(begin);
    }  

    // probabilistic multiple of the merged fraction
    vector<Index> &preMerge = this->fracMerge.back();
    vector<bool> &preLabel = this->mfLabel.back();
    vector<Index> merge = preMerge;
    vector<bool> label = preLabel;
    begin = 0;
    while(begin < this->corSize){
        if(preMerge.at(begin) == 0 || !preLabel.at(begin)){
            begin++;
            continue;
        }
        Index end = begin + 1;
        while(end < this->corSize + 1){
            if(end == this->corSize || (preMerge.at(end) != 0 && !preLabel.at(end))){
                break;
            } 
            end++;
        }
                
        vector<double> probMultVec;
        vector<Index> probMultVecI;
        probMultVec.reserve(end - begin);
        probMultVecI.reserve(end - begin);
        for(Index loc = begin; loc < end; loc++){
            // 0. it's not a begin of a fraction
            // 1. the next of this fraction excess the end
            if(preMerge.at(loc) == 0 || preMerge.at(loc) == end){
                continue;
            }
            Index left = loc;
            Index middle = preMerge.at(loc);
            Index right = preMerge.at(preMerge.at(loc));
            const wstring fracLeft = FracToWStr(left, middle - left);
            const wstring fracRight = FracToWStr(middle, right - middle);
            const wstring fracMiddle = FracToWStr(left, right - left);
            if(option == 'p'){
                map<wstring, Index>::iterator freqIteLeft = this->fracFreq.find(fracLeft);
                double fracFreqLeft = freqIteLeft == this->fracFreq.end() ? 0 : freqIteLeft->second;
                double fracProbLeft = fracFreqLeft / this->corSize;
                map<wstring, Index>::iterator freqIteRight = this->fracFreq.find(fracRight);
                double fracFreqRight = freqIteRight == this->fracFreq.end() ? 0 : freqIteRight->second;
                double fracProbRight = fracFreqRight / this->corSize;
                map<wstring, Index>::iterator freqIteMiddle = fracFreq2.find(fracMiddle);
                double fracFreqMiddle = freqIteMiddle == fracFreq2.end() ? 0 : freqIteMiddle->second;        
                double fracProbMiddle = fracFreqMiddle / this->corSize;

                double viscosity = fracFreqMiddle / (fracFreqLeft + fracFreqRight) / 2 ; //(fracFreqLeft > fracFreqRight ? fracFreqLeft : fracFreqRight);
                // double viscosity = fracProbMiddle / (fracProbLeft * fracProbRight);
                if(viscosity > threshold){
                    probMultVec.push_back(-viscosity); // probabilistic viscosity
                    probMultVecI.push_back(loc);
                }
            }
            else if(option == 'e'){         
                Index indLeft = this->indexer[fracLeft];
                Index indRight = this->indexer[fracRight];

                double entLeft = this->fracEntNext.GetEnt(indLeft);
                double probLeft = this->fracEntNext.GetProb(indLeft, indRight);
                Index totLeft = this->fracEntNext.GetTot(indLeft);
                double viscLeft = totLeft >= minTot ? (entLeft > 0 ? -log2(probLeft) / entLeft : 0) : DBL_MAX;

                double entRight = this->fracEntPrior.GetEnt(indRight);
                double probRight = this->fracEntPrior.GetProb(indRight, indLeft);
                Index totRight = this->fracEntNext.GetTot(indRight);
                double viscRight = totRight >= minTot ? (entRight > 0 ? -log2(probRight) / entRight : 0) : DBL_MAX;

                double viscosity = viscLeft < viscRight ? viscLeft : viscRight;
                if(viscosity < threshold){
                    probMultVec.push_back(viscosity); // entropy viscosity
                    probMultVecI.push_back(loc);
                }
            }
        }

        // sort and merge one by one
        isort(probMultVec, probMultVecI);
        Index size = probMultVecI.size();
        for(Index i = 0; i < size; i++){
            Index left = probMultVecI.at(i);
            if(left == 0){
                continue;
            }
            Index middle = merge.at(left);
            if(middle == 0){
                continue;
            }
            Index right = merge.at(middle);
            if(right == 0){
                continue;
            }
            // this merge is valid only if the right fraction has not been merged into the next.
            if(merge.at(middle) == preMerge.at(middle) && right - left <= this->maxLen){ 
                merge.at(left) = right;
                merge.at(middle) = 0;
                label.at(middle) = true;
                this->indexer.Add(FracToWStr(left, right - left));
                if(mfSet.find(FracToWStr(left, right - left)) != mfSet.end()){
                    label.at(left) = false;
                }
            }
        }

        // go to the head of next sentence
        begin = end;
    }
    this->fracMerge.push_back(merge);
    this->mfLabel.push_back(label);

    FracFreq();
    EvalEnt();
}

Index Corpus::GetFracFreq(const wstring &frac){
    map<wstring, Index>::iterator it = this->fracFreq.find(frac);
    return it == this->fracFreq.end() ? 0 : it->second;
}

void Corpus::PrintLastMerge(bool index, const string separator){
    vector<Index> &lastMerge = this->fracMerge.back();
    vector<bool> &lastLabel = this->mfLabel.back();
    for(Index i = 0; i < this->corSize; i++){
        if(index && lastMerge[i] != 0){
            cout<<"[" << i<<"_"<<lastMerge[i]<<" "<<lastLabel[i]<<"]";
        }
        if(lastMerge[i] != 0){
            cout<<FracToStr(i, lastMerge[i] - i);
            cout<<separator;
        }
    }
}

void Corpus::PrintMergeForbid(){
    for(set<wstring>::iterator it = mfSet.begin(); it != mfSet.end(); it++){
        cout<<from_utf((*it), "utf-8")<<endl;
    }
}

Index Corpus::GetSize(){
    return this->corSize;
}