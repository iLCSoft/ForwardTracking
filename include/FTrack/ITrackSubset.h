#ifndef ITrackSubset_h
#define ITrackSubset_h


#include "ITrack.h"


namespace FTrack {
 
   /** An interface for getting a subset of tracks.
    * It receives the tracks over the methods addTrack() and addTracks().
    * Once all the tracks are loaded, via the method calculateBestSet() the accepted
    * and rejected tracks are calculated. These can then be returned with 
    * getAcceptedTracks() and getRejectedTracks().
    * 
    * An implemantation deriving from this class needs to implement the virtual method
    * calculateBestSet().
    * 
    */
   class ITrackSubset{
      
   public:
      
      /** Adds a track
       */
      void addTrack( ITrack* track ){ _tracks.push_back( track ); };
      
      /** Adds tracks
       */
      void addTracks( std::vector<ITrack*> tracks ){ _tracks.insert( _tracks.end() , tracks.begin() , tracks.end() ); };
      
      /** Calculates the best subset of tracks
       */
      void virtual calculateBestSet() = 0;
      
      /** @return the subset of the best tracks, i.e. all the tracks that were accepted
       */
      std::vector< ITrack* > getAcceptedTracks(){ return _acceptedTracks;} ;
      
      /** @return the tracks that got rejected (and are therefore not in the best subset)
       */
      std::vector< ITrack* > getRejectedTracks(){ return _rejectedTracks;} ;
      
      virtual ~ITrackSubset(){ /*no_op*/ }
      
   
   protected:
      
      std::vector< ITrack* > _tracks;
      std::vector< ITrack* > _acceptedTracks;
      std::vector< ITrack* > _rejectedTracks;
      
      
   };
     
   
   
   
}


#endif

