#ifndef HitCon_h
#define HitCon_h

#include "IHitConnector.h"

#include "SectorSystemFTD.h"

// TODO: rename
// TODO: split this up into several hitconnectors: one for a fixed step size. one for hopping to layer 0 and so on.


namespace FTrack{
   
   /** Used to connect two hits.
    * 
    * Allows:
    * 
    * - Jumping to layer 0 from layer 4 or less
    * - going to next layer
    * 
    */   
   class HitCon : public IHitConnector{
      
      
   public:
      
      HitCon ( const SectorSystemFTD* sectorSystemFTD );
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~HitCon(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
   };
   
   
}


#endif

