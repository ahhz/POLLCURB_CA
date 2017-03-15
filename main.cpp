#include "ca_settings.h"
#include "ca_raster_data.h"
#include "offset_view.h"
#include "neighbourhood_effect.h"
#include "random_effect.h"
#include "total_potential.h"
#include "allocate.h"
#include <blink/iterator/zip_range.h>

void take_step(ca_settings& settings, ca_raster_data& raster_data, int step)
{
  if (step == 0){
    calculate_neighbourhood_effect(raster_data.m_initial_landuse
      , raster_data.m_neighbourhood_effect[0], settings);

    auto zip = blink::iterator::make_zip_range
      ( std::ref(raster_data.m_land_use[0])
      , std::ref(raster_data.m_initial_landuse));

    for (auto& i : zip) {
      std::get<0>(i) = std::get<1>(i);
    }
  }
  else {
    calculate_neighbourhood_effect(
      raster_data.m_land_use[step]
      , raster_data.m_land_use[step - 1]
      , raster_data.m_neighbourhood_effect[step]
      , raster_data.m_neighbourhood_effect[step - 1]
      , settings);
  }
  
  int actives = settings.m_num_functions + settings.m_num_vacants;
  for (int i = 0; i < actives; ++i) {
    random_effect(raster_data.m_random_effect[step][i], settings);
    total_potential(raster_data.m_total_potential[step][i]
      , raster_data.m_random_effect[step][i]
      , raster_data.m_neighbourhood_effect[step][i]
      , raster_data.m_potential_sum_components[i]
      , raster_data.m_potential_product_components[i]
      , settings.m_weight_sum, settings.m_weight_product);
  }
  allocate(settings, raster_data, step);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cout << "The program expects a single command line argument"
      << ", for instance:" << std::endl << ">> cellular_automata demo.txt"
      << std::endl;
    return 0;
  }
  ca_settings settings;
  if (settings.read(argv[1])) {
    ca_raster_data raster_data(settings);
    raster_data.read_inputs();
    raster_data.create_outputs();
    int steps = settings.m_num_steps;
    //steps = 2; // only take two steps -> debugging
    for (int step = 0; step < steps; ++step) {
      std::cout << "Step " << step << std::endl;
      take_step(settings, raster_data, step);
      //break;
    }
  }
}
