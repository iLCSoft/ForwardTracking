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

bool Segment::deleteParent ( Segment* delParent ){
   
   
   
   for( unsigned int i=0; i < _parents.size(); i++){ //over all parents
      
      if ( _parents[i] == delParent ){ // if this is the parent to delete
         
         _parents.erase( _parents.begin() + i );
         
         return true;
         
      }
      
   }
   
   
   return false; //when we reached this point nothing got deleted -> return false 
}

bool Segment::deleteChild ( Segment* delChild ){
   
   

   
   for( unsigned int i=0; i < _children.size(); i++){
      
      if ( _children[i] == delChild ){
         
         _children.erase( _children.begin() + i );
        
         return true;
         
      }
      
   }
   
   
   return false;
   
   
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
   

