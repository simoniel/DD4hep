// $Id:$
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : A.Muennich
//
//  This is a special implementation for a FixedPadAngleDiskLayout!
//  The number of pads is distrubuted on a wedge shaped segment starting 
//  with the right edge of the first pad in the row at phiMin to the left 
//  edge of the last pad in the row at phiMax. All rows have the same height,
//  starting at rmin for the lower edge of the first row to rmax for the
//  upper edge of the last row. The row height is calculated from the number of pads.
//
//  Row height and pad height are identical, and pad gap is 0 
//  (the only thing that matters is the effective pitch).
//====================================================================
#include "DD4hep/LCDD.h"

#include "FixedPadAngleDiskLayout.h"
//#include "TPCModule.h"
#include "TGeoTube.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include <math.h>
#include <iostream>

using namespace std;

namespace DD4hep {
  
  using namespace Geometry;
  
  /// Standard constructor with arguments
  FixedPadAngleDiskLayout::FixedPadAngleDiskLayout(const Geometry::DetElement& d) : module(d) {
    pads = module.readout().segmentation();
    tube = module.volume().solid();
  }
  
  /// Standard extension constructor. det is the NEW detector element e.g. when reflecting a detector
  FixedPadAngleDiskLayout::FixedPadAngleDiskLayout(const FixedPadAngleDiskLayout& /* c */, const Geometry::DetElement& det)
    : module(det)
  {
    pads = module.readout().segmentation();
    tube = module.volume().solid();
  }

  std::string FixedPadAngleDiskLayout::getPadType()const {
    //ProjectiveCylinder pads= module.readout().segmentation();
    return pads.type();
  }
 
  int FixedPadAngleDiskLayout::getNPads()const {
    //ProjectiveCylinder pads= module.readout().segmentation();
    return pads.thetaBins()* pads.phiBins();
  }
  
  int FixedPadAngleDiskLayout::getNRows()const {
    //ProjectiveCylinder pads= module.readout().segmentation();
    return pads.thetaBins();
  }

  int FixedPadAngleDiskLayout::getNPadsInRow(int row)const {
    //ProjectiveCylinder pads= module.readout().segmentation();
    return pads.phiBins();
  }
  
  double FixedPadAngleDiskLayout::getRowHeight(int row)const {
    if(row>getNRows() || row<0)
      throw OutsideGeometryException("getRowHeight: Requested row not on module querried!");
    //all rows are the same for FixedPadAngleDiskLayout=ProjectiveCylinder 
    ////Tube       tube=module.volume().solid();
    double module_height= tube->GetRmax()-tube->GetRmin();
    return module_height/getNRows();
  }
  
  int FixedPadAngleDiskLayout::getRowNumber(int pad)const {
    if(pad>getNPads() || pad<0)
      throw OutsideGeometryException("getRowNumber: Requested pad not on module querried!");
    return pad/getNPadsInRow(0);
  }
  
  double FixedPadAngleDiskLayout::getPadPitch(int pad)const {
    if(pad>getNPads() || pad<0)
      throw OutsideGeometryException("getPadPitch: Requested pad not on module querried!");
    int row=getRowNumber(pad);
    /////Tube tube=module.volume().solid();
    double pad_radius=tube->GetRmin()+(row+0.5)*getRowHeight(0);
    double module_width= tube->GetPhi2()-tube->GetPhi1();
    double pad_angle=module_width/getNPadsInRow(row);
    return pad_radius*pad_angle*M_PI/180.;
  }
  
  int FixedPadAngleDiskLayout::getPadNumber(int pad)const {
   if(pad>getNPads() || pad<0)
      throw OutsideGeometryException("getPadNumber: Requested pad not on module querried!");
    return pad % getNPadsInRow(0);
  }

  int FixedPadAngleDiskLayout::getPadIndex(int row,int padNr)const {
    if(padNr>=getNPadsInRow(row) || padNr<0)
      throw OutsideGeometryException("getPadIndex: Requested pad not on module querried!");
    if(row>=getNRows() || padNr<0 )
      throw OutsideGeometryException("getPadIndex: Requested row not on module querried!");
    return padNr + row*getNPadsInRow(row);
  }

  int FixedPadAngleDiskLayout::getRightNeighbour(int pad)const {
    //if on edge their is no neighbour, should throw an exception
    int row=getRowNumber(pad);
    if(getPadNumber(pad)==getNPadsInRow(row)-1)
      throw OutsideGeometryException("getRightNeighbour: Requested pad is on right edge and has no right neighbour!");
    // if not on edge
    return pad + 1;
  }

  int FixedPadAngleDiskLayout::getLeftNeighbour(int pad)const {
    //if on edge their is no neighbour, should throw an exception
    if(getPadNumber(pad)==0)
      throw OutsideGeometryException("getLeftNeighbour: Requested pad is on left edge and has no left neighbour!");
    // if not on edge
    return pad - 1;
  }


  std::vector<double>  FixedPadAngleDiskLayout::getPadCenter (int pad) const {
    if(pad>getNPads())
      throw OutsideGeometryException("getPadCenter: Requested pad not on module querried!");
    int row=getRowNumber(pad);
    //////Tube tube=module.volume().solid();
    double pad_radius=tube->GetRmin()+(row+0.5)*getRowHeight(0);
    double module_width= tube->GetPhi2()-tube->GetPhi1();
    double pad_angle=(getPadNumber(pad)+0.5)*module_width/getNPadsInRow(row);
    //local center coordinates in module system
    double pad_x = pad_radius*cos(pad_angle*M_PI/180.);
    double pad_y = pad_radius*sin(pad_angle*M_PI/180.);
 
    //trafo to global coordinates
    Position global_w, local(pad_x,pad_y,0);//TPCModule(module).getModuleZPosition(); 
    module.localToWorld(local,global_w);

    vector<double> center;
    center.push_back(global_w.x);
    center.push_back(global_w.y);
    return center;
  }
  
  int FixedPadAngleDiskLayout::getNearestPad(double c0,double c1)const {
    //find z position of module in world coordinates

    Position fake_local(0,0,0);
    Position fake_global;
    module.localToWorld(fake_local,fake_global);
    // trafo to local coordinates
    Position global(c0,c1,fake_global.z), local;
    module.worldToLocal(global,local);
    Double_t point_local[3]={local.x,local.y,local.z};
    //check if it is on that module
    ////Tube tube=module.volume().solid();
    bool onMod=tube->Contains(point_local);
    if(!onMod)
      throw OutsideGeometryException("getNearestPad: Requested point not on module querried!");
    double module_width= tube->GetPhi2()-tube->GetPhi1();
    double radius=sqrt(point_local[0]*point_local[0]+point_local[1]*point_local[1]);
    int row=(radius-tube->GetRmin())/getRowHeight(0);
    //outer edge of last row belongs to last row
    if(row==getNRows())
      row=getNRows()-1;
    double pad_width=module_width/getNPadsInRow(row);    
    double angle=atan2(point_local[1],point_local[0])/M_PI*180;
    int padNr=static_cast<int>(angle/pad_width);
    return getPadIndex(row,padNr);
  }
 
}