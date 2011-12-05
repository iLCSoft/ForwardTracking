#ifndef FTDHit01_h
#define FTDHit01_h

#include "IFTDHit.h"


using namespace lcio;

namespace FTrack{
   
   
   /** A hit used for the ILD_01 model: 
    * The side is according to CellID0.
    * The layer is set the following way: the IP is layer 0. layer 1 are the petals of the first FTD, that are close
    * to the IP. The petals on the first FTD are layer 2. Layer 3 are the petals on the second disk that are close to the
    * IP. This way it is guaranteed, that tracks have to pass layer after layer and cannot hit twice on one layer.
    * Which kinda is the basic requirement for the use of the cellular automaton.
    * At the moment it is assumed, that the even module numbers are the ones closer to the IP. TODO: pass this via a reference to FTDLayerLayout
    * 
    * Module is set according to CellID0.
    * Sensor is set according to CellID0.
    */   
   class FTDHit01 : public IFTDHit{
      
      
   public:
      
      FTDHit01( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD );
       
      
   };
   
}


#endif

