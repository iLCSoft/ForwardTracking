#include "FTDSegRepresentation.h"

#include <UTIL/ILDConf.h>



using namespace FTrack;


FTDSegRepresentation::FTDSegRepresentation( AutCode* autCode ){
   
   
   _autCode = autCode;
   
   _nLayers  = autCode->getNLayers();
   _nModules = autCode->getNModules();
   _nSensors = autCode->getNSensors();
   
   
   
}

void FTDSegRepresentation::addSegment( Segment* seg , int code ){
   
   
   
   //add a new entry to the map (if there is already ony nothing will happen)
   std::vector < Segment* > emptyVec;
   _map_code_segs.insert( std::pair< int , std::vector < Segment* > >( code , emptyVec ) ); 
   
   //add it to the vector
   _map_code_segs [ code ].push_back( seg );
      
   
   
}



std::vector < Segment* > FTDSegRepresentation::getSegsFromSensor( int side , unsigned layer , unsigned module , unsigned sensor ){
   
   
   
   
   // Calculate the code corresponding to our parameters
   int code = _autCode->getCode( side , layer , module , sensor );
   
   
   // search for the entry with the specific CellID0
   std::map < int , std::vector < Segment*> >::iterator it = _map_code_segs.find( code );
   
   
   std::vector < Segment*> segVec;
   
   if ( it != _map_code_segs.end() ){ //entry is found
   
      segVec = it->second;     
      
   }
   

   
   
   return segVec;
   

   
}



std::vector <Segment* > FTDSegRepresentation::getSegsFromModule( int side , unsigned layer , unsigned module ){
   
   
   std::vector <Segment*> segVec;
   
   
   for ( unsigned int sensor=0; sensor < _nSensors; sensor++ ){ //over all sensors
      
      std::vector < Segment*> newSegVec = getSegsFromSensor( side , layer, module , sensor );
      
      segVec.insert( segVec.begin() , newSegVec.begin() , newSegVec.end() );
      
      
      
   }   
  
   return segVec;   
   
   
}


std::vector <Segment* > FTDSegRepresentation::getSegsFromLayer ( int side , unsigned layer ){
   
   
   std::vector <Segment*> segVec;
   
   
   for ( unsigned int module=0; module < _nModules ; module++ ){ //over all modules
      
      std::vector < Segment*> newSegVec = getSegsFromModule( side , layer, module );
      
      segVec.insert( segVec.begin() , newSegVec.begin() , newSegVec.end() );
      
      
      
   }   
  
   return segVec;   
   
   
}
      
      
      
      
std::vector <Segment* > FTDSegRepresentation::getSegsFromSide  ( int side ) {
   
   
   std::vector <Segment*> segVec;
   
   
   for ( unsigned int layer=0; layer < _nLayers; layer++ ){ //over all layers
      
      std::vector < Segment*> newSegVec = getSegsFromLayer( side , layer );
      
      segVec.insert( segVec.begin() , newSegVec.begin() , newSegVec.end() );
      
      
      
   }   
  
   return segVec;   
   
   
}



std::vector < Segment* > FTDSegRepresentation::getAllSegs(){
   
   
   std::vector <Segment*> segVec;
   
   
   for ( int side = -1; side <= 1 ; side+=2 ){ //over both sides
      
      std::vector < Segment*> newSegVec = getSegsFromSide( side );
      
      segVec.insert( segVec.begin() , newSegVec.begin() , newSegVec.end() );
      
      
      
   }   
  
   return segVec;   
   
   
}


std::set <int> FTDSegRepresentation::getCodes(){
   
   
 
   std::set <int> codes;
   
   std::map < int , std::vector < Segment*> >::iterator it;
   
   
   for( it= _map_code_segs.begin() ; it!=_map_code_segs.end() ; it++ ){
      
      codes.insert( it->first );
      
   }
   
   
   return codes; 
   
   
   
}

