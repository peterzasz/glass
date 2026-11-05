#ifndef STATISTICS_H
#define STATISTICS_H

#include "data.h"
#include "string"

void create_statistics_file(const Solution& _s, const std::string& path, const std::string& dataset, double seconds);

void plate_number_lower_bound(const Batch& batch, int bin_area, const std::string& path, const std::string& dataset);

#endif