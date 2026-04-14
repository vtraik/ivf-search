#pragma once

#include "utils.hpp"
#include <queue>
#include <vector>

template <typename T1, typename T2>
std::vector<DIST_ID>
knn(DataSet<T1>& dataset, DataPoint<T1>& query, uint32_t k, Metric<T1,T2> dist){
    struct
    {
        bool operator()(const DIST_ID p1, const DIST_ID p2) const { return p1.first > p2.first; }
    } compare_func;
    std::priority_queue<DIST_ID,std::vector<DIST_ID>,decltype(compare_func)> points;
    
    // populate min heap with datapoints
    for(int i=0; i<dataset.get_size(); i++){
        uint32_t id = dataset[i]->get_id();
        points.push({dist(dataset[i]->get_vector(),query.get_vector(),false),id}); 
    }
    
    // extract k nearest
    std::vector<DIST_ID> result;
    for(;k!=0 && points.size()!=0; --k){
       result.push_back(points.top());
       points.pop();
    }

    return result; 
}

