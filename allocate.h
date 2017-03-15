#pragma once

#include "ca_raster_data.h"
#include "ca_settings.h"
#include "coordinate_view.h"

#include <blink/raster/coordinate_2d.h>
#include <blink/iterator/zip_range.h>
#include <blink/iterator/range_range.h>

#include <queue>

struct allocator
{
  // create a vector of heaps, one for each function
  // create a heap of heaps
  struct inner_heap_elem
  {
    inner_heap_elem(double potential, blink::raster::coordinate_2d coordinate, 
      int function)
      : m_potential(potential)
      , m_coordinate(coordinate)
      , m_function(function)
    { }

    friend bool operator < (inner_heap_elem const& lhs, 
      inner_heap_elem const& rhs) 
    {
      return lhs.m_potential < rhs.m_potential;
    }
    friend bool operator > (inner_heap_elem const& lhs,
      inner_heap_elem const& rhs)
    {
      return lhs.m_potential > rhs.m_potential;
    }

    double m_potential;
    blink::raster::coordinate_2d m_coordinate;
    int m_function;
  };

  using inner_heap = std::priority_queue<inner_heap_elem>;

  struct outer_heap_elem
  {
    outer_heap_elem(inner_heap* inner_heap) : m_inner_heap(inner_heap)
    {}

    int function() const
    {
      return m_inner_heap->top().m_function;
    }

    blink::raster::coordinate_2d coordinates() const
    {
      return m_inner_heap->top().m_coordinate;
    }
    
    friend bool operator < (outer_heap_elem const& lhs,
      outer_heap_elem const& rhs)
    {
      return lhs.m_inner_heap->top() < rhs.m_inner_heap->top();
    }
    friend bool operator >(outer_heap_elem const& lhs,
      outer_heap_elem const& rhs)
    {
      return lhs.m_inner_heap->top() > rhs.m_inner_heap->top();
    }
    inner_heap* m_inner_heap;
  };

  using outer_heap = std::priority_queue<outer_heap_elem>;
  
  void initialize(ca_settings& settings, ca_raster_data& raster_data
    , int step, int region)
  {
    m_function_heaps.clear();
    m_function_heaps.resize(settings.m_num_functions);
    m_lu_target = &raster_data.m_land_use[step + 1];
    m_yet_to_allocate = settings.m_scenarios[region][step];
  }

  void insert(int function, double potential, blink::raster::coordinate_2d coordinates)
  {
    m_function_heaps[function].push(inner_heap_elem(potential, coordinates
      , function));
  }

  void allocate_functions()
  {
    outer_heap heap;
    for (std::size_t i = 0; i < m_function_heaps.size(); ++i)
    {
      heap.push(outer_heap_elem(&m_function_heaps[i]));
    }

    while (!heap.empty())
    {
      int function = heap.top().function();
      if (m_yet_to_allocate[function] == 0) {
        heap.pop();
      }
      else {
        blink::raster::coordinate_2d coord = heap.top().coordinates();
        if (m_lu_target->get(coord) != 0 ) {
          // already_assigned;
          inner_heap* inner = heap.top().m_inner_heap;
          heap.pop();
          inner->pop();
          if (!inner->empty())
          {
            heap.push(outer_heap_elem(inner));
          }
          else {
            std::cout << "Cannot meet constraints, check scenarios" << std::endl;
          }
        }
        else {
          // to_be_assigned;
          inner_heap* inner = heap.top().m_inner_heap;
          m_lu_target->put(coord, function + 1);
          --m_yet_to_allocate[function];
          heap.pop();
          inner->pop();
          if (!inner->empty())
          {
            heap.push(outer_heap_elem(inner));
          } else if (m_yet_to_allocate[function] > 0)
          {
            std::cout << "Cannot meet constraints, check scenarios" << std::endl;

          }
        }
      }
    }
  }
  blink::raster::gdal_raster<int>* m_lu_target;
  std::vector<inner_heap> m_function_heaps;
  std::vector<int> m_yet_to_allocate;
};

void allocate(ca_settings& settings, ca_raster_data& raster_data, int step)
{
  // first allocate region 0 and features and set rest to -1 
  
  std::vector<allocator> allocators(settings.m_num_regions);
  for (int i = 0; i < settings.m_num_regions; ++i)
  {
    allocators[i].initialize(settings, raster_data, step, i);
  }
  
  auto potential_range = blink::iterator::make_range_zip_range(
    std::ref(raster_data.m_total_potential[step]));
  
  coordinate_view coordinates(raster_data.m_regions.size1(),
    raster_data.m_regions.size2());

  auto zip = blink::iterator::make_zip_range(
    std::ref(raster_data.m_regions)                  // 0
    , std::ref(raster_data.m_initial_landuse)        // 1 
    , std::ref(raster_data.m_feature_data[step])     // 2
    , std::ref(raster_data.m_land_use[step + 1])     // 3
    , std::ref(potential_range)                      // 4
    , coordinates                                    // 5
    );
    
  int num_functions = settings.m_num_functions;
  int num_actives = settings.m_num_functions + settings.m_num_vacants;
  int first_feature = settings.m_num_functions + settings.m_num_vacants + 1;
  int unspecified = 0;
  for (auto&& i : zip) {

    if (std::get<0>(i) == 0) {
      std::get<3>(i) = std::get<1>(i);
      if (std::get<3>(i) == 15) std::cout << "place 3" << std::endl;

    }
    else if (std::get<2>(i) >= first_feature)  {
      std::get<3>(i) = std::get<2>(i);
      if (std::get<3>(i) == 15) 
        std::cout << "place 4 " 
            << raster_data.m_feature_data[step].get_gdal_dataset()->GetDescription() 
            << std::endl;

    }
    else {
      std::get<3>(i) = unspecified;

      // add to allocators
      blink::raster::coordinate_2d coord = std::get<5>(i);
      int region = std::get<0>(i) - 1;
      auto potentials = std::get<4>(i);
      for (int j = 0; j < num_functions; ++j) {
        allocators[region].insert(j, potentials[j], coord);
      }
    }
  }
  for (int i = 0; i < settings.m_num_regions; ++i)
  {
    allocators[i].allocate_functions();
  }

  // Finally allocate vacants
  int begin_vacant = settings.m_num_functions;
  int end_vacant = settings.m_num_functions + settings.m_num_vacants;

  auto zip2 = make_zip_range(
    std::ref(raster_data.m_land_use[step + 1]) // 0;
    , std::ref(potential_range) );               // 1;

  for (auto& i : zip2)
  {
    if (std::get<0>(i) == 15) std::cout << "place 1" << std::endl;
    int lu = std::get<0>(i);
    if (lu == unspecified) {
      double max_potential = -1e100;
      int max_vacant = unspecified;
      auto potentials = std::get<1>(i);
      for (int j = begin_vacant; j < end_vacant; ++j) {
        double pot = potentials[j];
        if (max_potential < pot) {
          max_potential = pot;
          max_vacant = j;
        }
      }
      std::get<0>(i) = max_vacant + 1;
    }
    if (std::get<0>(i) == 15) std::cout << "place 2" << std::endl;
  }
}