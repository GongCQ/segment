#ifndef TUHLAC_SEG_HEAD
#define TUHLAC_SEG_HEAD

#include "thulac/src/thulac_base.h"
#include "thulac/src/preprocess.h"
#include "thulac/src/postprocess.h"
#include "thulac/src/punctuation.h"
#include "thulac/src/cb_tagging_decoder.h"
#include "thulac/src/chinese_charset.h"
#include "thulac/src/thulac.h"
#include "thulac/src/filter.h"
#include "thulac/src/timeword.h"
#include "thulac/src/verbword.h"
#include "thulac/src/negword.h"
#include "thulac/src/wb_extended_features.h"
#include "thulac/src/wb_lattice.h"
#include "thulac/src/bigram_model.h"
#include "thulac_seg.h"
#include <boost/locale.hpp>
#include <sstream>
#include <fstream>
using namespace thulac;
using std::string;
using std::list;
using boost::locale::conv::from_utf;
using boost::locale::conv::to_utf;
using std::wstring;
using std::endl;
using std::cout;


const wstring RawToWStr(Raw raw){
    wstring wstr;
    wstr.reserve(raw.size());
    for(size_t s = 0; s < raw.size(); s++){
        unsigned int utf8code= raw[s];
        wstr.push_back((wchar_t)utf8code);
    }
    return wstr;
}

void ThulacSeg(const wstring &input, list<wstring> &segList, char *userDictPath, const char *modelPath) {
    string inputToStr = from_utf(input, "utf-8");
    ThulacSeg(inputToStr, segList, userDictPath, modelPath);
}

