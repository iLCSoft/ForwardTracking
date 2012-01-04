#include "FTDSecCon00.h"


using namespace FTrack;



FTDSecCon00::FTDSecCon00( const SectorSystemFTD* sectorSystemFTD ){
   
   _sectorSystemFTD = sectorSystemFTD;
   
}



std::set< int > FTDSecCon00::getTargetSectors ( int sector ){
   
   
   
   std::set <int> targetSectors;
   
      
   int side = _sectorSystemFTD->getSide( sector );
   unsigned layer = _sectorSystemFTD->getLayer( sector );
//    unsigned module = _sectorSystemFTD->getModule( sector );
//    unsigned sensor = _sectorSystemFTD->getSensor( sector );
   
//    unsigned nLayers = _sectorSystemFTD->getNumberOfLayers();
   unsigned nModules = _sectorSystemFTD->getNumberOfModules();
   unsigned nSensors = _sectorSystemFTD->getNumberOfSensors();
   
   
   //connect to the next layer
   if ( layer > 0 ){
      
      
      unsigned layerTarget = layer - 1; //The next layer
      
      for ( unsigned iModule=0; iModule < nModules ; iModule++){ //over all modules
         
         for ( unsigned iSensor=0; iSensor < nSensors ; iSensor++ ){ //over all sensors
            
            
            targetSectors.insert( _sectorSystemFTD->getSector ( side , layerTarget , iModule , iSensor ) ); 
            
         }
         
      }
     
   }
   
   //Allow jumping to layer 0 from layer 4 or less
   if ( ( layer > 1 )&& ( layer <= 4 ) ){
      
      
      unsigned layerTarget = 0;
      
      for ( unsigned iModule=0; iModule < nModules ; iModule++){ //over all modules
         
         for ( unsigned iSensor=0; iSensor < nSensors ; iSensor++ ){ //over all sensors
            
            
            targetSectors.insert( _sectorSystemFTD->getSector ( side , layerTarget , iModule , iSensor ) ); 
            
         }
         
      }
      
   }
   
   return targetSectors;
   
   
}


