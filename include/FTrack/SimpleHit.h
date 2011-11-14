#ifndef SimpleHit_h
#define SimpleHit_h

#include "IHit.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   
   /** A hit 
    */   
   class SimpleHit : public IHit{
      
      
   public:
      
      SimpleHit( float x , float y , float z , int side, unsigned layer , unsigned module, unsigned sensor, const SectorSystemFTD* const sectorSystemFTD );
      
      
      
      virtual const ISectorSystem* getSectorSystem() const { return _sectorSystemFTD; };
      
   private:
      
      int _side;
      unsigned _layer;
      unsigned _module;
      unsigned _sensor;
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      void calculateSector(){ _sector = _sectorSystemFTD->getSector( _side, _layer , _module , _sensor ); }
      
   };
   
}


#endif