void ThulacSeg(const string &input, list<wstring> &segList, char *userDictPath, const char *modelPath){
    bool t2s = true;
    char *user_specified_dict_name = userDictPath;
    bool seg_only = true;
    const char *model_path_char = modelPath;
    bool useFilter = false;

    std::string prefix;
	if(model_path_char != NULL){
		prefix = model_path_char;
		if(*prefix.rbegin() != '/'){
			prefix += "/";
		}
	}else{
		prefix = "models/";
	}

	TaggingDecoder* cws_decoder=new TaggingDecoder();
	if(seg_only){
		cws_decoder->threshold=0;
	}else{
		cws_decoder->threshold=15000;
		//cws_decoder->threshold=25000;
	}

	permm::Model* cws_model = new permm::Model((prefix+"cws_model.bin").c_str());
	DAT* cws_dat = new DAT((prefix+"cws_dat.bin").c_str());
	char** cws_label_info = new char*[cws_model->l_size];
	int** cws_pocs_to_tags = new int*[16];

	get_label_info((prefix+"cws_label.txt").c_str(), cws_label_info, cws_pocs_to_tags);
	cws_decoder->init(cws_model, cws_dat, cws_label_info, cws_pocs_to_tags);
	cws_decoder->set_label_trans();

	TaggingDecoder* tagging_decoder = NULL;
	permm::Model* tagging_model = NULL;
	DAT* tagging_dat = NULL;
	char** tagging_label_info = NULL;
	int** tagging_pocs_to_tags = NULL;

    LatticeFeature* lf = new LatticeFeature();
	DAT* sogout=new DAT((prefix+"sgT.dat").c_str());
	lf->node_features.push_back(new SogouTFeature(sogout));

	std::vector<std::string> n_gram_model;
	std::vector<std::string> dictionaries;
	dictionaries.push_back(prefix+"sgW.dat");
	for(int i=0;i<dictionaries.size();++i){
		lf->node_features.push_back(new DictNodeFeature(new DAT(dictionaries[i].c_str())));
	}
	lf->filename=prefix+"model_w";
	lf->load();
	hypergraph::Decoder<int,LatticeEdge> decoder;
	decoder.features.push_back(lf);

	Preprocesser* preprocesser = new Preprocesser();
	preprocesser->setT2SMap((prefix+"t2s.dat").c_str());

	Postprocesser* ns_dict = new Postprocesser((prefix+"ns.dat").c_str(), "ns", false);
	Postprocesser* idiom_dict = new Postprocesser((prefix+"idiom.dat").c_str(), "i", false);
	Postprocesser* nz_dict = new Postprocesser((prefix+"nz.dat").c_str(), "nz", false);
	Postprocesser* ni_dict = new Postprocesser((prefix+"ni.dat").c_str(), "ni", false);
	Postprocesser* noun_dict = new Postprocesser((prefix+"noun.dat").c_str(), "n", false);
	Postprocesser* adj_dict = new Postprocesser((prefix+"adj.dat").c_str(), "a", false);
	Postprocesser* verb_dict = new Postprocesser((prefix+"verb.dat").c_str(), "v", false);
	// Postprocesser* vm_dict = new Postprocesser((prefix+"vm.dat").c_str(), "vm", false);
	Postprocesser* y_dict = new Postprocesser((prefix+"y.dat").c_str(), "y", false);

	Postprocesser* user_dict = NULL;
	if(user_specified_dict_name){
		user_dict = new Postprocesser(user_specified_dict_name, "uw", true);
	}

	//Punctuation* punctuation = new Punctuation((prefix+"pun.dat").c_str());
	Punctuation* punctuation = new Punctuation((prefix+"singlepun.dat").c_str());

	NegWord* negword = new NegWord((prefix+"neg.dat").c_str());
	TimeWord* timeword = new TimeWord();
	VerbWord* verbword = new VerbWord((prefix+"vM.dat").c_str(), (prefix+"vD.dat").c_str());

	Filter* filter = NULL;
	if(useFilter){
		filter = new Filter((prefix+"xu.dat").c_str(), (prefix+"time.dat").c_str());
	}

	POCGraph poc_cands;
	POCGraph new_poc_cands;
	int rtn=1;
	thulac::RawSentence oriRaw;
	thulac::RawSentence raw;
	thulac::RawSentence tRaw;
	thulac::SegmentedSentence segged;
	thulac::TaggedSentence tagged;

	const int BYTES_LEN=100000;
	char* s=new char[ BYTES_LEN];
	char* out=new char[BYTES_LEN];
	bool isFirstLine = true;
	int codetype = -1;
	Chinese_Charset_Conv conv;
	std::ostringstream ost;
	clock_t start = clock();
	bool containsT = false;

	std::string ori;
	if(input.size() > 99999){
		ori = input.substr(0, 99999);
		cout<<"warning : truncate 99999"<<endl;
	}
	else{
		ori = input;
	}
    strcpy(s,ori.c_str());
    size_t in_left=ori.length(); 
    
    if(isFirstLine){ 
        size_t out_left=BYTES_LEN;
        codetype = conv.conv(s,in_left,out,out_left);
        if(codetype >=0){
            int outlen=BYTES_LEN - out_left;
            //thulac::get_raw(oriRaw,out,outlen);
            //std::cout<<"Here"<<std::endl;
            thulac::get_raw(oriRaw,out,outlen);
        }else{
            std::cout<<"File should be encoded in UTF8 or GBK."<<"\n";
        }
        isFirstLine = false;
    }else{
        if(codetype == 0){
            //thulac::get_raw(oriRaw,s,in_left);
            thulac::get_raw(oriRaw,s,in_left);
        }else{
            size_t out_left=BYTES_LEN;
            codetype = conv.conv(s,in_left,out,out_left,codetype);
            int outlen=BYTES_LEN - out_left;
            //thulac::get_raw(oriRaw,out,outlen); 
            thulac::get_raw(oriRaw,out,outlen);
        }
    }
    
    if(preprocesser->containsT(oriRaw)){
        preprocesser->clean(oriRaw,tRaw,poc_cands);
        preprocesser->T2S(tRaw, raw);
        containsT = true;
    }else{
        preprocesser->clean(oriRaw,raw,poc_cands);
    }   


    if(raw.size()){
        cws_decoder->segment(raw, poc_cands, new_poc_cands);
        if(seg_only){
            cws_decoder->segment(raw, poc_cands, new_poc_cands);
            cws_decoder->get_seg_result(segged);
            ns_dict->adjust(segged);
            idiom_dict->adjust(segged);
            nz_dict->adjust(segged);
            noun_dict->adjust(segged);
            if(user_dict){
                user_dict->adjust(segged);
            }
            punctuation->adjust(segged);
            timeword->adjust(segged);
            if(useFilter){
                filter->adjust(segged);
            }
            if(codetype==0){
                for(int j = 0; j < segged.size(); j++){
                    // if(j!=0) std::cout<<" ";   // gcq: print segment result here
                    // std::cout<<segged[j];
                    segList.push_back(RawToWStr(segged[j]));
                }
            }else{
                for(int j = 0; j < segged.size(); j++){
                    if(j!=0) ost<<" ";
                    ost<<segged[j];
                }
                std::string str=ost.str();
                strcpy(s,str.c_str());
                size_t in_left=str.size();
                size_t out_left=BYTES_LEN;
                codetype = conv.invert_conv(s,in_left,out,out_left,codetype);
                int outlen=BYTES_LEN - out_left;
                std::cout<<std::string(out,outlen);
                ost.str("");
            }
        }
    }

    delete [] s;
	delete [] out;

	delete preprocesser;
	delete ns_dict;
	delete idiom_dict;
	delete nz_dict;
	delete ni_dict;
	delete noun_dict;
	delete adj_dict;
	delete verb_dict;
	// delete vm_dict;
	delete y_dict;
	if(user_dict != NULL){
		delete user_dict;
	}

	delete negword;
	delete timeword;
	delete verbword;
	delete punctuation;
	if(useFilter){
		delete filter;
	}

	delete lf;

	delete cws_decoder;
	if(cws_model != NULL){
		for(int i = 0; i < cws_model->l_size; i ++){
			if(cws_label_info) delete[](cws_label_info[i]);
		}
	}
	delete[] cws_label_info;

	if(cws_pocs_to_tags){
		for(int i = 1; i < 16; i ++){
			delete[] cws_pocs_to_tags[i];
		}
	}
	delete[] cws_pocs_to_tags;

	delete cws_dat;

	if(cws_model!=NULL) delete cws_model;

	delete tagging_decoder;
	if(tagging_model != NULL){
		for(int i = 0; i < tagging_model->l_size; i ++){
			if(tagging_label_info) delete[](tagging_label_info[i]);
		}
	}   
	delete[] tagging_label_info;
     
	if(tagging_pocs_to_tags){
		for(int i = 1; i < 16; i ++){
			delete[] tagging_pocs_to_tags[i];
		}
	}
	delete[] tagging_pocs_to_tags;

	delete tagging_dat;
	if(tagging_model!=NULL) 
        delete tagging_model;
};

#endif