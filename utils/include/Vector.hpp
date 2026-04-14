#pragma once 

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <type_traits>

template <typename T>
class Vector{
    std::vector<T> data; 
    public:
        Vector() = default;
        Vector(int size, int val=0): data(size,val){}
        Vector(const Vector<T>& vec): data(vec.data){}
        
        template <typename U>
        Vector(const Vector<U>& vec){
            for(int i=0; i<vec.get_size(); ++i)
                data.push_back(static_cast<T>(vec[i])); // only upcast int types -> double
        }

        template<typename U>
        void init(const Vector<U>& vec){
            for(int i=0; i<vec.get_size(); ++i)
                data[i] = vec[i];
        }
        
        std::vector<T>& get_vector(){
            return data;
        }
        
        uint32_t get_size() const{
            return data.size(); 
        }
        
        T* get_data(){
            return data.data();
        }
        
        T& operator[](uint32_t index) {
            return data[index];
        }

        const T& operator[](uint32_t index) const {
            return data[index];
        }

        // vector operations
        template <typename U>
		Vector<T> operator+(const Vector<U>& vec) const{
            uint32_t size = data.size();
            if(size != vec.get_size())
                throw std::runtime_error("Vector addition: Dimensions of vectors dont match");

            Vector<T> res(vec);
            for(int i=0; i<size; i++){
                res[i] += data[i];
            }
            return res;
        }
        
        template <typename U>
		T operator*(const Vector<U>& vec) const{
            uint32_t size = data.size();
            if(size != vec.get_size())
                throw std::runtime_error("Vector dot product: Dimensions of vectors dont match");

            T sum = 0;
            for(int i=0; i<size; i++){
                sum += data[i] * vec[i];
            }
            return sum;
        }

        template <typename U>
		Vector<T>& operator+=(const Vector<U>& vec){
            uint32_t size = data.size();
            if(size != vec.get_size())
                throw std::runtime_error("Vector addition: Dimensions of vectors dont match");

            for(int i=0; i<size; i++){
                data[i] += vec[i];
            }
            return *this;
        }
        
        template <typename U>
		Vector<T>& operator-=(const Vector<U>& vec){
            uint32_t size = data.size();
            if(size != vec.get_size())
                throw std::runtime_error("Vector subtraction: Dimensions of vectors dont match");

            for(int i=0; i<size; i++){
                data[i] -= vec[i];
            }
            return *this;
        }

        // scalar operations
		Vector<T>& operator+=(const T& scalar){
            uint32_t size = data.size();
            for(int i=0; i<size; i++){
                data[i] += scalar;
            }
            return *this;

        }

		Vector<T>& operator*=(const T& scalar){
            uint32_t size = data.size();
            for(int i=0; i<size; i++){
                data[i] *= scalar;
            }
            return *this;
        }

		Vector<T>& operator/=(const T& scalar){
            uint32_t size = data.size();
            for(int i=0; i<size; i++){
                data[i] /= scalar;
            }
            return *this;
        }

};


template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& vec) {
    os << "[";
    for (int i=0; i<vec.get_size(); ++i) {
        if(std::is_same<T,uint8_t>::value)
            os << static_cast<int>(vec[i]);
        else
            os << vec[i];
        if (i != vec.get_size() - 1) os << ", ";
    }
    os << "]";
    return os;
} 
