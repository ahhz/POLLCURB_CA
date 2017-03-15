#pragma once
#include "ca_settings.h"
#include <blink/raster/gdal_raster.h>

#include <algorithm>
#include <cmath>
#include <random>

template<typename RealType>
void random_effect(blink::raster::gdal_raster<RealType>& target, ca_settings& settings)
{
  static std::default_random_engine generator; //static so we always use the same
  
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  for (auto& i : target) {
    double rand = distribution(generator);
    rand = std::max(rand, 0.000001); // to avoid out-of-range values
    i = 1 + std::pow(-std::log(rand), settings.m_alpha);
  }
}