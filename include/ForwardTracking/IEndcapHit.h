#ifndef IEndcapHit_h
#define IEndcapHit_h

#include <iostream>

#include "EVENT/TrackerHit.h"
#include "lcio.h"

#include "KiTrack/IHit.h"

#include "SectorSystemEndcap.h"

using namespace lcio;

namespace KiTrackMarlin{
   
   
   /** An interface for a hit for the ILD using an lcio TrackerHit as basis.
    * 
    * It comes along with a layer, phi and theta.
    */   
   class IEndcapHit : public IHit{
      
      
   public:
      
      
      TrackerHit* getTrackerHit() { return _trackerHit; };
      
      
      int getTheta() { return _theta; }
      unsigned getPhi() { return _phi; }
      

      //void setLayer( unsigned layer ){ _layer = layer; calculateSector();}
      //void setPhi( unsigned phi ){ _phi = phi; calculateSector();}
      //void setTheta( unsigned theta ){ _theta = theta; calculateSector();}
      void setLayer( unsigned layer ){ _layer = layer; }
      void setPhi( unsigned phi ){ _phi = phi; }
      void setTheta( unsigned theta ){ _theta = theta; }    
      
      
      virtual const ISectorSystem* getSectorSystem() const { return _sectorSystemEndcap; };
      
   protected:
      
      TrackerHit* _trackerHit;
      
      
      int _layer;
      int _phi;
      int _theta;
      
      const SectorSystemEndcap* _sectorSystemEndcap;
      
      /** Calculates and sets the sector number
       */

      //void calculateSector(){ _sector = _sectorSystemEndcap->getSector( _layer, _phi, _theta ); }
      
   };
   
}


#endif

