#ifndef FTDNeighborPetalHitCon_h
#define FTDNeighborPetalHitCon_h

#include "IHitConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two hits.
    * 
    * Allows:
    * 
    * Connections to the neighbouring petals
    * 
    */   
   class FTDNeighborPetalHitCon : public IHitConnector{
      
      
   public:
      
      /**
       * 
       */
      FTDNeighborPetalHitCon ( const SectorSystemFTD* sectorSystemFTD );
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDNeighborPetalHitCon(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      
      
   };
   
   
}


#endif

