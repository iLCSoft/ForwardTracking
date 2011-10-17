#include "TrackingFeedbackProcessor.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include "HelixClass.h"

#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include "TVector3.h"
#include "fstream"
#include "SimpleCircle.h"
#include <algorithm>

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
// for calculating the chi2 probability. 
#include "Math/ProbFunc.h"

#include <sstream>
#include <MarlinCED.h>
#include "FTrackTools.h"


#include <IMPL/TrackerHitPlaneImpl.h>



// the criteria
#include "Crit2_RZRatio.h"
#include "Crit2_StraightTrack.h"

#include "Crit3_ChangeRZRatio.h"  
#include "Crit3_PTMin.h"
#include "Crit3_3DAngle.h"
#include "Crit3_IPCircleDist.h"  

#include "Crit4_2DAngleChange.h"        
#include "Crit4_distToExtrapolation.h"  
#include "Crit4_PhiZRatioChange.h"
#include "Crit4_distOfCircleCenters.h"
#include "Crit4_NoZigZag.h"
#include "Crit4_RChange.h"



using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





bool hitComp (TrackerHit* i, TrackerHit* j) {
    
   return ( fabs(i->getPosition()[2]) < fabs( j->getPosition()[2]) ); //compare their z values

}



const char* TRACK_TYPE_NAMES[] = {"COMPLETE" , "COMPLETE_PLUS" , "INCOMPLETE" , "INCOMPLETE_PLUS" , "GHOST" , "LOST"}; 
enum TrackType { COMPLETE , COMPLETE_PLUS , INCOMPLETE , INCOMPLETE_PLUS , GHOST , LOST };

struct MyRelation{
   
   LCRelation* lcRelation;
   bool isLost;
   bool isFoundCompletely;
   std::map<Track*,TrackType> relatedTracks;
   
   MyRelation( LCRelation* rel ){isLost = true; isFoundCompletely = false; relatedTracks.clear(); lcRelation = rel;};
};




TrackingFeedbackProcessor aTrackingFeedbackProcessor ;


TrackingFeedbackProcessor::TrackingFeedbackProcessor() : Processor("TrackingFeedbackProcessor") {

    // modify processor description
   _description = "TrackingFeedbackProcessor gives feedback about the Track Search" ;


    // register steering parameters: name, description, class-variable, default value

   registerInputCollection(LCIO::TRACK,
                           "AutTrkCollection",
                           "Name of Cellular Automaton Track output collection",
                           _AutTrkCollection,
                           std::string("AutTracks")); 
   
   
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
   
   registerProcessorParameter("NumberOfHitsMin",
                              "The minimum number of hits a track must have",
                              _nHitsMin,
                              int (4)  );   
   

}



void TrackingFeedbackProcessor::init() { 

    streamlog_out(DEBUG) << "   init called  " << std::endl ;

    // usually a good idea to
    printParameters() ;
    
    _treeName = "trackCands";
    setUpRootFile(_rootFileName, _treeName);      //prepare the root file.

    _nRun = 0 ;
    _nEvt = 0 ;
    
    
    //Initialise the TrackFitter (as the system and the method of fitting will stay the same over the events, we might set it up here,
    //( otherwise we would have to repeat this over and over again)
    
    //First set some bools
    _trackFitter.setMSOn( true );        // Multiple scattering on
    _trackFitter.setElossOn( true );     // Energy loss on
    _trackFitter.setSmoothOn( false );   // Smoothing off
    
    //Then initialise
    _trackFitter.initialise( "KalTest" , marlin::Global::GEAR , "" ); //Use KalTest as Fitter

    
    
    //Add the criteria that will be checked
    _crits2.push_back( new Crit2_RZRatio( 1.01 ) ); 
    _crits2.push_back( new Crit2_StraightTrack( 1.1 ) );
    
    _crits3.push_back( new Crit3_ChangeRZRatio( 1.) );
    _crits3.push_back( new Crit3_PTMin (0.1) );
    _crits3.push_back( new Crit3_3DAngle (10) );
    _crits3.push_back( new Crit3_IPCircleDist (10) );
    
    _crits4.push_back( new  Crit4_2DAngleChange ( 1. ) );
    _crits4.push_back( new  Crit4_PhiZRatioChange ( 1. ) );
    _crits4.push_back( new  Crit4_distToExtrapolation ( 1. ) );
    _crits4.push_back( new  Crit4_distOfCircleCenters ( 1. ) );
    _crits4.push_back( new  Crit4_NoZigZag ( 1. ) );
    _crits4.push_back( new  Crit4_RChange ( 1. ) );
    
    
    
//      MarlinCED::init(this) ;

}


