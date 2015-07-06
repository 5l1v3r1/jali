#ifndef _JALI_MESH_MSTK_H_
#define _JALI_MESH_MSTK_H_

#include <memory>
#include <vector>
#include <sstream>

#include <MSTK.h>

#include <Mesh.hh>
#include <Point.hh>
#include <GeometricModel.hh>
#include <LabeledSetRegion.hh>
#include <PointRegion.hh>
#include <LogicalRegion.hh>
#include <dbc.hh>
#include <errors.hh>

namespace Jali 
{

// Mesh class based on the MSTK framework. 
//
// Instantiating a const version of this class only guarantees that
// the underlying mesh topology and geometry does not change. For
// purposes of memory savings we use lazy initialization of face and
// edge lists (they are already present in the underlying MSTK mesh
// data structures), which means that the data structures holding the
// mesh information may still change.


class Mesh_MSTK : public Mesh
{
      
public:

  // Constructors that read the mesh from a file

  // The request_faces and request_edges arguments have to be at the
  // end and not in the middle because if we omit them and specify a
  // pointer argument like gm or verbosity_obj, then there is implicit
  // conversion of the pointer to bool, thereby defeating the intent
  // of the call and making the pointer argument seem NULL. In C++11,
  // we could "delete" the illegal version of the call effectively
  // blocking the implicit conversion.
  

  Mesh_MSTK (const char *filename, const MPI_Comm& incomm,
	     const JaliGeometry::GeometricModelPtr& gm = 
	     (JaliGeometry::GeometricModelPtr) NULL,
	     const bool request_faces = true,
	     const bool request_edges = false);

  Mesh_MSTK (const char *filename, const MPI_Comm& incomm, 
             int space_dimension,
	     const JaliGeometry::GeometricModelPtr& gm = 
	     (JaliGeometry::GeometricModelPtr) NULL,
	     const bool request_faces = true,
	     const bool request_edges = false);

  // Constructors that generate a mesh internally (regular hexahedral mesh only)

  // 3D
  Mesh_MSTK(const double x0, const double y0, const double z0,
	    const double x1, const double y1, const double z1,
	    const unsigned int nx, const unsigned int ny, 
            const unsigned int nz, 
            const MPI_Comm& incomm,
            const JaliGeometry::GeometricModelPtr& gm = 
            (JaliGeometry::GeometricModelPtr) NULL,
	    const bool request_faces = true,
	    const bool request_edges = false);


  // 2D
  Mesh_MSTK(const double x0, const double y0,
	    const double x1, const double y1,
	    const int nx, const int ny, 
	    const MPI_Comm& comm,
	    const JaliGeometry::GeometricModelPtr& gm = 
	    (JaliGeometry::GeometricModelPtr) NULL,
	    const bool request_faces = true,
	    const bool request_edges = false);

  // Construct a mesh by extracting a subset of entities from another
  // mesh. The subset may be specified by a setname or a list of
  // entities. In some cases like extracting a surface mesh from a
  // volume mesh, constructor can be asked to "flatten" the mesh to a
  // lower dimensional space or to extrude the mesh to give higher
  // dimensional cells

  Mesh_MSTK(const Mesh *inmesh,
            const std::vector<std::string>& setnames,
            const Entity_kind entity_kind,
            const bool flatten = false,
            const bool extrude = false,
	    const bool request_faces = true,
	    const bool request_edges = false);

  Mesh_MSTK(const Mesh& inmesh,
            const std::vector<std::string>& setnames,
            const Entity_kind entity_kind,
            const bool flatten = false,
            const bool extrude = false,
	    const bool request_faces = true,
	    const bool request_edges = false);

  Mesh_MSTK(const Mesh& inmesh,
            const std::vector<int>& entity_list,
            const Entity_kind entity_kind,
            const bool flatten = false,
            const bool extrude = false,
	    const bool request_faces = true,
	    const bool request_edges = false);


  ~Mesh_MSTK ();


  // Get parallel type of entity
    
  Parallel_type entity_get_ptype(const Entity_kind kind, 
				 const Entity_ID entid) const;


  // Get cell type
    
  Cell_type cell_get_type(const Entity_ID cellid) const;


  // Parent entity in the source mesh if mesh was derived from another mesh

  Entity_ID entity_get_parent (const Entity_kind kind, const Entity_ID entid) const;


        
   
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
		       Entity_ID_List *nodeids) const;
    
    
  // Get nodes of face 
  // On a distributed mesh, all nodes (OWNED or GHOST) of the face 
  // are returned
  // In 3D, the nodes of the face are returned in ccw order consistent
  // with the face normal
  // In 2D, nfnodes is 2
    
  void face_get_nodes (const Entity_ID faceid, 
		       Entity_ID_List *nodeids) const;
    

  // Get nodes of edge On a distributed mesh all nodes (OWNED or
  // GHOST) of the face are returned

  void edge_get_nodes (const Entity_ID edgeid, Entity_ID *point0,
		       Entity_ID *point1) const;

  // Upward adjacencies
  //-------------------
    
