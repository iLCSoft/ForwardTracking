#include "TrackingFeedbackProcessor.h"




// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include "HelixClass.h"


#include <cmath>
#include <fstream>
#include "SimpleCircle.h"
#include <algorithm>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"


#include <sstream>
#include <MarlinCED.h>
#include "FTrackTools.h"
#include "FTDTrack.h"
#include "FTDHit01.h"


#include "Criteria.h"


#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include <gear/BField.h>
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"

using namespace lcio ;
using namespace marlin ;
using namespace FTrack;










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
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results (without .root) ",
                              _rootFileName,
                              std::string("FTrackFeedback.root") );
      
      
   registerProcessorParameter("PtMin",
                              "The minimum transversal momentum pt above which tracks are of interest in GeV ",
                              _ptMin,
                              double (0.2)  );   
   
   registerProcessorParameter("DistToIPMax",
                              "The maximum distance from the origin of the MCP to the IP (0,0,0)",
                              _distToIPMax,
                              double (250. ) );   
   
   registerProcessorParameter("Chi2ProbCut",
                              "Tracks with a chi2 probability below this value won't be considered",
                              _chi2ProbCut,
                              double (0.005) ); 
   
   registerProcessorParameter("NumberOfHitsMin",
                              "The minimum number of hits a track must have",
                              _nHitsMin,
                              int (4)  );   
   
   
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

}



void TrackingFeedbackProcessor::init() { 

   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

   _treeName = "trackCands";
   setUpRootFile(_rootFileName, _treeName);      //prepare the root file.

   _nRun = 0 ;
   _nEvt = 0 ;


   //Initialise the TrackFitter of the tracks:
   FTDTrack::initialiseFitter( "KalTest" , marlin::Global::GEAR , "" , _MSOn , _ElossOn , _SmoothOn  );



   //Add the criteria that will be checked
   _crits2.push_back( new Crit2_RZRatio( 1. , 1. ) ); 
   _crits2.push_back( new Crit2_StraightTrackRatio( 1. , 1. ) );
   _crits2.push_back( new Crit2_HelixWithIP( 1. , 1. ) );
   _crits2.push_back( new Crit2_DeltaRho( 0. , 0. ) );
   _crits2.push_back( new Crit2_DeltaPhi( 0. , 0. ) );
   for( unsigned i=0; i< _crits2.size(); i++ ) _crits2[i]->setSaveValues( true );

   _crits3.push_back( new Crit3_ChangeRZRatio( 1. , 1. ) );
   _crits3.push_back( new Crit3_PT (0. , 0.) );
   _crits3.push_back( new Crit3_3DAngle (0. , 0.) );
   _crits3.push_back( new Crit3_IPCircleDist (0. , 0.) );
   _crits3.push_back( new Crit3_2DAngle( 0. , 0. ) );
   for( unsigned i=0; i< _crits3.size(); i++ ) _crits3[i]->setSaveValues( true );


   _crits4.push_back( new  Crit4_2DAngleChange ( 1. , 1. ) );
   _crits4.push_back( new  Crit4_PhiZRatioChange ( 1. , 1. ) );
   _crits4.push_back( new  Crit4_DistToExtrapolation ( 1. , 1. ) );
   _crits4.push_back( new  Crit4_DistOfCircleCenters ( 1. , 1. ) );
   _crits4.push_back( new  Crit4_NoZigZag ( 1. , 1. ) );
   _crits4.push_back( new  Crit4_RChange ( 1. , 1. ) );
   for( unsigned i=0; i< _crits4.size(); i++ ) _crits4[i]->setSaveValues( true );



   unsigned int nLayers = 8; // layer 0 is for the IP
   unsigned int nModules = 1;
   unsigned int nSensors = 2; // there is at the moment only one sensor, namely sensor 1, but as usually things start with 0...
   
   
   try {
      
      const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
      const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
      streamlog_out( MESSAGE ) << "  TrackingFeedbackProcessor - Use FTDLayerLayout" << std::endl ;
      
      nLayers = 2*ftdLayers.getNLayers() + 1; //TODO: explain
      nModules = ftdLayers.getNPetals(0); // TODO: this is just taking the petals from the first disk -> should this be more general?
      
      
   } catch (gear::UnknownParameterException& e) {
      
      streamlog_out( MESSAGE ) << "  TrackingFeedbackProcessor - Use Loi style FTDParameters" << std::endl ;
      
      const gear::GearParameters& pFTD = Global::GEAR->getGearParameters("FTD");
      
      nLayers = pFTD.getDoubleVals( "FTDZCoordinate" ).size() + 1;
   }
   
   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );


