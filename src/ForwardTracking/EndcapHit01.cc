#include "EndcapHit01.h"
#include "SectorSystemEndcap.h"

#include "UTIL/ILDConf.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <climits>

using namespace KiTrackMarlin;


EndcapHit01::EndcapHit01( TrackerHit* trackerHit , const SectorSystemEndcap* const sectorSystemEndcap ){
   
   
   _sectorSystemEndcap = sectorSystemEndcap;
   
   _trackerHit = trackerHit;

   //Set the position of the EndcapHit01
   const double* pos= trackerHit->getPosition();
   _x = pos[0];
   _y = pos[1]; 
   _z = pos[2]; 


   //////////////////////////////////////////

   // //find out layer, module, sensor

   // UTIL::BitField64  cellID( ILDCellID0::encoder_string );

   // //cellID.setValue( trackerHit->getCellID0() );
   // _layer = cellID[ ILDCellID0::layer ] + 1 ;   // + 1 to take into account the IP (considered as layer 0 ) 
   // //_layer = cellID[ ILDCellID0::layer ];
   // int det_id = 0 ;
   // det_id  = cellID[lcio::ILDCellID0::subdet] ;
   // if ( det_id == lcio::ILDDetID::SIT) { _layer = _layer + 6; }   // need to find a more elegant way...


   // std::string TRICK = "system:8,barrel:3,layer:4,module:14,sensor:2,side:32:-2,strip:20";
   // UTIL::BitField64 cellid_decoder( TRICK ) ;


   UTIL::BitField64  cellid_decoder( ILDCellID0::encoder_string );
   cellid_decoder.setValue( trackerHit->getCellID0() );

   long64 id = trackerHit->getCellID0() ;
   cellid_decoder.setValue( id ) ;

   _layer = cellid_decoder["layer"].value();
   // FIXEME: subdet should play a role: layer number should increase goign from a subdetector to another
   int subdet = cellid_decoder["subdet"].value();
   //if (subdet==2) _layer = _layer+0; //FIXME: think how to do in a cleaner way
   // if (subdet==4) _layer = _layer+6; //FIXME: think how to do in a cleaner way
   // else if (subdet==6) _layer = _layer+6+1; //FIXME: think how to do in a cleaner way
   if (subdet==4) _layer = _layer+6; //FIXME: think how to do in a cleaner way
   else if (subdet==6) _layer = _layer+6+2; //FIXME: think how to do in a cleaner way
   if (subdet==3) _layer = _layer+6; //FIXME: think how to do in a cleaner way
   else if (subdet==5) _layer = _layer+6+2; //FIXME: think how to do in a cleaner way
   // int side = cellid_decoder["side"].value();
   // int module = cellid_decoder["module"].value();
   // int sensor = cellid_decoder["sensor"].value();


   /////////////////////////////////////////

   double radius = 0;
      
   for (int i=0; i<3; ++i) {
     radius += pos[i]*pos[i];
   }

   radius = sqrt(radius);
      
   double _cosTheta = (pos[2]/radius);
   double _phi = atan2(pos[1],pos[0]);
   double _theta = acos( _cosTheta ) ;
      
   if (_phi < 0.) _phi = _phi + 2*M_PI;   

   // YV, for debugging. Calculate sector here and not through the IVXHit base class
   //calculateSector();

   _sector = _sectorSystemEndcap->getSector( _layer, _phi, _cosTheta );

   
   //We assume a real hit. If it is virtual, this has to be set.
   _isVirtual = false;
   
   
}


