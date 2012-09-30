#include "TrackingFeedbackProcessor.h"


#include <cmath>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <set>

#include "marlin/VerbosityLevels.h"
#include "MarlinCED.h"
#include "gear/BField.h"

#include "Tools/Fitter.h"
#include "Tools/KiTrackMarlinTools.h"



using namespace lcio ;
using namespace marlin ;



TrackingFeedbackProcessor aTrackingFeedbackProcessor ;


TrackingFeedbackProcessor::TrackingFeedbackProcessor() : Processor("TrackingFeedbackProcessor") {

    // modify processor description
   _description = "TrackingFeedbackProcessor gives feedback about the Track Search" ;


    // register steering parameters: name, description, class-variable, default value

   registerInputCollection(LCIO::TRACK,
                           "TrackCollection",
                           "Name of Track collection to check",
                           _TrackCollection,
                           std::string("ForwardTracks")); 
   
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   
   registerProcessorParameter("TableFileName",
                              "Name of the table file for saving the results ",
                              _tableFileName,
                              std::string("TrackingFeedback.csv") );   
   
   registerProcessorParameter("CutPtMin",
                              "The minimum transversal momentum pt above which tracks are of interest in GeV ",
                              _cutPtMin,
                              double (0.1)  );   
   
   registerProcessorParameter("CutDistToIPMax",
                              "The maximum distance from the origin of the MCP to the IP (0,0,0)",
                              _cutDistToIPMax,
                              double (10000 ) );   
   
   registerProcessorParameter("CutChi2Prob",
                              "Tracks with a chi2 probability below this value won't be considered",
                              _cutChi2Prob,
                              double (0.005) ); 
   
   registerProcessorParameter("CutNumberOfHitsMin",
                              "The minimum number of hits a track must have",
                              _cutNHitsMin,
                              int (4)  );   
   
   registerProcessorParameter("CutNumberOfHitsMin_HitsCountOncePerLayer",
                              "Whether the hits used for the cut CutNumberOfHitsMin only count once for each layer, i.e double hits on a layer count as one",
                              _cutNHitsMin_HitsCountOncePerLayer,
                              bool (false) );
   
   registerProcessorParameter("CutThetaMin",
                              "The minimum theta of the track in deg",
                              _cutThetaMin,
                              double (0)  );
   
   registerProcessorParameter("CutThetaMax",
                              "The maximum theta of the track in deg",
                              _cutThetaMax,
                              double (91)  );
   
   registerProcessorParameter("CutFitFails",
                              "Whether to cut all tracks that fail at fitting",
                              _cutFitFails,
                              bool( false ) );
   
   
   registerProcessorParameter("MultipleScatteringOn",
                              "Use MultipleScattering in Fit",
                              _MSOn,
                              bool(true));
   
   registerProcessorParameter("EnergyLossOn",
                              "Use Energy Loss in Fit",
                              _ElossOn,
                              bool(true));
   
   registerProcessorParameter("SmoothOn",
                              "Smooth All Measurement Sites in Fit",
                              _SmoothOn,
                              bool(false));
   
   registerProcessorParameter("DrawMCPTracks",
                              "Draw the helices of the MCP (values at IP) in CED ",
                              _drawMCPTracks,
                              bool(false));
   
   registerProcessorParameter("SaveAllEventsSummary",
                              "If true the results of all events are summed up and saved in the file specified under SummaryFileName ",
                              _saveAllEventsSummary,
                              bool(false));

   registerProcessorParameter("SummaryFileName",
                              "All events are summed up and saved in this file, if SaveAllEventsSummary == true",
                              _summaryFileName,
                              std::string("TrackingFeedbackSum.csv") );   
   
   registerProcessorParameter("RootFileName",
                              "Name for the root file where the tracks are saved",
                              _rootFileName,
                              std::string("Feedback.root") );
   
   registerProcessorParameter("RateOfFoundHitsMin",
                              "More than this rate of hits of the real track must be in a reco track to be assigned",
                              _rateOfFoundHitsMin,
                              float(0.5) );
   
   registerProcessorParameter("RateOfAssignedHitsMin",
                              "More than this rate of hits of the reco track must belong to the true track to be assigned",
                              _rateOfAssignedHitsMin,
                              float(0.5) );
  
   
}



