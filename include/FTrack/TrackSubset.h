#ifndef TrackSubset_h
#define TrackSubset_h


#include "ITrack.h"


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
      void addTrack( ITrack* track ){ _tracks.push_back( track ); };
      
      /** Adds tracks
       */
      void addTracks( std::vector<ITrack*> tracks ){ _tracks.insert( _tracks.end() , tracks.begin() , tracks.end() ); };
      
      /** Calculates the best subset of tracks
       */
      void calculateBestSet();
      
      /** @return the subset of the best tracks, i.e. all the tracks that were accepted
       */
      std::vector< ITrack* > getBestTrackSubset(){ return _bestSubsetTracks;} ;
      
      /** @return the tracks that got rejected (and are therefore not in the best subset)
       */
      std::vector< ITrack* > getRejectedTracks(){ return _rejectedTracks;} ;
      
      
      std::vector< ITrack* > getIncompatilbeTracks(){ return _incompatibleTracks;}
      
      std::vector< ITrack* > getCompatilbeTracks(){ return _compatibleTracks;} ;
      
   
   private:
      
      std::vector< ITrack* > _tracks;
      std::vector< ITrack* > _incompatibleTracks;
      std::vector< ITrack* > _compatibleTracks;
      std::vector< ITrack* > _bestSubsetTracks;
      std::vector< ITrack* > _rejectedTracks;
      
     
      /**
       * @return whether two tracks are compatible, i.e. are not in conflict
       */
      bool areCompatible( ITrack* trackA , ITrack* trackB );
      
   };
     
   
   
   
}


#endif

