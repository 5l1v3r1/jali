/**
 * @file   LogicalRegion.cc
 * @author Rao Garimella
 * @date
 *
 * @brief  Implementation of Labeled Set Region class which derives its
 *         definition from named set of mesh entities in a mesh file
 *
 *
 */

#include "LogicalRegion.hh"
#include "dbc.hh"
#include "errors.hh"

namespace JaliGeometry {

// -------------------------------------------------------------
//  class LogicalRegion
// -------------------------------------------------------------

// -------------------------------------------------------------
// LogicalRegion:: constructors / destructor
// -------------------------------------------------------------
LogicalRegion::LogicalRegion(const std::string name,
                             const unsigned int id,
                             const std::string operation_str,
                             const std::vector<std::string> region_names,
                             const LifeCycleType lifecycle)
  : Region(name,id,3,lifecycle), operation_(NOBOOLEAN),
    region_names_(region_names)
{
  // Region dimension is set arbitrarily as 3 since the set of
  // entities in the mesh will determine the dimension

  if (operation_str == "Complement")
    operation_ = COMPLEMENT;
  else if (operation_str == "Union")
    operation_ = UNION;
  else if (operation_str == "Intersect")
    operation_ = INTERSECT;
  else if (operation_str == "Subtract")
    operation_ = SUBTRACT;
  else {
    Errors::Message mesg("Unknown logical operation type requested on regions");
    Exceptions::Jali_throw(mesg);
  }

}

LogicalRegion::LogicalRegion(const char *name,
                             const unsigned int id,
                             const std::string operation_str,
                             const std::vector<std::string> region_names,
                             const LifeCycleType lifecycle)
  : Region(name,id,3,lifecycle), operation_(NOBOOLEAN),
    region_names_(region_names)
{
  // Region dimension is set arbitrarily as 3 since the set of
  // entities in the mesh will determine the dimension


  if (operation_str == "Complement")
    operation_ = COMPLEMENT;
  else if (operation_str == "Union")
    operation_ = UNION;
  else if (operation_str == "Intersect")
    operation_ = INTERSECT;
  else if (operation_str == "Subtract")
    operation_ = SUBTRACT;
  else {
    Errors::Message mesg("Unknown logical operation type requested on regions");
    Exceptions::Jali_throw(mesg);
  }
}

LogicalRegion::LogicalRegion(const LogicalRegion& old)
  : Region(old)
{
  // empty
}

LogicalRegion::~LogicalRegion(void)
{
  // empty
}


// -------------------------------------------------------------
// LogicalRegion::inside
// -------------------------------------------------------------
bool
LogicalRegion::inside(const Point& p) const
{
  Errors::Message mesg("In/out check not implemented for logical regions \n because the check may not be implemented for one of its component regions");
  Exceptions::Jali_throw(mesg);
}

} // namespace JaliGeometry
