#pragma once;
#include <cellular_automata/ca_settings.h>
#include <blink/raster/gdal_raster.h>
#include <blink/raster/utility.h>


struct ca_raster_data
{
  ca_raster_data(ca_settings& settings) : m_settings(settings)
  {

  }

  void read_inputs()
  {
    m_initial_landuse = blink::raster::open_gdal_raster<int>(m_settings.m_initial_landuse, GA_ReadOnly);
    m_regions = blink::raster::open_gdal_raster<int>(m_settings.m_regions, GA_ReadOnly);
   
    m_feature_data.clear();
    for (auto& file : m_settings.m_feature_maps) {
      m_feature_data.push_back(blink::raster::open_gdal_raster<int>(file, GA_ReadOnly));
    }

    m_potential_sum_components.clear();
    for (auto& file : m_settings.m_potential_sum_components) {
      m_potential_sum_components.push_back(blink::raster::open_gdal_raster<double>(file, GA_ReadOnly));
    }

    m_potential_product_components.clear();
    for (auto& file : m_settings.m_potential_product_components) {
      m_potential_product_components.push_back(blink::raster::open_gdal_raster<double>(file, GA_ReadOnly));
    }
  }

  void create_outputs()
  {
    std::string dir_name = "output";
    boost::filesystem::create_directory(dir_name);
    m_total_potential.clear();
    m_random_effect.clear();
    m_neighbourhood_effect.clear();
    m_total_potential.resize(m_settings.m_num_steps);
    m_random_effect.resize(m_settings.m_num_steps);
    m_neighbourhood_effect.resize(m_settings.m_num_steps);
    for (int i = 0; i < m_settings.m_num_steps; ++i) {

      m_total_potential[i].resize(m_settings.m_num_vacants + m_settings.m_num_functions);
      m_random_effect[i].resize(m_settings.m_num_vacants + m_settings.m_num_functions);
      m_neighbourhood_effect[i].resize(m_settings.m_num_vacants + m_settings.m_num_functions);

      std::string step_name = m_settings.m_step_names[i];

      for (int j = 0; j < m_settings.m_num_vacants + m_settings.m_num_functions; ++j) {
        std::string landuse_name = m_settings.get_landuse_name(j + 1);
        std::string common = landuse_name + "_" + step_name + ".tif";
        std::string tp_name = dir_name + "/" + "total_potential_" + common;
        std::string re_name = dir_name + "/" + "random_effect_" + common;
        std::string ne_name = dir_name + "/" + "nbh_effect_" + common;
        m_total_potential[i][j] = blink::raster::create_gdal_raster_from_model<double>(tp_name, m_initial_landuse);
        m_random_effect[i][j] = blink::raster::create_gdal_raster_from_model<double>(re_name, m_initial_landuse);
        m_neighbourhood_effect[i][j] = blink::raster::create_gdal_raster_from_model<double>(ne_name, m_initial_landuse);
      }
    }
    m_land_use.resize(m_settings.m_num_steps + 1);
    for (int i = 0; i < m_settings.m_num_steps + 1; ++i) {
      std::string step_name = m_settings.m_step_names[i];
      std::string name = dir_name + "/" + "LU" + "_" + step_name + ".tif";
      m_land_use[i] = blink::raster::create_gdal_raster_from_model<int>(name, m_initial_landuse);
    }

  }

  // inputs
  blink::raster::gdal_raster<int>  m_initial_landuse;
  blink::raster::gdal_raster<int>  m_regions;
  std::vector< blink::raster::gdal_raster<int> >  m_feature_data;
  std::vector< blink::raster::gdal_raster<double> > m_potential_sum_components;
  std::vector< blink::raster::gdal_raster<double> > m_potential_product_components;
  
  //outputs
  std::vector< std::vector< blink::raster::gdal_raster<double> > >  m_total_potential;
  std::vector< std::vector< blink::raster::gdal_raster<double> > >  m_random_effect;
  std::vector< std::vector< blink::raster::gdal_raster<double> > >  m_neighbourhood_effect;
  std::vector< blink::raster::gdal_raster<int> >  m_land_use;

   ca_settings& m_settings;
};