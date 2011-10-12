#include "Segment.h"

using namespace FTrack;

Segment::Segment( std::vector <AutHit*> autHits){ 

   _autHits = autHits; 
   
   _state.push_back(0); 
   
   _children.clear(); 
   _parents.clear();

}



Segment::Segment( AutHit* autHit){ 
   
   _autHits.push_back( autHit) ;
   _state.push_back(0); 
   _children.clear(); 
   _parents.clear();
   

}



void Segment::resetState(){
   
   
   for ( unsigned i = 0; i <_state.size(); i++){
      
      _state[i] = 0;
      
   }
   
}


void Segment::setSkippedLayers( int skippedLayers ){
   
   
   if (skippedLayers >= 0 ){
      
      
      _state.resize( skippedLayers + 1 );
      
   }
   else {
      
      
      //TODO: make an exception here!
      
      _state.resize(0);
      
   }
   
}
   