  // Cells of type 'ptype' connected to a node
    
  void node_get_cells (const Entity_ID nodeid, 
		       const Parallel_type ptype,
		       Entity_ID_List *cellids) const;
    
  // Faces of type 'ptype' connected to a node
    
  void node_get_faces (const Entity_ID nodeid, 
		       const Parallel_type ptype,
		       Entity_ID_List *faceids) const;
    
  // Get faces of ptype of a particular cell that are connected to the
  // given node
    
  void node_get_cell_faces (const Entity_ID nodeid, 
			    const Entity_ID cellid,
			    const Parallel_type ptype,
			    Entity_ID_List *faceids) const;    
    

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
			       Entity_ID_List *fadj_cellids) const;

  // Node connected neighboring cells of given cell
  // (a hex in a structured mesh has 26 node connected neighbors)
  // The cells are returned in no particular order

  void cell_get_node_adj_cells(const Entity_ID cellid,
			       const Parallel_type ptype,
			       Entity_ID_List *nadj_cellids) const;

    
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
    
  // Coordinates of cells in standard order (Exodus  II convention)
  // STANDARD CONVENTION WORKS ONLY FOR STANDARD CELL TYPES IN 3D
  // For a general polyhedron this will return the node coordinates in
  // arbitrary order
  // Number of nodes is vector size divided by number of spatial dimensions
    
  void cell_get_coordinates (const Entity_ID cellid, 
			     std::vector<JaliGeometry::Point> *ccoords) const;
    
  // Modify the coordinates of a node

  void node_set_coordinates (const Entity_ID nodeid, 
                             const JaliGeometry::Point coords);

  void node_set_coordinates (const Entity_ID nodeid, const double *coords);

    
    
  //
  // Boundary Conditions or Sets
  //----------------------------
  //
    

  // Get number of entities of type 'category' in set

  unsigned int get_set_size (const Set_Name setname, 
			     const Entity_kind kind,
			     const Parallel_type ptype) const;


  unsigned int get_set_size (const char *setname, 
			     const Entity_kind kind,
			     const Parallel_type ptype) const;

  // Get list of entities of type 'category' in set

  void get_set_entities (const Set_Name setname, 
			 const Entity_kind kind, 
			 const Parallel_type ptype, 
			 std::vector<Entity_ID> *entids) const; 


  void get_set_entities (const char *setname, 
			 const Entity_kind kind, 
			 const Parallel_type ptype, 
			 std::vector<Entity_ID> *entids) const; 


