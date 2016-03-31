/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
// -------------------------------------------------------------
// file: MeshFactory.cpp
// -------------------------------------------------------------
// -------------------------------------------------------------
// Battelle Memorial Institute
// Pacific Northwest Laboratory
// -------------------------------------------------------------
// -------------------------------------------------------------
// Created March 10, 2011 by William A. Perkins
// Last Change: Mon Aug  1 13:46:30 2011 by William A. Perkins <d3g096@PE10900.pnl.gov>
// -------------------------------------------------------------


static const char* SCCS_ID = "$Id$ Battelle PNL";

#include <boost/format.hpp>

#include "MeshFactory.hh"
#include "MeshFileType.hh"
#include "FrameworkTraits.hh"
#include "Geometry.hh"

namespace Jali {


// -------------------------------------------------------------
//  class MeshFactory
// -------------------------------------------------------------

// -------------------------------------------------------------
// MeshFactory:: constructors / destructor
// -------------------------------------------------------------
MeshFactory::MeshFactory(const MPI_Comm& communicator)
  : my_comm(communicator),
    my_preference(default_preference()) {}

MeshFactory::~MeshFactory(void) {}

// -------------------------------------------------------------
// MeshFactory::preference
// -------------------------------------------------------------
/**
 * local -- but better be the same on all processes
 *
 * This routine populates the framework preference list, but only
 * with available frameworks.  If none of the preferred frameworks
 * are available, the preference list is left empty and an exception
 * is thrown.
 *
 * @param pref list of mesh framework preferences
 */
void
MeshFactory::preference(const FrameworkPreference& pref) {
  my_preference.clear();
  my_preference = available_preference(pref);
  if (my_preference.empty()) {
    Message e("specified framework(s) not available: ");
    for (FrameworkPreference::const_iterator i = pref.begin();
         i != pref.end(); i++) {
      e.add_data(framework_name(*i).c_str());
      e.add_data(" ");
      Exceptions::Jali_throw(e);
    }
  }
}

// -------------------------------------------------------------
// MeshFactory::create
// -------------------------------------------------------------
/**
 * Collective
 *
 * This creates a mesh by reading the specified file (or file set).
 *
 * @param filename mesh file to read
 *
 * @return mesh instance
 */
std::shared_ptr<Mesh>
MeshFactory::create(const std::string& filename,
                    const JaliGeometry::GeometricModelPtr &gm,
                    const bool request_faces,
                    const bool request_edges,
                    const bool request_wedges,
                    const bool request_corners,
                    const int num_tiles) {

  // check the file format
  Format fmt = file_format(my_comm, filename);

  if (fmt == UnknownFormat) {
    FileMessage
        e(boost::str(boost::format("%s: unknown file format") %
                     filename).c_str());
    Exceptions::Jali_throw(e);
  }

  Message e("MeshFactory::create: error: ");
  int ierr[1], aerr[1];
  ierr[0] = 0;
  aerr[0] = 0;

  int numproc;
  MPI_Comm_size(my_comm, &numproc);

  std::shared_ptr<Mesh> result;
  for (FrameworkPreference::const_iterator i = my_preference.begin();
       i != my_preference.end(); i++) {
    if (framework_reads(*i, fmt, numproc > 1)) {
      try {
        result = framework_read(my_comm, *i, filename, gm,
                                request_faces, request_edges,
                                request_wedges, request_corners,
                                num_tiles);
        if (gm && (gm->dimension() != result->space_dimension())) {
          Errors::Message
              mesg("Geometric model and mesh dimension do not match");
          Exceptions::Jali_throw(mesg);
        }
        return result;
      } catch (const Message& msg) {
        ierr[0] += 1;
        e.add_data(msg.what());
      } catch (const std::exception& stde) {
        ierr[0] += 1;
        e.add_data("internal error: ");
        e.add_data(stde.what());
      }
      MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
      if (aerr[0] > 0) Exceptions::Jali_throw(e);
    }
  }
  e.add_data(boost::str(boost::format("%s: unable to read mesh file") %
                        filename).c_str());
  Exceptions::Jali_throw(e);
}

/**
 * Collective
 *
 * This creates a mesh by generating a block of hexahedral cells.
 *
 * Hopefully, if any one process has an error, all processes will
 * throw an Mesh::Message exception.
 *
 * @param x0 origin x-coordinate
 * @param y0 origin y-coordinate
 * @param z0 origin z-coordinate
 * @param x1 maximum x-coordinate
 * @param y1 maximum y-coordinate
 * @param z1 maximum z-coordinate
 * @param nx number of cells in the x-direction
 * @param ny number of cells in the y-direction
 * @param nz number of cells in the z-direction
 *
 * @return mesh instance
 */
std::shared_ptr<Mesh>
MeshFactory::create(double x0, double y0, double z0,
                    double x1, double y1, double z1,
                    int nx, int ny, int nz,
                    const JaliGeometry::GeometricModelPtr &gm,
                    const bool request_faces,
                    const bool request_edges,
                    const bool request_wedges,
                    const bool request_corners,
                    const int num_tiles) {

  std::shared_ptr<Mesh> result;
  Message e("MeshFactory::create: error: ");
  int ierr[1], aerr[1];
  ierr[0] = 0;
  aerr[0] = 0;

  unsigned int dim = 3;

  if (gm && (gm->dimension() != 3)) {
    Errors::Message mesg("Geometric model and mesh dimension do not match");
    Exceptions::Jali_throw(mesg);
  }

  if (nx <= 0 || ny <= 0 || nz <= 0) {
    ierr[0] += 1;
    e.add_data(boost::str(
        boost::format("invalid mesh cells requested: %d x %d x %d") %
                          nx % ny % nz).c_str());
  }
  MPI_Allreduce(&ierr, &aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);

  if (x1 - x0 <= 0.0 || y1 - y0 <= 0.0 || z1 - z0 <= 0.0) {
    ierr[0] += 1;
    e.add_data(boost::str(
        boost::format("invalid mesh dimensions requested: %.6g x %.6g x %.6g") %
                          (x1 - x0) % (y1 - y0) % (z1 - z0)).c_str());
  }
  MPI_Allreduce(&ierr, &aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);

  int numprocs;
  MPI_Comm_size(my_comm, &numprocs);

  for (FrameworkPreference::const_iterator i = my_preference.begin();
       i != my_preference.end(); i++) {
    if (framework_generates(*i, numprocs > 1, dim)) {
      try {
        result = framework_generate(my_comm, *i,
                                    x0, y0, z0, x1, y1, z1, nx, ny, nz, gm,
                                    request_faces, request_edges,
                                    request_wedges, request_corners,
                                    num_tiles);
        return result;
      } catch (const Message& msg) {
        ierr[0] += 1;
        e.add_data(msg.what());
      } catch (const std::exception& stde) {
        ierr[0] += 1;
        e.add_data("internal error: ");
        e.add_data(stde.what());
      }
      MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
      if (aerr[0] > 0) Exceptions::Jali_throw(e);
    }
  }
  e.add_data("unable to generate mesh");
  Exceptions::Jali_throw(e);
}

/**
 * Collective
 *
 * This creates a mesh by generating a block of quadrilateral cells.
 *
 * Hopefully, if any one process has an error, all processes will
 * throw an Mesh::Message exception.
 *
 * @param x0 origin x-coordinate
 * @param y0 origin y-coordinate
 * @param x1 maximum x-coordinate
 * @param y1 maximum y-coordinate
 * @param nx number of cells in the x-direction
 * @param ny number of cells in the y-direction
 *
 * @return mesh instance
 */

std::shared_ptr<Mesh>
MeshFactory::create(double x0, double y0,
                    double x1, double y1,
                    int nx, int ny,
                    const JaliGeometry::GeometricModelPtr &gm,
                    const bool request_faces,
                    const bool request_edges,
                    const bool request_wedges,
                    const bool request_corners,
                    const int num_tiles) {

  std::shared_ptr<Mesh> result;
  Message e("MeshFactory::create: error: ");
  int ierr[1], aerr[1];
  ierr[0] = 0;
  aerr[0] = 0;

  unsigned int dim = 2;

  if (gm && (gm->dimension() != 2)) {
    Errors::Message mesg("Geometric model and mesh dimension do not match");
    Exceptions::Jali_throw(mesg);
  }

  if (nx <= 0 || ny <= 0) {
    ierr[0] += 1;
    e.add_data(boost::str(
        boost::format("invalid mesh cells requested: %d x %d") %
                          nx % ny).c_str());
  }
  MPI_Allreduce(&ierr, &aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);

  if (x1 - x0 <= 0.0 || y1 - y0 <= 0.0) {
    ierr[0] += 1;
    e.add_data(boost::str(
        boost::format("invalid mesh dimensions requested: %.6g x %.6g") %
                          (x1 - x0) % (y1 - y0)).c_str());
  }

  MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);

