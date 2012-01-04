#ifndef FTDHit00_h
#define FTDHit00_h

#include "IFTDHit.h"


using namespace lcio;

namespace FTrack{
   
   
   /** A hit used for the ILD_00 model.
    * 
    * - The side is according to CellID0.
    * - The layer is set according to the CellID0 +1.
    * - Module and Sensor are set to 0.
    */   
   class FTDHit00 : public IFTDHit{
      
      
   public:
      
      FTDHit00( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD );
      
    
   };
   
}


#endif

