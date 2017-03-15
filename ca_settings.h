#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/filesystem.hpp>

enum lu_type
{
  function,
  vacant,
  feature, 
  nodata
};


struct ca_settings
{
  ca_settings() : nodata_string("Nodata")
  {}
 
  bool read_potential_component(std::istream& is, int active, boost::filesystem::path& filename, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    std::getline(ss, c, '\t');
    if (active != std::stoi(a) || b != get_landuse_name(active))
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: " << active << '\t' << get_landuse_name(active) << '\t' << "<filename>" << std::endl;
      return false;
    }
    filename = c;
    return true;
  }
  bool read_rule_header(std::istream& is, int lu, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a,'\t');
    std::getline(ss, b, '\t');
    std::getline(ss, c, '\t');
    if (a != "Of" || std::stoi(b) != lu || c != get_landuse_name(lu))
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: Of " << '\t' << lu << '\t' << get_landuse_name(lu) << std::endl;
      return false;
    }
    return true;
  }
  bool read_rule(std::istream& is, int lu, int active, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    std::getline(ss, c, '\t');
    if (a != "On" || std::stoi(b) != active || c != get_landuse_name(active))
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: On " << '\t' << active << '\t' << get_landuse_name(active) << std::endl;
      return false;
    }
    for (int i = 0; i < m_num_rings; ++i) {
      std::string str;
      std::getline(ss, str,'\t');
      m_rules[lu - 1][active - 1][i] = std::stoi(str);
    }
    return true;
  }

  bool read_scenario_header(std::istream& is, int index, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    std::getline(ss, c, '\t');

    if (a != "Region" || std::stoi(b) != index || c != get_region_name(index))
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: Region " << '\t' << index << '\t' << get_region_name(index) << std::endl;
      for (int i = 1; i <= m_num_steps; ++i)
      {
        std::cout << '\t' << m_step_names[i];
      }
      std::cout << std::endl;
      return false;
    }

    for (int i = 1; i <= m_num_steps; ++i)
    {
      std::string stepname;
      std::getline(ss, stepname,'\t');
      if (stepname != m_step_names[i]) {
        std::cout << "Reading failed in line " << line_number
          << " wrong step name for step " << i << std::endl;
        return false;
      }
    }


    return true;
  }

  bool read_scenario(std::istream& is, int region, int function, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);
    std::string a, b, c;
    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    std::getline(ss, c, '\t');

    if (a != "Function" || std::stoi(b) != function || c != get_function_name(function))
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: Function " << '\t' << function << '\t' << get_function_name(function) 
        << "<scenario values>" << std::endl;;
       return false;
    }

    for (int i = 1; i <= m_num_steps; ++i)
    {
      std::string stepstr;
      std::getline(ss, stepstr,'\t');
      int step_value = std::stoi(stepstr);
      m_scenarios[region - 1][i - 1][function - 1] = step_value;
    }
    return true;
  }


  bool read_legend_value(std::istream& is, int index, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);

    std::getline(ss, line, '\t');
    int read_index = std::stoi(line);

    std::getline(ss, line, '\t');
    std::string read_type = line;
    std::pair<lu_type, int> li = get_type_and_index(index);
    std::string expected_type;
    switch (li.first)
    {
    case function: expected_type = "Function"; break;
    case vacant:   expected_type = "Vacant"  ; break;
    case feature:  expected_type = "Feature" ; break;
    default:       expected_type = "Nodata"  ; break;
    }

    if (read_index != index || read_type != expected_type) {
      std::cout << "Reading failed in line " << line_number 
        << ", expected label: " << index <<'\t' << expected_type << std::endl;
      return false;
    }

    std::getline(ss, line, '\t');
    std::string read_legend = line;
    get_landuse_name(index) = read_legend;
    return true;
  }

  bool read_filename(std::istream& is, const std::string& label, boost::filesystem::path& path, int line_number)
  {
    std::string line, a, b;
    std::getline(is, line);
    std::stringstream ss(line);
    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    if (a != label)
    {
      std::cout << "Reading failed in line " << line_number
        << ", expected: " << label << '\t' << "<filename>" << std::endl;
      return false;
    }
    path = b;
    return true;
  }

  bool read_region_name(std::istream& is, int index, int line_number)
  {
    std::string line;
    std::getline(is, line);
    std::stringstream ss(line);

    std::getline(ss, line, '\t');
    int read_index = std::stoi(line);
    if (read_index != index) {
      std::cout << "Reading failed in line " << line_number
        << ", expected: " << index << '\t' << "<region name>" << std::endl;
      return false;
    }
    std::getline(ss, line, '\t');
    std::string read_name = line;
    get_region_name(index) = read_name;
    return true;
  }

  bool read_step_name(std::istream& is, int step, int line_number)
  {
    std::string line, a, b;
    std::getline(is, line);
    std::stringstream ss(line);

    std::getline(ss, a, '\t');
    std::getline(ss, b, '\t');
    
    int read_step = std::stoi(a);
    if (step != read_step) {
      std::cout << "Reading failed in line " << line_number
        << ", expected: " << step << '\t' << "<step name>" << std::endl;
      return false;
    }
    m_step_names[step] = b;
    return true;
  }


  template<typename T>
  bool read_labelled_value(std::istream& is, const std::string& label, T& target, int line_number)
  {
    std::string line, read_label;
    std::getline(is, line);
    std::stringstream ss(line);

    T value;
    std::getline(ss, read_label, '\t');
    ss  >> value;
    if (read_label != label)
    {
      std::cout << "Reading failed in line " << line_number << " expected label: " << label << std::endl;
      return false;
    }
    else {
      target = value;
      return true;
    }

  }

  bool read_label(std::istream& is, const std::string& label, int line_number)
  {
    std::string line, read_label;
    std::getline(is, line);
    std::stringstream ss(line);
    std::getline(ss, read_label, '\t');
    if (read_label != label)
    {
      std::cout << "Reading failed in line " << line_number << " expected label: " << label << std::endl;
      return false;
    }
    else {
      return true;
    }
  }

  bool read(const boost::filesystem::path& path)
  {
    int line_number = 0;
    std::ifstream ifs(path.c_str());
  
    if (!ifs.good()) {
      std::cout << "Could not open file: " << path << std::endl;
      return false;
    }
    
    if (!read_label(ifs, "CAModelFile", ++line_number)) return false;


    int version;
    if (!read_labelled_value(ifs, "Version", version, ++line_number)) return false;
    if (version != 1) {
      std::cout << "Reading Failed: expected version = 1" << std::endl;
      return false;
    }
    if (!read_labelled_value(ifs, "NFunctions", m_num_functions, ++line_number)) return false;
    if (!read_labelled_value(ifs, "NVacants", m_num_vacants, ++line_number)) return false;
    if (!read_labelled_value(ifs, "NFeatures", m_num_features, ++line_number)) return false;
    if (!read_labelled_value(ifs, "NRegions", m_num_regions, ++line_number)) return false;
    if (!read_labelled_value(ifs, "NSteps", m_num_steps, ++line_number)) return false;

    resize_all(m_num_functions, m_num_vacants, m_num_features, m_num_rings, m_num_regions, m_num_steps);


    if (!read_label(ifs, "Legend", ++line_number)) return false;
    for (int i = 0; i < m_num_functions + m_num_vacants + m_num_features; ++i) {
      if (!read_legend_value(ifs, i + 1, ++line_number)) return false;
    }
    if (!read_label(ifs, "Regions", ++line_number)) return false;
    for (int i = 0; i < m_num_regions; ++i) {
      if (!read_region_name(ifs, i + 1, ++line_number)) return false;
    }
    if (!read_label(ifs, "Steps", ++line_number)) return false;
    for (int step = 0; step <= m_num_steps; ++step)
    {
      // step = 0 is initial value
      if (!read_step_name(ifs, step, ++line_number)) return false;

    }
    if (!read_label(ifs, "Scenarios", ++line_number)) return false;
    for (int region = 0; region < m_num_regions; ++region)
    {
      if (!read_scenario_header(ifs, region + 1, ++line_number)) return false;
      for (int function = 0; function < m_num_functions; ++function) {
        if (!read_scenario(ifs, region + 1,  function + 1, ++line_number)) return false;

      }
    }
    if (!read_label(ifs, "Rules", ++line_number)) return false;
    std::string dummy;
    ++line_number; std::getline(ifs, dummy);
    for (int lu = 0; lu < m_num_functions + m_num_vacants + m_num_features; ++lu) {
      if (!read_rule_header(ifs, lu + 1, ++line_number)) return false;
      for (int active = 0; active < m_num_functions + m_num_vacants; ++active) {
        if (!read_rule(ifs, lu + 1, active + 1, ++line_number)) return false;
      }
    }
    if (!read_label(ifs, "Input Maps", ++line_number)) return false;
    if (!read_filename(ifs, "Initial", m_initial_landuse, ++line_number)) return false;
    if (!read_filename(ifs, "Regions", m_regions, ++line_number)) return false;
    if (!read_label(ifs, "Feature maps", ++line_number)) return false;
    for (int i = 1; i <= m_num_steps; ++i)
    {
      if (!read_filename(ifs, m_step_names[i], m_feature_maps[i-1], ++line_number)) return false;

    }
    if (!read_label(ifs, "Potential Sum", ++line_number)) return false;
    for (int i = 0; i < m_num_functions + m_num_vacants; ++i)
    {
      if (!read_potential_component(ifs, i + 1, m_potential_sum_components[i], ++line_number)) return false;
    }

    if (!read_label(ifs, "Potential Product", ++line_number)) return false;
    for (int i = 0; i < m_num_functions + m_num_vacants; ++i)
    {
      if (!read_potential_component(ifs, i + 1, m_potential_product_components[i], ++line_number)) return false;
    }
    if (!read_label(ifs, "Parameters", ++line_number)) return false;
    if (!read_labelled_value(ifs, "alpha", m_alpha, ++line_number)) return false;
    if (!read_labelled_value(ifs, "wsum", m_weight_sum, ++line_number)) return false;
    if (!read_labelled_value(ifs, "wprod", m_weight_product, ++line_number)) return false;
    return true;

  }

  void resize_all(int functions, int vacants, int features, int rings, int regions, int steps)
  {
    m_function_names.resize(functions);
    m_vacant_names.resize(vacants);
    m_feature_names.resize(features);
    m_region_names.resize(regions); 
    m_step_names.resize(steps + 1); // because 0 is initial time
    resize_rules(functions, vacants, features, rings);
    resize_scenarios(regions, functions, steps);
    resize_potential_maps(functions, vacants);
    resize_feature_maps(steps);
  }

  void resize_feature_maps(int steps)
  {
    m_feature_maps.resize(steps);
  }

  void resize_rules(int functions, int vacants, int features, int rings)
  {
    int landuses = functions + vacants + features;
    int actives = functions + vacants;
    m_rules.assign(landuses, std::vector<std::vector<double> >(actives, 
      std::vector<double>(rings, 0.0)));
  }

  void resize_scenarios(int regions, int functions, int steps)
  {
    m_scenarios.assign(regions, std::vector<std::vector<int>>(steps, std::vector<int>(functions, 0)));
  }

  void resize_potential_maps(int functions, int vacants)
  {
    m_potential_sum_components.resize(functions + vacants);
    m_potential_product_components.resize(functions + vacants);
  }

  std::string& get_function_name(int i)
  {
    return m_function_names[i - 1];
  }
  
  std::string& get_vacant_name(int i)
  {
    return m_function_names[i - m_num_functions - 1];
  }
  
  std::string& get_feature_name(int i)
  {
    return m_function_names[i - m_num_functions - m_num_vacants - 1];
  }

  std::string& get_region_name(int i)
  {
    if (i == 0) {
      return nodata_string;
    }
    else {
      return m_region_names[i-1];
    }
  }

  std::string& get_landuse_name(int i)
  {
    std::pair<lu_type, int> ti = get_type_and_index(i);
    switch (ti.first)
    {
    case nodata: return nodata_string;
    case function: return m_function_names[ti.second];
    case vacant: return m_vacant_names[ti.second];
    case feature: return m_feature_names[ti.second];
    default: return nodata_string;
    }
  }
  
  std::pair<lu_type, int> get_type_and_index(int i)
  {
    if (i == 0) {
      return std::make_pair(nodata, 0);
    }
    else if (i <= m_num_functions) {
      return std::make_pair(function, i-1);
    }
    else if (i <= m_num_functions + m_num_vacants) {
      return std::make_pair(vacant, i - 1 - m_num_functions);
    }
    else {
      return std::make_pair(feature, i - 1 - m_num_functions - m_num_vacants);
    }
  }

  std::vector<std::string> m_function_names;
  std::vector<std::string> m_vacant_names;
  std::vector<std::string> m_feature_names;
  std::vector<std::string> m_region_names;
  std::vector<std::string> m_step_names;
  std::vector<std::vector<std::vector<double> > > m_rules;
  std::vector<std::vector<std::vector< int  > > > m_scenarios;
  
  std::vector<boost::filesystem::path> m_potential_sum_components;
  std::vector<boost::filesystem::path> m_potential_product_components;
  std::vector<boost::filesystem::path> m_feature_maps;
  boost::filesystem::path m_initial_landuse;
  boost::filesystem::path m_regions;
  int m_num_functions;
  int m_num_vacants;
  int m_num_features;
  int m_num_steps;
  int m_num_regions;
  static const int m_num_rings = 30;
  static const int m_max_radius_sq = 64;
  double m_alpha;
  double m_weight_sum;
  double m_weight_product;
  std::string nodata_string;
 };