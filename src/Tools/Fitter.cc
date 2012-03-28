#include "Fitter.h"

#include <algorithm>

#include "gear/GEAR.h"
#include "marlin/Global.h"
#include "UTIL/ILDConf.h"

#include "MarlinTrk/HelixTrack.h"

#include "FTrackILDTools.h"

using namespace MarlinTrk;




float Fitter::_bField = 3.5;


bool compare_TrackerHit_z( EVENT::TrackerHit* a, EVENT::TrackerHit* b ){
   
   return ( fabs(a->getPosition()[2]) < fabs( b->getPosition()[2]) ); //compare their z values
   
}











Fitter::Fitter( Track* track , MarlinTrk::IMarlinTrkSystem* trkSystem ): _trkSystem( trkSystem ){
   
 
   _trackerHits = track->getTrackerHits();
   
   fit();
   
   
}

Fitter::Fitter( std::vector < TrackerHit* > trackerHits , MarlinTrk::IMarlinTrkSystem* trkSystem ): _trkSystem( trkSystem ){
   
   _trackerHits = trackerHits;
   
   fit();
   
}


void Fitter::fit()throw( FitterException ){
   
   //create the MarlinTrk
   _marlinTrk = _trkSystem->createTrack();
   
   
   /**********************************************************************************************/
   /*       Add the hits to the MarlinTrack                                                      */
   /**********************************************************************************************/
   
   // hits are in reverse order 
   std::sort( _trackerHits.begin(), _trackerHits.end(), compare_TrackerHit_z );
   // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
   // So the direction of the hits when following the index from 0 on is:
   // from inside out: from the IP into the distance. 
   // (It is important to keep in mind, in which direction we fit, when using MarlinTrk)
   
   EVENT::TrackerHitVec::iterator it;
   
   
   unsigned number_of_added_hits = 0;
   unsigned ndof_added = 0;
   std::vector< TrackerHit* > added_hits;
   
   for( it = _trackerHits.begin() ; it != _trackerHits.end() ; ++it ) {
      
      TrackerHit* trkHit = *it;
      bool isSuccessful = false; 
      
      if( BitSet32( trkHit->getType() )[ UTIL::ILDTrkHitTypeBit::COMPOSITE_SPACEPOINT ]   ){ //it is a composite spacepoint
         
         //Split it up and hits to the MarlinTrk
         const LCObjectVec rawObjects = trkHit->getRawHits();                    
         std::vector< TrackerHit* > rawHits;
         for( unsigned k=0; k<rawObjects.size(); k++ ) rawHits.push_back( dynamic_cast< TrackerHit* >( rawObjects[k] ) );
         std::sort( rawHits.begin(), rawHits.end(), FTrackILD::compare_TrackerHit_z );
         
         
         
         for( unsigned k=0; k< rawHits.size(); k++ ){
            
            
            if( _marlinTrk->addHit( rawHits[k] ) == IMarlinTrack::success ){
               
               isSuccessful = true; //if at least one hit from the spacepoint gets added
               ++ndof_added; // 1 degree of freedom for each strip hit
            }
            
         }
         
      }
      else { // normal non composite hit
         
         if (_marlinTrk->addHit( trkHit ) == 0) {
            isSuccessful = true;
            ndof_added += 2;
         }
      }
      
      if (isSuccessful) {
         added_hits.push_back(trkHit);
         ++number_of_added_hits;
      }
      else{
         streamlog_out(DEBUG2) << "Fitter::fit(): Hit " << it - _trackerHits.begin() << " Dropped " << std::endl;          
      }
      
   }
   
   if( ndof_added < 8 ) {
      
      std::stringstream s;
      s << "Fitter::fit(): Cannot fit less with less than 8 degrees of freedom. Number of hits =  " << number_of_added_hits << " ndof = " << ndof_added << "\n";
      
      throw FitterException( s.str() );
      
   }
   
   /**********************************************************************************************/
   /*       Create a helix from the first, last and middle hit                                   */
   /**********************************************************************************************/
   
   
   // initialise with space-points not strips 
   // make a helix from 3 hits to get a trackstate
   const double* x1 = added_hits[0]->getPosition();
   const double* x2 = added_hits[ added_hits.size()/2 ]->getPosition();
   const double* x3 = added_hits.back()->getPosition();
   
   HelixTrack helixTrack( x1, x2, x3, _bField, IMarlinTrack::backward );
   
   helixTrack.moveRefPoint(0.0, 0.0, 0.0);
   
   const float referencePoint[3] = { helixTrack.getRefPointX() , helixTrack.getRefPointY() , helixTrack.getRefPointZ() };
   
   
   /**********************************************************************************************/
   /*       Create a TrackStateImpl from the helix values and use it to initalise the fit        */
   /**********************************************************************************************/
   
   EVENT::FloatVec covMatrix;
   
   covMatrix.resize(15);
   
   for (unsigned icov = 0; icov<covMatrix.size(); ++icov) {
      covMatrix[icov] = 0;
   }
   
   covMatrix[0]  = ( 1.e4 ); //sigma_d0^2
   covMatrix[2]  = ( 1.e4 ); //sigma_phi0^2
   covMatrix[5]  = ( 1.e4 ); //sigma_omega^2
   covMatrix[9]  = ( 1.e4 ); //sigma_z0^2
   covMatrix[14] = ( 1.e4 ); //sigma_tanl^2
   
   
   TrackStateImpl trackState( TrackState::AtOther, 
                              helixTrack.getD0(), 
                              helixTrack.getPhi0(), 
                              helixTrack.getOmega(), 
                              helixTrack.getZ0(), 
                              helixTrack.getTanLambda(), 
                              covMatrix, 
                              referencePoint) ;
                              
   _marlinTrk->initialise( trackState, _bField, IMarlinTrack::backward ) ;
   
   //     _marlinTrk->initialise( IMarlinTrack::backward ) ;
   
   /**********************************************************************************************/
   /*       Do the fit                                                                           */
   /**********************************************************************************************/
   
   int fit_status = _marlinTrk->fit() ; 
   
   if( fit_status != IMarlinTrack::success ){ 
      
      std::stringstream s;
      s << "Fitter::fit(): MarlinTrk->fit() wasn't successful, fit_status = " << fit_status << "\n";
    
   }
   
   
   // fitting finished get hits in the fit for safety checks:
   
   std::vector<std::pair<EVENT::TrackerHit*, double> > hits_in_fit;
   
   // remember the hits are ordered in the order in which they were fitted
   // here we are fitting inwards so the first is the last and vice verse
   
   _marlinTrk->getHitsInFit(hits_in_fit);
   
   if( hits_in_fit.size() < 3 ) {
      
      
      std::stringstream s;
      s << "Fitter::fit() Less than 3 hits in fit: Only " << hits_in_fit.size() << 
      " of " << _trackerHits.size() << " hits\n";
      
      throw FitterException( s.str() );
      
   }
   EVENT::TrackerHit* first_hit_in_fit = hits_in_fit.back().first;
   if (!first_hit_in_fit) {
      throw FitterException( std::string("Fitter::fit(): TrackerHit pointer to first hit == NULL ")  ) ;
   }
   
   
   EVENT::TrackerHit* last_hit_in_fit = hits_in_fit.front().first;
   if (!last_hit_in_fit) {
      throw FitterException( std::string("Fitter::fit(): TrackerHit pointer to last hit == NULL ")  ) ;
   }
   
  

   return;

   
}
   
   
   const TrackState* Fitter::getTrackState( int trackStateLocation )throw( FitterException ){
 
   
   return getTrackStatePlus( trackStateLocation )->getTrackState();
   
}

