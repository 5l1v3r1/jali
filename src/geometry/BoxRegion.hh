/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
/**
 * @file   BoxRegion.hh
 * @author Rao Garimella, William A. Perkins
 * @date Wed Sep 28 08:54:19 2011
 * 
 * @brief  Declaration of BoxRegion class (adapted from RectangularRegion)
 * 
 * 
 */

#ifndef _BoxRegion_hh_
#define _BoxRegion_hh_

#include "Region.hh"
#include "dbc.hh"
#include "errors.hh"

namespace JaliGeometry {

// -------------------------------------------------------------
//  class BoxRegion
// -------------------------------------------------------------
/// A rectangular region in space, defined by two points

class BoxRegion : public Region {
public:

  /// Default constructor uses two corner points (order not important).

  BoxRegion(const std::string name, const unsigned int id, const Point& p0, 
            const Point& p1, const LifeCycleType lifecycle=PERMANENT);
  BoxRegion(const char *name, const unsigned int id, const Point& p0, 
            const Point& p1, const LifeCycleType lifecycle=PERMANENT);

  /// Protected copy constructor to avoid unwanted copies.
  BoxRegion(const BoxRegion& old);

  /// Destructor
  ~BoxRegion(void);


  // Type of the region
  inline
  RegionType type() const { return BOX; }

  /// Get the first point defining the region
  inline
  const Point& point0(void) const { return p0_; }

  /// Get the second point defining the region
  inline
  const Point& point1(void) const { return p1_; }

  /// Is the the specified point inside this region
  bool inside(const Point& p) const;

  /// corners
  inline
  void corners(Point *lo_corner, Point *hi_corner) const
  {
    if (lo_corner == NULL) {
      Errors::Message mesg("lo_corner not specified");
      Exceptions::Jali_throw(mesg);
    }
    if (hi_corner == NULL) {
      Errors::Message mesg("hi_corner not specified");
      Exceptions::Jali_throw(mesg);
    }
    //    lo_corner->init(p0_.dim());
    lo_corner->set(p0_);
    //    hi_corner->init(p1_.dim());
    hi_corner->set(p1_);
  }

  // Is the box degenerate - zero length in one or more directions and
  // if so in how many directions?
  bool is_degenerate(int *ndeg) const;

protected:
  
  const Point p0_;              /**< one corner of the region  */
  const Point p1_;              /**< the other corner of the region */

  /// Is the specified value between the two values (inclusive, order not important)
  static bool between_(const double& x, const double& x0, const double& x1);

};

/// A smart pointer to BoxRegion instances
//
// typedef Teuchos::RCP<BoxRegion> BoxRegionPtr;

// RVG: I am not able to correctly code a region factory using smart
// pointers so I will revert to a simpler definition

typedef BoxRegion *BoxRegionPtr;

} // namespace JaliGeometry


#endif
