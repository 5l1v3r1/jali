/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
#ifndef _MESH_SIMPLE_H_
#define _MESH_SIMPLE_H_

#include <memory>
#include <vector>
#include "mpi.h"

#include "Mesh.hh"
#include "Region.hh"

#include "GeometricModel.hh"
#include "dbc.hh"
#include "errors.hh"

namespace Jali {

class GenerationSpec;

class Mesh_simple : public virtual Mesh
{
      
public:
      
  // the request_faces and request_edges arguments have to be at the
  // end and not in the middle because if we omit them and specify a
  // pointer argument like gm or verbosity_obj, then there is implicit
  // conversion of the pointer to bool, thereby defeating the intent
  // of the call and making the pointer argument seem NULL. In C++11,
  // we could "delete" the illegal version of the call effectively
  // blocking the implicit conversion.
  
  Mesh_simple (double x0, double y0, double z0,
	       double x1, double y1, double z1,
	       int nx, int ny, int nz, const MPI_Comm& communicator,
	       const JaliGeometry::GeometricModelPtr gm = 
               (JaliGeometry::GeometricModelPtr) NULL,
	       const bool request_faces = true,
	       const bool request_edges = false);
  
  Mesh_simple (double x0, double y0,
	       double x1, double y1,
	       int nx, int ny, const MPI_Comm& communicator,
	       const JaliGeometry::GeometricModelPtr &gm = 
               (JaliGeometry::GeometricModelPtr) NULL,
	       const bool request_faces = true,
	       const bool request_edges = false);
  
  // Construct a mesh by extracting a subset of entities from another
  // mesh. In some cases like extracting a surface mesh from a volume
  // mesh, constructor can be asked to "flatten" the mesh to a lower
  // dimensional space or to extrude the mesh to give higher
  // dimensional cells

  Mesh_simple(const Mesh *inmesh,
              const std::vector<std::string>& setnames,
              const Entity_kind setkind,
              const bool flatten = false,
              const bool extrude = false,
	      const bool request_faces = true,
	      const bool request_edges = false);

  Mesh_simple(const Mesh& inmesh,
              const std::vector<std::string>& setnames,
              const Entity_kind setkind,
              const bool flatten = false,
              const bool extrude = false,
	      const bool request_faces = true,
	      const bool request_edges = false);

  Mesh_simple(const Mesh& inmesh, 
              const std::vector<int>& entity_id_list, 
              const Entity_kind entity_kind,
              const bool flatten = false,
              const bool extrude = false,
              const bool request_faces = true,
              const bool request_edges = false);
  
  virtual ~Mesh_simple ();
  
  void update ();


  // Get parallel type of entity
    
  Parallel_type entity_get_ptype(const Entity_kind kind, 
				 const Entity_ID entid) const;


  // Get cell type
    
  Cell_type cell_get_type(const Entity_ID cellid) const;
        
   
  //
  // General mesh information
  // -------------------------
  //
    
  // Number of entities of any kind (cell, face, node) and in a
  // particular category (OWNED, GHOST, USED)
    
  unsigned int num_entities (const Entity_kind kind,
			     const Parallel_type ptype) const;
    
    
  // Global ID of any entity
    
  Entity_ID GID(const Entity_ID lid, const Entity_kind kind) const;
    
    
    
  //
  // Mesh Entity Adjacencies 
  //-------------------------


  // Downward Adjacencies
  //---------------------
    
    
  // Get nodes of cell 
  // On a distributed mesh, all nodes (OWNED or GHOST) of the cell 
  // are returned
  // Nodes are returned in a standard order (Exodus II convention)
  // STANDARD CONVENTION WORKS ONLY FOR STANDARD CELL TYPES in 3D
  // For a general polyhedron this will return the nodes in
  // arbitrary order
  // In 2D, the nodes of the polygon will be returned in ccw order 
  // consistent with the face normal
    
  void cell_get_nodes (const Entity_ID cellid, 
		       std::vector<Entity_ID> *nodeids) const;
    
    
  // Get nodes of face 
  // On a distributed mesh, all nodes (OWNED or GHOST) of the face 
  // are returned
  // In 3D, the nodes of the face are returned in ccw order consistent
  // with the face normal
  // In 2D, nfnodes is 2
    
  void face_get_nodes (const Entity_ID faceid, 
		       std::vector<Entity_ID> *nodeids) const;
    

