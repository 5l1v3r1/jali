/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
/**
 * @file   PointRegion.hh
 * @author Rao Garimella
 * @date
 *
 * @brief  Declaration of PointRegion class
 *
 *
 */

#ifndef _PointRegion_hh_
#define _PointRegion_hh_

#include <string>

#include "Region.hh"


namespace JaliGeometry {

// -------------------------------------------------------------
//  class PointRegion
// -------------------------------------------------------------
/// A point in space

class PointRegion : public Region {
public:

  PointRegion(const std::string name, const unsigned int id, const Point& p,
              const LifeCycle_type lifecycle = LifeCycle_type::PERMANENT);
  PointRegion(const char *name, const unsigned int id, const Point& p,
              const LifeCycle_type lifecycle = LifeCycle_type::PERMANENT);

  /// Protected copy constructor to avoid unwanted copies.
  PointRegion(const PointRegion& old);

  /// Destructor
  ~PointRegion(void);

  // Type of the region
  inline Region_type type() const { return Region_type::POINT; }

  /// Get the point defining the region
  const Point& point(void) const { return p_; }

  /// Is the specified point inside this region - in this case it
  /// means coincident with the region point

  bool inside(const Point& p) const;

protected:

  const Point p_;              /* point */

};

/// A smart pointer to PointRegion instances
// typedef Teuchos::RCP<PointRegion> PointRegionPtr;

// RVG: I am not able to correctly code a region factory using smart
// pointers so I will revert to a simpler definition

typedef PointRegion *PointRegionPtr;

} // namespace JaliGeometry


#endif
