#include "EndcapTrack.h"


#include <algorithm>

#include "UTIL/ILDConf.h"

// Root, for calculating the chi2 probability. 
#include "Math/ProbFunc.h"



using namespace KiTrackMarlin;

// FIX ME
/** @return if the radius of hit a is bigger than that of hit b */

bool compare_IHit_R_3Dhits_EndcapTrack( IHit* a, IHit* b ){

  double r2_a = fabs((a->getX()*a->getX()) + (a->getY()*a->getY()));
  double r2_b = fabs((b->getX()*b->getX()) + (b->getY()*b->getY()));
   
   return ( r2_a < r2_b ); //compare their radii
   
}



EndcapTrack::EndcapTrack( MarlinTrk::IMarlinTrkSystem* trkSystem ){
   
   _trkSystem = trkSystem;
   _chi2Prob = 0.;
 
   _lcioTrack = new TrackImpl();
   
   
}

EndcapTrack::EndcapTrack( std::vector< IEndcapHit* > hits , MarlinTrk::IMarlinTrkSystem* trkSystem ){
   
   
   _trkSystem = trkSystem;
   _chi2Prob = 0.;
   
   _lcioTrack = new TrackImpl();
   
   for( unsigned i=0; i < hits.size(); i++ ){
      
      addHit( hits[i] );
      
      
   }
   
}





EndcapTrack::EndcapTrack( const EndcapTrack& f ){

   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;
   _trkSystem = f._trkSystem;

}

EndcapTrack & EndcapTrack::operator= (const EndcapTrack & f){
   
   if (this == &f) return *this;   //protect against self assignment
   
   //make a new copied lcio track
   _lcioTrack = new TrackImpl( *f._lcioTrack );
   
   
   _hits = f._hits;
   _chi2Prob = f._chi2Prob;
   _trkSystem = f._trkSystem;
   
   return *this;
   
}



void EndcapTrack::addHit( IEndcapHit* hit ){
   
   
   
   if ( hit != NULL ){
      
      _hits.push_back( hit );
      
      // and sort the track again
      sort( _hits.begin(), _hits.end(), compare_IHit_R_3Dhits_EndcapTrack );
      
      
      _lcioTrack->addHit( hit->getTrackerHit() );
      
   }
   
}






void EndcapTrack::fit() throw( FitterException ){
   
   
  Fitter fitter( _lcioTrack , _trkSystem , 1 );
   
   
   _lcioTrack->setChi2( fitter.getChi2( lcio::TrackState::AtIP ) );
   _lcioTrack->setNdf( fitter.getNdf( lcio::TrackState::AtIP ) );
   _chi2Prob = fitter.getChi2Prob( lcio::TrackState::AtIP );
   
   TrackStateImpl* trkState = new TrackStateImpl( *fitter.getTrackState( lcio::TrackState::AtIP ) ) ;
   trkState->setLocation( TrackState::AtIP ) ;
   _lcioTrack->addTrackState( trkState );
   
   
}


double EndcapTrack::getQI() const{
  
   
   double QI = _chi2Prob;
   
   // make sure QI is between 0 and 1
   if (QI > 1. ) QI = 1.;
   if (QI < 0. ) QI = 0.;
   
   return QI;
   
}

/*
double EndcapTrack::getPT() const{

  double Omega = _lcioTrack->getOmega();
  double PT = fabs((0.3*3.5)/(1000*Omega));

  return PT ;

}
*/



