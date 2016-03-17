/* -*-  mode: c++; c-default-style: "google"; indent-tabs-mode: nil -*- */
// -------------------------------------------------------------
/**
 * @file   test_mesh_geometry.cc
 * @author Rao V. Garimella
 * @date   Tue May 15, 2012
 * 
 * @brief  
 * 
 * 
 */
// -------------------------------------------------------------
// -------------------------------------------------------------

#include <UnitTest++.h>

#include <mpi.h>
#include <iostream>

#include "Mesh.hh"
#include "MeshFactory.hh"
#include "FrameworkTraits.hh"
#include "Geometry.hh"

TEST(MESH_GEOMETRY_PLANAR)
{

  int nproc, me;
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);
  MPI_Comm_rank(MPI_COMM_WORLD,&me);

  const Jali::Framework frameworks[] = {  
    Jali::MSTK
  };
  const char *framework_names[] = {
    "MSTK"
  };
  
  const int numframeworks = sizeof(frameworks)/sizeof(Jali::Framework);


  Jali::Framework the_framework;
  for (int i = 0; i < numframeworks; i++) {


    // Set the framework

    the_framework = frameworks[i];

    if (!Jali::framework_available(the_framework)) continue;

    std::cerr << "Testing geometry operators with " << framework_names[i] << std::endl;


    // Create the mesh

    Jali::MeshFactory factory(MPI_COMM_WORLD);
    std::shared_ptr<Jali::Mesh> mesh;

    int ierr = 0;
    int aerr = 0;
    try {
      Jali::FrameworkPreference prefs(factory.preference());
      prefs.clear(); 
      prefs.push_back(the_framework);

      factory.preference(prefs);

      mesh = factory(0.0,0.0,1.0,1.0,2,2);

    } catch (const Jali::Message& e) {
      std::cerr << ": mesh error: " << e.what() << std::endl;
      ierr++;
    } catch (const std::exception& e) {
      std::cerr << ": error: " << e.what() << std::endl;
      ierr++;
    }

    MPI_Allreduce(&ierr,&aerr,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    CHECK_EQUAL(aerr,0);


    double exp_cell_volume[4] = {0.25,0.25,0.25,0.25};
    double exp_cell_centroid[4][2] = {{0.25,0.25},
                                      {0.25,0.75},
                                      {0.75,0.25},
                                      {0.75,0.75}};
    double exp_face_area[12] = {0.5,0.5,0.5,0.5,
                                0.5,0.5,0.5,0.5,
                                0.5,0.5,0.5,0.5};
    double exp_face_centroid[12][2] = {{0.25,0.0},{0.5,0.25},
                                       {0.25,0.5},{0.0,0.25},
                                       {0.5,0.75},{0.25,1.0},
                                       {0.0,0.75},{0.75,0.0},
                                       {1.0,0.25},{0.75,0.5},
                                       {1.0,0.75},{0.75,1.0}};

    int ncells = mesh->num_entities(Jali::CELL,Jali::OWNED);
    int nfaces = mesh->num_entities(Jali::FACE,Jali::ALL);
    int nnodes = mesh->num_entities(Jali::NODE,Jali::ALL);

    int spacedim = 2;

    for (int i = 0; i < ncells; i++) {

      JaliGeometry::Point centroid = mesh->cell_centroid(i);

      // Search for a cell with the same centroid in the 
      // expected list of centroid

      bool found = false;

      for (int j = 0; j < ncells; j++) {
        if (fabs(exp_cell_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_cell_centroid[j][1]-centroid[1]) < 1.0e-10) {

          found = true;
          CHECK_EQUAL(exp_cell_volume[j],mesh->cell_volume(i));
          break;

        }
      }

      CHECK_EQUAL(found,true);

      Jali::Entity_ID_List cfaces;
      JaliGeometry::Point normal_sum(2), normal(2);      

      mesh->cell_get_faces(i,&cfaces);
      normal_sum.set(0.0);

      for (int j = 0; j < cfaces.size(); j++) {
        normal = mesh->face_normal(cfaces[j],false,i);
        normal_sum += normal;
      }

      double val = L22(normal_sum);
      CHECK_CLOSE(val,0.0,1.0e-20);                
    }

    for (int i = 0; i < nfaces; i++) {
      JaliGeometry::Point centroid = mesh->face_centroid(i);

      bool found = false;

      for (int j = 0; j < nfaces; j++) {
        if (fabs(exp_face_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_face_centroid[j][1]-centroid[1]) < 1.0e-10) {

          found = true;

          CHECK_EQUAL(exp_face_area[j],mesh->face_area(i));

          // Check the natural normal

          JaliGeometry::Point normal = mesh->face_normal(i);
            
      
          // Check the normal with respect to each connected cell
          
          Jali::Entity_ID_List cellids;
          mesh->face_get_cells(i,Jali::ALL,&cellids);
          
          for (int k = 0; k < cellids.size(); k++) {
            int dir;
            JaliGeometry::Point normal_wrt_cell = 
              mesh->face_normal(i,false,cellids[k],&dir);

            //            Jali::Entity_ID_List cellfaces;
            //            std::vector<int> cellfacedirs;
            //            mesh->cell_get_faces_and_dirs(cellids[k],&cellfaces,&cellfacedirs);
            //
            //            bool found2 = false;
            //            int dir = 1;
            //            for (int m = 0; m < cellfaces.size(); m++) {
            //              if (cellfaces[m] == i) {
            //                found2 = true;
            //                dir = cellfacedirs[m];
            //                break;
            //              }
            //            }
            //
            //            CHECK_EQUAL(found2,true);

            JaliGeometry::Point normal1(normal);
            normal1 *= dir;

            CHECK_ARRAY_EQUAL(&(normal1[0]),&(normal_wrt_cell[0]),spacedim);

            
            JaliGeometry::Point cellcentroid = mesh->cell_centroid(cellids[k]);
            JaliGeometry::Point facecentroid = mesh->face_centroid(i);

            JaliGeometry::Point outvec = facecentroid-cellcentroid;


            double dp = outvec*normal_wrt_cell;
            dp /= (norm(outvec)*norm(normal_wrt_cell));         


            CHECK_CLOSE(dp,1.0,1e-10);
          }

          break;
        }
      }

      CHECK_EQUAL(found,true);
    }

  } // for each framework i

}


