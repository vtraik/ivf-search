#pragma once

#include <string>
#include <unordered_map>

#include "ivfflat.hpp"

template <typename T>
void ivfflat_out(std::unordered_map<std::string,void*>& value);


#include "../modules/ivfflat_out.cpp"
