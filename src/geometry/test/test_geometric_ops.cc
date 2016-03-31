#include <UnitTest++.h>

#include <iostream>


#include "../Geometry.hh"

#include "mpi.h"


TEST(Geometric_Ops)
{

  //            7-----------6
  //           /|          /|
  //          / |         / |
  //         4-----------5  |
  //         |  |        |  |
  //         |  3--------|--2
  //         | /         | /
  //         |/          |/
  //         0-----------1


  std::vector<JaliGeometry::Point> ccoords3, fcoords3;

  int nf = 6;

  std::vector<unsigned int> nfnodes;
  for (int i = 0; i < 6; i++) nfnodes.push_back(4);

  int hex_fnodes[6][4] = {{0,3,2,1},{1,2,6,5},{5,6,7,4},
                          {0,4,7,3},{0,1,5,4},{2,3,7,6}};

  double hex_ccoords1[8][3] = {{0.0,0.0,0.0},{1.0,0.0,0.0},
                              {1.0,1.0,0.0},{0.0,1.0,0.0},
                              {0.0,0.0,1.0},{1.0,0.0,1.0},
                              {1.0,1.0,1.0},{0.0,1.0,1.0}};

  double exp_hex_fnormals1[6][3] = {{0.0,0.0,-1.0},
                                    {1.0,0.0,0.0},
                                    {0.0,0.0,1.0},
                                    {-1.0,0.0,0.0},
                                    {0.0,-1.0,0.0},
                                    {0.0,1.0,0.0}};


  for (int i = 0; i < 8; i++) {
    JaliGeometry::Point xyz3(3);

    xyz3.set(hex_ccoords1[i][0],hex_ccoords1[i][1],hex_ccoords1[i][2]);

    ccoords3.push_back(xyz3);
  }


  for (int i = 0; i < nf; i++) {
    std::vector<JaliGeometry::Point> locfcoords3;

    for (int j = 0; j < nfnodes[i]; j++) {
      JaliGeometry::Point xyz3(3);

      int k = hex_fnodes[i][j];
      xyz3.set(hex_ccoords1[k][0],hex_ccoords1[k][1],hex_ccoords1[k][2]);

      locfcoords3.push_back(xyz3);
      fcoords3.push_back(xyz3);
    }

    double farea;
    JaliGeometry::Point normal(3), fcentroid(3);
    JaliGeometry::polygon_get_area_centroid_normal(locfcoords3,&farea,&fcentroid,&normal);

    CHECK_EQUAL(exp_hex_fnormals1[i][0],normal.x());
    CHECK_EQUAL(exp_hex_fnormals1[i][1],normal.y());
    CHECK_EQUAL(exp_hex_fnormals1[i][2],normal.z());
  }

  JaliGeometry::Point inpnt3(3), outpnt3(3);

  inpnt3.set(0.3,0.4,0.6);
  outpnt3.set(2.0,0.4,0.6);

  CHECK_EQUAL(true,JaliGeometry::point_in_polyhed(inpnt3,ccoords3,nf,nfnodes,fcoords3));
  CHECK_EQUAL(false,JaliGeometry::point_in_polyhed(outpnt3,ccoords3,nf,nfnodes,fcoords3));

  double volume, exp_volume = 1.0;
  JaliGeometry::Point centroid(3), exp_centroid(3);

  exp_centroid.set(0.5,0.5,0.5);

  JaliGeometry::polyhed_get_vol_centroid(ccoords3,nf,nfnodes,fcoords3,&volume,&centroid);

  CHECK_EQUAL(exp_volume,volume);

  CHECK_EQUAL(exp_centroid.x(),centroid.x());
  CHECK_EQUAL(exp_centroid.y(),centroid.y());
  CHECK_EQUAL(exp_centroid.z(),centroid.z());

}

