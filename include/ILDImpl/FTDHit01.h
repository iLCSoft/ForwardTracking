#ifndef FTDHit01_h
#define FTDHit01_h

#include "IFTDHit.h"


using namespace lcio;

namespace FTrack{
   
   
   /** A hit used for the ILD_01 model. 
    * 
    * - The side is according to CellID0.
    * - Layedr is set according to CellID0 +1 
    * - Module is set according to CellID0.
    * - Sensor is set according to CellID0.
    */   
   class FTDHit01 : public IFTDHit{
      
      
   public:
      
      FTDHit01( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD );
       
      
   };
   
}


#endif

