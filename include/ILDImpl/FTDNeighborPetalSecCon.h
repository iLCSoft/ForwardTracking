#ifndef FTDNeighborPetalSecCon_h
#define FTDNeighborPetalSecCon_h

#include "ISectorConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two sectors.
    * 
    * Allows:
    * 
    * - Connections to the neighbouring petals (the one to the left and the one to the right on the same layer and side)
    * 
    */   
   class FTDNeighborPetalSecCon : public ISectorConnector{
      
      
   public:
      
      /**
       * 
       */
      FTDNeighborPetalSecCon ( const SectorSystemFTD* sectorSystemFTD );
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDNeighborPetalSecCon(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      
      
   };
   
   
}


#endif

