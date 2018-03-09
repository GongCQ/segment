#include "public.h"
#include "iostream"
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
using std::swap;
using std::cout;
using std::endl;
using boost::locale::conv::to_utf;
using boost::locale::conv::from_utf;
using namespace boost::filesystem;

void isort(vector<double> &vec, vector<size_t> &ivec){
	size_t size = vec.size();
	bool noswap = false;
	while(!noswap){
		noswap = true;
		for(size_t s = 0; s + 1 < size; s++){
			if(vec[s] > vec[s + 1]){
				swap(vec[s], vec[s + 1]);
				swap(ivec[s], ivec[s + 1]);
				noswap = false;
			}
		}
	}	
}

vector<size_t> isort(vector<double> &vec){
	size_t size = vec.size();
	vector<size_t> ivec = vector<size_t>(size);
	for(size_t s = 0; s < size; s++){
		ivec[s] = s;
	}
	isort(vec, ivec);
	return ivec;
}

void GetAllFileName(const string &pathStr, set<string> &fileNameSet){ 
    path director(pathStr);
    directory_iterator dirIteEnd;
	for(directory_iterator dirIte(director); dirIte != dirIteEnd; dirIte++)	{
		if(!is_directory(*dirIte)) {
			string fileName = (*dirIte).path().filename().string();
			fileNameSet.insert(fileName);
		}
	}
}

void ReadMergeForbid(const string &pathStr, set<wstring> &mfSet){
	boost::filesystem::ifstream mfFile;
    mfFile.open(pathStr);
    string mfLine;
    while(!mfFile.eof()){
        getline(mfFile, mfLine);
        wstring mfWline = to_utf<wchar_t>(mfLine, "utf-8");
        mfSet.insert(mfWline);
    }
	mfSet.insert(L"\f");
}