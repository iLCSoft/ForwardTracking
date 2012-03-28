#include "TrackSubsetHopfieldNN.h"

#include <CLHEP/Random/RandFlat.h>
#include "marlin/VerbosityLevels.h"


#include "HopfieldNeuralNet.h"

using namespace FTrack;


void TrackSubsetHopfieldNN::calculateBestSet(){
   
   
   unsigned nAccepted=0;
   unsigned nRejected=0;
   unsigned nCompWithAll=0;
   unsigned nIncompatible=0;
   
   
   std::vector< ITrack* > tracks = _tracks;
   
   unsigned nTracks = tracks.size();
   
   // the information for the Hopfield Neural Network:
   
   std::vector < std::vector <bool> > G; // a matrix telling, if two neurons (tracks) are compatible
   G.resize( nTracks );
   for (unsigned i=0; i<nTracks; i++) G[i].resize( tracks.size() );
   
   std::vector < double > QI ; // the quality indicators of the neurons (tracks)
   QI.resize( nTracks );
      
   std::vector < double > states; // the initial state to start from.
   states.resize( nTracks );
   
   
   double omega = 0.75;
   
   double initStateMin = 0.;
   double initStateMax = 0.1;
   
   
   /**********************************************************************************************/
   /*                1. Find out which tracks are compatible and get the QIs                     */
   /**********************************************************************************************/
   
   
   for ( unsigned i=0; i < nTracks ; i++){ //over all tracks
      
      ITrack* trackA = tracks[i]; //the track we want to look at.
      
      // Get a qualitiy indicator for the track
      QI[i] = trackA->getQI();
      
      streamlog_out(DEBUG2) << "\n QI of track " << i << " = " << QI[i];
      
      
      // Set an initial state
      states[i] = CLHEP::RandFlat::shoot ( initStateMin , initStateMax ); //random ( uniformly ) values from initStateMin to initStateMax
      
      
      // Fill the states in the G matrix. (whether two tracks are compatible or not
      for ( unsigned j=i+1; j < nTracks ; j++ ){ // over all tracks that come after the current one (TODO: explain, why not previous ones too)
         
         ITrack* trackB = tracks[j]; // the track we check if it is in conflict with trackA
   
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
   
   // output of the G matrix:
   if( !G.empty() ) streamlog_out(DEBUG2) << "\nG:";
   

   for ( unsigned i=0; i < G.size(); i++ ){
      
      streamlog_out(DEBUG2) << "\n";
      
      for ( unsigned j=0; j < G[i].size(); j++ ){
         
         streamlog_out(DEBUG2) << G[i][j] << "  ";
         
      }
      
   }
   
   
   /**********************************************************************************************/
   /*                2. Save tracks, that are compatible with all others                         */
   /**********************************************************************************************/
   
   for( unsigned i=0; i < tracks.size(); i++ ){
      
      
      bool isCompatibleWithAll = true;
      
      //check if this track is compatible with all others
      for( unsigned j=0; j < tracks.size(); j++){
         
         //G[i][j] == 0 i and j are compatible, if G[i][j] == 1 i and j are incompatible
         if( (i != j) && G[i][j] ){
            
            isCompatibleWithAll = false;
            break;
            
         }
         
         
      }
      
      
      if ( isCompatibleWithAll ){ //if it is compatible with all others, we don't need the Hopfield Neural Net, we can just save it
         
         
         //add the track to the good ones
         _acceptedTracks.push_back( tracks[i] );
         nCompWithAll++;
         
         _compatibleTracks.push_back( tracks[i] );
         
         //And now erase it from the ones we will still check:
         tracks.erase( tracks.begin() + i );
         states.erase( states.begin() + i );
         QI.erase( QI.begin() + i );
         
         for( unsigned j=0; j<G.size(); j++ ) G[j].erase( G[j].begin() + i );
         G.erase( G.begin() + i );
         
         i--;
         
      }
      else{
         
         nIncompatible++;
         _incompatibleTracks.push_back( tracks[i] );
         
      }
      
   }
      
   streamlog_out( DEBUG3 ) << "\n " << nCompWithAll << " tracks are compatible with all others, " << nIncompatible
                           << " tracks are interfering and will be checked for the best subset";
   
   /**********************************************************************************************/
   /*                3. Let the Neural Network perform to find the best subset                   */
   /**********************************************************************************************/  
   
   if( !tracks.empty() ){
   
      HopfieldNeuralNet net( G , QI , states , omega);
      
      net.setT (2.1);
      net.setTInf(0.1);
      net.setLimitForStable(0.01);
      
      unsigned nIterations=1;
      
      streamlog_out(DEBUG1) << "\nstates: ( ";
      for ( unsigned int i=0; i< states.size(); i++) streamlog_out(DEBUG1) << states[i] << " "; 
      streamlog_out(DEBUG1) << ")";
      
      while ( !net.doIteration() ){ // while the Neural Net is not (yet) stable
         
         nIterations++;
         
         std::vector <double> newStates = net.getStates();
      
         streamlog_out(DEBUG1) << "\nstates: ( ";      
         
         for ( unsigned int i=0; i< newStates.size(); i++) streamlog_out(DEBUG1) << newStates[i] << " "; 
                        
         streamlog_out(DEBUG1) << ")";
         
      }
      
      
      
      streamlog_out( DEBUG3 ) << "\n Hopfield Neural Network is stable after " << nIterations << " iterations.";
      
      
      
      
      /**********************************************************************************************/
      /*                4. Now just sort the tracks into accepted and rejected ones                 */
      /**********************************************************************************************/  
      
      
      states = net.getStates();
      
      double activationMin = 0.75; // the minimal value of state to be accepted
      
      
      
      
      for ( unsigned i=0; i < states.size(); i++ ){
         
         
         if ( states[i] >= activationMin ){
            
            _acceptedTracks.push_back( tracks[i] );
            nAccepted++;
            
         }
         else{
            
            _rejectedTracks.push_back( tracks[i] );
            nRejected++;
            
         }
         
      }
      
      
      streamlog_out( DEBUG3 ) << "Hopfield Neural Network accepted " << nAccepted 
                              << " tracks and rejected " << nRejected << " tracks of all in all " 
                              << nAccepted + nRejected << "incomaptible tracks.\n";
      
      
   }
   
 
      streamlog_out( DEBUG3 )   << "So in sum " << nAccepted + nCompWithAll
                                << " tracks survived and " << nRejected << " tracks got rejected.\n";
   
}




bool TrackSubsetHopfieldNN::areCompatible( ITrack* trackA , ITrack* trackB ){
   
   
   std::vector< IHit*> hitsA = trackA->getHits();
   std::vector< IHit*> hitsB = trackB->getHits();
   

   for( unsigned i=0; i < hitsA.size(); i++){
      
      for( unsigned j=0; j < hitsB.size(); j++){
         
         if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
         
      }
      
   }
   
   return true;
   
}



