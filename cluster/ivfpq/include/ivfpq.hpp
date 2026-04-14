#pragma once

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <queue>
#include <random>
#include <unordered_map>
#include "Vector.hpp"
#include "utils.hpp"
#include "cluster.hpp"
#include "lloyd.hpp"

#include <omp.h>

template <typename T>
class Ivfpq{
    std::vector<Cluster<T>*> clusters;
    std::vector<DataPoint<T>*> ds_subset;
    std::vector<Vector<float>> total_sub_clusters;
    std::vector<std::vector<uint32_t>> codes;
    std::unordered_map<uint32_t,std::vector<uint32_t>> inv_l;
    std::vector<uint32_t> res_ids;
    uint32_t init_seed;
    uint32_t kclusters;
    uint32_t nprobes;
    uint32_t M;
    uint32_t nbits;
    uint32_t N;
    uint32_t range;
    uint32_t subvec_size;
    std::mt19937 rng;

    public:

        Ivfpq(const DataSet<T>& dataset, uint32_t seed, uint32_t kclust,
        uint32_t nprobe, uint32_t m, uint32_t bits, uint32_t n, uint32_t R, uint32_t train_size):
        init_seed(seed), kclusters(kclust), nprobes(nprobe), M(m), nbits(bits),
        N(n), range(R), subvec_size(dataset.get_vec_dim()/M), rng(seed){
            if(nprobes >= kclusters)
                throw std::runtime_error("nprobes shouldn't be >= clusters\n");

            uint32_t ds_size = dataset.get_size();
            uint32_t subs_size = train_size;
            std::vector<uint32_t> indx(ds_size);

            // pick a random subset (train_size) from dataset
            std::iota(indx.begin(),indx.end(),1);
            std::shuffle(indx.begin(), indx.end(), rng);
            std::vector<uint32_t> subset_indexes = std::vector<uint32_t>(indx.begin(), indx.begin() + subs_size);
            for(auto& indx : subset_indexes){
                ds_subset.push_back(dataset[indx]); // susbet datapoints
            }

        }

        ~Ivfpq(){
            for(auto cl : clusters){
                delete cl;
            }
        }

        void build(DataSet<T>& ds,Metric<T,double> dist){
            // coarse clustering
            lloyd(ds,ds_subset,clusters,rng,kclusters);
            uint32_t vec_size = ds.get_vec_dim();

            // compute residual vector
            std::vector<float> residual;
            std::vector<uint32_t> resid_cluster(kclusters); // cluster[i] = indexof where cluster[i+1] starts
            int cl_num=0,count=0;
            for(int c=0; c<clusters.size(); c++){
                std::set<DataPoint<T>*>* s = clusters[c]->get_points();
                int cl_size = s->size();
                int point_num = 0;
                for(auto& point : *s){
                    Vector<T>& r = point->get_vector();
                    Vector<double>& cen = clusters[c]->get_centroid();
                    for(int k=0; k<vec_size; k++){
                        residual.push_back(r[k]-cen[k]);
                    }
                    res_ids.push_back(point->get_id());
                    point_num+=vec_size;
                    count++;
                    point->set_null(); // delete datapoint
                }
                resid_cluster[c] = count; // index of where next cluster begins
                clusters[c]->set_points_null(); // free this cluster's set pointers
                cl_num+=cl_size;
            }

            if(vec_size % M != 0)
                throw std::runtime_error("M is not multiple of vec size\n");

            // train on subspaces
            int k =  1u << nbits;

            total_sub_clusters.resize(k*M);
            for(auto& v : total_sub_clusters){
                v.get_vector().resize(subvec_size);
            }

            #pragma omp parallel for num_threads(6)
            for(int i=0; i<M; i++){
                int offset = i*subvec_size;
                std::mt19937 local_rng(init_seed + i);
                lloyd(residual,ds.get_size(),vec_size,subvec_size,offset, total_sub_clusters,k*i, local_rng,k);
            }

            // encodes
            codes.resize(ds.get_size());
            for(int i=0; i<codes.size(); i++){
                codes[i].resize(M);
            }

            uint32_t res = 0;
            uint32_t cluster=0,cluster_indx=resid_cluster[0];
            for(size_t i=0; i<residual.size(); i+=vec_size){
                uint32_t indx=0;
                uint32_t subspace=0;
                for(size_t l=i; l<i+vec_size; l+=subvec_size){ // for all subvectors r_i
                    double min_d = DBL_MAX;
                    uint32_t min_indx = 0;
                    for(size_t p=indx; p<indx+k; p++){
                        const float* cih = total_sub_clusters[p].get_data(); // p-th cluster of ri
                        double dist_sq = 0.0;
                        for(size_t t=0; t<subvec_size; t++){ // compute dist of r_i with c_i,h
                            double d =  residual[l+t] - cih[t];
                            dist_sq += d*d;
                        }
                        if(dist_sq < min_d){
                            min_d = dist_sq;
                            min_indx = p-indx;
                        }
                    }
                    codes[res_ids[res]][subspace] = min_indx; // code[dpid][subspace]
                    indx+=k;
                    subspace++;
                }
                // find coarse cluster that the residual vector belongs
                if(res > cluster_indx){
                    cluster++;
                    cluster_indx=resid_cluster[cluster];
                }
                // map this residual's vector coarse cluster to codes[dpid]
                inv_l[cluster].push_back(res_ids[res]);
                res++;
            }

        }

        std::vector<DIST_ID> query(DataPoint<T>& queryp,
            std::vector<uint32_t>* range_res=nullptr){
            Metric<T,double> dist = L2;
            std::vector<std::pair<double,int>> centr_distances;
            for(int i=0; i<clusters.size(); i++){
               centr_distances.push_back(
                       {dist(queryp.get_vector(),clusters[i]->get_centroid(),false),i}
                       );
            }
            std::sort(centr_distances.begin(),centr_distances.end());
            centr_distances.resize(nprobes); // keep only nprobe closest

            int k = 1u << nbits;
            std::vector<std::vector<float>> lut;
            lut.resize(M);
            for(int i=0; i<M; i++){
                lut[i].resize(k);
            }


            Metric<float,float> dist2 = L2;
            std::priority_queue<DIST_ID,std::vector<DIST_ID>> pq;
            for(int c=0; c<centr_distances.size(); c++){
                Vector<float> res(queryp.get_vector());
                res -= clusters[centr_distances[c].second]->get_centroid();
                int subspace = 0;
                Vector<float> sub(subvec_size);
                // compute lut
                for(int i=0; i<queryp.get_size(); i+=subvec_size){
                    auto start = res.get_vector().begin()+i;
                    sub.get_vector().assign(start,start+subvec_size);
                    for(int h=0; h<k; h++){
                        lut[subspace][h] = dist2(sub,
                                total_sub_clusters[subspace*k+h],true); // ||ri(q) - ci,h ||^2
                    }
                    subspace++;
                }
                // compute aprox distance
                for(uint32_t id : inv_l[centr_distances[c].second]){ // for coarse centroid's
                    double distance = 0.0;
                    for(int p=0; p<M; p++){
                        distance += lut[p][codes[id][p]];
                    }
                    distance = std::sqrt(distance);
                    if(range_res != nullptr && distance <= range)
                        range_res->push_back(id);

                    if(pq.size() < N){
                        pq.push({distance,id});
                    }
                    else if(distance < pq.top().first){
                        pq.pop();
                        pq.push({distance,id});
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

};
