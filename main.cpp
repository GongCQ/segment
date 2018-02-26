#include "corpus.h"
#include "public.h"
#include <iostream>
// #include <string>
// #include <locale>
using namespace std;
// using namespace boost;
// using namespace boost::filesystem;


int main(){  
    Index maxLen = 1;
    Corpus cor("./corpus/sample.txt", "./corpus/merge_forbid.txt", 12);
    cout<<"size of corpus is "<<cor.GetSize()<<endl;
    cor.MergeSimple();

    const unsigned int minTot = 5;
    cor.MergeOnce(0.7, 'e', minTot);
    cor.MergeOnce(0.6, 'e', minTot);
    cor.MergeOnce(0.5, 'e', minTot);
    cor.MergeOnce(0.5, 'e', minTot);
    cor.MergeOnce(0.5, 'e', minTot);

    // cor.MergeOnce(100);

    // cor.MergeOnce(0.03, 'p');
    // cor.MergeOnce(0.04, 'p');
    // cor.MergeOnce(0.05, 'p');
    cout<<"===================================================="<<endl;
    cor.PrintLastMerge(false, " ");
}