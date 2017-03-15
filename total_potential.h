#pragma once
#include "ca_settings.h"
#include <blink/raster/gdal_raster.h>
#include <blink/iterator/zip_range.h>

#include <cmath>

template<typename RealType>
void total_potential(blink::raster::gdal_raster<RealType>& target
  , blink::raster::gdal_raster<RealType>& random_effect
  , blink::raster::gdal_raster<RealType>& neighbourhood_effect
  , blink::raster::gdal_raster<RealType>& sum_component
  , blink::raster::gdal_raster<RealType>& product_component
  , double sum_weight
  , double product_weight)
{
  auto zip = blink::iterator::make_zip_range
    (std::ref(target)                //0
    , std::ref(random_effect)        //1 
    , std::ref(neighbourhood_effect) //2
    , std::ref(sum_component)        //3
    , std::ref(product_component));  //4 

  for (auto& i : zip)
  {
    std::get<0>(i) = std::get<1>(i) * std::get<2>(i)
      * std::pow(std::get<4>(i), product_weight) + sum_weight * std::get<3>(i);
  }
}