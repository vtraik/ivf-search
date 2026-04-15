#pragma once

#include <limits>
#include <random>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <omp.h>
#include "utils.hpp"
#include "cluster.hpp"

#include <chrono>

#define MAX_ITER 20

template <typename T,typename U>
void lloyd(const DataSet<T>& ds, std::vector<DataPoint<U>*>& ds_subset,
std::vector<Cluster<U>*>& clusters,
std::mt19937& rng, uint32_t kclusters) {
    // pick k centroids from subset randomly
    // std::shuffle(ds_subset.begin(),ds_subset.end(),rng);
    // for(int i=0; i<kclusters; ++i){
    //     clusters.push_back(new Cluster<U>(ds_subset[i]->get_vector()));
    // }
    initialize_centroids(ds_subset,clusters,rng,kclusters); // kmeans++ init step
    int it =1;
    while(true){
        int changed_cluster = 0;
        // assign each vector to centroid
        for(auto& dp : ds_subset){
            Vector<U>& point = dp->get_vector();
            Cluster<U>* closest_cen = find_nearest_centroid(point,clusters);
            if(assign_to_centroid(dp,clusters,closest_cen))
                changed_cluster++;
        }
        // update
        for(auto& cl : clusters){
            cl->update_centroid(ds_subset,rng);
        }

        if(changed_cluster == 0 && it > 1)
            break;

        if(it == MAX_ITER)
            break;
        it++;
    }

    // assign the rest ds to closest centroids
    #pragma omp parallel for
    for(int i=0; i<ds.get_size(); ++i){
        bool not_in_subset =
            std::find(ds_subset.begin(),ds_subset.end(),ds[i]) == ds_subset.end();
        if(not_in_subset){
            Cluster<U>* clos_cen = find_nearest_centroid(ds[i]->get_vector(),clusters);
            #pragma omp critical
            {
                clos_cen->add(ds[i]);
            }
        }
    }
}

void lloyd(std::vector<float>& residual,uint32_t train_size,uint32_t dim, uint32_t subvec_size,uint32_t offset,
std::vector<Vector<float>>& total_sub_clusters, uint32_t index,
std::mt19937& rng, uint32_t kclusters) {
    std::uniform_int_distribution<size_t> dist(0, train_size - 1);

    // initialize centroids
    for (size_t k = 0; k < kclusters; ++k) {
        size_t idx = dist(rng);
        const float* xi = &residual[idx * dim + offset];
        std::copy(xi, xi + subvec_size, total_sub_clusters[index+k].get_data());
    }


    std::vector<uint32_t> assigned_cluster(train_size, 0);
    std::vector<uint32_t> num_of_vecs(kclusters, 0);

    size_t iter;
    for (iter = 0; iter < MAX_ITER; ++iter) {
        int changed_per_it = 0;

        // assign to closest centroid
        for (size_t i = 0; i < train_size; ++i) {
            const float* xi = &residual[i * dim + offset];
            double best_dist = DBL_MAX;
            size_t best_k = 0;

            for (size_t k = 0; k < kclusters; ++k) {
                const float* ck = total_sub_clusters[index+k].get_data();
                double dist_sq = 0.0;
                for (size_t d = 0; d < subvec_size; ++d) {
                    double diff = xi[d] - ck[d];
                    dist_sq += diff * diff;
                }
                if (dist_sq < best_dist) {
                    best_dist = dist_sq;
                    best_k = k;
                }
            }
            if (assigned_cluster[i] != best_k) {
                assigned_cluster[i] = best_k;
                changed_per_it++;
            }
        }

        // update centroids
        std::fill(num_of_vecs.begin(), num_of_vecs.end(), 0);

        // reinit to zero
        for (int j=0; j<kclusters; j++){
            float* d = total_sub_clusters[index+j].get_data();
            std::fill(d, d + subvec_size, 0.0);
        }

        // compute sum for each cluster
        for (size_t i = 0; i < train_size; ++i) {
            const float* xi = &residual[i * dim + offset];
            size_t k = assigned_cluster[i];
            float* ck = total_sub_clusters[index+k].get_data();
            for (size_t d = 0; d < subvec_size; ++d) // add subvector xi to centroid's sum
                ck[d] += xi[d];
            num_of_vecs[k]++; // keep counter on elements of cluster
        }

        // compute mean on each cluster
        for (size_t k = 0; k < kclusters; ++k) {
            float* ck = total_sub_clusters[index+k].get_data();
            if (num_of_vecs[k] > 0) {
                float inv = 1.0 / num_of_vecs[k];
                for (size_t d = 0; d < subvec_size; ++d) // multiply each element with 1/num_elem_cl
                    ck[d] *= inv;
            } else {
                // reinitialize empty cluster randomly
                size_t idx = dist(rng);
                const float* xi = &residual[idx * dim + offset];
                std::copy(xi, xi + subvec_size, ck);
            }
        }

        if(((float)changed_per_it) / train_size < 1e-3 && iter > 5)
            break;

    }

}



