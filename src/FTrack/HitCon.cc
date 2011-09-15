#include "HitCon.h"



using namespace FTrack;



HitCon::HitCon( AutCode* autCode ){
   
   _autCode = autCode;
   
}



std::set< int > HitCon::getTargetCode ( int code ){
   
   
   
   std::set <int> targetCode;
   
      
   int side = _autCode->getSide( code );
   unsigned layer = _autCode->getLayer( code );
   unsigned module = _autCode->getModule( code );
   unsigned sensor = _autCode->getSensor( code );
   
   unsigned nLayers = _autCode->getNLayers();
   unsigned nModules = _autCode->getNModules();
   unsigned nSensors = _autCode->getNSensors();
   
   
   //connect to the next layer
   if ( layer > 0 ){
      
      
      unsigned layerTarget = layer - 1; //The next layer
      
      for ( unsigned iModule=0; iModule < nModules ; iModule++){ //over all modules

         for ( unsigned iSensor=0; iSensor < nSensors ; iSensor++ ){ //over all sensors

            
            targetCode.insert( _autCode->getCode ( side , layerTarget , iModule , iSensor ) ); 
         
         }
         
      }
         
   }
   
   //Allow jumping to layer 0 from layer 4 or less
   if ( ( layer > 1 )&& ( layer <= 4 ) ){
      
      
      unsigned layerTarget = 0; //The next layer
      
      for ( unsigned iModule=0; iModule < nModules ; iModule++){ //over all modules

         for ( unsigned iSensor=0; iSensor < nSensors ; iSensor++ ){ //over all sensors

            
            targetCode.insert( _autCode->getCode ( side , layerTarget , iModule , iSensor ) ); 
            
         }
         
      }
      
   }
   
   
   
   return targetCode;
   
   
   
}