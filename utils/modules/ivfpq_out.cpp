#include <iostream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <unordered_map>
#include "knn.hpp"

#define QUERIES 12

using namespace std;

template <typename T>
void ivfpq_out(unordered_map<string,void*>& value, DS_CHOICE choice){

    int seed = *(int*)value["seed"];
    int k = *(int*)value["kclusters"];
    int nprobe = *(int*)value["nprobe"];
    int N = *(int*)value["N"];
    int R = *(int*)value["R"];
    int M = *(int*)value["M"];
    int nbits = *(int*)value["nbits"];
    bool do_range = *(bool*)value["range"];
    string input = *(string*)value["d"];
    string query = *(string*)value["q"];
    string output = *(string*)value["o"];


    DataSet<T> dataset(input,choice);
    uint32_t train_s = 2000;
    Ivfpq<T> ivf(dataset,seed,k,nprobe,M,nbits,N,R, train_s);
    Metric<T,double> dist = L2;
    Metric<T,T> dist2 = L2;
    DataSet<T> dataset_test(query,choice,QUERIES);

    ofstream out(output);
    if(!out)
        throw runtime_error ("could not open " + output + " for writing\n");

    out << "IVFPQ" << endl << endl;
    double af = 0;
    double avrg_aprox = 0;
    double avrg_abs = 0;
    int query_set_size = dataset_test.get_size();
    vector<uint32_t> range_res;
    vector<uint32_t>* range_ptr = do_range ? &range_res : nullptr;
    double rec = 0.0;

    vector<vector<DIST_ID>> res(query_set_size);

    for(int i=0; i<query_set_size; i++){
        auto t1 = chrono::high_resolution_clock::now();
        res[i] = knn(dataset,*dataset_test[i],N,dist2);
        auto t2 = chrono::high_resolution_clock::now();
        double dur1 = chrono::duration_cast<chrono::duration<float>>(t2 - t1).count();
        avrg_abs += dur1;
    }

    ivf.build(dataset,dist);

    for(int i=0; i<query_set_size; i++){
        int hits = 0;
        range_res.clear();
        out << "Query: " << dataset_test[i]->get_id() << endl;

        auto t1 = chrono::high_resolution_clock::now();
        std::vector<DIST_ID> res_aprox = ivf.query(*dataset_test[i],range_ptr);
        auto t2 = chrono::high_resolution_clock::now();
        double dur2 = chrono::duration_cast<chrono::duration<float>>(t2 - t1).count();
        avrg_aprox += dur2;

        for(int j=0; j<res_aprox.size(); ++j){
            out << "Nearest neighbor-" << j+1 << " " << res_aprox[j].second << endl;
            out << "distanceApproximate: " << res_aprox[j].first << endl;
            out << "distanceTrue: " << res[i][j].first << endl;

            af += res_aprox[j].first / res[i][j].first;

            auto it = std::find_if(res_aprox.begin(), res_aprox.end(),
                       [&res,i, j](const std::pair<double, uint32_t>& p) {
                           return p.second == res[i][j].second;
                       });
            if(it != res_aprox.end())
                hits++;
            out << endl;
        }

        rec += ((double) hits) / res.size();

        if(range_res.size() != 0){
            out << "R-near neighbours" << endl;
            for(auto neigh : range_res){
                out << neigh << endl;
            }
            out << endl;
        }
    }

    out << endl;

    out << "Average AF: " << af / (query_set_size*N) << endl;
    out << "Recall@N: " << rec / query_set_size << endl;
    out << "QPS: " << query_set_size / avrg_aprox << endl;
    out << "tApproximateAverage: " << avrg_aprox / query_set_size << endl;
    out << "tTrueAverage: " <<  avrg_abs / query_set_size << endl;
}
