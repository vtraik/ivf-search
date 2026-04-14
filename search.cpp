#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>
#include <array>
#include "ivfflat_out.hpp"
#include "ivfpq_out.hpp"
#include "parser.hpp"
#include "utils/include/utils.hpp"

using namespace std;

unordered_map<string,TYPE> valid = {
    {"d",STRING}, {"q",STRING}, {"k",INT},
    {"o",STRING}, {"N",INT}, {"R",INT},
    {"type",STRING}, {"range",BOOL},
    {"M",INT}, {"kclusters",INT}, {"nprobe",INT},
    {"ivfflat",NOVAL}, {"seed",INT}, {"nbits",INT}, {"ivfpq",NOVAL}
};

unordered_map<string,void*> value;

int main(int argc, const char** argv){
try{
    // parse args
    parse(argc,argv,value,valid);
    // find requested algorithm
    array<string,2> algos = {"ivfflat", "ivfpq"};
    uint8_t c = 0;
    for(auto& a : algos){
        auto it =value.find(a);
        if(it != value.end()) break;
        c++;
    }

    // call functs with their params
    if(algos[c] == "ivfflat"){
        if(*(string*)value["type"] == "mnist")
            ivfflat_out<uint8_t>(value,MNIST);
        else if(*(string*)value["type"] == "sift")
            ivfflat_out<float>(value,SIFT);
        else if(*(string*)value["type"] == "swissprot")
            ivfflat_out<float>(value,SWISSPROT);
    }else if(algos[c] == "ivfpq"){
        if(*(string*)value["type"] == "mnist")
            ivfpq_out<uint8_t>(value,MNIST);
        else if(*(string*)value["type"] == "sift")
            ivfpq_out<float>(value,SIFT);
        else if(*(string*)value["type"] == "swissprot")
            ivfpq_out<float>(value,SWISSPROT);
    }

    free_parser(value,valid);
}

catch (exception& e) {
    cerr << e.what();
    free_parser(value,valid); // free parser before exiting
    return -1;
}

}
