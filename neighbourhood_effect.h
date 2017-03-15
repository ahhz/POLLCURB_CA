#pragma once
#include <blink/raster/coordinate_2d.h>

template<typename LU, typename NbhRaster>
void calculate_neighbourhood_effect(
  LU& landuse, std::vector<NbhRaster>& nbh, 
  const ca_settings& settings)
{
  int rows = static_cast<int>(landuse.size1());
  int cols = static_cast<int>(landuse.size2());
  std::vector<std::pair<blink::raster::coordinate_2d, int>> offsets;
  std::vector<int> offset_lookup(65,-1);
  for (int i = 0; i <= 8; ++i) {
    for (int j = i; j <= 8; ++j) {
      int dsq = i*i + j*j;
      if (dsq <= 64) offset_lookup[dsq] = 1;
    }
  }
  int count = 0;
  for (int i = 0, index = 0; i <= 64; ++i) {
    if (offset_lookup[i] != -1)  offset_lookup[i] = count++;
  }
  for (int i = -8; i <= 8; ++i) {
    for (int j = -8; j <= 8; ++j) {
      int dsq = i*i + j*j;
      if (dsq <= 64) {
        int ring = offset_lookup[dsq];
        offsets.push_back(std::make_pair(blink::raster::coordinate_2d(i, j), ring));
      }
    }
  }
  for (int row = 0; row < rows; ++row){
    for (int col = 0; col < cols; ++col) {
      for (std::size_t to = 0; to < nbh.size(); ++to) {
        nbh[to].put(blink::raster::coordinate_2d(row, col), 0);
      }
    }
  }

  for (int row = 0; row < rows; ++row){
    for (int col = 0; col < cols; ++col) {
      blink::raster::coordinate_2d cell(row, col);
      int from = landuse.get(cell)-1;
      // TODO: deal with nodata
      for (std::size_t off = 0; off < offsets.size(); ++off) {
        int ring = offsets[off].second;
        blink::raster::coordinate_2d nb = cell + offsets[off].first;

        if (nb.row >= 0 && nb.col >= 0 && nb.row < rows && nb.col < cols) {
          for (std::size_t to = 0; to < nbh.size(); ++to) {
            nbh[to].put(nb, nbh[to].get(nb) + settings.m_rules[from][to][ring]);
          }
        }
      }
    }
  }
}

template<typename LU, typename NbhRaster>
void calculate_neighbourhood_effect(
  LU& landuse, LU& old_landuse,
  std::vector<NbhRaster>& nbh, 
  std::vector<NbhRaster>& old_nbh, 
  const ca_settings& settings)
{
  int rows = static_cast<int>(landuse.size1());
  int cols = static_cast<int>(landuse.size2());
  std::vector<std::pair<blink::raster::coordinate_2d, int>> offsets;
  std::vector<int> offset_lookup(65, -1);
  for (int i = 0; i <= 8; ++i) {
    for (int j = i; j <= 8; ++j) {
      int dsq = i*i + j*j;
      if (dsq <= 64) offset_lookup[dsq] = 1;
    }
  }
  int count = 0;
  for (int i = 0, index = 0; i <= 64; ++i) {
    if (offset_lookup[i] != -1)  offset_lookup[i] = count++;
  }
  for (int i = -8; i <= 8; ++i) {
    for (int j = -8; j <= 8; ++j) {
      int dsq = i*i + j*j;
      if (dsq <= 64) {
        int ring = offset_lookup[dsq];
        offsets.push_back(std::make_pair(blink::raster::coordinate_2d(i, j), ring));
      }
    }
  }
  for (int row = 0; row < rows; ++row){
    for (int col = 0; col < cols; ++col) {
      for (std::size_t to = 0; to < nbh.size(); ++to) {
        nbh[to].put(blink::raster::coordinate_2d(row, col)
          , old_nbh[to].get(blink::raster::coordinate_2d(row, col)));
      }
    }
  }

  for (int row = 0; row < rows; ++row){
    for (int col = 0; col < cols; ++col) {
      blink::raster::coordinate_2d cell(row, col);
      int from = landuse.get(cell)-1;
      int old_from = old_landuse.get(cell)-1;
      if (from != old_from) {
        for (std::size_t off = 0; off < offsets.size(); ++off) {
          int ring = offsets[off].second;
          blink::raster::coordinate_2d nb = cell + offsets[off].first;

          if (nb.row >= 0 && nb.col >= 0 && nb.row < rows && nb.col < cols) {
            for (std::size_t to = 0; to < nbh.size(); ++to) {
              nbh[to].put(nb, nbh[to].get(nb)
                + settings.m_rules[from][to][ring]
                - settings.m_rules[old_from][to][ring]);
            }
          }
        }
      }
    }
  }
}