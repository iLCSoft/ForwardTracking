#include "EndcapSectorConnector.h"


using namespace KiTrackMarlin;


// Constructor
EndcapSectorConnector::EndcapSectorConnector( const SectorSystemEndcap* sectorSystemEndcap , unsigned layerStepMax, unsigned lastLayerToIP ){
   
   _sectorSystemEndcap = sectorSystemEndcap ;
   _layerStepMax = layerStepMax ;
   _lastLayerToIP = lastLayerToIP ;

   _nLayers = sectorSystemEndcap->getNLayers();
   _nDivisionsInPhi = sectorSystemEndcap->getPhiSectors();
   _nDivisionsInTheta = sectorSystemEndcap->getThetaSectors();

}



std::set< int > EndcapSectorConnector::getTargetSectors ( int sector ){
   
   
   std::set <int> targetSectors;

   //std::cout << " check : n layers " << _nLayers << " n phi divisions " << _nDivisionsInPhi << " n theta divisions " << _nDivisionsInTheta << std::endl ;

   // Decode the sector integer,  and take the layer, phi and theta bin
   
   int iTheta = sector/(_nLayers*_nDivisionsInPhi) ;
   
   int iPhi = ((sector - (iTheta*_nLayers*_nDivisionsInPhi)) / _nLayers) ;
   
   int layer = sector - (iTheta*_nLayers*_nDivisionsInPhi) - (iPhi*_nLayers) ; 

   // search for sectors at the neighbouring theta nad phi bins

   int iPhi_Up    = iPhi + 8;
   int iPhi_Low   = iPhi - 8;
   int iTheta_Up  = iTheta + 1; 
   int iTheta_Low = iTheta - 1;
   if (iTheta_Low < 0) iTheta_Low = 0;
   if (iTheta_Up  >= int(_nDivisionsInTheta)) iTheta_Up = _nDivisionsInTheta-1;
   
   //*************************************************************************************

   
   for( unsigned layerStep = 1; layerStep <= _layerStepMax; layerStep++ ){
     
     //streamlog_out(DEBUG3) << " EndcapSectorConnector: layer " << layer << std::endl ;
     if ( layer >= int(layerStep) ){ // +1 makes sense if I use IP as innermost layer
       
       unsigned layerTarget = layer - layerStep;

       //streamlog_out(DEBUG3) << " EndcapSectorConnector: layer " << layer << " layerTarget " << layerTarget << std::endl ;

       //if (layerTarget >= 0 && layerTarget < 7 ){   // just a test to run cellular automaton over the whole VXD - SIT
	 
	 for (int ip = iPhi_Low ; ip <= iPhi_Up ; ip++){

	   // catch wrap-around
	   if (ip < 0) ip = _nDivisionsInPhi-1;          
	   if (ip >= int(_nDivisionsInPhi)) ip = ip - _nDivisionsInPhi;
	   
	   for (int iT = iTheta_Low ; iT <= iTheta_Up ; iT++){
	     
	     targetSectors.insert( _sectorSystemEndcap->getSector ( layerTarget , ip , iT ) ); 
	     
	   }
	 }
	 //}
     }
   }
   

   if ( layer > 0 && ( layer <= int(_lastLayerToIP) ) ){
      
     for (int ip = iPhi_Low ; ip <= iPhi_Up ; ip++){
       
       for (int iT = iTheta_Low ; iT <= iTheta_Up ; iT++){
	 
	 targetSectors.insert( 0 ) ;
       }
     }
   }
   
										 
   return targetSectors;
   
   
}


