#ifndef PARSER_H
#define PARSER_H

#include "data.h"
#include <vector>
#include <string>

Batch read_batch(std::string path);
std::vector<Bin> read_bins(std::string path);
void write_solution(const Solution& s, const std::string& path);

#endif