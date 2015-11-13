#ifndef EndcapHit01_h
#define EndcapHit01_h

#include "IEndcapHit.h"


using namespace lcio;

namespace KiTrackMarlin{
   
   
   /** A class for hits in the VXD (the 01 is just for historical reasons and may be renamed)
    * 
    * - The side is according to CellID0.
    * - Layer is set according to CellID0 +1 (so we can use layer 0 for the IP)
    * - Module is set according to CellID0.
    * - Sensor is set according to CellID0 -1. (because currently sensors of the VXD start with 1 in the CellID0, if this changes, this has to be modified)
    */   
   class EndcapHit01 : public IEndcapHit{
      
      
   public:
      
      EndcapHit01( TrackerHit* trackerHit , const SectorSystemEndcap* const sectorSystemEndcap );
      
      
   };

  //void setSectorisationInPhi(int PhiSectors);
  //void setSectorisationInTheta(int ThetaSectors);
   
}


#endif