template <typename T>
void initialize_centroids(const std::vector<DataPoint<T>*>& ds_subset,
std::vector<Cluster<T>*>& clusters, std::mt19937& rng, uint32_t kclusters){
    std::uniform_int_distribution<int> uni(0, ds_subset.size() - 1);
    // init 1st centr
    Metric<T,double> dist = L2;
    Vector<T>& first_v = ds_subset[uni(rng)]->get_vector();
    clusters.push_back(new Cluster<T>(first_v));
    std::vector<double> dist_squared;

    while(clusters.size() < kclusters){
        dist_squared.clear();
        for(int i=0; i<ds_subset.size(); ++i){
            Vector<T>& point = ds_subset[i]->get_vector();
            double min_d = dist(point,clusters[0]->get_centroid(),true);
            for(int j=1; j<clusters.size(); ++j){
                double d = dist(point,clusters[j]->get_centroid(),true);
                if(d < min_d){
                    min_d = d;
                }
            }
            dist_squared.push_back(min_d);
        }
        double total = 0.0;
        for(auto& d : dist_squared)
            total += d;

        std::uniform_real_distribution<> dis(0.0, total);
        double threshold = dis(rng);
        double cumulative = 0.0;

        for(int i=0; i<ds_subset.size(); ++i){
            cumulative += dist_squared[i];
            if(cumulative >= threshold){
                clusters.push_back(new Cluster<T>(ds_subset[i]->get_vector()));
                break;
            }

        }

    }
}

// assign point to closest centroid.
template <typename U>
bool assign_to_centroid(DataPoint<U>* point, std::vector<Cluster<U>*>& clusters, Cluster<U>* clos_clust){
    bool changed = false;
    for(auto cl : clusters){
        if(cl->is_in_cluster(point)){
            if(cl != clos_clust){ // add only if it has changed closest centroid
                changed = true;
                cl->remove(point);
                clos_clust->add(point);
            }
            return changed;
        }
    }
    clos_clust->add(point); // isnt yet in any cluster
    return changed;
}

template <typename T, typename U>
Cluster<U>* find_nearest_centroid(Vector<T>& point, std::vector<Cluster<U>*>& clusters){
    Metric<T,double> dist = L2;
    double min_d = DBL_MAX;
    Cluster<U>* closest_cen = nullptr;
    for(auto& cl : clusters){
        Vector<double>& cen = cl->get_centroid();
        double distance = dist(point,cen,false);
        if(distance < min_d){
            min_d = distance;
            closest_cen = cl;
        }
    }
    return closest_cen;
}


template <typename U, typename L>
std::pair<double,std::vector<double>> silhouette(std::vector<Cluster<U>*>& clusters,Metric<L,double> dist){
    std::pair<double,std::vector<double>> res;
    res.second.resize(clusters.size());

    #pragma omp parallel for schedule(dynamic)
    for(int i=0; i<clusters.size(); i++){
        size_t cluster_size = clusters[i]->get_points()->size();
        for(auto& p : *clusters[i]->get_points()){
            double min_d = DBL_MAX;
            Cluster<U>* sec_clos = nullptr;

            for(auto& clust : clusters){
                if(clusters[i] == clust) continue;
                double distance = dist(p->get_vector(),clust->get_centroid(),false);
                if(distance < min_d){
                    min_d = distance;
                    sec_clos = clust;
                }
            }

            double a = avrg_dist(p,clusters[i]);
            double b = avrg_dist(p,sec_clos);
            double si = (b-a) / std::max(a,b);
            res.second[i] += si;
        }
        res.second[i] /= cluster_size; // cluster's coef
    }


    res.first = 0.0;
    for(auto& cl_coef : res.second)
        res.first += cl_coef;

    res.first /= clusters.size(); // overall coef
    return res;
}

template <typename U, typename L>
std::pair<double,std::vector<double>>
silhouette_aprox(std::vector<Cluster<U>*>& clusters,Metric<L,double> dist, int seed){
    std::pair<double,std::vector<double>> res;
    res.second.resize(clusters.size());

    #pragma omp parallel for schedule(dynamic)
    for(int i=0; i<clusters.size(); i++){
        size_t cluster_size = clusters[i]->get_points()->size();
        std::uniform_int_distribution<int> uni(0, cluster_size - 1);
        std::mt19937 rng(seed+i);
        std::set<DataPoint<U>*>& s = *clusters[i]->get_points();
        for(int j=0; j<500; j++){
            int indx = uni(rng);
            auto it = s.begin();
            std::advance(it, indx); // get indx-th element of set
            DataPoint<U>* p = *it;
            double min_d = DBL_MAX;
            Cluster<U>* sec_clos = nullptr;
            for(auto& clust : clusters){
                if(clusters[i] == clust) continue;
                double distance = dist(p->get_vector(),clust->get_centroid(),false);
                if(distance < min_d){
                    min_d = distance;
                    sec_clos = clust;
                }
            }
            double a = avrg_dist(p,clusters[i]);
            double b = avrg_dist(p,sec_clos);
            double si = (b-a) / std::max(a,b);
            res.second[i] += si;
        }
        res.second[i] /= cluster_size; // cluster's coef
    }


    res.first = 0.0;
    for(auto& cl_coef : res.second)
        res.first += cl_coef;

    res.first /= clusters.size(); // overall coef
    return res;
}




template <typename T>
double avrg_dist(DataPoint<T>* point, Cluster<T>* cluster){
    double sum = 0.0;
    Metric<T,T> dist = L2;
    for(auto& p : *cluster->get_points()){
        if(p == point) continue;
        sum += dist(point->get_vector(),p->get_vector(),false);
    }
    int size = cluster->get_points()->size() - 1;
    return size <= 0 ? 0.0 : sum / size;

}
