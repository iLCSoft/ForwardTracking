#include "TrueTrackCritAnalyser.h"
#include <iostream>
#include <algorithm>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"


#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include "TVector3.h"


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



#include "AutCode.h"
#include "Automaton.h"
#include "FTDRepresentation.h"
#include "SegmentBuilder.h"
#include "HitCon.h"


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





TrueTrackCritAnalyser aTrueTrackCritAnalyser ;


TrueTrackCritAnalyser::TrueTrackCritAnalyser() : Processor("TrueTrackCritAnalyser") {
   
   // modify processor description
   _description = "TrueTrackCritAnalyser: Analysis of different criteria for true tracks in the FTD" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results",
                              _rootFileName,
                              std::string("TrueTracksCritAnalysis.root") );
   
   
   
   
}



void TrueTrackCritAnalyser::init() { 
   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;
   
   // usually a good idea to
   printParameters() ;
   

   
   _nRun = 0 ;
   _nEvt = 0 ;
   
   
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
   

   
   
   std::set < std::string > branchNames2; //branch names of the 2-hit criteria
   std::set < std::string > branchNames3;
   std::set < std::string > branchNames4;
   
   
   // Set up the root file
   // Therefore first get all the possible names of the branches
   
   // create a virtual hit
   TrackerHitPlaneImpl virtualHit;
   double pos[] = {0. , 0. , 0.};
   virtualHit.setPosition(  pos  ) ;
    // create the AutHit and set its parameters
   AutHit* virtualAutHit = new AutHit( &virtualHit );
   virtualAutHit->setIsVirtual ( true );
   
   std::vector <AutHit*> autHitVec;
   autHitVec.push_back( virtualAutHit );
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 1-segments (2 hit criteria)                         */
   /**********************************************************************************************/
   
   Segment virtual1Segment( autHitVec );
   
   
   for ( unsigned int i=0; i < _crits2 .size() ; i++ ){ //for all criteria

      //get the map
      _crits2 [i]->areCompatible( &virtual1Segment , &virtual1Segment ); // It's a bit of a cheat: we calculate it for virtual hits to get a map containing the
                                                                   // names of the values ( and of course values that are useless, but we don't use them here anyway)
      
      std::map < std::string , float > newMap = _crits2 [i]->getMapOfValues();
      std::map < std::string , float > ::iterator it;
      
      for ( it = newMap.begin() ; it != newMap.end() ; it++ ){ //over all values in the map

         
         branchNames2.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   
   
   // Also insert branches for additional information
   branchNames2.insert( "MCP_pt" ); //transversal momentum
   branchNames2.insert( "MCP_distToIP" ); //the distance of the origin of the partivle to the IP
   
   // Set up the root file with the tree and the branches
   _treeName2 = "2Hit_Criteria";
   setUpRootFile( _rootFileName, _treeName2, branchNames2 );      //prepare the root file.
   
   
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 2-segments (3 hit criteria)                         */
   /**********************************************************************************************/
   
   autHitVec.push_back( virtualAutHit );
   Segment virtual2Segment( autHitVec );
   
   
   for ( unsigned int i=0; i < _crits3 .size() ; i++ ){ //for all criteria

      //get the map
      _crits3 [i]->areCompatible( &virtual2Segment , &virtual2Segment ); // It's a bit of a cheat: we calculate it for virtual hits to get a map containing the
      // names of the values ( and of course values that are useless, but we don't use them here anyway)
      
      std::map < std::string , float > newMap = _crits3 [i]->getMapOfValues();
      std::map < std::string , float > ::iterator it;
      
      for ( it = newMap.begin() ; it != newMap.end() ; it++ ){ //over all values in the map

         
         branchNames3.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   
   
   // Also insert branches for additional information
   branchNames3.insert( "MCP_pt" ); //transversal momentum
   branchNames3.insert( "MCP_distToIP" ); //the distance of the origin of the partivle to the IP
   
   // Set up the root file with the tree and the branches
   _treeName3 = "3Hit_Criteria"; 
   bool createNewFile = false;
   setUpRootFile( _rootFileName, _treeName3, branchNames3 , createNewFile );      //prepare the root file.
  
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 3-segments (4 hit criteria)                         */
   /**********************************************************************************************/
   
   autHitVec.push_back( virtualAutHit );
   Segment virtual3Segment( autHitVec );
   
   
   for ( unsigned int i=0; i < _crits4 .size() ; i++ ){ //for all criteria

      //get the map
      _crits4 [i]->areCompatible( &virtual3Segment , &virtual3Segment ); // It's a bit of a cheat: we calculate it for virtual hits to get a map containing the
      // names of the values ( and of course values that are useless, but we don't use them here anyway)
      
      std::map < std::string , float > newMap = _crits4 [i]->getMapOfValues();
      std::map < std::string , float > ::iterator it;
      
      for ( it = newMap.begin() ; it != newMap.end() ; it++ ){ //over all values in the map

         
         branchNames4.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   
   
   // Also insert branches for additional information
   branchNames4.insert( "MCP_pt" ); //transversal momentum
   branchNames4.insert( "MCP_distToIP" ); //the distance of the origin of the partivle to the IP
   
   // Set up the root file with the tree and the branches
   _treeName4 = "4Hit_Criteria"; 
   
   setUpRootFile( _rootFileName, _treeName4, branchNames4 , createNewFile );      //prepare the root file.
   
 
   delete virtualAutHit;
   
}


void TrueTrackCritAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void TrueTrackCritAnalyser::processEvent( LCEvent * evt ) { 
   
   
   
   std::vector < std::map < std::string , float > > rootDataVec2;
   std::vector < std::map < std::string , float > > rootDataVec3;
   std::vector < std::map < std::string , float > > rootDataVec4;
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   
   
   if( col != NULL ){
      
      int nMCTracks = col->getNumberOfElements();

   

      for( int i=0; i < nMCTracks; i++){ // for every true track
      
     
      
         LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
         MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
         Track*    track = dynamic_cast <Track*>      (rel->getFrom() );

         
         
         // Additional information on the track
         const double* p = mcp->getMomentum();
         
         double pt=  sqrt( p[0]*p[0]+p[1]*p[1] );
         
         const double* vtx = mcp->getVertex();
         double distToIP = sqrt( vtx[0]*vtx[0] + vtx[0]*vtx[0] + vtx[0]*vtx[0] );
         
         
         
         std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
         // sort the hits in the track
         sort( trackerHits.begin(), trackerHits.end(), compare_z );
         // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
         // So the direction of the hits when following the index from 0 on is:
         // from inside out: from the IP into the distance.
         
         // Add the IP as a hit
         TrackerHitPlaneImpl* virtualIPHit = new TrackerHitPlaneImpl ;
         
         double pos[] = {0. , 0. , 0.};
         virtualIPHit->setPosition(  pos  ) ;
         
         trackerHits.insert( trackerHits.begin() , virtualIPHit );
         
         // Make authits from the trackerHits
         std::vector <AutHit*> autHits;
         
         for ( unsigned j=0; j< trackerHits.size(); j++ ){
            
            autHits.push_back( new AutHit( trackerHits[j] ) );
            
         }
         
         // Now we have a vector of autHits starting with the IP followed by all the hits from the track.
         // So we now are able to build segments from them
         
         std::vector <Segment*> segments1;
         
         for ( unsigned j=0; j < autHits.size()-1; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            segAutHits.insert( segAutHits.begin() , autHits.begin()+j , autHits.begin()+j+1 );
            
            segments1.push_back( new Segment( segAutHits ) );
            
         }
         
         std::vector <Segment*> segments2;
         
         for ( unsigned j=0; j < autHits.size()-2; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            
            segAutHits.push_back( autHits[j+1] );
            segAutHits.push_back( autHits[j] );
            
            segments2.push_back( new Segment( segAutHits ) );
            
         }
         
         std::vector <Segment*> segments3;
         
         for ( unsigned j=0; j < autHits.size()-3; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            
            segAutHits.push_back( autHits[j+2] );
            segAutHits.push_back( autHits[j+1] );
            segAutHits.push_back( autHits[j] );
            
            segments3.push_back( new Segment( segAutHits ) );
            
         }
         
         // Now we have the segments of the track ( ordered) in the vector
         
         // Perform the checks on them:
         
         for ( unsigned j=0; j < segments1.size()-1; j++ ){
            
            // the data that will get stored
            std::map < std::string , float > rootData;
            
            //make the check on the segments, store it in the the map...
            Segment* child = segments1[j];
            Segment* parent = segments1[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits2 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits2 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits2 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               rootData.insert( newMap.begin() , newMap.end() );
               
            }
            
            rootData["MCP_pt"] = pt;
            rootData["MCP_distToIP"] = distToIP;
            
            rootDataVec2.push_back( rootData );
            
         }
         
         
         for ( unsigned j=0; j < segments2.size()-1; j++ ){
            
            // the data that will get stored
            std::map < std::string , float > rootData;
            
            //make the check on the segments, store it in the the map...
            Segment* child = segments2[j];
            Segment* parent = segments2[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits3 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits3 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits3 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               rootData.insert( newMap.begin() , newMap.end() );
               
            }
            
            rootData["MCP_pt"] = pt;
            rootData["MCP_distToIP"] = distToIP;
            
            rootDataVec3.push_back( rootData );
            
         }
         
         
         for ( unsigned j=0; j < segments3.size()-1; j++ ){
            
            // the data that will get stored
            std::map < std::string , float > rootData;
            
            //make the check on the segments, store it in the the map...
            Segment* child = segments3[j];
            Segment* parent = segments3[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits4 .size(); iCrit++){ // over all criteria

               
               //get the map
               _crits4 [iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits4 [iCrit]->getMapOfValues(); //get the values that were calculated
               
               rootData.insert( newMap.begin() , newMap.end() );
               
            }
            
            rootData["MCP_pt"] = pt;
            rootData["MCP_distToIP"] = distToIP;
            
            rootDataVec4.push_back( rootData );
            
         }
         
         
         
         for (unsigned i=0; i<segments1.size(); i++) delete segments1[i];
         for (unsigned i=0; i<segments2.size(); i++) delete segments2[i];
         for (unsigned i=0; i<segments3.size(); i++) delete segments3[i];
         for (unsigned i=0; i<autHits.size(); i++) delete autHits[i];
         
         delete virtualIPHit;
         
      }
         
            

      saveToRoot( _rootFileName, _treeName2, rootDataVec2 );
      saveToRoot( _rootFileName, _treeName3, rootDataVec3 );
      saveToRoot( _rootFileName, _treeName4, rootDataVec4 );
      
         
      
    
   }
 
 

   //-- note: this will not be printed if compiled w/o MARLINDEBUG4=1 !

   streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
   << "   in run:  " << evt->getRunNumber() << std::endl ;


   _nEvt ++ ;
   
   
}



void TrueTrackCritAnalyser::check( LCEvent * evt ) { 
   // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrueTrackCritAnalyser::end(){ 
   
   //   streamlog_out( DEBUG ) << "MyProcessor::end()  " << name() 
   //      << " processed " << _nEvt << " events in " << _nRun << " runs "
   //      << std::endl ;
   
   for (unsigned i=0; i<_crits2 .size(); i++) delete _crits2 [i];
   for (unsigned i=0; i<_crits3 .size(); i++) delete _crits3 [i];
   for (unsigned i=0; i<_crits4 .size(); i++) delete _crits4 [i];
      
   
   
}