void TrackingFeedbackProcessor::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void TrackingFeedbackProcessor::processEvent( LCEvent * evt ) { 


//-----------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

  MarlinCED::newEvent(this , 0) ; 

  CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();

  pHandler.update(evt); 

//-----------------------------------------------------------------------

   
   ////////////////////////////////////////////////////////////////
   //
   //getting feedback data of the tracks
   //
   //////////////////////////////////////////////////////////////
   
     
   
   int nComplete = 0;         // complete tracks without extra points
   int nCompletePlus = 0;     // complete tracks with extra points
   int nLost = 0;             // lost tracks = tracks that do exist in reality (mcp), but are not represented in the tracksearch results  
   int nIncomplete = 0;       // incomplete tracks without extra points. i.e.: tracks that are too short (for example 1 or 2 hits are still missing)
   int nIncompletePlus = 0;   // incomplete tracks with extra points. the reconstructed track belongs to the true track that hold more than half of the points of the track
   int nGhost = 0;            // ghost tracks = tracks that are reconstructed, but don't actually exist. Pure fiction. a ghost track 
                              // is a track, where no real track owns more than half of the tracks hits.
   int nFoundCompletely = 0;      // when a true track is found completely (restored track is a complete or complete plus track)
   
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();
   std::vector<MyRelation*> myRelations; 
   
   // fill the vector with the relations
   for( int i=0; i < nMCTracks; i++){
      
      bool isOfInterest = true;  // A bool to hold information wether this track we are looking at is interesting for us at all
                                 // So we might apply different criteria to it.
                                 // For example: if a track is very curly we might not want to consider it at all.
                                 // So when we want to know how high the percentage of reconstructed tracks is, we might
                                 // want to only consider certain true tracks 
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
      //CED begin

      
      MarlinCED::drawMCParticle( mcp, true, evt, 2, 1, 0xff000, 10, 3.5 );

 
      //CED end
      

      
      //////////////////////////////////////////////////////////////////////////////////
      //If distance from origin is not too high      
      double dist = sqrt(mcp->getVertex()[0]*mcp->getVertex()[0] + 
                     mcp->getVertex()[1]*mcp->getVertex()[1] + 
                     mcp->getVertex()[2]*mcp->getVertex()[2] );
      
      

      if ( dist > _distToIPMax ) isOfInterest = false;   // exclude point too far away from the origin. Of course we want them reconstructed too,
                                                // but at the moment we are only looking at the points that are reconstructed by a simple
                                                // Cellular Automaton, which uses the point 0 as a point in the track
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      //////////////////////////////////////////////////////////////////////////////////
      //If pt is not too low
      
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );
      
      if ( pt < _ptMin ) isOfInterest = false;
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      //////////////////////////////////////////////////////////////////////////////////
      //If there are less than 4 hits in the track
      
      if ( (int) track->getTrackerHits().size() < _nHitsMin ) isOfInterest = false;
      //
      //////////////////////////////////////////////////////////////////////////////////
      
      if ( isOfInterest ) myRelations.push_back( new MyRelation( rel ) );
      
   }
   
   nMCTracks = myRelations.size();
   
   col = evt->getCollection( _AutTrkCollection ) ;

   
   if( col != NULL ){

      int nTracks = col->getNumberOfElements()  ;

      //TODO: kann man das nicht einfacher gestalten??? eleganter? das sind schon wieder so viele for schleifen
      
      for(int i=0; i< nTracks ; i++){ //over all tracks
         
         Track* track = dynamic_cast <Track*> ( col->getElementAt(i) ); 
         TrackerHitVec hitVec = track->getTrackerHits();
         
         std::vector<MyRelation*> hitRelations; //containing all the true tracks (relations) that correspond to the hits of the track
                                                //if for example a track consists of 3 points from one true track and two from another
                                                //at the end this vector will have five entries: 3 times one true track and 3 times the other.
                                                //so at the end, all we have to do is to count them.
         
         for( unsigned int j=0; j < hitVec.size(); j++ ){ //over all hits in the track
            
            for( unsigned int k=0; k < myRelations.size(); k++){ //check all relations if they correspond 
            
               
               Track* trueTrack = dynamic_cast <Track*>  ( myRelations[k]->lcRelation->getFrom() ); //get the true track
         
               if ( find (trueTrack->getTrackerHits().begin() , trueTrack->getTrackerHits().end() , hitVec[j] ) 
                     !=  trueTrack->getTrackerHits().end()) // if the hit is contained in this truetrack
                        hitRelations.push_back( myRelations[k] );//add the track (i.e. its relation) to the vector hitRelations
            }
                    
         } 
         

         // After those two for loops we have the vector hitRelations filled with all the true track (relations) that correspond
         // to our reconstructed track. Ideally this vector would now only consist of the same true track again and again.
         //
         // Before we can check what kind of track we have here, we have to get some data.
         // We wanna know the most represented true track:
         
         int nHitsOneTrack = 0;          //number of most true hits corresponding to ONE true track 
         MyRelation* dominantTrueTrack = NULL;    //the true track most represented in the track 
         
        
         sort ( hitRelations.begin() , hitRelations.end()); //Sorting, so all the same elements are at one place
         
         int n=0;                         // number of hits from one track
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
         
         int nHitsTrack = hitVec.size();   //number of hits of the reconstructed track
         int nHitsTrueTrack = 0;           //number of hits of the true track
         
         if (dominantTrueTrack != NULL ){ 
            Track* trueTrack = dynamic_cast <Track*> ( dominantTrueTrack->lcRelation->getFrom() );
            nHitsTrueTrack = trueTrack->getTrackerHits().size();  
            
         }
                                         
                 
                
         
         //So now we have all the values we need to know to tell, what kind of track we have here.
         
         if ( nHitsOneTrack <= nHitsTrack/2. ){  // less than half the points at maximum correspond to one track, this is not a real track, but
            nGhost++;                           // a ghost track
                     
         }
         else{                                   // more than half of the points form a true track!
            
            TrackType trackType;
            
           
            
            if (nHitsOneTrack > nHitsTrueTrack/2.) //If more than half of the points from the true track are covered
               dominantTrueTrack->isLost = false;  // this is guaranteed no lost track  
            
            if (nHitsOneTrack < nHitsTrueTrack)    // there are too few good hits,, something is missing --> incomplete
               
               if (nHitsOneTrack < nHitsTrack){       // besides the hits from the true track there are also additional ones-->
                  nIncompletePlus++;                  // incomplete with extra points
                  trackType = INCOMPLETE_PLUS;
               }   
               else{                                   // the hits from the true track fill the entire track, so its an
                  nIncomplete++;                      // incomplete with no extra points
                  trackType = INCOMPLETE;
               }
               
            else{                                   // there are as many good hits as there are hits in the true track
                                                   // i.e. the true track is represented entirely in this track
               dominantTrueTrack->isFoundCompletely = true;
               
               
               if (nHitsOneTrack < nHitsTrack){       // there are still additional hits stored in the track, it's a
                  nCompletePlus++;                    // complete track with extra points
                  trackType = COMPLETE_PLUS;
               }
               else{                                   // there are no additional points, finally, this is the perfect
                  nComplete++;                        // complete track
                  trackType= COMPLETE;
               }
            }
               
               //we want the true track to know all reconstructed tracks containing him
               dominantTrueTrack->relatedTracks.insert( std::pair<Track*, TrackType> ( track ,trackType ) );  
               
         }   
         
               
      }
      
      // So now we checked all the values. But really all?
      // No, there is a little value that resisted us so far:
      // The lost tracks. We couldn't raise the value nLost on the go because we could only see which ones are not lost
      // and there is no 1:1 relationship.
      // But therefore we used the struct MyRelation and marked all of their isLost booleans
      // to false if we found a corresponding track.
      // so all we have to do is count how many with isLost == true are left.
      
      for( unsigned int i=0; i < myRelations.size(); i++){
         if ( myRelations[i]->isLost == true ) nLost++;
         if ( myRelations[i]->isFoundCompletely ==true ) nFoundCompletely++;
      }
                  
      
      
      /**********************************************************************************************/
      /*              Output the data                                                               */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG ).precision (8);
      
      
      for( unsigned int i=0; i < myRelations.size(); i++){ // for all relations
         
         MCParticle* mcp = dynamic_cast <MCParticle*> (myRelations[i]->lcRelation->getTo());
         Track* cheatTrack = dynamic_cast<Track*>  (myRelations[i]->lcRelation->getFrom());      
         
         double mcpPt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] );    
         double mcpP= sqrt( mcpPt*mcpPt + mcp->getMomentum()[2]*mcp->getMomentum()[2] );
         
         
         streamlog_out( MESSAGE0 ) << "\n\n\nTrue Track" << i << ": p= " << mcpP << "  pt= " << mcpPt << std::endl;
         streamlog_out( MESSAGE0 ) << "px= " << mcp->getMomentum()[0] << " py= " <<  mcp->getMomentum()[1] << " pz= " <<  mcp->getMomentum()[2] << std::endl;
         streamlog_out( MESSAGE0 ) << "PDG= " << mcp->getPDG() << std::endl;
         streamlog_out( MESSAGE0 ) << "x= " << mcp->getVertex()[0] << " y= " <<  mcp->getVertex()[1] << " z= " <<  mcp->getVertex()[2] << std::endl;
         streamlog_out( MESSAGE0 ) << "/gun/direction " << mcp->getMomentum()[0]/mcpP << " " <<  mcp->getMomentum()[1]/mcpP << " " <<  mcp->getMomentum()[2]/mcpP << std::endl;

         //Fit the track
         //first: empty the stored tracks (if there are any)
         _trackFitter.clearTracks();
         
         //then: fill in our trackCandidates:
         _trackFitter.addTrack( cheatTrack );
         
         //And get back fitted tracks
         std::vector <Track*> fittedTracks = _trackFitter.getFittedTracks();

         double chi2 = fittedTracks[0]->getChi2();
         double ndf = fittedTracks[0]->getNdf();
         
         double chi2Prob = ROOT::Math::chisquared_cdf_c( chi2 , ndf );
                  
         streamlog_out( MESSAGE0 )  << "chi2Prob = " << chi2Prob 
                                    << "( chi2=" << chi2 <<", Ndf=" << ndf << " )\n";
         
                                             
         std::vector<TrackerHit*> cheatTrackHits = cheatTrack->getTrackerHits(); // we write it into an own vector so wen can sort it
         sort (cheatTrackHits.begin() , cheatTrackHits.end() , compare_z );
         // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
         // So the direction of the hits when following the index from 0 on is:
         // from inside out: from the IP into the distance.
         
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
         TrackerHitPlaneImpl* virtualIPHit = new TrackerHitPlaneImpl ;
         
         double pos[] = {0. , 0. , 0.};
         virtualIPHit->setPosition(  pos  ) ;
         
         cheatTrackHits.insert( cheatTrackHits.begin() , virtualIPHit );
         
         // Make authits from the trackerHits
         std::vector <AutHit*> autHits;
         
         for ( unsigned j=0; j< cheatTrackHits.size(); j++ ){
            
            autHits.push_back( new AutHit( cheatTrackHits[j] ) );
            
         }
         
         // Now we have a vector of autHits starting with the IP followed by all the hits from the track.
         // So we now are able to build segments from them
         
         
         
         std::vector <Segment*> segments1;
         
         for ( unsigned j=0; j < autHits.size(); j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            
            
            segAutHits.push_back( autHits[j] ) ;
            
            segments1.push_back( new Segment( segAutHits ) );
            
         }
         
         
         std::vector <Segment*> segments2;
         
         for ( unsigned j=0; j < autHits.size()-1; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            
            segAutHits.push_back( autHits[j+1] );
            segAutHits.push_back( autHits[j] );
            
            
            
            segments2.push_back( new Segment( segAutHits ) );
            
         }
         
         
         std::vector <Segment*> segments3;
         
         for ( unsigned j=0; j < autHits.size()-2; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            
            segAutHits.push_back( autHits[j+2] );
            segAutHits.push_back( autHits[j+1] );
            segAutHits.push_back( autHits[j] );
            
            segments3.push_back( new Segment( segAutHits ) );
            
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
         for (unsigned j=0; j<segments2.size(); j++) delete segments2[j];
         for (unsigned j=0; j<segments3.size(); j++) delete segments3[j];
         for (unsigned j=0; j<autHits.size(); j++) delete autHits[j];
         
         delete virtualIPHit;
         
         
         
         
         // Print out, if this track is lost or not found completely
         
         if (myRelations[i]->isLost == true) streamlog_out( MESSAGE0 ) << "LOST" <<std::endl;
         if (myRelations[i]->isFoundCompletely== false) streamlog_out( MESSAGE0 ) << "NOT FOUND COMPLETELY";
         
         
         
         // Print out all the tracks associated to the true one:
         
         std::map<Track*,TrackType> relTracks = myRelations[i]->relatedTracks;
         for (std::map<Track*,TrackType>::const_iterator it = relTracks.begin(); it != relTracks.end(); ++it)
         {
            streamlog_out( DEBUG3 ) << TRACK_TYPE_NAMES[it->second] << "\t";
            
            for (unsigned int k = 0; k < it->first->getTrackerHits().size(); k++){
               
               streamlog_out( DEBUG3 ) << "(" << it->first->getTrackerHits()[k]->getPosition()[0] << ","
                     << it->first->getTrackerHits()[k]->getPosition()[1] << ","
                     << it->first->getTrackerHits()[k]->getPosition()[2] << ")";
               
            }
            
            streamlog_out( DEBUG3 )<<"\n";
         }
         
         
         streamlog_out( MESSAGE0 )  << "\n\n";

      }
      
      
      //The statistics:
      
      float pLost=-1.;
      float pGhost=-1.; 
      float pComplete=-1.;
      float pFoundCompletely=-1.;
      
      if (nMCTracks > 0) pLost = 100.* nLost/nMCTracks;           //percentage of true tracks that are lost

      if (nTracks > 0) pGhost = 100.* nGhost/nTracks;             //percentage of found tracks that are ghosts
      
      if (nMCTracks > 0) pComplete = 100. * nComplete/nMCTracks;  //percentage of true tracks that where found complete with no extra points
   
      if (nMCTracks > 0) pFoundCompletely = 100. * nFoundCompletely/nMCTracks;
      
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << "nMCTracks = " << nMCTracks <<std::endl;
      streamlog_out( MESSAGE0 ) << "nTracks = " << nTracks <<std::endl;
      streamlog_out( MESSAGE0 ) << "nFoundCompletely = " << nFoundCompletely << " , " << pFoundCompletely << "%" <<std::endl;
      streamlog_out( MESSAGE0 ) << "nLost = " << nLost << " , " << pLost << "%" << std::endl;
      streamlog_out( MESSAGE0 ) << "nGhost = " << nGhost << " , " << pGhost << "%" << std::endl;   
      
      streamlog_out( MESSAGE0 ) << "nComplete = " << nComplete << " , " << pComplete << "%" <<std::endl;
      streamlog_out( MESSAGE0 ) << "nCompletePlus = " << nCompletePlus <<std::endl;
      
      streamlog_out( MESSAGE0 ) << "nIncomplete = " << nIncomplete <<std::endl;
      streamlog_out( MESSAGE0 ) << "nIncompletePlus = " << nIncompletePlus <<std::endl;
      
      streamlog_out( MESSAGE0 ) << std::endl;
      
      
      streamlog_out( MESSAGE0 ) << std::endl;
      streamlog_out( MESSAGE0 ) << std::endl;
      
      ////
      
      ofstream myfile;
      myfile.open ("Feedback.csv" , std::ios::app);
      
      if (isFirstEvent()) myfile << std::endl;

      
      
      myfile << std::endl;
      myfile << "nFoundCompletely\t" << nFoundCompletely << "\t\t" << pFoundCompletely << "\t%\t\t";
      myfile << "nLost\t" << nLost << "\t\t" << pLost << "\t%" << "\t\t";
      myfile << "nGhost\t" << nGhost << "\t\t" << pGhost << "\t%" << "\t\t"; 
      
      myfile << "nMCTracks\t" << nMCTracks <<"\t\t";
      myfile << "nTracks\t" << nTracks <<"\t\t";
      
      myfile << "nComplete\t" << nComplete << "\t" << pComplete << "%\t\t";
      myfile << "nCompletePlus\t" << nCompletePlus <<"\t\t\t";
      
      myfile << "nIncomplete\t" << nIncomplete <<"\t\t\t";
      myfile << "nIncompletePlus\t" << nIncompletePlus <<"\t\t\t";
      
      myfile.close();

      
      //Save it to a root file

      std::map < std::string , int > rootData;
      std::map < std::string , int >::iterator it;

            
      rootData.insert ( std::pair < std::string , int> ( "nMCTracks" ,          nMCTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nRecoTracks" ,        nTracks ) );
      rootData.insert ( std::pair < std::string , int> ( "nFoundCompletely" ,   nFoundCompletely ) );
      rootData.insert ( std::pair < std::string , int> ( "nLost" ,              nLost ) );
      rootData.insert ( std::pair < std::string , int> ( "nGhost" ,             nGhost ) );
      rootData.insert ( std::pair < std::string , int> ( "nComplete" ,          nComplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nCompletePlus" ,      nCompletePlus ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncomplete" ,        nIncomplete ) );
      rootData.insert ( std::pair < std::string , int> ( "nIncompletePlus" ,    nIncompletePlus ) );
      
        
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


     MarlinCED::draw(this); //CED
        
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

}