//      MarlinCED::init(this) ;

}


void TrackingFeedbackProcessor::processRunHeader( LCRunHeader* run) { 

   _nRun++;
   
} 



void TrackingFeedbackProcessor::processEvent( LCEvent * evt ) { 


//-----------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

//    MarlinCED::newEvent(this , 0) ; 
// 
//    CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
// 
//    pHandler.update(evt); 

//-----------------------------------------------------------------------



   
   _nComplete = 0;         
   _nCompletePlus = 0;     
   _nLost = 0;             
   _nIncomplete = 0;       
   _nIncompletePlus = 0;   
   _nGhost = 0;            
   _nFoundCompletely = 0;
   
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();
   _myRelations.clear(); 
   streamlog_out( MESSAGE0 ) << "\nNumber of MCP Track Relations: " << nMCTracks;
   
   /**********************************************************************************************/
   /*             Check the tracks, if they are of interest                                      */
   /**********************************************************************************************/
   
   for( int i=0; i < nMCTracks; i++){
      
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
//       MarlinCED::drawMCParticle( mcp, true, evt, 2, 1, 0xff000, 10, 3.5 );
      
      streamlog_out( MESSAGE0 ) << "\nNumber of hits: " << track->getTrackerHits().size();
      
      
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );
      
      //Only store the good tracks
      if (( getDistToIP( mcp ) < _distToIPMax )&&                       //distance to IP
         ( pt > _ptMin )&&                                              //transversal momentum
         ((int) track->getTrackerHits().size() >= _nHitsMin )&&         //number of hits in track        
         ( getChi2Prob( track ) > _chi2ProbCut )){                      //chi2 probability
        
         _myRelations.push_back( new MyRelation( rel ) );
         
      }
      
   }
   
   nMCTracks = _myRelations.size();
   
   //The restored tracks, that we want to check for how good they are
   col = evt->getCollection( _TrackCollection ) ;

   
   if( col != NULL ){
      
      int nTracks = col->getNumberOfElements()  ;
      
      //check all the reconstructed tracks
      for(int i=0; i< nTracks ; i++){
         
         Track* track = dynamic_cast <Track*> ( col->getElementAt(i) ); 
         checkTheTrack( track );
         
      }
      
      //check the relations for lost ones and completes
      for( unsigned int i=0; i < _myRelations.size(); i++){
         if ( _myRelations[i]->isLost == true ) _nLost++;
         if ( _myRelations[i]->isFoundCompletely ==true ) _nFoundCompletely++;
      }
      
      
      /**********************************************************************************************/
      /*              Output the data                                                               */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG ).precision (8);
      
      
      for( unsigned int i=0; i < _myRelations.size(); i++){ // for all relations
         
         MCParticle* mcp = dynamic_cast <MCParticle*> (_myRelations[i]->lcRelation->getTo());
         Track* cheatTrack = dynamic_cast<Track*>  (_myRelations[i]->lcRelation->getFrom());      
         
         double mcpPt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );    
         double mcpP= sqrt( mcpPt*mcpPt + mcp->getMomentum()[2]*mcp->getMomentum()[2] );
         
         
         streamlog_out( MESSAGE0 ) << "\n\n\nTrue Track" << i << ": p= " << mcpP << "  pt= " << mcpPt << std::endl;
         streamlog_out( MESSAGE0 ) << "px= " << mcp->getMomentum()[0] << " py= " <<  mcp->getMomentum()[1] << " pz= " <<  mcp->getMomentum()[2] << std::endl;
         streamlog_out( MESSAGE0 ) << "PDG= " << mcp->getPDG() << std::endl;
         streamlog_out( MESSAGE0 ) << "x= " << mcp->getVertex()[0] << " y= " <<  mcp->getVertex()[1] << " z= " <<  mcp->getVertex()[2] << std::endl;
         streamlog_out( MESSAGE0 ) << "/gun/direction " << mcp->getMomentum()[0]/mcpP << " " <<  mcp->getMomentum()[1]/mcpP << " " <<  mcp->getMomentum()[2]/mcpP << std::endl;
         
         
         std::vector<TrackerHit*> cheatTrackHits = cheatTrack->getTrackerHits(); // we write it into an own vector so wen can sort it
         sort (cheatTrackHits.begin() , cheatTrackHits.end() , compare_TrackerHit_z );
         // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
         // So the direction of the hits when following the index from 0 on is:
         // from inside out: from the IP into the distance.
         
         std::vector <IHit*> hits;
        
         // check the chi2 of different lengths
         ///////////////////////////////////////
         
         
         if( cheatTrackHits.size() >= 3 ){
            
            
            FTDTrack myTrack;
            
           
            for( unsigned j=0; j<2; j++ ){//add the first 2 hits
               
               
               IHit* hit = new FTDHit01( cheatTrackHits[j] , _sectorSystemFTD );
               
               hits.push_back( hit );
               
               myTrack.addHit( hit );  
               
               
            }
              
            for( unsigned j=2; j<cheatTrackHits.size(); j++ ){ //for 3 hits plus (enough to fit)
               
               // add a hit and fit
               
               IHit* hit = new FTDHit01( cheatTrackHits[j] , _sectorSystemFTD );
               
               hits.push_back( hit );
               
               myTrack.addHit( hit );  
               
               myTrack.fit();
               
               streamlog_out( MESSAGE0 )  << "\t" << j+1 << "-hit track: chi2Prob = " << myTrack.getChi2Prob() 
               << "( chi2=" << myTrack.getChi2() <<", Ndf=" << myTrack.getNdf() << " )\n";
               
               
               
            }
            
            
         }
         
         
         ////////////////////////////////////////
         
         // Make a helix from the mcp
         HelixClass helixClass;
         
         float vertex[3]; 
         vertex[0] = mcp->getVertex()[0];
         vertex[1] = mcp->getVertex()[1];
         vertex[2] = mcp->getVertex()[2];
         float momentum[3];
         momentum[0] = mcp->getMomentum()[0];
         momentum[1] = mcp->getMomentum()[1];
         momentum[2] = mcp->getMomentum()[2];
         
         helixClass.Initialize_VP( vertex, momentum, mcp->getCharge(), 3.5);
         
         
         streamlog_out( MESSAGE0 ) << "mcpHelix parameter: " 
                                   << "D0 = " << helixClass.getD0()
                                   << ",  Phi = " << helixClass.getPhi0()
                                   << ",  Omega = " << helixClass.getOmega()
                                   << ",  Z0 = " << helixClass.getZ0()
                                   << ",  tan(Lambda) = " << helixClass.getTanLambda();
         
        
         for (unsigned int j=0; j< cheatTrackHits.size(); j++){ //over all Hits in the track
             
            
            double x = cheatTrackHits[j]->getPosition()[0];
            double y = cheatTrackHits[j]->getPosition()[1];
            double z = cheatTrackHits[j]->getPosition()[2];
            
            float mcpHelixPoint[3] = {0.,0.,0.};
            helixClass.getPointInZ( z , vertex, mcpHelixPoint);
            
            double xDist = mcpHelixPoint[0]-x;
            double yDist = mcpHelixPoint[1]-y;
            
            double dist = sqrt( xDist*xDist + yDist*yDist );
            
            
            streamlog_out( MESSAGE0 )  << "\n( "  
                  << x << " , " 
                  << y << " , " 
                  << z << " ) type: "
                  << cheatTrackHits[j]->getType() << " , xy-dist to mcp-helix= "
                  << dist
                  << " , zDist= " << mcpHelixPoint[2]-z;
                  
                
         }
         
         
         // Add the IP as a hit
         IHit* virtualIPHit = createVirtualIPHit(1 , _sectorSystemFTD );
         hits.insert( hits.begin() , virtualIPHit );
         
         
         // Now we have a vector of autHits starting with the IP followed by all the hits from the track.
         // So we now are able to build segments from them
         
         
         
         std::vector <Segment*> segments1;
         
         for ( unsigned j=0; j < hits.size(); j++ ){
            
            
            std::vector <IHit*> segHits; // the hits the segment will contain
            
            
            segHits.push_back( hits[j] );
            
            segments1.push_back( new Segment( segHits ) );
            
         }
         
         
         std::vector <Segment*> segments2;
         
         for ( unsigned j=0; j < hits.size()-1; j++ ){
            
            
            std::vector <IHit*> segHits;
            
            segHits.push_back( hits[j+1] );
            segHits.push_back( hits[j] );
            
            
            
            segments2.push_back( new Segment( segHits ) );
            
         }
         
         
         std::vector <Segment*> segments3;
         
         for ( unsigned j=0; j < hits.size()-2; j++ ){
            
            
            std::vector <IHit*> segHits;
            
            segHits.push_back( hits[j+2] );
            segHits.push_back( hits[j+1] );
            segHits.push_back( hits[j] );
            
            segments3.push_back( new Segment( segHits ) );
            
         }
         
         
         // Now we have the segments of the track ( ordered) in the vector
         
         // Perform the checks on them:
         
         // the data that will get printed
         std::map < std::string , std::vector<float> > crit2Data;
         
         for ( unsigned j=0; j < segments1.size()-1; j++ ){
            

            
            //make the check on the segments, store it in the the map...
            Segment* child = segments1[j];
            Segment* parent = segments1[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits2 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits2 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits2 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               std::map < std::string , float >::iterator it;
               
               for ( it = newMap.begin() ; it != newMap.end(); it++) crit2Data[it->first].push_back( it->second );
                  
                              
               
            }
            
         }
         
         //print it
         std::map < std::string , std::vector<float> >::iterator it;
         
         streamlog_out( MESSAGE0 ) << "\n  Crit2:";
         
         for ( it = crit2Data.begin() ; it != crit2Data.end() ; it++ ){
            
            streamlog_out( MESSAGE0 ) << "\n" << it->first;
            
            
            std::vector<float> critValues = it->second;
            
            for ( unsigned j=0; j<critValues.size(); j++ ){
               
               streamlog_out( MESSAGE0 ) << "\t" << critValues[j];
               
            }
            
         }
            
         // the data that will get printed
         std::map < std::string , std::vector<float> > crit3Data;
         
         for ( unsigned j=0; j < segments2.size()-1; j++ ){
            
   
            //make the check on the segments, store it in the the map...
            Segment* child = segments2[j];
            Segment* parent = segments2[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits3 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits3 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits3 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               std::map < std::string , float >::iterator it;
               for ( it = newMap.begin() ; it != newMap.end(); it++) crit3Data[it->first].push_back( it->second );
               
            }
            
            
         }
         
         //print it
                  
         streamlog_out( MESSAGE0 ) << "\n  Crit3:";
         
         for ( it = crit3Data.begin() ; it != crit3Data.end() ; it++ ){
            
            streamlog_out( MESSAGE0 ) << "\n" << it->first;
            
            
            std::vector<float> critValues = it->second;
            
            for ( unsigned j=0; j<critValues.size(); j++ ){
               
               streamlog_out( MESSAGE0 ) << "\t" << critValues[j];
               
            }
            
         }
         
         // the data that will get printed
         std::map < std::string , std::vector<float> > crit4Data;
         
         for ( unsigned j=0; j < segments3.size()-1; j++ ){
            
           
            //make the check on the segments, store it in the the map...
            Segment* child = segments3[j];
            Segment* parent = segments3[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits4 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits4 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits4 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               std::map < std::string , float >::iterator it;
               for ( it = newMap.begin() ; it != newMap.end(); it++) crit4Data[it->first].push_back( it->second );
               
            }
            
           
         }
         
         
         //print it
         
         streamlog_out( MESSAGE0 ) << "\n  Crit4:";
         
         for ( it = crit4Data.begin() ; it != crit4Data.end() ; it++ ){
            
            streamlog_out( MESSAGE0 ) << "\n" << it->first;
            
            
            std::vector<float> critValues = it->second;
            
            for ( unsigned j=0; j<critValues.size(); j++ ){
               
               streamlog_out( MESSAGE0 ) << "\t" << critValues[j];
               
            }
            
         }
         
         streamlog_out( MESSAGE0 ) << "\n";
         
         for (unsigned j=0; j<segments1.size(); j++) delete segments1[j];
         segments1.clear();
         for (unsigned j=0; j<segments2.size(); j++) delete segments2[j];
         segments2.clear();
         for (unsigned j=0; j<segments3.size(); j++) delete segments3[j];
         segments3.clear();
         for (unsigned j=0; j<hits.size(); j++) delete hits[j];
         hits.clear();
         
         
         
         
         
         // Print out, if this track is lost or not found completely
         
         if (_myRelations[i]->isLost == true) streamlog_out( MESSAGE0 ) << "LOST\n" <<std::endl;
         if (_myRelations[i]->isFoundCompletely== false) streamlog_out( MESSAGE0 ) << "NOT FOUND COMPLETELY\n";
         
         
         
         // Print out all the tracks associated to the true one:
         
         std::map<Track*,TrackType> relTracks = _myRelations[i]->relatedTracks;
         for (std::map<Track*,TrackType>::const_iterator it = relTracks.begin(); it != relTracks.end(); ++it)
         {
            streamlog_out( MESSAGE0 ) << TRACK_TYPE_NAMES[it->second] << "\t";
            
            Track* track = it->first;
            
            std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
            // sort the hits in the track
            
            // Make authits from the trackerHits
            std::vector <FTDHit01*> autHits2;
            
            for ( unsigned j=0; j< trackerHits.size(); j++ ) autHits2.push_back( new FTDHit01( trackerHits[j] , _sectorSystemFTD ) );
            
            FTDTrack myTrack;
            for( unsigned j=0; j<autHits2.size(); j++ ) myTrack.addHit( autHits2[j] );
            
            myTrack.fit();
            
           
            
            // print out the hits
            for (unsigned int k = 0; k < track->getTrackerHits().size(); k++){
               
               streamlog_out( MESSAGE0 ) << "(" << track->getTrackerHits()[k]->getPosition()[0] << ","
                     << track->getTrackerHits()[k]->getPosition()[1] << ","
                     << track->getTrackerHits()[k]->getPosition()[2] << ")";
               
            }
            
            streamlog_out( MESSAGE0 )<<"chi2prob = " << myTrack.getChi2Prob() << "\n";
            
            
            
            for( unsigned j=0; j<autHits2.size(); j++ ) delete autHits2[j];
            
            
         }
         
         
         streamlog_out( MESSAGE0 )  << "\n\n";

      }
      
      _nLostSum += _nLost;
      _nGhostSum += _nGhost;
      _nTrueTracksSum += nMCTracks;
      _nRecoTracksSum += nTracks; 
      
      
      //The statistics:
      
      float pLost=-1.;
      float pGhost=-1.; 
      float pComplete=-1.;
      float pFoundCompletely=-1.;
      
      if (nMCTracks > 0) pLost = 100.* _nLost/nMCTracks;           //percentage of true tracks that are lost

      if (nTracks > 0) pGhost = 100.* _nGhost/nTracks;             //percentage of found tracks that are ghosts
      
      if (nMCTracks > 0) pComplete = 100. * _nComplete/nMCTracks;  //percentage of true tracks that where found complete with no extra points
   
      if (nMCTracks > 0) pFoundCompletely = 100. * _nFoundCompletely/nMCTracks;
      
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << "nMCTracks = " << nMCTracks <<std::endl;
      streamlog_out( MESSAGE0 ) << "nTracks = " << nTracks <<std::endl;
      streamlog_out( MESSAGE0 ) << "nFoundCompletely = " << _nFoundCompletely << " , " << pFoundCompletely << "%" <<std::endl;
      streamlog_out( MESSAGE0 ) << "nLost = " << _nLost << " , " << pLost << "%" << std::endl;
      streamlog_out( MESSAGE0 ) << "nGhost = " << _nGhost << " , " << pGhost << "%" << std::endl;   
      
      streamlog_out( MESSAGE0 ) << "nComplete = " << _nComplete << " , " << pComplete << "%" <<std::endl;
      streamlog_out( MESSAGE0 ) << "nCompletePlus = " << _nCompletePlus <<std::endl;
      
      streamlog_out( MESSAGE0 ) << "nIncomplete = " << _nIncomplete <<std::endl;
      streamlog_out( MESSAGE0 ) << "nIncompletePlus = " << _nIncompletePlus <<std::endl;
      
      streamlog_out( MESSAGE0 ) << std::endl;
      
      
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << std::endl;
      
      ////
      
      ofstream myfile;
      myfile.open ("Feedback.csv" , std::ios::app);
      
      if (isFirstEvent()) myfile << std::endl;

      
      
      myfile << std::endl;
      myfile << "nFoundCompletely\t" << _nFoundCompletely << "\t\t" << pFoundCompletely << "\t%\t\t";
      myfile << "nLost\t" << _nLost << "\t\t" << pLost << "\t%" << "\t\t";
      myfile << "nGhost\t" << _nGhost << "\t\t" << pGhost << "\t%" << "\t\t"; 
      
      myfile << "nMCTracks\t" << nMCTracks <<"\t\t";
      myfile << "nTracks\t" << nTracks <<"\t\t";
      
      myfile << "nComplete\t" << _nComplete << "\t" << pComplete << "%\t\t";
      myfile << "nCompletePlus\t" << _nCompletePlus <<"\t\t\t";
      
      myfile << "nIncomplete\t" << _nIncomplete <<"\t\t\t";
      myfile << "nIncompletePlus\t" << _nIncompletePlus <<"\t\t\t";
      
      myfile.close();

      
      //Save it to a root file

      std::map < std::string , int > rootData;
      std::map < std::string , int >::iterator it;

            
      rootData.insert ( std::pair < std::string , int> ( "nMCTracks" ,          nMCTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nRecoTracks" ,        nTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nFoundCompletely" ,   _nFoundCompletely ) );
      rootData.insert ( std::pair < std::string , int> ( "nLost" ,              _nLost ) );
      rootData.insert ( std::pair < std::string , int> ( "nGhost" ,             _nGhost ) );
      rootData.insert ( std::pair < std::string , int> ( "nComplete" ,          _nComplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nCompletePlus" ,      _nCompletePlus ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncomplete" ,        _nIncomplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncompletePlus" ,    _nIncompletePlus ) );
      
        
      std::map < std::string , double > rootDataDouble;
      std::map < std::string , double >::iterator itD;
      
      rootDataDouble.insert ( std::pair < std::string , double> ( "pFoundCompletely" , pFoundCompletely ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pLost" , pLost ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pGhost" , pGhost ) );
      rootDataDouble.insert ( std::pair < std::string , double> ( "pComplete" , pComplete ) );
      
      
      TFile*   myRootFile = new TFile( _rootFileName.c_str(), "UPDATE"); //add values to the root file
      TTree*   myTree = (TTree*) myRootFile->Get(_treeName.c_str());
      
      
      
      
      for( it = rootData.begin() ; it != rootData.end() ; it++){
 
         
         if ( isFirstEvent() )  myTree->Branch( (*it).first.c_str(), & (*it).second  );
            
         else myTree->SetBranchAddress( (*it).first.c_str(), & (*it).second );   
               
         
      }
      
      
      for( itD = rootDataDouble.begin() ; itD != rootDataDouble.end() ; itD++){
 
        
         if ( isFirstEvent() ) myTree->Branch( (*itD).first.c_str(), & (*itD).second  ); 
       
         else myTree->SetBranchAddress( (*itD).first.c_str(), & (*itD).second );  
         
         
      }
      
      
      myTree->Fill();
      myTree->Write("",TObject::kOverwrite);
      
      myRootFile->Close();

      ////
   }
   
   
   //-- note: this will not be printed if compiled w/o MARLINMESSAGE0=1 !

   streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
                        << "   in run:  " << evt->getRunNumber() << std::endl ;


//    MarlinCED::draw(this); //CED


   for( unsigned int k=0; k < _myRelations.size(); k++) delete _myRelations[k];
   _myRelations.clear();

   _nEvt ++ ;
}



void TrackingFeedbackProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackingFeedbackProcessor::end(){ 

   
   //   streamlog_out( DEBUG ) << "MyProcessor::end()  " << name() 
   // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
   // 	    << std::endl ;

   for (unsigned i=0; i<_crits2 .size(); i++) delete _crits2 [i];
   for (unsigned i=0; i<_crits3 .size(); i++) delete _crits3 [i];
   for (unsigned i=0; i<_crits4 .size(); i++) delete _crits4 [i];

   delete _sectorSystemFTD;
   _sectorSystemFTD = NULL;
   
   
   
   ofstream myfile;
   myfile.open ("FeedbackSum.csv" , std::ios::app);
   
   
   double efficiency = double( _nTrueTracksSum - _nLostSum ) / double( _nTrueTracksSum );
   double ghostrate = double( _nGhostSum ) / double( _nRecoTracksSum );
   
   myfile << "\n";
   myfile << "Efficiency\t" << efficiency << "\t\t";
   myfile << "ghostrate\t"  << ghostrate  << "\t\t";
   
   
   myfile.close();
   

   
   

}


double TrackingFeedbackProcessor::getDistToIP( MCParticle* mcp ){
   
   double dist = sqrt(mcp->getVertex()[0]*mcp->getVertex()[0] + 
   mcp->getVertex()[1]*mcp->getVertex()[1] + 
   mcp->getVertex()[2]*mcp->getVertex()[2] );

   return dist;
   
}