  int numprocs;
  MPI_Comm_size(my_comm, &numprocs);

  for (FrameworkPreference::const_iterator i = my_preference.begin();
       i != my_preference.end(); i++) {
    if (framework_generates(*i, numprocs > 1, dim)) {
      try {
        result = framework_generate(my_comm, *i,
                                    x0, y0, x1, y1, nx, ny, gm,
                                    request_faces, request_edges,
                                    request_wedges, request_corners,
                                    num_tiles);
        return result;
      } catch (const Message& msg) {
        ierr[0] += 1;
        e.add_data(msg.what());
      } catch (const std::exception& stde) {
        ierr[0] += 1;
        e.add_data("internal error: ");
        e.add_data(stde.what());
      }
      MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
      if (aerr[0] > 0) Exceptions::Jali_throw(e);
    }
  }
  e.add_data("unable to generate mesh");
  Exceptions::Jali_throw(e);
}


/**
 * Collective
 *
 * This creates a mesh by generating a block of 1d cells.
 *
 * Hopefully, if any one process has an error, all processes will
 * throw an Mesh::Message exception.
 *
 * @param x vector of spatial coordinates of nodes
 *
 * @return mesh instance
 */

std::shared_ptr<Mesh>
MeshFactory::create(std::vector<double> x,
                    const JaliGeometry::GeometricModelPtr &gm,
                    const bool request_faces,
                    const bool request_edges,
                    const bool request_wedges,
                    const bool request_corners,
                    const int num_tiles,
                    const JaliGeometry::Geom_type geom_type) {
  std::shared_ptr<Mesh> result;
  Message e("MeshFactory::create: error: ");
  int ierr[1], aerr[1];
  ierr[0] = 0;
  aerr[0] = 0;

  unsigned int dim = 1;

  if (gm && (gm->dimension() != 1)) {
    Errors::Message mesg("Geometric model and mesh dimension do not match");
    Exceptions::Jali_throw(mesg);
  }

  if (x.size() < 2) {
    ierr[0] += 1;
    e.add_data(boost::str(boost::format("invalid num nodes requested: %d") %
                          x.size()).c_str());
  }
  MPI_Allreduce(&ierr, &aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);

  double delta = x.back() - x.front();
  if (delta <= 0.0) {
    ierr[0] += 1;
    e.add_data(boost::str(boost::format("invalid mesh coors requested: %.6g") %
                          delta).c_str());
  }
  MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
  if (aerr[0] > 0) Exceptions::Jali_throw(e);
  int numprocs;
  MPI_Comm_size(my_comm, &numprocs);

  for (FrameworkPreference::const_iterator i = my_preference.begin();
       i != my_preference.end(); i++) {
    if (framework_generates(*i, numprocs > 1, dim)) {
      try {
        result = framework_generate(my_comm, *i,
                                    x,
                                    gm,
                                    request_faces, request_edges,
                                    request_wedges, request_corners,
                                    num_tiles, geom_type);
        return result;
      } catch (const Message& msg) {
        ierr[0] += 1;
        e.add_data(msg.what());
      } catch (const std::exception& stde) {
        ierr[0] += 1;
        e.add_data("internal error: ");
        e.add_data(stde.what());
      }
      MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
      if (aerr[0] > 0) Exceptions::Jali_throw(e);
    }
  }
  e.add_data("unable to generate mesh");
  Exceptions::Jali_throw(e);
}

