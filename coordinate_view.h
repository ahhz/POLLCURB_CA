#pragma once
#include <blink/raster/coordinate_2d.h>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

class coordinate_iterator
  : public boost::iterator_facade<
  coordinate_iterator,
  blink::raster::coordinate_2d,
  boost::random_access_traversal_tag,
  const blink::raster::coordinate_2d&>
{
public:
  coordinate_iterator(int rows = 0, int cols = 0) : m_rows(rows), m_cols(cols)
  {}

  void find(const blink::raster::coordinate_2d& coord)
  {
    m_coordinates = coord;
  }

  std::ptrdiff_t get_advance(const blink::raster::coordinate_2d& coord) const
  {
    return (coord.row - m_coordinates.row) * static_cast<std::ptrdiff_t>
      (size2()) + coord.col - m_coordinates.col;
  }

  void find_begin()
  {
    m_coordinates.row = 0;
    m_coordinates.col = 0;
  }

  void find_end()
  {
    m_coordinates.row = size1();
    m_coordinates.col = 0;
  }


private:

  friend class boost::iterator_core_access;

  void decrement()
  {
    if (--m_coordinates.col < 0) {
      m_coordinates.col = size2() - 1;
      --m_coordinates.row;
    } 
  }

  void increment()
  {
    if (++m_coordinates.col == size2()) {
      m_coordinates.col = 0;
      ++m_coordinates.row;
    }
  }

  void advance(std::ptrdiff_t n)
  {
    std::ptrdiff_t new_index = index() + n;
    m_coordinates.row = new_index / size2();
    m_coordinates.col = new_index % size2();
  }

  bool equal(const coordinate_iterator& other) const
  {
    return m_coordinates == other.m_coordinates;
  }

  const blink::raster::coordinate_2d& dereference() const
  {
    return m_coordinates;
  }

   std::ptrdiff_t distance_to(const coordinate_iterator& other) const
  {
    return other.index() - index();
  }

  std::ptrdiff_t index() const
  {
    return m_coordinates.row * size2() + m_coordinates.col;
  }

  int size1() const
  {
    return m_rows;
  }

  int size2() const
  {
    return m_cols;
  }

  blink::raster::coordinate_2d m_coordinates;
  int m_rows;
  int m_cols;

};

struct coordinate_view
{
  using iterator = coordinate_iterator;

  coordinate_view(int rows, int cols) : m_rows(rows), m_cols(cols)
  {


  }

  iterator begin() const
  {
    iterator i(m_rows, m_cols);
    i.find_begin();
    return i;
  }

  iterator end() const
  {
    iterator i(m_rows, m_cols);
    i.find_end();
    return i;
  }

  int m_rows;
  int m_cols;
};
