#include "FTDRepresentation.h"

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <UTIL/ILDConf.h>



using namespace FTrack;


FTDRepresentation::FTDRepresentation( AutCode* autCode ){
   
 
   
   _autCode = autCode;
   
   _nLayers  = autCode->getNLayers();
   _nModules = autCode->getNModules();
   _nSensors = autCode->getNSensors();
   
   
}

void FTDRepresentation::addHit(  AutHit* hit ){
   
   
   
   
   int code = _autCode->getCode ( hit->getSide() , hit->getLayer() , hit->getModule() , hit->getSensor() ) ;
   
   //add a new entry to the map (if there is already one nothing will happen)
   std::vector <  AutHit* > emptyHitVec;
   _map_code_hits.insert( std::pair< int , std::vector <  AutHit* > >( code , emptyHitVec ) ); 
   
   //add it to the vector
   _map_code_hits [ code ].push_back( hit );
      
   
   streamlog_out(DEBUG0) << "\nAdded an AutHit with code " << code <<" to the FTDRepresentation.";
   
   
}



std::vector <  AutHit* > FTDRepresentation::getHitsFromSensor( int side , unsigned layer , unsigned module , unsigned sensor ){
   

   
   // Calculate the code corresponding to our parameters
   int code = _autCode->getCode( side , layer , module , sensor );
   
   
   // search for the entries with the specific code
   std::map < int , std::vector < AutHit*> >::iterator it = _map_code_hits.find( code );
      
   
   std::vector < AutHit*> hitVec;
   
   if ( it != _map_code_hits.end() ){ //entry is found
   
      hitVec = it->second;     

   }
   
   
   
   return hitVec;
   
}



std::vector < AutHit* > FTDRepresentation::getHitsFromModule( int side , unsigned layer , unsigned module ){
   
   
   std::vector < AutHit*> hitVec;
   
   
   for ( unsigned int sensor=0; sensor < _nSensors; sensor++ ){ //over all sensors
      
      std::vector <  AutHit*> newHitVec = getHitsFromSensor( side , layer, module , sensor );
      
      hitVec.insert( hitVec.begin() , newHitVec.begin() , newHitVec.end() );
      
      
      
   }   
  
   return hitVec;   
   
   
}


std::vector < AutHit* > FTDRepresentation::getHitsFromLayer ( int side , unsigned layer ){
   
   
   std::vector < AutHit*> hitVec;
   
   
   for ( unsigned int module=0; module < _nModules; module++ ){ //over all modules
      
      std::vector <  AutHit*> newHitVec = getHitsFromModule( side , layer, module );
      
      hitVec.insert( hitVec.begin() , newHitVec.begin() , newHitVec.end() );
      
      
      
   }   
  
   return hitVec;   
   
   
}
      
      
      
      
std::vector < AutHit* > FTDRepresentation::getHitsFromSide  ( int side ) {
   
   
   std::vector < AutHit*> hitVec;
   
   
   for ( unsigned int layer=0; layer < _nLayers; layer++ ){ //over all layers
      
      std::vector <  AutHit*> newHitVec = getHitsFromLayer( side , layer );
      
      hitVec.insert( hitVec.begin() , newHitVec.begin() , newHitVec.end() );
      
      
      
   }   
  
   return hitVec;   
   
   
}



std::vector <  AutHit* > FTDRepresentation::getAllHits(){
   
   
   std::vector < AutHit*> hitVec;
   
   
   for ( int side = -1; side <= 1 ; side+=2 ){ //over both sides
      
      std::vector <  AutHit*> newHitVec = getHitsFromSide( side );
      
      hitVec.insert( hitVec.begin() , newHitVec.begin() , newHitVec.end() );
      
      
      
   }   
  
   return hitVec;   
   
   
}



std::set <int> FTDRepresentation::getCodes(){
   
   
   
   std::set <int> codes;
   
   std::map < int , std::vector < AutHit*> >::iterator it;
   
   
   for( it= _map_code_hits.begin() ; it!=_map_code_hits.end() ; it++ ){
      
      codes.insert( it->first );
      
   }
   
   
   return codes; 
   
   
   
}