/**
 * This creates a mesh by extracting subsets of entities from an existing
 * mesh possibly flattening it by removing the last dimension or (in the
 * future) extruding it when it makes sense
 *
 * @param inmesh
 * @param setnames
 * @param setkind
 * @param flatten
 * @param extrude
 *
 * @return
 */
std::shared_ptr<Mesh>
MeshFactory::create(const std::shared_ptr<Mesh> inmesh,
                    const std::vector<std::string> setnames,
                    const Entity_kind setkind,
                    const bool flatten, const bool extrude,
                    const bool request_faces,
                    const bool request_edges,
                    const bool request_wedges,
                    const bool request_corners,
                    const int num_tiles) {

  std::shared_ptr<Mesh> result;
  Message e("MeshFactory::create: error: ");
  int ierr[1], aerr[1];
  ierr[0] = 0;
  aerr[0] = 0;

  int dim = inmesh->cell_dimension();
  int numprocs;
  MPI_Comm_size(my_comm, &numprocs);

  for (FrameworkPreference::const_iterator i = my_preference.begin();
       i != my_preference.end(); i++) {
    if (framework_extracts(*i, numprocs > 1, dim)) {
      try {
        result = framework_extract(my_comm, *i, inmesh,
                                   setnames, setkind,
                                   flatten, extrude,
                                   request_faces, request_edges,
                                   request_wedges, request_corners,
                                   num_tiles);
        return result;
      } catch (const Message& msg) {
        ierr[0] += 1;
        e.add_data(msg.what());
      } catch (const std::exception& stde) {
        ierr[0] += 1;
        e.add_data("internal error: ");
        e.add_data(stde.what());
      }
      MPI_Allreduce(ierr, aerr, 1, MPI_INT, MPI_SUM, my_comm);
      if (aerr[0] > 0) Exceptions::Jali_throw(e);
    }
  }
  e.add_data("unable to extract mesh");
  Exceptions::Jali_throw(e);
}

}  // namespace Jali