  // Get nodes of edge

  void edge_get_nodes (const Entity_ID edgeid, Entity_ID *nodeid0,
		       Entity_ID *nodeid1) const {
    Errors::Message mesg("Edges not implemented in this framework. Use MSTK");
    Exceptions::Jali_throw(mesg);
  }

  // Upward adjacencies
  //-------------------
    
  // Cells of type 'ptype' connected to a node
    
  void node_get_cells (const Entity_ID nodeid, 
		       const Parallel_type ptype,
		       std::vector<Entity_ID> *cellids) const;
    
  // Faces of type 'ptype' connected to a node
    
  void node_get_faces (const Entity_ID nodeid, 
		       const Parallel_type ptype,
		       std::vector<Entity_ID> *faceids) const;
    
  // Get faces of ptype of a particular cell that are connected to the
  // given node
    
  void node_get_cell_faces (const Entity_ID nodeid, 
			    const Entity_ID cellid,
			    const Parallel_type ptype,
			    std::vector<Entity_ID> *faceids) const;    
    

  // Same level adjacencies
  //-----------------------

  // Face connected neighboring cells of given cell of a particular ptype
  // (e.g. a hex has 6 face neighbors)

  // The order in which the cellids are returned cannot be
  // guaranteed in general except when ptype = USED, in which case
  // the cellids will correcpond to cells across the respective
  // faces given by cell_get_faces

  void cell_get_face_adj_cells(const Entity_ID cellid,
			       const Parallel_type ptype,
			       std::vector<Entity_ID> *fadj_cellids) const;

  // Node connected neighboring cells of given cell
  // (a hex in a structured mesh has 26 node connected neighbors)
  // The cells are returned in no particular order

  void cell_get_node_adj_cells(const Entity_ID cellid,
			       const Parallel_type ptype,
			       std::vector<Entity_ID> *nadj_cellids) const;

    
  //
  // Mesh entity geometry
  //--------------
  //
    
  // Node coordinates - 3 in 3D and 2 in 2D
    
  void node_get_coordinates (const Entity_ID nodeid, 
			     JaliGeometry::Point *ncoord) const;
    
    
  // Face coordinates - conventions same as face_to_nodes call 
  // Number of nodes is the vector size divided by number of spatial dimensions
    
  void face_get_coordinates (const Entity_ID faceid, 
			     std::vector<JaliGeometry::Point> *fcoords) const; 
    
  // Coordinates of cells in standard order (Exodus II convention)
  // STANDARD CONVENTION WORKS ONLY FOR STANDARD CELL TYPES IN 3D
  // For a general polyhedron this will return the node coordinates in
  // arbitrary order
  // Number of nodes is vector size divided by number of spatial dimensions
    
  void cell_get_coordinates (const Entity_ID cellid, 
			     std::vector<JaliGeometry::Point> *ccoords) const;
    
  // Modify the coordinates of a node

  void node_set_coordinates (const Entity_ID nodeid, const JaliGeometry::Point coords);

  void node_set_coordinates (const Entity_ID nodeid, const double *coords);

    
  //
  // Boundary Conditions or Sets
  //----------------------------
  //
    

  unsigned int get_set_size (const Set_Name setname, 
			     const Entity_kind kind,
			     const Parallel_type ptype) const;


  unsigned int get_set_size (const char *setname, 
			     const Entity_kind kind,
			     const Parallel_type ptype) const;


  void get_set_entities (const Set_Name setname, 
			 const Entity_kind kind, 
			 const Parallel_type ptype, 
			 Entity_ID_List *entids) const; 


  void get_set_entities (const char *setname, 
			 const Entity_kind kind, 
			 const Parallel_type ptype, 
			 Entity_ID_List *entids) const; 


  // this should be used with extreme caution:
  // modify coordinates
  void set_coordinate(Entity_ID local_node_id,
		      double* source_begin, double* source_end);


private:
  void update_internals_();
  void clear_internals_();

  std::vector<double> coordinates_;

  inline unsigned int node_index_(int i, int j, int k) const;
  inline unsigned int xyface_index_(int i, int j, int k) const;
  inline unsigned int yzface_index_(int i, int j, int k) const;
  inline unsigned int xzface_index_(int i, int j, int k) const;
  inline unsigned int cell_index_(int i, int j, int k) const;

