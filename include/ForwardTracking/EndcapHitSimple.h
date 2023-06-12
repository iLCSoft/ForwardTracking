#ifndef EndcapHitSimple_h
#define EndcapHitSimple_h

#include "KiTrack/IHit.h"

#include "SectorSystemEndcap.h"



namespace KiTrackMarlin{
   
   
   /** A hit 
    */   
   class EndcapHitSimple : public IHit{
      
      
   public:
      
      EndcapHitSimple( float x , float y , float z , int layer , int phi, int theta, const SectorSystemEndcap* const sectorSystemEndcap );
      EndcapHitSimple(const EndcapHitSimple&) = default;
      EndcapHitSimple& operator=(const EndcapHitSimple&) = default;
      ~EndcapHitSimple() = default;
      
      virtual const ISectorSystem* getSectorSystem() const { return _sectorSystemEndcap; };

   private:
      
      int _layer{};
      int _phi{};
      int _theta{};
      
      const SectorSystemEndcap* _sectorSystemEndcap{nullptr};
      
      //void calculateSector(){ _sector = _sectorSystemEndcap->getSector( _side, _layer , _module , _sensor ); }
      
   };
   
}


#endif

