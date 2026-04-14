#pragma once

#include <random>
#include <set>
#include "Vector.hpp"
#include "utils.hpp"

template <typename T>
class Cluster{
    Vector<double>* centroid;
    std::set<DataPoint<T>*>* points; 

    public:
        Cluster(Vector<T>& vec){
            centroid = new Vector<double>(vec.get_size());
            points = new std::set<DataPoint<T>*>;

            for(int i=0; i<vec.get_size(); ++i){
                (*centroid)[i] = static_cast<double>(vec[i]);
            }
        }

        ~Cluster(){
            delete centroid;
            if(points != nullptr)
                delete points;
        }

        Vector<double>& get_centroid(){ return *centroid; }
        std::set<DataPoint<T>*>* get_points(){ return points; }

        void set_points_null(){
            delete points;
            points = nullptr;
        }

        bool is_in_cluster(DataPoint<T>* ptr){
            return points->find(ptr) == points->end() ? false : true;
        }

        void remove(DataPoint<T>* point){
            points->erase(point);
        }
        
        void add(DataPoint<T>* point){
            points->insert(point);
        }
        
        template <typename U>
        void update_centroid(std::vector<DataPoint<U>*>& ds_subset,
            std::mt19937& rng){
            if(points->empty()){ // reinit to new point randomly
                std::uniform_int_distribution<size_t> index(0, ds_subset.size() - 1);
                size_t idx = index(rng);
                delete centroid;
                centroid = new Vector<double>(ds_subset[idx]->get_vector());
                return;
            }
            Vector<double>* sum = new Vector<double>(centroid->get_size(),0); 
            for(auto p : *points){
                *sum += p->get_vector();
            }

            centroid->init(*sum); // init centroid with sum
            *centroid /= points->size();
            delete sum;
        }

};

