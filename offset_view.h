#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>

template<typename Range>
struct offset_iterator :
  public boost::iterator_facade<
  offset_iterator<Range>,
  typename Range::value_type,
  boost::forward_traversal_tag,
  boost::optional<typename Range::iterator::reference> >
{
  typedef typename Range::coordinate_type coordinate_type;
  offset_iterator(Range& range, coordinate_type offset) 
    : m_rows(range.size1()), m_cols(range.size2()), m_iterator_end(range.end()), m_offset(offset)

  {
    m_countdown = -offset.row * m_cols - offset.col;
    if (m_countdown <= 0)
    {
      m_do_increment = true;
      m_iterator = range.begin() - m_countdown;
    }
    else {
      m_do_increment = false;
      m_iterator = range.begin();
    }
  }

  void increment()
  {
    if (++m_coordinates.col == m_cols)
    {
      m_coordinates.col = 0;
      ++m_coordinates.row;
    }
    if (m_do_increment) {
      if (++m_iterator == m_iterator_end)
      {
        m_do_increment = false;
      }
    }
    else if (m_countdown)
    {
      if (--m_countdown == 0){
        m_do_increment = true;
      }
    }
  }
  bool equal(const offset_iterator& that)
  {
    return m_coordinates == that.m_coordinates;
  }

  bool on_map() const
  {
    coordinate_type off = m_coordinates + m_offset;
    return off.row >= 0 && off.col >= 0 && off.row < m_rows && off.col < m_cols;
  }

  boost::optional<typename Range::iterator::reference> dereference() const
  {
    if (on_map()) return *m_iterator;
    return boost::none;
  }

  typename Range::iterator m_iterator;
  typename Range::iterator m_iterator_end;
  bool m_do_increment;
  int m_countdown;
  coordinate_type m_coordinates;
  coordinate_type m_offset;
  int m_rows;
  int m_cols;
};


template<typename Range>
struct offset_view
{
  typedef typename Range::coordinate_type coordinate_type;

  offset_view(Range& range, coordinate_type offset) : m_range(range), m_offset(offset)
  {}

  typedef typename offset_iterator<Range> iterator;

  iterator begin()
  {
    return iterator(m_range, m_offset);
  }

  iterator end()
  {
    iterator iter(m_range, m_offset);
    iter.m_coordinates = coordinate_type(m_range.size1(), 0);
    return iter;
  }
  Range& m_range;
  coordinate_type m_offset;
};

