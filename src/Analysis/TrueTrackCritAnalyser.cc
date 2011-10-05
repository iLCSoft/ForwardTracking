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
#include "Crit3_3DAngle.h"
#include "Crit2_StraightTrack.h"


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
   Crit2_RZRatio* crit2_RZRatio = new Crit2_RZRatio( 1.01 );
   Crit2_StraightTrack* crit2_StraightTrack = new Crit2_StraightTrack( 1.1 );
   
   _crits.push_back ( crit2_RZRatio ); 
   _crits.push_back ( crit2_StraightTrack );
   
   std::set < std::string > branchNames;
   
   
   // Set up the root file
   // Therefore first get all the possible names of the branches
   
   // create a virtual hit
   TrackerHitPlaneImpl virtualHit;
   double pos[] = {0. , 0. , 0.};
   virtualHit.setPosition(  pos  ) ;
    // create the AutHit and set its parameters
   AutHit virtualAutHit( &virtualHit );
   virtualAutHit.setIsVirtual ( true );
   Segment virtualSegment( &virtualAutHit );
   
   
   for ( unsigned int i=0; i < _crits.size() ; i++ ){ //for all criteria

      //get the map
      _crits[i]->areCompatible( &virtualSegment , &virtualSegment ); // It's a bit of a cheat: we calculate it for virtual hits to get a map containing the
                                                                   // names of the values ( and of course values that are useless, but we don't use them here anyway)
      
      std::map < std::string , float > newMap = _crits[i]->getMapOfValues();
      std::map < std::string , float > ::iterator it;
      
      for ( it = newMap.begin() ; it != newMap.end() ; it++ ){ //over all values in the map

         
         branchNames.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   

   
   // Also insert branches for additional information
   branchNames.insert( "pt" ); //transversal momentum
   branchNames.insert( "distToIP" ); //the distance of the origin of the partivle to the IP
   
   // Set up the root file with the tree and the branches
   _treeName = "values";
   setUpRootFile( _rootFileName, _treeName, branchNames );      //prepare the root file.
   
   
   
  
   
   
 
   
}


void TrueTrackCritAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void TrueTrackCritAnalyser::processEvent( LCEvent * evt ) { 
   
   
   
   std::vector < std::map < std::string , float > > rootDataVec;
  
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
         
         std::vector <Segment*> segments;
         
         for ( unsigned j=0; j < autHits.size()-1; j++ ){
            
            
            std::vector <AutHit*> segAutHits;
            segAutHits.insert( segAutHits.begin() , autHits.begin()+j , autHits.begin()+j+1 );
            
            segments.push_back( new Segment( segAutHits ) );
            
         }
         
         // Now we have the segments of the track (in order) in the vector
         
         // Perform the checks on them:
         
         for ( unsigned j=0; j < segments.size()-1; j++ ){
            
            // the data that will get stored
            std::map < std::string , float > rootData;
            
            //make the check on the segments, store it in the the map...
            Segment* child = segments[j];
            Segment* parent = segments[j+1];
            
            
            for( unsigned iCrit=0; iCrit < _crits.size(); iCrit++){ // over all criteria

               
               //get the map
               _crits[iCrit]->areCompatible( parent , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits[iCrit]->getMapOfValues(); //get the values that were calculated
               
               rootData.insert( newMap.begin() , newMap.end() );
               
            }
            
            
            
            
            
            rootData["pt"] = pt;
            rootData["distToIP"] = distToIP;
            
            rootDataVec.push_back( rootData );
            
            
         }
         
      }
         
            

      saveToRoot( _rootFileName, _treeName, rootDataVec );
         
      
    
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
   
   for (unsigned i=0; i<_crits.size(); i++){
      
      delete _crits[i];
      
   }
   
}






