#include "CritAnalyser.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"


#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include "TVector3.h"


#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"


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





CritAnalyser aCritAnalyser ;


CritAnalyser::CritAnalyser() : Processor("CritAnalyser") {
   
   // modify processor description
   _description = "CritAnalyser: Analysis of different criteria for hits on the FTD" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   registerInputCollection(LCIO::TRACKERHIT,
                           "FTDHitCollectionName",
                           "FTD Hit Collection Name",
                           _FTDHitCollection,
                           std::string("FTDTrackerHits")); 
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results",
                              _rootFileName,
                              std::string("CritAnalysis.root") );
   
   
   
   
}



void CritAnalyser::init() { 
   
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
   TrackerHitPlaneImpl* virtualHit = new TrackerHitPlaneImpl ;
   double pos[] = {0. , 0. , 0.};
   virtualHit->setPosition(  pos  ) ;
    // create the AutHit and set its parameters
   AutHit* virtualAutHit = new AutHit ( virtualHit );
   virtualAutHit->setIsVirtual ( true );
   Segment* virtualSegment = new Segment ( virtualAutHit );
   
   
   for ( unsigned int i=0; i < _crits.size() ; i++ ){ //for all criteria

      //get the map
      _crits[i]->areCompatible( virtualSegment , virtualSegment ); // It's a bit of a cheat: we calculate it for virtual hits to get a map containing the
                                                                   // names of the values ( and of course values that are useless, but we don't use them here anyway)
      
      std::map < std::string , float > newMap = _crits[i]->getMapOfValues();
      std::map < std::string , float > ::iterator it;
      
      for ( it = newMap.begin() ; it != newMap.end() ; it++ ){ //over all values in the map

         
         branchNames.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   
   // Also insert branches for additional information
   branchNames.insert( "pt" ); //transversal momentum
   
   // Set up the root file with the tree and the branches
   _treeName = "values";
   setUpRootFile( _rootFileName, _treeName, branchNames );      //prepare the root file.
   
   
   
  
   
   
 
   
}


void CritAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void CritAnalyser::processEvent( LCEvent * evt ) { 
   
   
   
   
   /**********************************************************************************************/
   /*                                                                                            */
   /*                Get and store the true tracks                                               */
   /*                                                                                            */
   /**********************************************************************************************/
   
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();

   _relations.clear();
   
   // fill the vector with the relations
   for( int i=0; i < nMCTracks; i++){
      
     
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );

      
      _relations.push_back( rel );
  
      
   }
      
   streamlog_out( DEBUG3 ) << "\nRelations: " << _relations.size() ;
 
 

   
   
   col = evt->getCollection( _FTDHitCollection ) ;
   
   
   if( col != NULL ){
      
      
      
      /**********************************************************************************************/
      /*                                                                                            */
      /*                Get all hits and store them in an Automaton                                 */
      /*                                                                                            */
      /**********************************************************************************************/
      
      int nHits = col->getNumberOfElements()  ;
      
      
      //TODO: get those from gear
      unsigned int nLayers = 8; // layer 0 is for the IP
      unsigned int nModules = 16;
      unsigned int nSensors = 2;
      
      AutCode autCode ( nLayers, nModules , nSensors );
      
      FTDRepresentation ftdRep ( &autCode );
      
      streamlog_out( DEBUG2 ) << "\nFTDRepresentation created.";
      
      
      std::vector< AutHit* > autHitsTBD; //AutHits to be deleted
      
      for(int i=0; i< nHits ; i++){ // for every hit
         
         
         TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         //Make an AutHit from the TrackerHit 
         AutHit* autHit = new AutHit ( trkHit );
         
         
         ftdRep.addHit( autHit );
         autHitsTBD.push_back( autHit );
         
         
      }
      
      streamlog_out( DEBUG3 ) << "\n " << nHits << " hits will be stored in the automaton" ;
      
      // Create a segmentbuilder
      SegmentBuilder segBuilder( &ftdRep );
      
      streamlog_out( DEBUG3 ) << "\nSegBuilder created.";
      
      // We load no criteria, as we want to check all connections
      
      // But a hitConnector is needed
      HitCon* hitCon = new HitCon( &autCode );
      segBuilder.addHitConnector ( hitCon );
      
      streamlog_out( DEBUG3 ) << "\nCode added.";
      
      // Get out the 1-segments
      Automaton automaton = segBuilder.get1SegAutomaton();

      streamlog_out( DEBUG3 ) << "\nAutomaton created.";
      
      /**********************************************************************************************/
      /*                                                                                            */
      /*                Get the 1-segments from the Automaton and check them                        */
      /*                                                                                            */
      /**********************************************************************************************/
      
      std::vector < std::map < std::string , float > > rootDataVec;
      
      std::vector <Segment*> segments = automaton.getSegments();
 
      streamlog_out( DEBUG3 ) << "\n 1-Segments to be checked = " << segments.size();
      
      
      for( unsigned i=0; i<segments.size(); i++ ){ // over all segments
         
         streamlog_out( DEBUG0 ) << "\n segment " << i;

         Segment* segment = segments[i];
         std::vector <Segment*> children = segment->getChildren();
         
         for( unsigned j=0; j<children.size(); j++){ // over all children
            
            

            Segment* child = children[j];
            
            // the data that will get stored
            std::map < std::string , float > rootData;
            
            for( unsigned iCrit=0; iCrit < _crits.size(); iCrit++){ // over all criteria

               
               //get the map
               _crits[iCrit]->areCompatible( segment , child ); //calculate their compatibility
               
               std::map < std::string , float > newMap = _crits[iCrit]->getMapOfValues(); //get the values that were calculated

               
               rootData.insert( newMap.begin() , newMap.end() );
               
            }
                        
            // Get the mcp of the track (or NULL, if the hits don't belong to a track
            LCRelation* rel = getRelation( segment , child );
            
            float pt = 0.;
            
            if ( rel != NULL){ // the segments belong to a track
               
               
               MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
               
               const double* p = mcp->getMomentum();
               
               pt=  sqrt( p[0]*p[0]+p[1]*p[1] );
               
               
               
               
            }
            
            rootData["pt"] = pt;
            
            rootDataVec.push_back( rootData );
            //Save it to a root file
            
//             saveToRoot( _rootFileName, _treeName , rootData );  
      
         }
         
      }
      
      
      saveToRoot( _rootFileName, _treeName, rootDataVec );
      
      delete hitCon;
 
   }
 
 

   //-- note: this will not be printed if compiled w/o MARLINDEBUG4=1 !

   streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
   << "   in run:  " << evt->getRunNumber() << std::endl ;


   _nEvt ++ ;
   
   
}



void CritAnalyser::check( LCEvent * evt ) { 
   // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void CritAnalyser::end(){ 
   
   //   streamlog_out( DEBUG ) << "MyProcessor::end()  " << name() 
   //      << " processed " << _nEvt << " events in " << _nRun << " runs "
   //      << std::endl ;
   
}



LCRelation* CritAnalyser::getRelation( Segment* parent , Segment* child ){
   
   
   TrackerHit* childTrackerHit = child->getAutHits()[0]->getTrackerHit();
   TrackerHit* parentTrackerHit = parent->getAutHits()[0]->getTrackerHit();
   
   for ( unsigned i=0; i < _relations.size(); i++ ){ //over all relations (tracks)
      
      Track* track = dynamic_cast <Track*>( _relations[i]->getFrom() );
      
      std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
      
      bool childOnTrack = false;
      bool parentOnTrack = false;
      
      for ( unsigned j=0; j < trackerHits.size() ; j++){ // over all hits on the true track
         
         
         if ( trackerHits[j] == childTrackerHit ) childOnTrack = true;
         if ( trackerHits[j] == parentTrackerHit ) parentOnTrack = true;
         
      }
      
      // now check if they are both on the track
      if ( childOnTrack && parentOnTrack ) return _relations[i];
      
   }
   
   return NULL;
   
   
   
}



