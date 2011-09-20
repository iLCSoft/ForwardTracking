#include "FTDTrackFitter.h"

#include "lcio.h"
#include <algorithm>
#include "marlin/VerbosityLevels.h"
#include "IMPL/TrackImpl.h"
#include "FTrackTools.h"


using namespace lcio;
using namespace FTrack;
using namespace MarlinTrk;


 




FTDTrackFitter::FTDTrackFitter(){
   
 
   _MSOn = true;
   _ElossOn = true;
   _SmoothOn = false;
   
   
   
}




void FTDTrackFitter::initialise( const std::string& systemType,  
                                 const gear::GearMgr* mgr , 
                                 const std::string& options ) {
   
   
   // set upt the geometry
   _trkSystem =  MarlinTrk::Factory::createMarlinTrkSystem( systemType , mgr , options ) ;
   
   // set the options   
   _trkSystem->setOption( IMarlinTrkSystem::CFG::useQMS,        _MSOn ) ;       //multiple scattering
   _trkSystem->setOption( IMarlinTrkSystem::CFG::usedEdx,       _ElossOn) ;     //energy loss
   _trkSystem->setOption( IMarlinTrkSystem::CFG::useSmoothing,  _SmoothOn) ;    //smoothing
   
   // initialise the tracking system
   _trkSystem->init() ;
   
   
   
}


std::vector < EVENT::Track* > FTDTrackFitter::getFittedTracks(){
   
   
   std::vector <Track*> fittedTracks;
   
   unsigned nInitFailed = 0;
   unsigned nFitFailed = 0;
   unsigned nPropagationFailed = 0;
   unsigned nSuccess = 0;
   
   
   // loop over the input tacks and refit
   for( unsigned int i=0; i< _tracks.size() ; ++i){
      
      
      Track* track = _tracks[i];
      
      //Create a new MarlinTrack
      MarlinTrk::IMarlinTrack* marlin_trk = _trkSystem->createTrack();
      
      EVENT::TrackerHitVec trkHits = track->getTrackerHits() ;        
      
      // sort the hits
      sort( trkHits.begin(), trkHits.end(), compare_z );
      
      // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
      // So the direction of the hits when following the index from 0 on is:
      // from inside out: from the IP into the distance. 
      // (It is important to keep in mind, in which direction we fit, when using MarlinTrk)
      
      
      streamlog_out (DEBUG2) << "\n Next fitting a track with " << trkHits.size() << " hits\n";
      
      EVENT::TrackerHitVec::iterator it = trkHits.begin();
      
      //add hits to the MarlinTrk (in the same sorted order --> marlin_trk is from inside to the outside as well
      for( it = trkHits.begin() ; it != trkHits.end() ; ++it )
      {
         
         marlin_trk->addHit(*it);
         
      }


      int init_status = marlin_trk->initialise( IMarlinTrack::backward ) ; 
      //Here we first needed the order. We initialize the track to be fitted from outside in
      // As our track is sorted from inside out, this means, that we go backwards 
      //(compared to the order the hits are stored in the track)

      
      if (init_status == 0){ //Initialisation worked
               
         int fit_status = marlin_trk->fit() ; //fit the track
                  
                  
         if( fit_status == 0 ){ //Fitting worked

                        
            double chi2 = 0.;
            int ndf = 0;
            const gear::Vector3D IP(0.,0.,0.); // nominal IP


            // propagate to the IP
            TrackStateImpl* trkState = new TrackStateImpl() ;
            int propagate_status = marlin_trk->propagate(IP, *trkState, chi2, ndf ) ;

            
            if ( propagate_status == 0 ) { //Propagation worked
               
               IMPL::TrackImpl* refittedTrack = new IMPL::TrackImpl();
               
               // Set the track state, chi2 and Ndf
               refittedTrack->addTrackState(trkState);
               refittedTrack->setChi2(chi2) ;
               refittedTrack->setNdf(ndf) ;
               
               // Add all the hits to the track
               for( it = trkHits.begin() ; it != trkHits.end() ; ++it )
               {
                  
                  refittedTrack->addHit(*it);
                  
               }
               
               
               //TODO: i skipped the whole relation stuff (see RefitProcessor) here, maybe I should add it
               
               
               unsigned size_of_vec = track->getSubdetectorHitNumbers().size() ;
               refittedTrack->subdetectorHitNumbers().resize(size_of_vec) ;
               for ( unsigned detIndex = 0 ;  detIndex < size_of_vec ; detIndex++ ) 
               {
                  refittedTrack->subdetectorHitNumbers()[detIndex] = track->getSubdetectorHitNumbers()[detIndex] ;
               }
               
               fittedTracks.push_back( refittedTrack );
               
               nSuccess++;
               
            }
            else nPropagationFailed++;
            
         }
         else nFitFailed++;
         
      }
      else nInitFailed++;





      delete marlin_trk;

   }


   streamlog_out (MESSAGE) << "\n Fitted " << nSuccess << " tracks. There also were:\n "
                          << nInitFailed << " failed initialisations, " << nFitFailed << " failed fits and " 
                          << nPropagationFailed << " failed propagations.\n";
   

   return fittedTracks;
   
   
   
}


