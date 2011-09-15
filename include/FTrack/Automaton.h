#ifndef Automaton_h
#define Automaton_h

#include <vector>
#include "Segment.h"
#include "ICriteria.h"

//TODO: should I really place the track candiate collector in here. maybe another place was better
#include "EVENT/Track.h"

namespace FTrack{
   
   
   
 
   class Automaton{
      
      
     
   public:
      
      std::vector < std::vector < Segment* > > segment() {return _segments;};
      
      void addSegment ( Segment* segment );
      
      /**Lengthens the segments by one via adding the first hit of the next segment it is connected to
       * to it.
       * And also connects those longer segments with each other.
       * Segments that don't have connected segments to use to get longer, they will die here. 
       * TODO: is this the best way. Should there be an alternative? Maybe in another method?
       */
      void lengthenSegments();
      
      void addCriteria ( ICriteria* criteria ){ _criteria.push_back( criteria ); };
      void clearCriteria() { _criteria.clear(); };
      
      /** Does the automaton search for neighbors and changes the states. TODO: explain
       */
      void doAutomaton();
      
      /**
       * erases all segments that don't have state corresponding to their layer
       */
      void cleanBadStates();
      
      /**
       * erase all connections between segments, that don't satisfy the criteria
       */
      void cleanBadConnections();
      
      /**Resets all the states to 0
       */
      void resetStates();
      
      
      /**returns the tracks
       */
      std::vector <Track*> getTracks();
      
      /**returns all the tracks starting from this segment
       */
      std::vector <Track*> getTracksOfSegment ( Segment* segment, std::vector< TrackerHit*> hits );
      
   private:
      
      /** Here the segments are stored.
       * The first index corresponds to the layer.
       * The second corresponds to the index of the segment.
       */
      std::vector < std::vector < Segment* > > _segments;
      
      std::vector < ICriteria* > _criteria;
      
      
      
      
      
   };  
   
   
   
   
   
}






#endif