void TrackingFeedbackProcessor::init() { 

   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

  

   _nRun = 0 ;
   _nEvt = 0 ;


   _Bz = Global::GEAR->getBField().at( gear::Vector3D(0., 0., 0.) ).z();    //The B field in z direction
   
   
   if ( _drawMCPTracks ) MarlinCED::init(this) ;

   _nComplete_Sum            = 0;
   _nCompletePlus_Sum        = 0; 
   _nLost_Sum                = 0;
   _nIncomplete_Sum          = 0;
   _nIncompletePlus_Sum      = 0;
   _nGhost_Sum               = 0;
   _nFoundCompletely_Sum     = 0;
   _nTrueTracks_Sum          = 0;
   _nRecoTracks_Sum          = 0;
   _nDismissedTrueTracks_Sum = 0; 
   
   
   /**********************************************************************************************/
   /*       Initialise the MarlinTrkSystem, needed by the tracks for fitting                     */
   /**********************************************************************************************/
   
   // set upt the geometry
   _trkSystem =  MarlinTrk::Factory::createMarlinTrkSystem( "KalTest" , marlin::Global::GEAR , "" ) ;
   
   if( _trkSystem == 0 ) throw EVENT::Exception( std::string("  Cannot initialize MarlinTrkSystem of Type: ") + std::string("KalTest" )  ) ;
   
   
   // set the options   
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useQMS,        _MSOn ) ;       //multiple scattering
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::usedEdx,       _ElossOn) ;     //energy loss
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing,  _SmoothOn) ;    //smoothing
   
   // initialise the tracking system
   _trkSystem->init() ;
   
   
   /**********************************************************************************************/
   /*       Prepare the root output                                                              */
   /**********************************************************************************************/
   
   _rootFile = new TFile( _rootFileName.c_str(),  "RECREATE" );
   
   _treeNameTrueTracks = "trueTracks";
   _treeTrueTracks = new TTree( _treeNameTrueTracks.c_str(), _treeNameTrueTracks.c_str() );
   
   _treeNameRecoTracks = "recoTracks";
   _treeRecoTracks = new TTree( _treeNameRecoTracks.c_str(), _treeNameRecoTracks.c_str() );
   
   makeRootBranches();
   
   

   
}


void TrackingFeedbackProcessor::processRunHeader( LCRunHeader* run) { 

   _nRun++;
   
} 



