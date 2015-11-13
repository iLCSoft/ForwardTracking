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
      
      
      
      virtual const ISectorSystem* getSectorSystem() const { return _sectorSystemEndcap; };
      
      virtual ~EndcapHitSimple(){}
      
   private:
      
      int _layer;
      int _phi;
      int _theta;
      
      const SectorSystemEndcap* _sectorSystemEndcap;
      
      //void calculateSector(){ _sector = _sectorSystemEndcap->getSector( _side, _layer , _module , _sensor ); }
      
   };
   
}


#endif

