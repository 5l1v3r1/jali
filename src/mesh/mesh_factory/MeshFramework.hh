// Emacs Mode Line: -*- Mode:c++;-*-
// -------------------------------------------------------------
/**
 * @file   MeshFramework.hh
 * @author William A. Perkins
 * @date Tue May 17 11:37:06 2011
 * 
 * @brief  some typedefs to identify mesh frameworks
 * 
 * 
 */
// -------------------------------------------------------------
// -------------------------------------------------------------
// Battelle Memorial Institute
// Pacific Northwest Laboratory
// -------------------------------------------------------------
// -------------------------------------------------------------
// Created March 11, 2011 by William A. Perkins
// Last Change: Tue May 17 11:37:06 2011 by William A. Perkins <d3g096@PE10900.pnl.gov>
// -------------------------------------------------------------

// SCCS ID: $Id$ Battelle PNL

#ifndef _MeshFramework_hh_
#define _MeshFramework_hh_

#include <string>
#include <vector>

namespace Jali {

  /// A type to identify available mesh frameworks
  enum Framework {
    Simple = 1,
    MSTK,
    MOAB,
    STKMESH  
  };

  /// A type with which an ordered preference list of Framework 
  typedef std::vector<Framework> FrameworkPreference;

  /// Get a name for a given framework
  extern std::string framework_name(const Framework& fw);

  /// Generate the default framework preferences
  extern FrameworkPreference default_preference(void);

  /// Modify framework preferences to get those available
  extern FrameworkPreference available_preference(const FrameworkPreference& p);

} // close namespace Jali

#endif
