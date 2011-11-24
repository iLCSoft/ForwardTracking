#ifndef Segment_h
#define Segment_h




#include <vector>
#include <list>
#include "IHit.h"

namespace FTrack{

   
   /** A segment is a part of a track. This track can be real or not (for example ghosttracks). 
    * It may consist of only one hit or of 2000 hits or whatever number is useful.
    * 
    * What makes it different from a Track, is that it is connected. It has: children and parents.
    * Children are segments connected to this one on lower layers and parents are segments connected
    * on higher layers.
    * 
    * This is a very useful concept, as it allows to combine them to a track or to compare different possibilities
    * a track can be formed.
    * 
    * Segments are especially used in the Automaton.
    * 
    * 
    * 
    * I will throughout all documentation use words like 1-segment and 2-segment.
    * This simply refers to the number of hits stored in it.
    * 
    * So a 1-segment simply is a hit ( an AutHit to be more specific ), but with the possibility
    * to be connected to others.
    * 
    * 
    *   
    * An important property of the segment is its state.
    * The state is a concept used in the cellular automaton and is merely an integer. At the beginning this
    * is 0. When the automaton performs its work it will rate the segments it contains and raise their states
    * if they a good. So the state is a kind of quality indicator telling how good a segment is.
    * 
    * TODO: why are there inner and outer states?
    * 
    * TODO: what is the order of the autHits? (from outside to inside -> but should this be always that way or flexible?)
    * 
    */   
   class Segment {
   
   
   public:
         
         Segment( std::vector <IHit*> hits);
         Segment( IHit* hit);
         
         
         void deleteParent ( Segment* delParent ){ _parents.remove( delParent );};
         void deleteChild ( Segment* delChild ){ _children.remove( delChild );};
         
   
         std::list <Segment*> getChildren() { return _children;};
         std::list <Segment*> getParents()  { return _parents;};
         std::vector <IHit*> getHits() {return _hits;};
         
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
         
         bool isActive(){ return _active;}
         void setActive( bool active ){ _active = active; }
     
   private:
         
         std::list <Segment*> _children; 
         std::list <Segment*> _parents;
         
         
   
         std::vector <IHit*> _hits;
         
         std::vector<int> _state;
         
         unsigned _layer;
         bool _active;
      
   };


} //end of FTrack Namespace

#endif

