#ifndef FTDHitCon01_h
#define FTDHitCon01_h

#include "IHitConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two hits.
    * 
    * Allows:
    * 
    * - Connections to layer +1, +2 and +3 
    * - Connection to IP from layer 8 or lower
    * 
    */   
   class FTDHitCon01 : public IHitConnector{
      
      
   public:
      
      /**
       * @param layerRange how many layer the particle might go
       * @param lastLayerToIP the highest layer that is allowed to directly connect to the IP
       */
      FTDHitCon01 ( const SectorSystemFTD* sectorSystemFTD , unsigned layerStepMax , unsigned lastLayerToIP);
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDHitCon01(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      unsigned _layerStepMax;
      unsigned _lastLayerToIP;
      
      
   };
   
   
}


#endif

