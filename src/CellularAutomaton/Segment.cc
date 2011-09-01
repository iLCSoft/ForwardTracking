#include "Segment.h"

using namespace FTrack;

Segment::Segment( std::vector <EVENT::TrackerHit*> trackerHits){ 

   _trackerHits = trackerHits; 
   
   _state.push_back(0); 
   
   _children.clear(); 
   _parents.clear();

}



Segment::Segment( EVENT::TrackerHit* trackerHit){ 
   
   _trackerHits.push_back( trackerHit) ;
   _state.push_back(0); 
   _children.clear(); 
   _parents.clear();

}

bool Segment::deleteParent ( Segment* delParent ){
   
   
   
   for( unsigned int i=0; i < _parents.size(); i++){
      
      if ( _parents[i] == delParent ){
         
         _parents.erase( _parents.begin() + i );
         
         return true;
         
      }
      
   }
   
   
   return false;
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

