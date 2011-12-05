#ifndef FTDHitCon00_h
#define FTDHitCon00_h

#include "IHitConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two hits.
    * 
    * Allows:
    * 
    * - Jumping to layer 0 from layer 4 or less
    * - going to next layer
    * 
    */   
   class FTDHitCon00 : public IHitConnector{
      
      
   public:
      
      FTDHitCon00 ( const SectorSystemFTD* sectorSystemFTD );
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDHitCon00(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
   };
   
   
}


#endif

