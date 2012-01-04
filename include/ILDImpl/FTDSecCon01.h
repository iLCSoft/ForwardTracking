#ifndef FTDSecCon01_h
#define FTDSecCon01_h

#include "ISectorConnector.h"

#include "SectorSystemFTD.h"



namespace FTrack{
   
   /** Used to connect two sectors.
    * 
    * made for the ILD_01 geometry (thus the 01 in the name) 
    * 
    * Allows:
    * 
    * - going to layers on the inside (how far see constructor)
    * - going to same petal or petals around (how far see constructor)
    * - jumping to the IP (from where see constructor)
    */   
   class FTDSecCon01 : public ISectorConnector{
      
      
   public:
      
      /** @param layerStepMax the maximum distance of the next layer on the inside
       * 
       *  @param petalStepMax the maximum distance of the next petal (in + and - direction )
       * 
       *  @param lastLayerToIP the highest layer from where a direct jump to the IP is allowed
       */
      FTDSecCon01 ( const SectorSystemFTD* sectorSystemFTD , unsigned layerStepMax , unsigned petalStepMax , unsigned lastLayerToIP);
      
      virtual std::set <int>  getTargetSectors ( int sector );
      
      virtual ~FTDSecCon01(){};
      
   private:
      
      const SectorSystemFTD* _sectorSystemFTD;
      
      unsigned _layerStepMax;
      unsigned _petalStepMax;
      unsigned _lastLayerToIP;
      
      
   };
   
   
}


#endif

