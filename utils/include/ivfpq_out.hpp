#pragma once

#include <string>
#include <unordered_map>

#include "ivfpq.hpp"

template <typename T>
void ivfpq_out(std::unordered_map<std::string,void*>& value);


#include "../modules/ivfpq_out.cpp"