  int nx_, ny_, nz_;  // number of cells in the three coordinate directions
  double x0_, x1_, y0_, y1_, z0_, z1_;  // coordinates of lower left front and upper right back of brick


  int num_cells_;
  int num_nodes_;
  int num_faces_;

  // Local-id tables of entities
  std::vector<Entity_ID> cell_to_face_;
  std::vector<int> cell_to_face_dirs_;
  std::vector<Entity_ID> cell_to_node_;
  std::vector<Entity_ID> face_to_node_;
  std::vector<Entity_ID> face_to_cell_;
  std::vector<Entity_ID> node_to_face_;
  std::vector<Entity_ID> node_to_cell_;

  // The following are mutable because they have to be modified 
  // after the class construction even though the class is instantiated
  // as a constant class

  mutable std::vector<std::vector<Entity_ID> > side_sets_;
  mutable std::vector<std::vector<Entity_ID> > element_blocks_;
  mutable std::vector<std::vector<Entity_ID> > node_sets_;
  mutable std::vector<JaliGeometry::RegionPtr> element_block_regions_;
  mutable std::vector<JaliGeometry::RegionPtr> side_set_regions_;
  mutable std::vector<JaliGeometry::RegionPtr> node_set_regions_;


  // Get faces of a cell.

  // Get faces of a cell and directions in which the cell uses the face 

  // The Jali coding guidelines regarding function arguments is purposely
  // violated here to allow for a default input argument

  // On a distributed mesh, this will return all the faces of the
  // cell, OWNED or GHOST. If ordered = true, the faces will be
  // returned in a standard order according to Exodus II convention
  // for standard cells; in all other situations (ordered = false or
  // non-standard cells), the list of faces will be in arbitrary order

  // In 3D, direction is 1 if face normal points out of cell
  // and -1 if face normal points into cell
  // In 2D, direction is 1 if face/edge is defined in the same
  // direction as the cell polygon, and -1 otherwise

  void cell_get_faces_and_dirs_internal (const Entity_ID cellid,
                                Entity_ID_List *faceids,
                                std::vector<int> *face_dirs,
                                const bool ordered=false) const;

  // Cells connected to a face
    
  void face_get_cells_internal (const Entity_ID faceid, 
                                const Parallel_type ptype,
                                std::vector<Entity_ID> *cellids) const;


  // Edges of a cell

  void cell_get_edges_internal (const Entity_ID cellid,
                                Entity_ID_List *edgeids) const 
  { 
    Errors::Message mesg("Edges not implemented in this framework. Use MSTK");
    Exceptions::Jali_throw(mesg);
  }

  // edges and directions of a 2D cell 

  void cell_2D_get_edges_and_dirs_internal (const Entity_ID cellid,
                                            Entity_ID_List *edgeids,
                                            std::vector<int> *edge_dirs) const
  {
    Errors::Message mesg("Edges not implemented in this framework. Use MSTK");
    Exceptions::Jali_throw(mesg);
  }



  // Edges and edge directions of a face

  void face_get_edges_and_dirs_internal (const Entity_ID cellid,
					 Entity_ID_List *edgeids,
					 std::vector<int> *edgedirs,
					 bool ordered=true) const
  {
    Errors::Message mesg("Edges not implemented in this framework. Use MSTK");
    Exceptions::Jali_throw(mesg);
  };

    
};

  // -------------------------
  // Template & inline members
  // ------------------------


  unsigned int Mesh_simple::node_index_(int i, int j, int k) const
  {
    return i + j*(nx_+1) + k*(nx_+1)*(ny_+1);
  }

  unsigned int Mesh_simple::cell_index_(int i, int j, int k) const
  {
    return i + j*nx_ + k*nx_*ny_;
  }

  unsigned int Mesh_simple::xyface_index_(int i, int j, int k) const
  {
    return i + j*nx_ + k*nx_*ny_;
  }

  unsigned int Mesh_simple::xzface_index_(int i, int j, int k) const
  {
    return i + j*nx_ + k*nx_*(ny_+1) +  xyface_index_(0,0,nz_+1);
  }

  unsigned int Mesh_simple::yzface_index_(int i, int j, int k) const
  {
    return i + j*(nx_+1) + k*(nx_+1)*ny_ + xzface_index_(0,0,nz_);
  }

} // close namespace Jali



#endif /* _MESH_MAPS_H_ */
