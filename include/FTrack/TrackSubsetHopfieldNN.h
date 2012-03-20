#ifndef TrackSubsetHopfieldNN_h
#define TrackSubsetHopfieldNN_h

#include "ITrackSubset.h"



namespace FTrack {
 
   /** A class to get the best subset of tracks with a Hopfield Neural Network
    * 
    */
   class TrackSubsetHopfieldNN : public ITrackSubset{
      
   public:
      
     
      void virtual calculateBestSet();
      
      
      std::vector< ITrack* > getIncompatilbeTracks(){ return _incompatibleTracks;}
      
      std::vector< ITrack* > getCompatibleTracks(){ return _compatibleTracks;} ;
      
      virtual ~TrackSubsetHopfieldNN(){ /*no_op*/ }
      
      
   private:
      
      std::vector< ITrack* > _incompatibleTracks;
      std::vector< ITrack* > _compatibleTracks;
      
     
      /**
       * @return whether two tracks are compatible, i.e. are not in conflict
       */
      bool areCompatible( ITrack* trackA , ITrack* trackB );
      
   };
     
   
   
   
}


#endif

