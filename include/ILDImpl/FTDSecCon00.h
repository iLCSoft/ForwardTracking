#ifndef FTDSecCon00_h
#define FTDSecCon00_h

#include "ISectorConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two sectors.
    * 
    * Allows:
    * 
    * - Jumping to layer 0 from layer 4 or less
    * - going to next layer
    * 
    */   
   class FTDSecCon00 : public ISectorConnector{
      
      
   public:
      
      FTDSecCon00 ( const SectorSystemFTD* sectorSystemFTD );
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDSecCon00(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
   };
   
   
}


#endif

