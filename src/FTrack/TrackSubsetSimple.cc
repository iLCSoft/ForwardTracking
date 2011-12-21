#include "TrackSubsetSimple.h"

#include <algorithm>
#include "marlin/VerbosityLevels.h"



using namespace FTrack;




bool compare_QI( ITrack* a, ITrack* b ){
   
   return ( a->getQI() > b->getQI() ); //compare their QI
   
}



void TrackSubsetSimple::calculateBestSet(){
   
   
   unsigned nAccepted=0;
   unsigned nRejected=0;

   
   
   std::vector< ITrack* > tracks = _tracks;

   // sort the vector from big QI to small QI
   sort( tracks.begin(), tracks.end() , compare_QI );

   
   /*The idea here is: the first track is (now that we sorted) the one with the highest
    * QI. Check all other tracks if they are complatible with it.
    * If one of them is incompatible throw it away. Once done with that, store the
    * first track as accepted (and delete it from the vector) and do it once more for
    * the new highest QI track.
    * Repeat until all tracks are stored.
    */   
   while( tracks.size() > 0 ){
      
      
      // check all tracks with smaller QI if they are compatible
      for( unsigned i=1; i < tracks.size(); i++){
         
         
         if( areCompatible( tracks[0] , tracks[i] ) == false ){ // the track is incompatible
            
            // reject it
            nRejected++;
            _rejectedTracks.push_back( tracks[i] );
            
            // and delete it from the tracks we are looking at
            tracks.erase( tracks.begin() + i );
            i--;
            
         }
         
      }
      
      // store the first track
      nAccepted++;
      _acceptedTracks.push_back( tracks[0] );
      // delete it from the current tracks
      tracks.erase( tracks.begin() );
      
   }
   
  
   
   streamlog_out( DEBUG4 ) << "\n\t TrackSubsetSimple accepted " << nAccepted 
                           << " tracks and rejected " << nRejected << " tracks of all in all " 
                           << nAccepted + nRejected << " tracks.";
   
   

}




bool TrackSubsetSimple::areCompatible( ITrack* trackA , ITrack* trackB ){
   
   
   std::vector< IHit*> hitsA = trackA->getHits();
   std::vector< IHit*> hitsB = trackB->getHits();
   

   for( unsigned i=0; i < hitsA.size(); i++){
      
      for( unsigned j=0; j < hitsB.size(); j++){
         
         if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
         
      }
      
   }
   
   return true;
   
}



