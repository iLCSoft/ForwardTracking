#ifndef TrackSubset_h
#define TrackSubset_h


#include "EVENT/Track.h"
#include "lcio.h"

using namespace lcio;

namespace FTrack {
 
   /** A class to get the best subset of tracks.
    * 
    * Uses the HopfieldNeural Network for this task.
    * 
    */
   class TrackSubset{
      
   public:
      
      /** Adds a track
       */
      void addTrack( Track* track ){ _tracks.push_back( track ); };
      
      /** Adds tracks
       */
      void addTracks( std::vector<Track*> tracks ){ _tracks.insert( _tracks.end() , tracks.begin() , tracks.end() ); };
      
      /** Calculates the best subset of tracks
       */
      void calculateBestSet();
      
      /** @return the subset of the best tracks, i.e. all the tracks that were accepted
       */
      std::vector< Track* > getBestTrackSubset(){ return _bestSubsetTracks;} ;
      
      /** @return the tracks that got rejected (and are therefore not in the best subset)
       */
      std::vector< Track* > getRejectedTracks(){ return _rejectedTracks;} ;
      
      
   
   private:
      
      std::vector< Track* > _tracks;
      std::vector< Track* > _bestSubsetTracks;
      std::vector< Track* > _rejectedTracks;
      

      
      /**
       * @return a qualitiy indicator for the track, ranging from 0 to 1
       */
      double getQI( Track* track );
     
      /**
       * @return whether two tracks are compatible, i.e. are not in conflict
       */
      bool areCompatible( Track* trackA , Track* trackB );
      
   };
     
   
   
   
}


#endif