double TrackingFeedbackProcessor::getChi2Prob( Track* track ){
   
   
   std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
   
   // Make authits from the trackerHits
   std::vector <IHit*> hits;
   
   for ( unsigned j=0; j< trackerHits.size(); j++ ) hits.push_back( new FTDHit01( trackerHits[j] , _sectorSystemFTD) );
   
   
   FTDTrack myTrack( hits );
   
   myTrack.fit();
   
   double chi2Prob = myTrack.getChi2Prob();
   
   for( unsigned j=0; j < hits.size(); j++ ) delete hits[j];
   
   
   return chi2Prob;   
   
   
}
 
 
 
void TrackingFeedbackProcessor::checkTheTrack( Track* track ){ 
 
   
   std::vector <TrackerHit*> hitVec = track->getTrackerHits();

   std::vector<MyRelation*> hitRelations; //to contain all the true tracks relations that correspond to the hits of the track
                                          //if for example a track consists of 3 points from one true track and two from another
                                          //at the end this vector will have five entries: 3 times one true track and 3 times the other.
                                          //so at the end, all we have to do is to count them.

   for( unsigned int j=0; j < hitVec.size(); j++ ){ //over all hits in the track
      
      
      for( unsigned int k=0; k < _myRelations.size(); k++){ //check all relations if they correspond 
         
         
         Track* trueTrack = dynamic_cast <Track*>  ( _myRelations[k]->lcRelation->getFrom() ); //get the true track
         
         if ( find (trueTrack->getTrackerHits().begin() , trueTrack->getTrackerHits().end() , hitVec[j] ) 
            !=  trueTrack->getTrackerHits().end())      // if the hit is contained in this truetrack
         hitRelations.push_back( _myRelations[k] );     //add the track (i.e. its relation) to the vector hitRelations
      }
      
   } 


   // After those two for loops we have the vector hitRelations filled with all the true track (relations) that correspond
   // to our reconstructed track. Ideally this vector would now only consist of the same true track again and again.
   //
   // Before we can check what kind of track we have here, we have to get some data.
   // We wanna know the most represented true track:

   unsigned nHitsOneTrack = 0;              //number of most true hits corresponding to ONE true track 
   MyRelation* dominantTrueTrack = NULL;    //the true track most represented in the track 

   sort ( hitRelations.begin() , hitRelations.end()); //Sorting, so all the same elements are at one place

   unsigned n = 0;                  // number of hits from one track
   MyRelation* previousRel = NULL;  // used to store the previous relation, so we can compare, if there was a change. at first we don't have
   // a previous one, so we set it NULL
   for (unsigned int j=0; j< hitRelations.size(); j++){ 
      
      if ( hitRelations[j] == previousRel ) n++;   //for every repeating element count 1. (that's the reason we sorted before, so we can just count them like this)
               else n = 1; //previous was different --> we start again with 1
               
               previousRel = hitRelations[j];
      
      if (n > nHitsOneTrack){ //we have a new winner (a true track) with (currently) the most hits in this track
                              
                  nHitsOneTrack = n;
                  dominantTrueTrack = hitRelations[j];
                  
      }
      
   }

   unsigned nHitsTrack = hitVec.size();   //number of hits of the reconstructed track
   unsigned nHitsTrueTrack = 0;           //number of hits of the true track

   if (dominantTrueTrack != NULL ){ 
      Track* trueTrack = dynamic_cast <Track*> ( dominantTrueTrack->lcRelation->getFrom() );
      nHitsTrueTrack = trueTrack->getTrackerHits().size();  
      
   }




   //So now we have all the values we need to know to tell, what kind of track we have here.

   if ( nHitsOneTrack <= nHitsTrack/2. ){  // less than half the points at maximum correspond to one track, this is not a real track, but
      _nGhost++;                           // a ghost track
     
   }
   else{                                   // more than half of the points form a true track!
      
      TrackType trackType;
      
      if (nHitsOneTrack > nHitsTrueTrack/2.) //If more than half of the points from the true track are covered
         dominantTrueTrack->isLost = false;  // this is guaranteed no lost track  
         
      if (nHitsOneTrack < nHitsTrueTrack){    // there are too few good hits,, something is missing --> incomplete
      
         if (nHitsOneTrack < nHitsTrack){       // besides the hits from the true track there are also additional ones-->
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
         dominantTrueTrack->isFoundCompletely = true;
         
         
         if (nHitsOneTrack < nHitsTrack){       // there are still additional hits stored in the track, it's a
            _nCompletePlus++;                    // complete track with extra points
            trackType = COMPLETE_PLUS;
         }
         else{                                   // there are no additional points, finally, this is the perfect
            _nComplete++;                        // complete track
            trackType= COMPLETE;
         }
      }
      
      //we want the true track to know all reconstructed tracks containing him
      dominantTrueTrack->relatedTracks.insert( std::pair<Track*, TrackType> ( track ,trackType ) );  
      
   }   
 
 
}
