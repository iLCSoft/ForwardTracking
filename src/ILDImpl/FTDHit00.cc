#include "FTDHit00.h"


#include "UTIL/ILDConf.h"

using namespace FTrack;

FTDHit00::FTDHit00( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD ){
   
   
   _sectorSystemFTD = sectorSystemFTD;
   
   _trackerHit = trackerHit;

   //Set the position of the FTDHit00
   const double* pos= trackerHit->getPosition();
   _x = pos[0];
   _y = pos[1]; 
   _z = pos[2]; 


   //find out layer, module, sensor

   UTIL::BitField64  cellID( ILDCellID0::encoder_string );

   cellID.setValue( trackerHit->getCellID0() );
      
   _side   = cellID[ ILDCellID0::side   ];
   _layer  = cellID[ ILDCellID0::layer  ] + 1;
   _module = 0;
   _sensor = 0;
 
   
   calculateSector();
   
   
   //We assume a real hit. If it is virtual, this has to be set.
   _isVirtual = false;
   
   
}
