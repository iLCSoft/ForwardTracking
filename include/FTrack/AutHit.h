#ifndef AutHit_h
#define AutHit_h

#include "IHit.h"
#include "EVENT/TrackerHit.h"
#include "lcio.h"

#include "SectorSystemFTD.h"

using namespace lcio;

namespace FTrack{
   
   
   /** A hit using an lcio TrackerHit as basis.
    */   
   class AutHit : public IHit{
      
      
   public:
      
      AutHit( TrackerHit* trackerHit , const SectorSystemFTD* const sectorSystemFTD );
      
      
      TrackerHit* getTrackerHit() { return _trackerHit; };
      
      
      int getSide() { return _side; }
      unsigned getModule() { return _module; }
      unsigned getSensor() { return _sensor; }
      
      void setSide( int side ){ _side = side; calculateSector();}
      void setLayer( unsigned layer ){ _layer = layer; calculateSector();}
      void setModule( unsigned module ){ _module = module; calculateSector();}
      void setSensor( unsigned sensor ){ _layer = sensor; calculateSector();}
      
      
      
      virtual const ISectorSystem* getSectorSystem() const { return _sectorSystemFTD; };
      
   private:
      
      TrackerHit* _trackerHit;
      
      
      int _side;
      unsigned _layer;
      unsigned _module;
      unsigned _sensor;
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      /** Calculates and sets the sector number
       */
      void calculateSector(){ _sector = _sectorSystemFTD->getSector( _side, _layer , _module , _sensor ); }
   
   };
   
}


#endif

