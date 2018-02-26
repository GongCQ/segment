#include "public.h"
#include "iostream"
using std::swap;
using std::cout;
using std::endl;

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