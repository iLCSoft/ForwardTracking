#ifndef FTDHit01_h
#define FTDHit01_h

#include "IFTDHit.h"


using namespace lcio;

namespace FTrack{
   
   
   /** A hit used for the ILD_01 model. 
    * 
    * - The side is according to CellID0.
    * - Layer is set according to CellID0 +1 (so we can use layer = 0 for the IP)
    * - Module is set according to CellID0.
    * - Sensor is set according to CellID0 -1. (because currently sensors of the FTD start with 1 in the CellID0, if this changes, this has to be modified)
    */   
   class FTDHit01 : public IFTDHit{
      
      
   public:
      
      FTDHit01( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD );
       
      
   };
   
}


#endif