void TrackingFeedbackProcessor::processEvent( LCEvent * evt ) { 


//-----------------------------------------------------------------------
// Reset drawing buffer and START drawing collection
   if ( _drawMCPTracks ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
   }
//-----------------------------------------------------------------------


   _nComplete = 0;         
   _nCompletePlus = 0;     
   _nLost = 0;             
   _nIncomplete = 0;       
   _nIncompletePlus = 0;   
   _nGhost = 0;            
   _nFoundCompletely = 0;
   _nTrueTracks = 0;          
   _nRecoTracks = 0;          
   _nDismissedTrueTracks = 0; 
   
   LCCollection* col = NULL;
   
   
   try {
      
      col = evt->getCollection( _colNameMCTrueTracksRel ) ;
      
   }
   catch(DataNotAvailableException &e) {
      
      streamlog_out( ERROR ) << "Collection " <<  _colNameMCTrueTracksRel <<  " is not available!\n";     
      return;
      
   }
   
      
   int nMCTracks = col->getNumberOfElements();
   
   streamlog_out( DEBUG4 ) << "Number of MCP Track Relations: " << nMCTracks << "\n";
   
   
   /**********************************************************************************************/
   /*             Check the true tracks, if they are of interest                                 */
   /**********************************************************************************************/
   _trueTracks.clear();
   
   for( int i=0; i < nMCTracks; i++){
      
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
      if ( _drawMCPTracks ) MarlinCED::drawMCParticle( mcp, true, evt, 2, 1, 0xff000, 10, 3.5 );
      
      
      
      double chi2Prob;
      
      try{
      
      Fitter fitter( track, _trkSystem );
      chi2Prob = fitter.getChi2Prob( lcio::TrackState::AtIP );
      
      }
      catch( FitterException e ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " fit failed: " <<  e.what() << "\n";
         
         if( _cutFitFails ){
            
            streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because fit failed: " <<  e.what() << "\n";
            _nDismissedTrueTracks++;
            continue;
         }
         
      }
      
      
      //Only store the good tracks
      
      //distance to IP
      double dist= getDistToIP( mcp );
      if( dist > _cutDistToIPMax ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because it is too far from the IP. " 
            <<  "distance to IP = " << dist << ", distToIPMax = " << _cutDistToIPMax << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      
      //transversal momentum
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );
      if( pt < _cutPtMin ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because pt is too low. " 
         <<  "pt = " << pt << ", ptMin = " << _cutPtMin << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      
      //Theta
      double theta = ( 180./M_PI ) * atan( fabs( pt / mcp->getMomentum()[2] ) ) ;
      if( theta > _cutThetaMax ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because theta is too high. " 
         <<  "theta = " << theta << ", thetaMax = " << _cutThetaMax << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      if( theta < _cutThetaMin ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because theta is too low. " 
         <<  "theta = " << theta << ", thetaMin = " << _cutThetaMin << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      
      //number of hits in track
      std::vector< TrackerHit* > hitsInTrack = track->getTrackerHits();
      unsigned nHitsInTrack = getNumberOfHitsFromDifferentLayers( hitsInTrack );
//       unsigned nHitsInTrack = hitsInTrack.size();
      
      if( int( nHitsInTrack ) < _cutNHitsMin ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because it has too few hits. " 
         <<  "hits in track = " << nHitsInTrack << ", hits in Track min = " << _cutNHitsMin << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      
      //chi2 probability
      if( chi2Prob < _cutChi2Prob ){
         
         streamlog_out( DEBUG3 ) << "Monte Carlo Track " << i << " rejected, because chi2prob is too low. " 
         <<  "chi2prob = " << chi2Prob << ", chi2ProbMin = " << _cutChi2Prob << "\n";
         _nDismissedTrueTracks++;
         continue;
         
      }
      
      
      
      
      _trueTracks.push_back( new TrueTrack( track, mcp , _trkSystem ) );
      
      
      
   }
   
   _nTrueTracks = _trueTracks.size();
   
   //The restored tracks, that we want to check for how good they are
   try {
      
      col = evt->getCollection(  _TrackCollection ) ;
      
   }
   catch(DataNotAvailableException &e) {
      
      streamlog_out( ERROR ) << "Collection " <<   _TrackCollection <<  " is not available!\n";     
      return;
      
   }
   

   /**********************************************************************************************/
   /*              Check the reconstructed tracks (to what true tracks they belong)              */
   /**********************************************************************************************/
   _recoTracks.clear();
   
   if( col != NULL ){
      
      _nRecoTracks = col->getNumberOfElements()  ;
      streamlog_out( DEBUG4 ) << "Number of Reco Tracks: " << _nRecoTracks << "\n";
      
      //check all the reconstructed tracks
      for(unsigned i=0; i< _nRecoTracks ; i++){
         
         Track* track = dynamic_cast <Track*> ( col->getElementAt(i) ); 
         RecoTrack* recoTrack = new RecoTrack( track, _trkSystem );
         _recoTracks.push_back( recoTrack );
         checkTheTrack( recoTrack );
         
      }
      
      //check the relations for lost ones and completes
      for( unsigned int i=0; i < _trueTracks.size(); i++){
         if ( _trueTracks[i]->isLost() == true ) _nLost++;
         if ( _trueTracks[i]->isFoundCompletely() ==true ) _nFoundCompletely++;
      }
      
      
      
      /**********************************************************************************************/
      /*              Print various information on the true track and the related reco tracks       */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG4 ).precision (4);
      
//       for( unsigned i=0; i < _trueTracks.size(); i++ ){
//          
//          TrueTrack* trueTrack = _trueTracks[i];
//          
//          const MCParticle* mcp = trueTrack->getMCP();
//          
//          double px = mcp->getMomentum()[0];
//          double py = mcp->getMomentum()[1];
//          
//          double pt = sqrt( px*px + py*py );    
//          
//          if( pt > 10 ){
//             
//             
//             
//             streamlog_out( MESSAGE4 ) << "\n\nTrue Track " << i << "\n";
//             std::string info = trueTrack->getMCPInfo();
//             streamlog_out( MESSAGE ) << info;
//             info = trueTrack->getTrueTrackInfo();
//             streamlog_out( MESSAGE4 ) << info;
//             info = trueTrack->getFoundInfo();
//             streamlog_out( MESSAGE4 ) << info;
//             info = trueTrack->getRelatedTracksInfo();
//             streamlog_out( MESSAGE4 ) << info;
//          }
//          
//       }
      
      for( unsigned i=0; i < _trueTracks.size(); i++ ){
       
         
         TrueTrack* trueTrack = _trueTracks[i];
         
         streamlog_out( DEBUG4 ) << "\n\nTrue Track " << i << "\n";
         std::string info = trueTrack->getMCPInfo();
         streamlog_out( DEBUG4 ) << info;
         info = trueTrack->getTrueTrackInfo();
         streamlog_out( DEBUG4 ) << info;
         info = trueTrack->getFoundInfo();
         streamlog_out( DEBUG4 ) << info;
         info = trueTrack->getRelatedTracksInfo();
         streamlog_out( DEBUG4 ) << info;
         
      }
      
      
      /**********************************************************************************************/
      /*              Print and save the summary of the feedback for this event                     */
      /**********************************************************************************************/
      
      _nComplete_Sum            += _nComplete;            
      _nCompletePlus_Sum        += _nCompletePlus;      
      _nLost_Sum                += _nLost;               
      _nIncomplete_Sum          += _nIncomplete;       
      _nIncompletePlus_Sum      += _nIncompletePlus;   
      _nGhost_Sum               += _nGhost;            
      _nFoundCompletely_Sum     += _nFoundCompletely;     
      _nTrueTracks_Sum          += _nTrueTracks;          
      _nRecoTracks_Sum          += _nRecoTracks;          
      _nDismissedTrueTracks_Sum += _nDismissedTrueTracks;
      
      //The statistics:
      
      float pLost=-1.;
      float ghostrate=-1.; 
      float efficiency = -1.;
      float pComplete=-1.;
      float pFoundCompletely=-1.;
      
      if (_nTrueTracks > 0) pLost = float(_nLost)/float(_nTrueTracks);           
      if (_nTrueTracks > 0) efficiency = 1. - pLost;
      if (_nRecoTracks > 0) ghostrate = float(_nGhost)/float(_nRecoTracks);             
      if (_nTrueTracks > 0) pComplete = float(_nComplete)/float(_nTrueTracks);    
      if (_nTrueTracks > 0) pFoundCompletely = float(_nFoundCompletely)/float(_nTrueTracks);
      
      // the data that will get stored
      std::vector< std::pair < std::string , float > > data;
      
      data.push_back( std::make_pair( "efficiency" , efficiency ) );  
      data.push_back( std::make_pair( "ghostrate" , ghostrate ) );  
      data.push_back( std::make_pair( "pLost" , pLost ) );  
      data.push_back( std::make_pair( "pComplete" , pComplete ) );  
      data.push_back( std::make_pair( "pFoundCompletely" , pFoundCompletely ) );  
      data.push_back( std::make_pair( "nComplete" , _nComplete ) );  
      data.push_back( std::make_pair( "nCompletePlus" , _nCompletePlus ) );  
      data.push_back( std::make_pair( "nLost" , _nLost ) );  
      data.push_back( std::make_pair( "nIncomplete" , _nIncomplete ) );  
      data.push_back( std::make_pair( "nIncompletePlus" , _nIncompletePlus ) );  
      data.push_back( std::make_pair( "nGhost" , _nGhost ) );  
      data.push_back( std::make_pair( "nFoundCompletely" , _nFoundCompletely ) );  
      data.push_back( std::make_pair( "nTrueTracks" , _nTrueTracks ) );  
      data.push_back( std::make_pair( "nRecoTracks" , _nRecoTracks ) );  
      data.push_back( std::make_pair( "nDismissedTrueTracks" , _nDismissedTrueTracks ) );  

      streamlog_out( MESSAGE0 ) << "\n\n";
      for( unsigned i=0; i<data.size(); i++ ) streamlog_out( MESSAGE0 ) << data[i].first << "= " << data[i].second << "\n";      
      streamlog_out( MESSAGE0 ) << "\n";


      
      std::ofstream myfile;
      myfile.open (_tableFileName.c_str() , std::ios::app);
      
      if (isFirstEvent()) myfile << "\n";
      myfile<< "\n";
      for( unsigned i=0; i<data.size(); i++ ) myfile << data[i].first << "\t" << data[i].second << "\t\t";   
      myfile.close();

      
      /**********************************************************************************************/
      /*              Save the root information                                                     */
      /**********************************************************************************************/
      
      saveRootInformation();
      
      
      
 
   }
   
  
   if ( _drawMCPTracks )   MarlinCED::draw(this); 


   for( unsigned int k=0; k < _trueTracks.size(); k++) delete _trueTracks[k];
   _trueTracks.clear();
   for( unsigned int k=0; k < _recoTracks.size(); k++) delete _recoTracks[k];
   _recoTracks.clear();   

   _nEvt ++ ;
}



void TrackingFeedbackProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackingFeedbackProcessor::end(){ 


   
 
   if( _saveAllEventsSummary ){
      
      std::ofstream myfile;
      myfile.open ( _summaryFileName.c_str() , std::ios::app);
      
      
      double efficiency = double( _nTrueTracks_Sum - _nLost_Sum ) / double( _nTrueTracks_Sum );
      double ghostrate = double( _nGhost_Sum ) / double( _nRecoTracks_Sum );
      
      
      myfile << "\n";
      myfile << "Efficiency\t" << efficiency << "\t\t";
      myfile << "ghostrate\t"  << ghostrate  << "\t\t";
      
      
      
      myfile.close();
      
   }   
   
   _rootFile->Write();
   _rootFile->Close();
   delete _rootFile;
   

}


double TrackingFeedbackProcessor::getDistToIP( MCParticle* mcp ){
   
   double dist = sqrt(mcp->getVertex()[0]*mcp->getVertex()[0] + 
   mcp->getVertex()[1]*mcp->getVertex()[1] + 
   mcp->getVertex()[2]*mcp->getVertex()[2] );

   return dist;
   
}

 
 
 
 void TrackingFeedbackProcessor::checkTheTrack( RecoTrack* recoTrack ){ 
 
   
   const Track* track = recoTrack->getTrack();
   std::vector <TrackerHit*> hitVec = track->getTrackerHits();
   unsigned nHitsTrack = hitVec.size();   //number of hits of the reconstructed track
   
   std::vector<TrueTrack*> relatedTrueTracks; //to contain all the true tracks relations that correspond to the hits of the track
                                          //if for example a track consists of 3 points from one true track and two from another
                                          //at the end this vector will have five entries: 3 times one true track and 2 times the other.
                                          //so at the end, all we have to do is to count them.

   for( unsigned int j=0; j < hitVec.size(); j++ ){ //over all hits in the track
      
      
      for( unsigned int k=0; k < _trueTracks.size(); k++){ //check all relations if they correspond 
         
         
         const Track* trueTrack = _trueTracks[k]->getTrueTrack();
         
         if ( find (trueTrack->getTrackerHits().begin() , trueTrack->getTrackerHits().end() , hitVec[j] ) 
            !=  trueTrack->getTrackerHits().end())      // if the hit is contained in this truetrack
         relatedTrueTracks.push_back( _trueTracks[k] );     //add the track (i.e. its relation) to the vector relatedTrueTracks
      }
      
   } 


   // After those two for loops we have the vector relatedTrueTracks filled with all the true tracks (relations) that correspond
   // to the hits in our reconstructed track. 
   // Ideally this vector would only consist of the same true track again and again, i.e. every hit from the reconstructed
   // track comes from the true hit.
   //
   // Now we need to find out to what true track the reconstructed belongs or if it doesn't belong to any true track
   // at all (a ghost).
   
   unsigned nHitsFromAssignedTrueTrack = 0;
   TrueTrack* assignedTrueTrack = getAssignedTrueTrack( relatedTrueTracks , nHitsFromAssignedTrueTrack );
   streamlog_out( DEBUG3 ) << "Assigned true track = " << assignedTrueTrack << "\n";


   if ( assignedTrueTrack == NULL ){    // no true track could be assigned --> a ghost track
      _nGhost++;                           
     
   }
   else{                                   // assigned to a true track
      
      unsigned nHitsTrueTrack = assignedTrueTrack->getTrueTrack()->getTrackerHits().size();
      
      TrackType trackType;
      
      
      if (nHitsFromAssignedTrueTrack < nHitsTrueTrack){    // there are too few good hits,, something is missing --> incomplete
         
         if (nHitsFromAssignedTrueTrack < nHitsTrack){       // besides the hits from the true track there are also additional ones-->
            _nIncompletePlus++;                 // incomplete with extra points
            trackType = INCOMPLETE_PLUS;
         }   
         else{                                   // the hits from the true track fill the entire track, so its an
            _nIncomplete++;                      // incomplete with no extra points
            trackType = INCOMPLETE;
         }
      
      }
      else{                                  // there are as many good hits as there are hits in the true track
                                             // i.e. the true track is represented entirely in this track
         
         if (nHitsFromAssignedTrueTrack < nHitsTrack){        // there are still additional hits stored in the track, it's a
            _nCompletePlus++;                                 // complete track with extra points
            trackType = COMPLETE_PLUS;
         }
         else{                                                  // there are no additional points, finally, this is the perfect
            _nComplete++;                                       // complete track
            trackType= COMPLETE;
            
         }
      }
      
      recoTrack->setType( trackType );      
      
      // we want the true track to know all reconstructed tracks and vice versa
      assignedTrueTrack->addRecoTrack( recoTrack );
      recoTrack->addTrueTrack( assignedTrueTrack );
      
      
   }   
 
 
}


TrueTrack* TrackingFeedbackProcessor::getAssignedTrueTrack( std::vector<TrueTrack*> relatedTrueTracks , unsigned& nHitsFromAssignedTrueTrack ){

   TrueTrack* assignedTrueTrack = NULL;    //the true track most represented in the track 
   
   sort ( relatedTrueTracks.begin() , relatedTrueTracks.end() ); //Sorting, so all the same elements are at one place
   
   
   // Find the true track with the most hits in the reconstructed one
   
   TrueTrack* previousTT = NULL;  // used to store the previous true track
   unsigned n=0;
   unsigned nMax=0;
   
   for (unsigned j=0; j< relatedTrueTracks.size(); j++){ 
      
      if ( relatedTrueTracks[j] == previousTT ) n++;   //for every repeating element count 1. (that's the reason we sorted before, so we can just count them like this)
      else n = 1;                                      //previous was different --> we start again with 1
      
      previousTT = relatedTrueTracks[j];
      
      if (n > nMax){ //we have a new winner (a true track) with (currently) the most hits in this track
         
         nMax = n;
         assignedTrueTrack = relatedTrueTracks[j];
         
      }
      
   }
   
   
   
   if( assignedTrueTrack == NULL ) return NULL; // no track could be associated
   if( relatedTrueTracks.empty() ) return NULL; // no true tracks were passed
   
   unsigned nHitsAssignedTT = assignedTrueTrack->getTrueTrack()->getTrackerHits().size();
   if( nHitsAssignedTT == 0 )      return NULL; // assigned true track has no hits (should really not be)
   

   // Now we still might want to do some checks on the reconstructed and the true track, if we really want
   // to assign them to each other:
   
   bool assign = true;
   
   
   if( float( nMax ) / float( relatedTrueTracks.size() )  < _rateOfAssignedHitsMin ) assign = false;

   if( float( nMax ) / float( nHitsAssignedTT )  < _rateOfFoundHitsMin ) assign = false;
   
   
   if ( assign ){
      
      nHitsFromAssignedTrueTrack = nMax;
      return assignedTrueTrack;
      
   }
   
   return NULL;
   
   
}


void TrackingFeedbackProcessor::saveRootInformation(){
   
   for( unsigned i=0; i < _trueTracks.size(); i++ ){
      
      TrueTrack* trueTrack = _trueTracks[i];
      
      _trueTrack_nComplete =       trueTrack->getNumberOfTracksWithType( COMPLETE );
      _trueTrack_nCompletePlus =   trueTrack->getNumberOfTracksWithType( COMPLETE_PLUS );
      _trueTrack_nIncomplete =     trueTrack->getNumberOfTracksWithType( INCOMPLETE );
      _trueTrack_nIncompletePlus = trueTrack->getNumberOfTracksWithType( INCOMPLETE_PLUS );
      _trueTrack_nSum = _trueTrack_nComplete + _trueTrack_nCompletePlus + _trueTrack_nIncomplete + _trueTrack_nIncompletePlus;
      
      const double* p = trueTrack->getMCP()->getMomentum();
      double pt = sqrt( p[0]*p[0] + p[1]*p[1] );
      _trueTrack_pt = pt;   
      
      _trueTrack_theta = ( 180./M_PI ) * atan( fabs( pt / p[2] ) ) ;
      _trueTrack_nHits = trueTrack->getTrueTrack()->getTrackerHits().size();
      
      
      _trueTrack_vertexX = trueTrack->getMCP()->getVertex()[0];
      _trueTrack_vertexY = trueTrack->getMCP()->getVertex()[1];
      _trueTrack_vertexZ = trueTrack->getMCP()->getVertex()[2];
      
      
      try{
         
         Fitter fitter( trueTrack->getTrueTrack(), _trkSystem );
         _trueTrack_chi2prob = fitter.getChi2Prob( lcio::TrackState::AtIP );
         _trueTrack_chi2 = fitter.getChi2( lcio::TrackState::AtIP );
         _trueTrack_Ndf = fitter.getNdf( lcio::TrackState::AtIP );
         
      }
      catch( FitterException e ){
         
         _trueTrack_chi2prob = -1;
         _trueTrack_chi2 = -1;
         _trueTrack_Ndf = -1;
         
      }
      
      
      _treeTrueTracks->Fill();
      
   }
   
   for( unsigned i=0; i < _recoTracks.size(); i++ ){
      
      RecoTrack* recoTrack = _recoTracks[i];
      
      
      // |omega| = K*Bz/pt --> pt = K*Bz/ |omega|
      const double K= 0.00029979;
      const double omegaAbs = fabs( recoTrack->getTrack()->getOmega() );
      
      
      double pt = 0;
      if( omegaAbs > 0.00000001 ) pt = K*_Bz / omegaAbs; // make sure not to divide by 0 
      // ( an omega of 0.00000001 would mean a track of about 100TeV for a 3.5 Tesla field )
      
      
      
      
      
      _recoTrack_nTrueTracks = recoTrack->getTrueTracks().size();
      _recoTrack_pt = pt;
      
      _treeRecoTracks->Fill();
      
      
      try{
         
         Fitter fitter( recoTrack->getTrack(), _trkSystem );
         _recoTrack_chi2prob = fitter.getChi2Prob( lcio::TrackState::AtIP );
         _recoTrack_chi2 = fitter.getChi2( lcio::TrackState::AtIP );
         _recoTrack_Ndf = fitter.getNdf( lcio::TrackState::AtIP );
         
      }
      catch( FitterException e ){
         
         _recoTrack_chi2prob = -1;
         _recoTrack_chi2 = -1;
         _recoTrack_Ndf = -1;
         
      }
      
   }  
   
   
   
}


void TrackingFeedbackProcessor::makeRootBranches(){
   
   _treeTrueTracks->Branch( "nComplete", &_trueTrack_nComplete );
   _treeTrueTracks->Branch( "nCompletePlus", &_trueTrack_nCompletePlus );
   _treeTrueTracks->Branch( "nIncomplete", &_trueTrack_nIncomplete );
   _treeTrueTracks->Branch( "nIncompletePlus", &_trueTrack_nIncompletePlus );
   _treeTrueTracks->Branch( "nSum", &_trueTrack_nSum );
   
   
   _treeTrueTracks->Branch( "pT" , &_trueTrack_pt );
   _treeTrueTracks->Branch( "theta" , &_trueTrack_theta );
   _treeTrueTracks->Branch( "nHits" , &_trueTrack_nHits );
   _treeTrueTracks->Branch( "vertexX" , &_trueTrack_vertexX );
   _treeTrueTracks->Branch( "vertexY" , &_trueTrack_vertexY );
   _treeTrueTracks->Branch( "vertexZ" , &_trueTrack_vertexZ );
   _treeTrueTracks->Branch( "evtNr" , &_nEvt );
   _treeTrueTracks->Branch( "chi2prob" , &_trueTrack_chi2prob );
   _treeTrueTracks->Branch( "chi2" , &_trueTrack_chi2 );
   _treeTrueTracks->Branch( "Ndf" , &_trueTrack_Ndf );
   
   
   _treeRecoTracks->Branch( "nTrueTracks", &_recoTrack_nTrueTracks );
   _treeRecoTracks->Branch( "pT" , &_recoTrack_pt );
   _treeRecoTracks->Branch( "evtNr" , &_nEvt );
   _treeRecoTracks->Branch( "chi2prob" , &_recoTrack_chi2prob );
   _treeRecoTracks->Branch( "chi2" , &_recoTrack_chi2 );
   _treeRecoTracks->Branch( "Ndf" , &_recoTrack_Ndf );
//    _treeRecoTracks->Branch( "theta" , &_recoTrack_theta );   
   
}

unsigned TrackingFeedbackProcessor::getNumberOfHitsFromDifferentLayers( std::vector< TrackerHit* > hits ){
 
   
   std::set< int > layers;
   unsigned nHitsFromDifferentLayers = 0;
   
   for( unsigned i=0; i<hits.size(); i++ ){
      
      
      int layer = KiTrackMarlin::getCellID0Layer( hits[i]->getCellID0() );
      
      std::set< int >::iterator it = layers.find( layer );
      
      
      if( it == layers.end() ){ // This layer is not already used
         
         
         layers.insert( layer );
         nHitsFromDifferentLayers++;
         
      }
      
   }
   
   
   return nHitsFromDifferentLayers;  
   
   
}