TEST(MESH_GEOMETRY_SURFACE)
{

// DISABLED FOR NOW

 return;


 int nproc, me;
 MPI_Comm_size(MPI_COMM_WORLD,&nproc);
 MPI_Comm_rank(MPI_COMM_WORLD,&me);

  const Jali::Framework frameworks[] = {  
    Jali::MSTK
  };
  const char *framework_names[] = {
    "MSTK"
  };
  
  const int numframeworks = sizeof(frameworks)/sizeof(Jali::Framework);


  Jali::Framework the_framework;
  for (int i = 0; i < numframeworks; i++) {


    // Set the framework

    the_framework = frameworks[i];

    if (!Jali::framework_available(the_framework)) continue;

    std::cerr << "Testing geometry operators with " << framework_names[i] << std::endl;


    // Create the mesh

    Jali::MeshFactory factory(MPI_COMM_WORLD);
    std::shared_ptr<Jali::Mesh> mesh;

    int ierr = 0;
    int aerr = 0;
    try {
      Jali::FrameworkPreference prefs(factory.preference());
      prefs.clear(); 
      prefs.push_back(the_framework);

      factory.preference(prefs);

      mesh = factory("test/surfquad.exo");

    } catch (const Jali::Message& e) {
      std::cerr << ": mesh error: " << e.what() << std::endl;
      ierr++;
    } catch (const std::exception& e) {
      std::cerr << ": error: " << e.what() << std::endl;
      ierr++;
    }

    MPI_Allreduce(&ierr,&aerr,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    CHECK_EQUAL(aerr,0);


    double exp_cell_volume[4] = {0.25,0.25,0.25,0.25};
    double exp_cell_centroid[4][3] = {{0.25,0.25,0.0},
                                      {0.25,0.75,0.0},
                                      {0.5,0.25,0.25},
                                      {0.5,0.75,0.25}};
    double exp_face_area[12] = {0.5,0.5,0.5,0.5,
                                0.5,0.5,0.5,0.5,
                                0.5,0.5,0.5,0.5};
    double exp_face_centroid[12][3] = {{0.25,0.0,0.0},{0.5,0.25,0.0},
                                       {0.25,0.5,0.0},{0.0,0.25,0.0},
                                       {0.5,0.75,0.0},{0.25,1.0,0.0},
                                       {0.0,0.75,0.0},{0.5,0.0,0.25},
                                       {0.5,0.25,0.5},{0.5,0.5,0.25},
                                       {0.5,0.75,0.5},{0.5,1.0,0.25}};

    int ncells = mesh->num_entities(Jali::CELL,Jali::OWNED);
    int nfaces = mesh->num_entities(Jali::FACE,Jali::ALL);
    int nnodes = mesh->num_entities(Jali::NODE,Jali::ALL);

    int spacedim = 3;

    for (int i = 0; i < ncells; i++) {

      JaliGeometry::Point centroid = mesh->cell_centroid(i);

      // Search for a cell with the same centroid in the 
      // expected list of centroid

      bool found = false;

      for (int j = 0; j < ncells; j++) {
        if (fabs(exp_cell_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_cell_centroid[j][1]-centroid[1]) < 1.0e-10 &&
            fabs(exp_cell_centroid[j][2]-centroid[2]) < 1.0e-10) {

          found = true;
          CHECK_EQUAL(exp_cell_volume[j],mesh->cell_volume(i));
          break;

        }
      }

      CHECK_EQUAL(found,true);
    }

    for (int i = 0; i < nfaces; i++) {
      JaliGeometry::Point centroid = mesh->face_centroid(i);

      bool found = false;

      for (int j = 0; j < nfaces; j++) {
        if (fabs(exp_face_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_face_centroid[j][1]-centroid[1]) < 1.0e-10 &&
            fabs(exp_face_centroid[j][2]-centroid[2]) < 1.0e-10) {

          found = true;

          CHECK_EQUAL(exp_face_area[j],mesh->face_area(i));

      
          // Check the normal with respect to each connected cell
          
          Jali::Entity_ID_List cellids;
          mesh->face_get_cells(i,Jali::ALL,&cellids);


          JaliGeometry::Point facecentroid = mesh->face_centroid(i);
          
          for (int k = 0; k < cellids.size(); k++) {
            int dir;
            JaliGeometry::Point normal_wrt_cell = 
              mesh->face_normal(i,false,cellids[k],&dir);

            //            Jali::Entity_ID_List cellfaces;
            //            std::vector<int> cellfacedirs;
            //            mesh->cell_get_faces_and_dirs(cellids[k],&cellfaces,&cellfacedirs);

            //            bool found2 = false;
            //            int dir = 1;
            //            for (int m = 0; m < cellfaces.size(); m++) {
            //              if (cellfaces[m] == i) {
            //                found2 = true;
            //                dir = cellfacedirs[m];
            //                break;
            //              }
            //            }

            //            CHECK_EQUAL(found2,true);


            JaliGeometry::Point cellcentroid = mesh->cell_centroid(cellids[k]);

            JaliGeometry::Point outvec = facecentroid-cellcentroid;

            double dp = outvec*normal_wrt_cell;
            dp /= (norm(outvec)*norm(normal_wrt_cell));         

            CHECK_CLOSE(dp,1.0,1e-10);

          }


          if (cellids.size() == 2 && 
              ((fabs(facecentroid[0]-0.5) < 1.0e-16) &&
               (fabs(facecentroid[2]) < 1.0e-16))) {

            // An edge on the crease. The two normals should be different 
   
            JaliGeometry::Point n0 = mesh->face_normal(i,false,cellids[0]);
            JaliGeometry::Point n1 = mesh->face_normal(i,false,cellids[1]);

            double dp = n0*n1/(norm(n0)*norm(n1));

            CHECK_CLOSE(dp,0.0,1e-10);

          }

          break;
        }
      }

      CHECK_EQUAL(found,true);
    }

  } // for each framework i

}




TEST(MESH_GEOMETRY_SOLID)
{

  int nproc, me;
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);
  MPI_Comm_rank(MPI_COMM_WORLD,&me);

  const Jali::Framework frameworks[] = {  
    Jali::MSTK,
    Jali::Simple
  };
  const char *framework_names[] = {
    "MSTK",
    "Simple"
  };
  
  const int numframeworks = sizeof(frameworks)/sizeof(Jali::Framework);


  Jali::Framework the_framework;
  for (int i = 0; i < numframeworks; i++) {


    // Set the framework

    the_framework = frameworks[i];

    if (!Jali::framework_available(the_framework)) continue;

    std::cerr << "Testing geometry operators with " << framework_names[i] << std::endl;


    // Create the mesh

    Jali::MeshFactory factory(MPI_COMM_WORLD);
    std::shared_ptr<Jali::Mesh> mesh;

    int ierr = 0;
    int aerr = 0;
    try {
      Jali::FrameworkPreference prefs(factory.preference());
      prefs.clear(); 
      prefs.push_back(the_framework);

      factory.preference(prefs);

      mesh = factory(0.0,0.0,0.0,1.0,1.0,1.0,2,2,2);

    } catch (const Jali::Message& e) {
      std::cerr << ": mesh error: " << e.what() << std::endl;
      ierr++;
    } catch (const std::exception& e) {
      std::cerr << ": error: " << e.what() << std::endl;
      ierr++;
    }

    MPI_Allreduce(&ierr,&aerr,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    CHECK_EQUAL(aerr,0);


    double exp_cell_volume[8] = {0.125,0.125,0.125,0.125,
                                 0.125,0.125,0.125,0.125};
    double exp_cell_centroid[8][3] = {{0.25,0.25,0.25},
                                      {0.75,0.25,0.25},
                                      {0.25,0.75,0.25},
                                      {0.75,0.75,0.25},
                                      {0.25,0.25,0.75},
                                      {0.75,0.25,0.75},
                                      {0.25,0.75,0.75},
                                      {0.75,0.75,0.75}};
    double exp_face_area[36] = {0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25,
                                0.25,0.25,0.25,0.25};
    double exp_face_centroid[36][3] = {{0.0,0.25,0.25},
                                       {0.0,0.75,0.25},
                                       {0.0,0.25,0.75},
                                       {0.0,0.75,0.75},

                                       {0.5,0.25,0.25},
                                       {0.5,0.75,0.25},
                                       {0.5,0.25,0.75},
                                       {0.5,0.75,0.75},

                                       {1.0,0.25,0.25},
                                       {1.0,0.75,0.25},
                                       {1.0,0.25,0.75},
                                       {1.0,0.75,0.75},

                                       {0.25,0.0,0.25},
                                       {0.75,0.0,0.25},
                                       {0.25,0.0,0.75},
                                       {0.75,0.0,0.75},
                                                 
                                       {0.25,0.5,0.25},
                                       {0.75,0.5,0.25},
                                       {0.25,0.5,0.75},
                                       {0.75,0.5,0.75},
                                                 
                                       {0.25,1.0,0.25},
                                       {0.75,1.0,0.25},
                                       {0.25,1.0,0.75},
                                       {0.75,1.0,0.75},

                                       {0.25,0.25,0.0},
                                       {0.75,0.25,0.0},
                                       {0.25,0.75,0.0},
                                       {0.75,0.75,0.0},
                                                     
                                       {0.25,0.25,0.5},
                                       {0.75,0.25,0.5},
                                       {0.25,0.75,0.5},
                                       {0.75,0.75,0.5},
                                                     
                                       {0.25,0.25,1.0},
                                       {0.75,0.25,1.0},
                                       {0.25,0.75,1.0},
                                       {0.75,0.75,1.0},
                                       
    };


    int ncells = mesh->num_entities(Jali::CELL,Jali::OWNED);
    int nfaces = mesh->num_entities(Jali::FACE,Jali::ALL);
    int nnodes = mesh->num_entities(Jali::NODE,Jali::ALL);

    int spacedim = 3;

    for (int i = 0; i < ncells; i++) {

      JaliGeometry::Point centroid = mesh->cell_centroid(i);

      // Search for a cell with the same centroid in the 
      // expected list of centroid

      bool found = false;

      for (int j = 0; j < ncells; j++) {
        if (fabs(exp_cell_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_cell_centroid[j][1]-centroid[1]) < 1.0e-10 &&
            fabs(exp_cell_centroid[j][2]-centroid[2]) < 1.0e-10) {

          found = true;
          CHECK_EQUAL(exp_cell_volume[j],mesh->cell_volume(i));
          break;

        }
      }

      CHECK_EQUAL(found,true);

      Jali::Entity_ID_List cfaces;
      JaliGeometry::Point normal_sum(3), normal(3);      

      mesh->cell_get_faces(i,&cfaces);
      normal_sum.set(0.0);

      for (int j = 0; j < cfaces.size(); j++) {
        normal = mesh->face_normal(cfaces[j],false,i);
        normal_sum += normal;
      }

      double val = L22(normal_sum);
      CHECK_CLOSE(val,0.0,1.0e-20);                
    }

    for (int i = 0; i < nfaces; i++) {
      JaliGeometry::Point centroid = mesh->face_centroid(i);

      bool found = false;

      for (int j = 0; j < nfaces; j++) {
        if (fabs(exp_face_centroid[j][0]-centroid[0]) < 1.0e-10 &&
            fabs(exp_face_centroid[j][1]-centroid[1]) < 1.0e-10 &&
            fabs(exp_face_centroid[j][2]-centroid[2]) < 1.0e-10) {

          found = true;

          CHECK_EQUAL(exp_face_area[j],mesh->face_area(i));

          // Check the natural normal

          JaliGeometry::Point normal = mesh->face_normal(i);
            
      
          // Check the normal with respect to each connected cell
          
          Jali::Entity_ID_List cellids;
          mesh->face_get_cells(i,Jali::ALL,&cellids);
          
          for (int k = 0; k < cellids.size(); k++) {
            int dir;
            JaliGeometry::Point normal_wrt_cell = 
              mesh->face_normal(i,false,cellids[k],&dir);

            // Jali::Entity_ID_List cellfaces;
            // std::vector<int> cellfacedirs;
            // mesh->cell_get_faces_and_dirs(cellids[k],&cellfaces,&cellfacedirs);

            // bool found2 = false;
            // int dir = 1;
            // for (int m = 0; m < cellfaces.size(); m++) {
            //   if (cellfaces[m] == i) {
            //     found2 = true;
            //     dir = cellfacedirs[m];
            //     break;
            //   }
            // }

            // CHECK_EQUAL(found2,true);

            JaliGeometry::Point normal1(normal);
            normal1 *= dir;

            CHECK_ARRAY_EQUAL(&(normal1[0]),&(normal_wrt_cell[0]),spacedim);

            
            JaliGeometry::Point cellcentroid = mesh->cell_centroid(cellids[k]);
            JaliGeometry::Point facecentroid = mesh->face_centroid(i);

            JaliGeometry::Point outvec = facecentroid-cellcentroid;

            double dp = outvec*normal_wrt_cell;
            dp /= (norm(outvec)*norm(normal_wrt_cell));         


            CHECK_CLOSE(dp,1.0,1e-10);
          }

          break;
        }
      }

      CHECK_EQUAL(found,true);
    }

    
    // Now deform the mesh a little and verify that the sum of the
    // outward normals of all faces of cell is still zero

    JaliGeometry::Point ccoords(3);
    mesh->node_get_coordinates(13,&ccoords); // central node

    // Lets be sure this is the central node
    CHECK_EQUAL(ccoords[0],0.5);
    CHECK_EQUAL(ccoords[1],0.5);
    CHECK_EQUAL(ccoords[2],0.5);
    
    // Perturb it
    ccoords.set(0.7,0.7,0.7);
    mesh->node_set_coordinates(13,ccoords);

    // Now check the normals

    for (int i = 0; i < ncells; i++) {

      Jali::Entity_ID_List cfaces;
      JaliGeometry::Point normal_sum(3), normal(3);      

      mesh->cell_get_faces(i,&cfaces);
      normal_sum.set(0.0);

      for (int j = 0; j < cfaces.size(); j++) {
        normal = mesh->face_normal(cfaces[j],true,i);
        normal_sum += normal;
      }

      double val = L22(normal_sum);
      CHECK_CLOSE(val,0.0,1.0e-20);                

    }    

  } // for each framework i

}