double Fitter::getChi2Prob( int trackStateLocation )throw( FitterException ){
   
   
   return ROOT::Math::chisquared_cdf_c( getChi2( trackStateLocation ) , getNdf( trackStateLocation ) );   
   
}
   

double Fitter::getChi2( int trackStateLocation )throw( FitterException ){
   
   
   return getTrackStatePlus( trackStateLocation )->getChi2();
   
}
   
   int Fitter::getNdf( int trackStateLocation )throw( FitterException ){
   
   
   return getTrackStatePlus( trackStateLocation )->getNdf();
   
}
   
   
const TrackStatePlus* Fitter::getTrackStatePlus( int trackStateLocation )throw( FitterException ){
   
   
   // check if there is already an entry with this trackState location
   for( unsigned i=0; i<_trackStatesPlus.size(); i++ ){
      
      if( _trackStatesPlus[i]->getTrackState()->getLocation() == trackStateLocation ){
         
         return _trackStatesPlus[i];
         
      }
      
   }
   
   // If we reach this point, obviously no trackState with the given location has been created so far
   // Thus we create it now
   TrackStateImpl* trackStateImpl = new TrackStateImpl;
   int return_code = 0;
   double chi2;
   int ndf;
   switch( trackStateLocation ){
      
      
      
      case lcio::TrackState::AtIP:{
         
         
         const gear::Vector3D point(0.,0.,0.); // nominal IP
         
         
         return_code = _marlinTrk->propagate(point, *trackStateImpl, chi2, ndf ) ;
         
         if (return_code != MarlinTrk::IMarlinTrack::success ) {
            
            
            delete trackStateImpl;
            
            std::stringstream s;
            s << "Fitter::getTrackStatePlus(): Couldn't create TrackState at IP, return code from propagation = " << return_code << "\n";
            throw FitterException( s.str() );
            
            break;
         }
         else{
            
            trackStateImpl->setLocation( trackStateLocation );
            TrackStatePlus* trackStatePlus = new TrackStatePlus( trackStateImpl, chi2, ndf );
            _trackStatesPlus.push_back( trackStatePlus );
            return trackStatePlus;
            
         }
         
         
      }
         
         
      case lcio::TrackState::AtFirstHit:{
         
         
         std::vector<std::pair<EVENT::TrackerHit*, double> > hits_in_fit;
         
         // remember the hits are ordered in the order in which they were fitted
         // here we are fitting inwards so the first is the last and vice verse
         _marlinTrk->getHitsInFit(hits_in_fit);
         
         EVENT::TrackerHit* first_hit_in_fit = hits_in_fit.back().first;
         
         
         return_code = _marlinTrk->getTrackState(first_hit_in_fit, *trackStateImpl, chi2, ndf ) ;
         
         if(return_code !=MarlinTrk::IMarlinTrack::success){
            
            delete trackStateImpl;
            
            std::stringstream s;
            s << "Fitter::getTrackStatePlus(): Couldn't create TrackState at first hit, return code from propagation = " << return_code << "\n";
            throw FitterException( s.str() );
            
            break;
         }
         else{
            
            trackStateImpl->setLocation( trackStateLocation );
            TrackStatePlus* trackStatePlus = new TrackStatePlus( trackStateImpl, chi2, ndf );
            _trackStatesPlus.push_back( trackStatePlus );
            return trackStatePlus;
            
         }
         
      }
         
         
         
      case lcio::TrackState::AtLastHit:{
         
         
         std::vector<std::pair<EVENT::TrackerHit*, double> > hits_in_fit;
         _marlinTrk->getHitsInFit(hits_in_fit);
         
         EVENT::TrackerHit* last_hit_in_fit = hits_in_fit.front().first;
         
         
         return_code = _marlinTrk->getTrackState(last_hit_in_fit, *trackStateImpl, chi2, ndf ) ;
         
         if(return_code !=MarlinTrk::IMarlinTrack::success){
            
            
            delete trackStateImpl;
            
            std::stringstream s;
            s << "Fitter::getTrackStatePlus(): Couldn't create TrackState at last hit, return code from propagation = " << return_code << "\n";
            throw FitterException( s.str() );
            
            break;
         }
         else{
            
            trackStateImpl->setLocation( trackStateLocation );
            TrackStatePlus* trackStatePlus = new TrackStatePlus( trackStateImpl, chi2, ndf );
            _trackStatesPlus.push_back( trackStatePlus );
            return trackStatePlus;
            
         }
         break;
         
      }
         
         
      case lcio::TrackState::AtCalorimeter:{
         
         
         std::vector<std::pair<EVENT::TrackerHit*, double> > hits_in_fit;
         _marlinTrk->getHitsInFit(hits_in_fit);
         
         EVENT::TrackerHit* last_hit_in_fit = hits_in_fit.front().first;
         
         
         UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string ) ; 
         encoder.reset() ;  // reset to 0
         
         encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::ECAL ;
         encoder[lcio::ILDCellID0::side] = lcio::ILDDetID::barrel;
         encoder[lcio::ILDCellID0::layer]  = 0 ;
         
         int detElementID = 0;
         return_code = _marlinTrk->propagateToLayer(encoder.lowWord(), last_hit_in_fit, *trackStateImpl, chi2, ndf, detElementID, IMarlinTrack::modeForward ) ;
         
         if (return_code == MarlinTrk::IMarlinTrack::no_intersection ) { // try forward or backward
            
            
            const TrackState* trkStateLastHit = getTrackStatePlus( lcio::TrackState::AtLastHit )->getTrackState();
            
            if (trkStateLastHit->getTanLambda()>0) {
               encoder[lcio::ILDCellID0::side] = lcio::ILDDetID::fwd;
            }
            else{
               encoder[lcio::ILDCellID0::side] = lcio::ILDDetID::bwd;
            }
            return_code = _marlinTrk->propagateToLayer(encoder.lowWord(), last_hit_in_fit, *trackStateImpl, chi2, ndf, detElementID, IMarlinTrack::modeForward ) ;
         }
            
            
         if(return_code !=MarlinTrk::IMarlinTrack::success){
            
            delete trackStateImpl;
            
            std::stringstream s;
            s << "Fitter::getTrackStatePlus(): Couldn't create TrackState at Calorimeter, return code from propagation = " << return_code << "\n";
            throw FitterException( s.str() );
            
            break;
         }
         else{
            
            trackStateImpl->setLocation( trackStateLocation );
            TrackStatePlus* trackStatePlus = new TrackStatePlus( trackStateImpl, chi2, ndf );
            _trackStatesPlus.push_back( trackStatePlus );
            return trackStatePlus;
            
         }
         
      }
         
         
      default:{
         
         
         std::stringstream s;
         s << "Creation of a trackState for the given location " << trackStateLocation 
         << " is not yet implemented for the class Fitter. \nImplemented are: AtIP, AtFirstHit, AtLastHit, AtCalorimeter.\n"
         << "If another location is desired, it must be implemented in the method Fitter::getTrackStatePlus.\n";
         
         throw FitterException( s.str() );
         return NULL;
         
         
      }
      
   }
   
   
   
   return NULL; // if we haven't returned so far, there was no success, so we return NULL
   
   
   
}
   
   