private:

  MPI_Comm mpicomm;
  int myprocid, numprocs;

  Mesh_ptr mesh;

  int serial_run;
  

  // Local handles to entity lists (Vertices, "Faces", "Cells")
  
  // For a surface mesh, "Faces" refers to mesh edges and "Cells"
  // refers to mesh faces
  //
  // For a solid mesh, "Faces" refers to mesh faces and "Cells"
  // refers to mesh regions
  
  
  // These are MSTK's definitions of types of parallel mesh entities
  // These definitions are slightly different from what Jali has defined
  //
  // There are 2 types of entities relevant to this code - Owned and Ghost
  //
  // 1. OWNED - owned by this processor
  //
  // 2. GHOST - not owned by this processor
  
  // ALL = OWNED + GHOST in parallel, ALL = OWNED in serial
  

  MSet_ptr OwnedVerts, NotOwnedVerts;
  
  mutable MSet_ptr OwnedEdges, NotOwnedEdges;

  mutable MSet_ptr OwnedFaces, NotOwnedFaces;
  
  MSet_ptr OwnedCells, GhostCells;

  // Flags to indicate if face and edge info is initialized

  mutable bool faces_initialized, edges_initialized;

  // Marker to indicate if an entity is not owned

  int notwownedmark;

  // Deleted entity lists if some pre-processing had to be done
  // to the mesh to eliminate degenerate entities

  bool entities_deleted;
  List_ptr deleted_vertices, deleted_edges, deleted_faces, deleted_regions;

  // Local ID to MSTK handle map

  std::vector<MEntity_ptr> vtx_id_to_handle;
  mutable std::vector<MEntity_ptr> edge_id_to_handle;
  mutable std::vector<MEntity_ptr> face_id_to_handle;
  std::vector<MEntity_ptr> cell_id_to_handle;


  // flag whether to flip a face dir or not when returning nodes of a
  // face (relevant only on partition boundaries)
  
  mutable bool *faceflip;

  // flag whether to flip an edge dir or not when returning nodes of an edge
  // (relevant only on partition boundaries)

  mutable bool *edgeflip;

  // Attribute to precompute and store celltype

  MAttrib_ptr celltype_att;

  // Parent entity attribute - populated if the mesh is derived from
  // another mesh

  MAttrib_ptr rparentatt, fparentatt, eparentatt, vparentatt;

  const Mesh_MSTK *parent_mesh;

  // variables needed for mesh deformation

  double *meshxyz;
  double *target_cell_volumes, *min_cell_volumes, *target_weights;

  // Private methods
  // ----------------------------
  
  void clear_internals_();

  void pre_create_steps_(const int space_dimension, const MPI_Comm& incomm, 
                         const JaliGeometry::GeometricModelPtr& gm);
  void post_create_steps_(const bool request_faces, const bool request_edges);

  void collapse_degen_edges();
  Cell_type MFace_Celltype(MFace_ptr f);
  Cell_type MRegion_Celltype(MRegion_ptr r);
  void label_celltype();

  void init_pvert_lists();
  void init_pedge_lists();
  void init_pedge_dirs();
  void init_pface_lists();
  void init_pface_dirs();
  void init_pcell_lists();
  
  void init_vertex_id2handle_maps();
  void init_edge_id2handle_maps();
  void init_face_id2handle_maps();
  void init_cell_id2handle_maps();
  void init_global_ids();
  
  void init_cell_map();
  void init_face_map();
  void init_edge_map();
  void init_node_map();
  
  void init_nodes();
  void init_edges();
  void init_faces();
  void init_cells();

  void init_set_info();
  void inherit_labeled_sets(MAttrib_ptr copyatt);
  std::string internal_name_of_set(const JaliGeometry::RegionPtr region,
                                   const Entity_kind entity_kind) const;
  std::string other_internal_name_of_set(const JaliGeometry::RegionPtr r,
                                         const Entity_kind entity_kind) const;

  int  generate_regular_mesh(Mesh_ptr mesh, double x0, double y0, double z0,
			     double x1, double y1, double z1, int nx,
			     int ny, int nz);
  int  generate_regular_mesh(Mesh_ptr mesh, double x0, double y0,
			     double x1, double y1, int nx, int ny);

  void extract_mstk_mesh(const Mesh_MSTK& inmesh,
                         const List_ptr entity_ids,
                         const MType entity_dim,
                         const bool flatten = false,
                         const bool extrude = false,
			 const bool request_faces = true,
			 const bool request_edges = false);

  MSet_ptr build_set(const JaliGeometry::RegionPtr region,
                     const Entity_kind kind) const;


  // Downward Adjacencies
  //---------------------
    
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

  void cell_get_faces_and_dirs_ordered (const Entity_ID cellid,
                                         Entity_ID_List *faceids,
                                         std::vector<int> *face_dirs) const;

  void cell_get_faces_and_dirs_unordered (const Entity_ID cellid,
                                         Entity_ID_List *faceids,
                                          std::vector<int> *face_dirs) const;


  // Cells connected to a face
    
  void face_get_cells_internal (const Entity_ID faceid, 
                                const Parallel_type ptype,
                                Entity_ID_List *cellids) const;


  // Get edges of a cell

  void cell_get_edges_internal (const Entity_ID cellid,
				Entity_ID_List *edgeids) const;

  // Get edges and directions of a 2D cell

  void cell_2D_get_edges_and_dirs_internal (const Entity_ID cellid,
                                            Entity_ID_List *edgeids,
                                            std::vector<int> *edgedirs) const;

  // Edges and edge directions of a face

  void face_get_edges_and_dirs_internal (const Entity_ID cellid,
					 Entity_ID_List *edgeids,
					 std::vector<int> *edgedirs,
					 bool ordered=true) const;

  // Map from Jali's mesh entity kind to MSTK's mesh type.
  
  MType entity_kind_to_mtype(const Entity_kind kind) const {
    
    // The first index is cell dimension (0,1,2,3) and the second index
    // is the entity kind
    //
    // map order in each row is NODE, EDGE, FACE, CELL
    //
    // So, for a 1D mesh, nodes are MVERTEX type in MSTK, edges and faces
    // are also MVERTEX type, and cells are MEDGE type
    //
    // For a 2D mesh, nodes are MVERTEX type, edges and faces are MEDGE
    // type, and cells are MFACE type
    
    static MType const 
      kind2mtype[4][4] = {{MVERTEX, MVERTEX, MVERTEX, MVERTEX},  // 0d meshes
                          {MVERTEX, MVERTEX, MVERTEX, MEDGE},    // 1d meshes
                          {MVERTEX, MEDGE,   MEDGE,   MFACE},    // 2d meshes
                          {MVERTEX, MEDGE,   MFACE,   MREGION}}; // 3d meshes
    
    return kind2mtype[cell_dimension()][(int)kind];
  }
    
};


    
  inline Parallel_type Mesh_MSTK::entity_get_ptype(const Entity_kind kind, const Entity_ID entid) const {
    MEntity_ptr ment;
      
    switch(kind) {
    case CELL:
      ment = (MEntity_ptr) cell_id_to_handle[entid];
      break;
    case FACE:
      ment = (MEntity_ptr) face_id_to_handle[entid];
      break;
    case NODE:
      ment = (MEntity_ptr) vtx_id_to_handle[entid];
      break;
    }
      
    if (MEnt_PType(ment) == PGHOST)
      return GHOST;
    else
      return OWNED;
  }

} // End namespace Jali

#endif /* _JALI_MESH_MSTK_H_ */
