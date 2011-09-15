#ifndef Segment_h
#define Segment_h

/** A segment is a part of a track. This track can be real or not (ghosttacks for example)
 * 
 * It may consist of only one hit or of 2000 hits or whatever number is useful.
 * 
 * What makes it different from a Track, is that it has a 2 vectors: children and parents.
 * Via these vectors it can store which other segments it is connected to.
 * 
 * It has also a vector state, which is used in the cellular automaton approach, but may
 * as well be used for something different.
 * 
 * 
 * 
 * 
 * 
 * 
 */


#include <vector>
#include "AutHit.h"

namespace FTrack{

   class Segment {
   
   
   public:
         
         Segment( std::vector <AutHit*> autHits);
         Segment( AutHit* autHit);
         
         
         bool deleteParent ( Segment* delParent );
         bool deleteChild ( Segment* delChild );
         
   
         std::vector <Segment*> getChildren() { return _children;};
         std::vector <Segment*> getParents()  { return _parents;};
         std::vector <AutHit*> getAutHits() {return _autHits;};
         
         void addChild( Segment* child ){ _children.push_back(child); };
         void addParent( Segment* parent ){ _parents.push_back(parent); };
         
         unsigned getLayer() { return _layer; };
         void setLayer( unsigned layer ) { _layer = layer; }; 
         
         std::vector<int> getState() { return _state; }; //TODO: maybe this is better done by a more beautiful way than with just get and set
         //TODO: make a method that does simulate the skipped layers, so this needs not to be done with get and set. The vector shouldn't be exported at all!
         void setState ( std::vector<int> state ) { _state = state;};
         void raiseState() { if (_state.size() > 0) _state[0]++; };
         int getInnerState() { return _state[0];}; //TODO: this is error prone: check if state>0 and make an exception
         int getOuterState() { return _state.back();}; //TODO: --""--
         void resetState();
         
         void setSkippedLayers( int skippedLayers );
     
   private:
         
         std::vector <Segment*> _children; 
         std::vector <Segment*> _parents;
         
         
   
         std::vector <AutHit*> _autHits;
         
         std::vector<int> _state;
         
         unsigned _layer;
      
   };


} //end of FTrack Namespace

#endif

