#include "FTDTrack.h"



#include <algorithm>

#include "UTIL/ILDConf.h"

// Root, for calculating the chi2 probability. 
#include "Math/ProbFunc.h"



using namespace FTrack;

/** @return if the absolute z value of hit a is bigger than that of hit b */
bool compare_IHit_z( IHit* a, IHit* b ){
   
   return ( fabs( a->getZ() ) < fabs( b->getZ() ) ); //compare their z values
   
}



FTDTrack::FTDTrack( MarlinTrk::IMarlinTrkSystem* trkSystem ){
   
   _trkSystem = trkSystem;
 
   _lcioTrack = new TrackImpl();
   
   
}

FTDTrack::FTDTrack( std::vector< IHit* > hits , MarlinTrk::IMarlinTrkSystem* trkSystem ){
   
   
   _trkSystem = trkSystem;
   
   _lcioTrack = new TrackImpl();
   
   for( unsigned i=0; i < hits.size(); i++ ){
      
      addHit( hits[i] );
      
      
   }
   
}


FTDTrack::FTDTrack( const FTDTrack& f ){

   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;
   _trkSystem = f._trkSystem;

}

FTDTrack & FTDTrack::operator= (const FTDTrack & f){
   
   
   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;
   _trkSystem = f._trkSystem;
   
   return *this;
   
}



void FTDTrack::addHit( IHit* hit ){
   
   
   
   // add the hit
   IFTDHit* ftdHit = dynamic_cast< IFTDHit* >( hit );
   
   if ( ftdHit != NULL ){
      
      _hits.push_back( ftdHit );
      
      // and sort the track again
      sort( _hits.begin(), _hits.end(), compare_IHit_z );
      
      
      _lcioTrack->addHit( ftdHit->getTrackerHit() );
      
   }
   //TODO: throw exception, if cast was not succesfull. Question: is there a better way of dealing with this?
   
}





void FTDTrack::fit() throw( FitterException ){
   
   
   Fitter fitter( _lcioTrack , _trkSystem );
   
   
   _lcioTrack->setChi2( fitter.getChi2( lcio::TrackState::AtIP ) );
   _lcioTrack->setNdf( fitter.getNdf( lcio::TrackState::AtIP ) );
   _chi2Prob = fitter.getChi2Prob( lcio::TrackState::AtIP );
   
   TrackStateImpl* trkState = new TrackStateImpl( fitter.getTrackState( lcio::TrackState::AtIP ) ) ;
   trkState->setLocation( TrackState::AtIP ) ;
   _lcioTrack->addTrackState( trkState );
   
//    //check if this trackstate is already there
//    if( _lcioTrack->getTrackState( TrackState::AtIP ) == NULL ){
//       
//       _lcioTrack->addTrackState(trkState);
//       
//    }

   /*
   //Create a new MarlinTrack
   MarlinTrk::IMarlinTrack* marlin_trk = _trkSystem->createTrack();
   
   
   EVENT::TrackerHitVec trkHits = _lcioTrack->getTrackerHits() ;
   
   //-----------------------------------------------------------------------------
   // Now we have the tracker hits, but for fitting, we need to make sure,
   // that SpacePoints get seperated into the hits they consist of.
     
   EVENT::TrackerHitVec trkHitsWithSplittedSpacePoints;
   
   for( unsigned i=0; i<trkHits.size(); i++ ){
      
      TrackerHit* trkHit = trkHits[i];
      
      if( BitSet32( trkHit->getType() )[ UTIL::ILDTrkHitTypeBit::COMPOSITE_SPACEPOINT ]  ){ //spacepoint --> split it up
         
         const LCObjectVec rawObjects = trkHit->getRawHits();
         
         for( unsigned j=0; j< rawObjects.size(); j++ ){
            
            TrackerHit* rawHit = dynamic_cast< TrackerHit* >( rawObjects[j] );
            trkHitsWithSplittedSpacePoints.push_back( rawHit );
            
         }
         
      }
      else{ //no spacepoint --> use hit as it is
         
         trkHitsWithSplittedSpacePoints.push_back( trkHit );
         
      }
    
   }
   trkHits = trkHitsWithSplittedSpacePoints; //write it back into the original vector (reason a: the name is shorter ;), reason b: then this block might be just commented out for testing)
   
   //-----------------------------------------------------------------------------
   
   // sort the hits
   sort( trkHits.begin(), trkHits.end(), FTrackILD::compare_TrackerHit_z );
   
   // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
   // So the direction of the hits when following the index from 0 on is:
   // from inside out: from the IP into the distance. 
   // (It is important to keep in mind, in which direction we fit, when using MarlinTrk)
   
   
   EVENT::TrackerHitVec::iterator it = trkHits.begin();
   
   //add hits to the MarlinTrk (in the same sorted order --> marlin_trk is from inside to the outside as well
   for( it = trkHits.begin() ; it != trkHits.end() ; ++it )
   {
      
      marlin_trk->addHit(*it);
      
   }
   
   
   int init_status = marlin_trk->initialise( MarlinTrk::IMarlinTrack::backward ) ; 
   //Here we first needed the order. We initialize the track to be fitted from outside in
   // As our track is sorted from inside out, this means, that we go backwards 
   //(compared to the order the hits are stored in the track)
   
   
   if (init_status == 0){ //Initialisation worked
      
      int fit_status = marlin_trk->fit() ; //fit the track
      
      
      if( fit_status == 0 ){ //Fitting worked
    
         
         double chi2 = 0.;
         int Ndf = 0;
         const gear::Vector3D IP(0.,0.,0.); // nominal IP
         
         
         // propagate to the IP
         TrackStateImpl* trkState = new TrackStateImpl() ;

	 trkState->setLocation( TrackState::AtIP ) ;
	 
         int propagate_status = marlin_trk->propagate(IP, *trkState, chi2, Ndf ) ;
         
         
         if ( propagate_status == 0 ) { //Propagation worked
            
          
            // Set the track state, chi2 and Ndf
            
            
            //check if this trackstate is already there
            if( _lcioTrack->getTrackState( TrackState::AtIP ) == NULL ){
               
               _lcioTrack->addTrackState(trkState);
               
            }
            
            
            _lcioTrack->setChi2(chi2) ;
            _lcioTrack->setNdf(Ndf) ;
            
            
            _chi2Prob = ROOT::Math::chisquared_cdf_c( chi2 , Ndf );
            
            
            
            
         }
         
      }
    
   }

   delete marlin_trk;
  */
 
   
   
}


double FTDTrack::getQI() const{
  
   
   double QI = _chi2Prob * _hits.size() / 14.; // 14 for the largest possible number of hits on a FTDTrack. TODO: shouldn't be hardcoded
   
   // make sure QI is between 0 and 1
   if (QI > 1. ) QI = 1.;
   if (QI < 0. ) QI = 0.;
   
   return QI;
   
}







