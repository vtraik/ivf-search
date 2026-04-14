#include <unordered_map>
typedef enum {INT, DOUBLE, STRING, BOOL, NOVAL} TYPE; 

void parse(int argc, const char** argv,
std::unordered_map<std::string,void*>& value,
std::unordered_map<std::string,TYPE>& valid){
    for(int i=1; i<argc; i++){
        if(argv[i][0] == '-'){
            std::string arg = std::string(argv[i]).erase(0,1);
            auto it = valid.find(arg);
            if(it == valid.end())
                throw std::runtime_error(arg + ": not a valid argument\n");
            value[arg] = it->second == INT    ? (void*)new int(stoi(std::string(argv[i++ + 1]))) :
                         it->second == DOUBLE ? (void*)new double(stod(std::string(argv[i++ + 1]))) :
                         it->second == STRING ? (void*)new std::string(argv[i++ + 1]) :
                         it->second == BOOL   ? (void*)new bool(std::string(argv[i++ + 1]) == "true") :
                         it->second == NOVAL  ? nullptr : nullptr; 
        }
    }
}

void free_parser(
std::unordered_map<std::string,void*>& value,
std::unordered_map<std::string,TYPE>& valid){
    for(auto& [param,val] : value){
        if(valid[param] == INT)    delete (int*)val;
        if(valid[param] == DOUBLE) delete (double*)val;
        if(valid[param] == STRING) delete (std::string*)val;
        if(valid[param] == BOOL)   delete (bool*)val;
    }
}


