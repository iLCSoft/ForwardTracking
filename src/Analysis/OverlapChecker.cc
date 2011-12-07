#include "OverlapChecker.h"


#include <EVENT/LCCollection.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <EVENT/Track.h>


using namespace lcio ;
using namespace marlin ;




OverlapChecker aOverlapChecker ;


OverlapChecker::OverlapChecker() : Processor("OverlapChecker") {
   
   // modify processor description
   _description = "OverlapChecker: Checks if tracks are overlapping" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   
   
   registerInputCollection(LCIO::LCRELATION,
                           "TrackCollectionName",
                           "The collection of the tracks to check",
                           _trackCollectionName,
                           std::string("ForwardTracks"));
   

   
   
   
   
}



void OverlapChecker::init() { 
   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;
   
   // usually a good idea to
   printParameters() ;
   

   
   _nRun = 0 ;
   _nEvt = 0 ;
   
   
}


void OverlapChecker::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void OverlapChecker::processEvent( LCEvent * evt ) { 
   
   
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _trackCollectionName ) ;
   
   unsigned nTracks = col->getNumberOfElements();

   std::vector < std::vector <bool> > G; // a matrix telling, if two tracks are compatible
   G.resize( nTracks );
   for (unsigned i=0; i<nTracks; i++) G[i].resize( nTracks );
   
   
   std::vector< Track* > tracks;
   
   //store the tracks in a vector
   for( unsigned i=0; i < nTracks; i++){
      
      
      Track* track = dynamic_cast <Track*> (col->getElementAt(i) );  
      if( track != NULL ) tracks.push_back( track );
      
      
   }
   
   //check their overlap
   for( unsigned i=0; i < nTracks; i++){ //over all tracks
      
      
      Track* trackA = tracks[i];  
      
      
      // Fill the the G matrix. (whether two tracks are compatible or not)
      for ( unsigned j=i+1; j < nTracks ; j++ ){ // over all tracks that come after the current one (matrix is symmetric)
         
         Track* trackB = tracks[j]; // the track we check if it is in conflict with trackA
         
         if ( areCompatible( trackA , trackB ) ){ 
            
            G[i][j] = 0;
            G[j][i] = 0;
            
         }
         else{
            
            G[i][j] = 1;
            G[j][i] = 1;            
            
         }
         
      }
      
   }
   
   //print out the matrix G
   streamlog_out( MESSAGE0 ) << "\n\nG:";
   
   for( unsigned i=0; i < G.size(); i++ ){
      
      
      streamlog_out( MESSAGE0 ) << "\n";
      
      for( unsigned j=0; j < G[i].size(); j++ ){
         
         
         streamlog_out( MESSAGE0 ) << G[i][j] << "  ";
         
      }
   }
   
   streamlog_out( MESSAGE0 ) << "\n\n";
 
   
  

   //-- note: this will not be printed if compiled w/o MARLINDEBUG4=1 !

   streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
   << "   in run:  " << evt->getRunNumber() << std::endl ;


   _nEvt ++ ;
   
   
}



void OverlapChecker::check( LCEvent * evt ) { 
   // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void OverlapChecker::end(){ 
   
   //   streamlog_out( DEBUG ) << "MyProcessor::end()  " << name() 
   //      << " processed " << _nEvt << " events in " << _nRun << " runs "
   //      << std::endl ;
   
}



bool OverlapChecker::areCompatible( Track* trackA , Track* trackB ){
   
   
   std::vector< TrackerHit*> hitsA = trackA->getTrackerHits();
   std::vector< TrackerHit*> hitsB = trackB->getTrackerHits();
   
   
   for( unsigned i=0; i < hitsA.size(); i++){
      
      for( unsigned j=0; j < hitsB.size(); j++){
         
         if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
         
      }
      
   }
   
   return true;
   
}

