#include "TrackSubset.h"

#include <CLHEP/Random/RandFlat.h>
#include "marlin/VerbosityLevels.h"


#include "NeuralNet.h"

using namespace FTrack;


void TrackSubset::calculateBestSet(){
   
   unsigned nTracks = _tracks.size();
   
   // the information for the Hopfield Neural Network:
   
   std::vector < std::vector <bool> > G; // a matrix telling, if two neurons (tracks) are compatible
   G.resize( nTracks );
   for (unsigned i=0; i<nTracks; i++) G[i].resize( nTracks );
   
   std::vector < double > QI ; // the quality indicators of the neurons (tracks)
   QI.resize( nTracks );
      
   std::vector < double > states; // the initial state to start from.
   states.resize( nTracks );
   
   
   double omega = 0.5;
   
   double initStateMin = 0.;
   double initStateMax = 0.1;
   
   
   /**********************************************************************************************/
   /*                1. Find out which tracks are compatible and get the QIs                     */
   /**********************************************************************************************/
   
   
   for ( unsigned i=0; i < nTracks ; i++){ //over all tracks
      
      MyTrack* trackA = _tracks[i]; //the track we want to look at.
      
      // Get a qualitiy indicator for the track
      QI[i] = getQI( trackA );
      
      streamlog_out(DEBUG2) << "\n QI of track " << i << " = " << QI[i];
      
      
      // Set an initial state
      states[i] = CLHEP::RandFlat::shoot ( initStateMin , initStateMax ); //random ( uniformly ) values from initStateMin to initStateMax
      
      
      // Fill the states in the G matrix. (whether two tracks are compatible or not
      for ( unsigned j=i+1; j < nTracks ; j++ ){ // over all tracks that come after the current one (TODO: explain, why not previous ones too)
         
         MyTrack* trackB = _tracks[j]; // the track we check if it is in conflict with trackA
   
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
   /*                2. Let the Neural Network perform to find the best subset                   */
   /**********************************************************************************************/  
   
   NeuralNet net( G , QI , states , omega);
   
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
   /*                3. Now just sort the tracks into accepted and rejected ones                 */
   /**********************************************************************************************/  

   
   states = net.getStates();
   
   double activationMin = 0.75; // the minimal value of state to be accepted
   
   
   unsigned nAccepted=0;
   unsigned nRejected=0;
   
   for ( unsigned i=0; i < states.size(); i++ ){
      
      
      if ( states[i] >= activationMin ){
         
         _bestSubsetTracks.push_back( _tracks[i] );
         nAccepted++;
         
      }
      else{
         
         _rejectedTracks.push_back( _tracks[i] );
         nRejected++;
         
      }
      
   }
   
   
   streamlog_out( DEBUG3 ) << "\n Hopfield Neural Network found the best subset of " << nAccepted 
                           << " tracks and rejected " << nRejected << " tracks";
   
   
}


double TrackSubset::getQI( MyTrack* track ){
   
   

   
   return track->getChi2Prob();  
   
}

bool TrackSubset::areCompatible( MyTrack* trackA , MyTrack* trackB ){
   
   
   std::vector< AutHit*> hitsA = trackA->getHits();
   std::vector< AutHit*> hitsB = trackB->getHits();
   

   for( unsigned i=0; i < hitsA.size(); i++){
      
      for( unsigned j=0; j < hitsB.size(); j++){
         
         if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
         
      }
      
   }
   
   return true;
   
}



