#pragma once

#include <cmath>
#include <vector>
#include <fstream>
#include <cstdint>
/* #include <endian.h> */
#include "Vector.hpp"

#define DIST_ID std::pair<double,uint32_t>

typedef enum{MNIST, SIFT, SWISSPROT} DS_CHOICE;

template <typename T1, typename T2>
using Metric = double (*)(Vector<T1>&,Vector<T2>&,bool);

template <typename T1, typename T2>
double L2(Vector<T1>& v1, Vector<T2>& v2, bool squared){
    if(v1.get_size() != v2.get_size())
        throw std::runtime_error("L2: vector lengths dont match");

    double sum = 0;
    for(int i=0; i<v1.get_size(); i++){
        double d = static_cast<double>(v1[i]) - static_cast<double>(v2[i]);
        sum += d * d;
    }
    return squared ? sum : std::sqrt(sum);
}

template <typename T>
class DataPoint {
    Vector<T>* vec;
    const uint32_t id;

    public:
        DataPoint(std::ifstream& in ,uint32_t size, uint32_t vec_id): id(vec_id){
            vec = new Vector<T>(size); // init to 0
            in.read(reinterpret_cast<char*>(vec->get_data()),size*sizeof(T));
        }

        DataPoint(uint32_t size, uint32_t vec_id, uint32_t value=0): vec(size,value), id(vec_id){}

        DataPoint(const Vector<T>& v): id(0){
            vec = new Vector<T>(v); // init with v
        }

        ~DataPoint(){
            if(vec == nullptr) return;
            delete vec;
        }

        Vector<T>& get_vector(){
            return *vec;
        }

        uint32_t get_id(){
            return id;
        }

        size_t get_size(){
            return vec->get_size();
        }

        void set_null(){
            delete vec;
            vec = nullptr;
        }
};

template <typename T>
class DataSet {
    std::vector<DataPoint<T>*> datapoints;
    uint32_t vec_size;

    public:
        DataSet(std::string filepath,DS_CHOICE choice, uint32_t size=0){
            std::ifstream instream(filepath,std::ios::binary);
            if(choice == MNIST){
                if(!instream.is_open())
                    throw std::runtime_error("cannot open file: " + filepath);
                uint32_t num_imag,num_rows,num_cols;
                instream.ignore(4); // ignore magic number
                instream.read(reinterpret_cast<char*>(&num_imag),4);
                instream.read(reinterpret_cast<char*>(&num_rows),4);
                instream.read(reinterpret_cast<char*>(&num_cols),4);

                num_imag = size == 0 ? be32toh(num_imag) : size;
                num_rows = be32toh(num_rows);
                num_cols = be32toh(num_cols);

                vec_size = num_rows * num_cols;
                for(int i=0; i<num_imag; i++){
                    datapoints.push_back(new DataPoint<T>(instream,vec_size, i));
                }
            }else if(choice == SIFT){
                // read sift format
                if(!instream.is_open())
                    throw std::runtime_error("cannot open file: " + filepath);

                uint32_t num_imag;
                num_imag = size == 0 ? 1000000 : size;
                for(int i=0; i<num_imag; i++){
                    instream.read(reinterpret_cast<char*>(&vec_size),4);
                    datapoints.push_back(new DataPoint<T>(instream,vec_size,i));
                }
                vec_size = le32toh(vec_size);
            }else if(choice == SWISSPROT){
                // read swissprot format
                if(!instream.is_open())
                    throw std::runtime_error("cannot open file: " + filepath);

                vec_size = 320;
                uint32_t num_vec = size == 0 ? 50000 : size;
                for(int i=0; i<num_vec; i++){
                    datapoints.push_back(new DataPoint<T>(instream,vec_size,i));
                }
            }
        }

        ~DataSet(){
            if(datapoints[0] == nullptr) return;
            for(auto p : datapoints){
                delete p;
            }
        }

        DataPoint<T>* operator[](uint32_t index) const{
            return datapoints[index];
        }

        size_t get_size() const{
            return datapoints.size();
        }

        uint32_t get_vec_dim() const{
            return vec_size;
        }

        std::vector<DataPoint<T>*>& get_set() {
            return datapoints;
        }

};


double l2(const std::vector<float>& a, const std::vector<float>& b){
    double s=0; for (size_t i=0;i<a.size(); ++i){ double t=a[i]-b[i]; s+=t*t; } return std::sqrt(s);
}

std::vector<std::pair<uint32_t,double>>
brute_topk(const std::vector<std::vector<float>>& X, const std::vector<float>& q, int k){
    std::vector<std::pair<uint32_t,double>> v; v.reserve(X.size());
    for (uint32_t i=0;i<X.size(); ++i) v.emplace_back(i, l2(X[i], q));
    if ((int)v.size() > k){
        std::nth_element(v.begin(), v.begin()+k, v.end(),
            [](auto&a,auto&b){ return a.second < b.second; });
        v.resize(k);
    }
    std::sort(v.begin(), v.end(), [](auto&a,auto&b){ return a.second < b.second; });
    return v;
}
