#pragma once

#include <cmath>
#include <random>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <queue>

#include "utils.hpp"
#include "cluster.hpp"
#include "lloyd.hpp"

template <typename T>
class Ivfflat{
    std::vector<Cluster<T>*> clusters;
    std::vector<DataPoint<T>*> ds_subset;
    uint32_t init_seed;
    uint32_t kclusters;
    uint32_t nprobes;
    uint32_t N;
    uint32_t range;
    std::mt19937 rng;

    public:
        Ivfflat(const DataSet<T>& dataset, uint32_t seed, uint32_t kclust,
        uint32_t nprobe, uint32_t n, uint32_t R, uint32_t train_size):
        init_seed(seed), kclusters(kclust),
        nprobes(nprobe), N(n), range(R), rng(seed){
            if(nprobes >= kclusters)
                throw std::runtime_error("nprobes shouldn't be >= clusters\n");

            uint32_t ds_size = dataset.get_size();
            uint32_t subs_size = train_size;
            std::vector<uint32_t> indx(ds_size);

            // pick a random subset (sqrt(n)) from dataset
            std::iota(indx.begin(),indx.end(),1);
            std::shuffle(indx.begin(), indx.end(), rng);
            std::vector<uint32_t> subset_indexes = std::vector<uint32_t>(indx.begin(), indx.begin() + subs_size);
            for(auto indx : subset_indexes){
                ds_subset.push_back(dataset[indx]); // subset datapoints
            }
        }

        ~Ivfflat(){
            for(auto cl : clusters){
                delete cl;
            }
        }

        void build(const DataSet<T>& ds){
            lloyd(ds,ds_subset,clusters,rng,kclusters);
        }
        std::vector<DIST_ID> query(DataPoint<T>& queryp,Metric<T,double> dist,
                std::vector<uint32_t>* range_res=nullptr){
            std::vector<std::pair<double,Cluster<T>*>> centr_distances;
            for(auto cl : clusters){
               centr_distances.push_back(
                       {dist(queryp.get_vector(),cl->get_centroid(),false),cl}
                       );
            }
            std::sort(centr_distances.begin(),centr_distances.end());
            centr_distances.resize(nprobes); // keep only nprobe closest

            std::priority_queue<DIST_ID,std::vector<DIST_ID>> pq;
            // populate max heap with datapoints
            Metric<T,T> dist2=L2;
            for(auto cl : centr_distances){
                std::set<DataPoint<T>*>* points = cl.second->get_points();
                for(auto& p : *points){
                    double distance = dist2(p->get_vector(),queryp.get_vector(),false);
                    if(range_res != nullptr && distance <= range)
                        range_res->push_back(p->get_id());
                    if(pq.size() < N){
                        pq.push({distance,p->get_id()});
                    }else{
                        if(distance < pq.top().first){
                            pq.pop();
                            pq.push({distance,p->get_id()});
                        }
                    }
                }

            }

            // return N closest
            std::vector<DIST_ID> res;
            while(!pq.empty()){
                res.push_back({pq.top()});
                pq.pop();
            }
            std::reverse(res.begin(),res.end());
            return res;

        }

        std::vector<Cluster<T>*>& get_clusters(){
            return clusters;
        }
};
