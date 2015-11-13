#ifndef EndcapSectorConnector_h
#define EndcapSectorConnector_h

#include "KiTrack/ISectorConnector.h"

#include "SectorSystemEndcap.h"



namespace KiTrackMarlin{
   
   /** Used to connect two sectors on the VXD.
    * 
    * 
    * Allows:
    * 
    * - going to layers on the inside (how far see constructor)
    * - jumping to the IP (from where see constructor)
    */   
   class EndcapSectorConnector : public ISectorConnector{
      
      
   public:
      
    EndcapSectorConnector ( const SectorSystemEndcap* sectorSystemEndcap , unsigned layerStepMax, unsigned lastLayerToIP ) ;
      
      /** @return a set of all sectors that are connected to the passed sector */
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~EndcapSectorConnector(){};
      
   private:
      
      const SectorSystemEndcap* _sectorSystemEndcap;
      
      unsigned _layerStepMax;
      unsigned _nLayers;
      unsigned _lastLayerToIP;
      unsigned _nDivisionsInPhi ;
      unsigned _nDivisionsInTheta ;      
      
   };
   
   
}


#endif

