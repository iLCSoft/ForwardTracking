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
#include <EVENT/TrackerHit.h>

namespace FTrack{

   class Segment {
   
   
      public:
         
         Segment( std::vector <EVENT::TrackerHit*> trackerHits);
         Segment( EVENT::TrackerHit* trackerHit);
         
         
         bool deleteParent ( Segment* delParent );
         bool deleteChild ( Segment* delChild );
         
   
         //TODO this should be private, but for the moment and for testing, this is kinda hardcoded
         std::vector <Segment*> _children; 
         std::vector <Segment*> _parents;
         
         
   
         std::vector <EVENT::TrackerHit*> _trackerHits;
         
         std::vector<int> _state;
      
   };


} //end of FTrack Namespace

#endif

