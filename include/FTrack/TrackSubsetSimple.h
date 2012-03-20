#ifndef TrackSubsetSimple_h
#define TrackSubsetSimple_h

#include "ITrackSubset.h"



namespace FTrack {
 
   /** A class to get the best subset of tracks, by taking the track with the highest Quality
    * Indicator and throwing away all incompatible tracks. Of the remaining tracks repeat the procedure. 
    * 
    */
   class TrackSubsetSimple : public ITrackSubset{
      
   public:
      
     
      void virtual calculateBestSet();
      
      virtual ~TrackSubsetSimple(){ /*no_op*/ }
      
     
   private:
      
      
     
      /**
       * @return whether two tracks are compatible, i.e. are not in conflict
       */
      bool areCompatible( ITrack* trackA , ITrack* trackB );
      
      
   };
     
   
   
   
}


#endif

