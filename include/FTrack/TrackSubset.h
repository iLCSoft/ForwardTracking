#ifndef TrackSubset_h
#define TrackSubset_h


#include "MyTrack.h"


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
      void addTrack( MyTrack* track ){ _tracks.push_back( track ); };
      
      /** Adds tracks
       */
      void addTracks( std::vector<MyTrack*> tracks ){ _tracks.insert( _tracks.end() , tracks.begin() , tracks.end() ); };
      
      /** Calculates the best subset of tracks
       */
      void calculateBestSet();
      
      /** @return the subset of the best tracks, i.e. all the tracks that were accepted
       */
      std::vector< MyTrack* > getBestTrackSubset(){ return _bestSubsetTracks;} ;
      
      /** @return the tracks that got rejected (and are therefore not in the best subset)
       */
      std::vector< MyTrack* > getRejectedTracks(){ return _rejectedTracks;} ;
      
      
   
   private:
      
      std::vector< MyTrack* > _tracks;
      std::vector< MyTrack* > _bestSubsetTracks;
      std::vector< MyTrack* > _rejectedTracks;
      

      
      /**
       * @return a qualitiy indicator for the track, ranging from 0 to 1
       */
      double getQI( MyTrack* track );
     
      /**
       * @return whether two tracks are compatible, i.e. are not in conflict
       */
      bool areCompatible( MyTrack* trackA , MyTrack* trackB );
      
   };
     
   
   
   
}


#endif

