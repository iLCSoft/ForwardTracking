#include "EndcapHitSimple.h"


using namespace KiTrackMarlin;

EndcapHitSimple::EndcapHitSimple( float x , float y , float z , int layer , int phi, int theta, const SectorSystemEndcap* const sectorSystemEndcap ){
   
   
   _sectorSystemEndcap = sectorSystemEndcap;
   
   _x = x;
   _y = y; 
   _z = z; 
   
  
   _layer  = layer;
   _phi = phi;
   _theta = theta;
   
   _sector = _sectorSystemEndcap->getSector( layer, phi, theta );  // maybe a good idea to calculate a sector for the IP hit as well  
   //calculateSector();
   
   
   //We assume a real hit. If it is virtual, this has to be set.
   _isVirtual = false;
   
   
}

