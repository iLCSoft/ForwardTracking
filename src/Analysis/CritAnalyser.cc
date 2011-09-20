#include "CritAnalyser.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <EVENT/LCRelation.h>
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
#include "CritRZRatio.h"
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
   _crits.push_back ( new CritRZRatio( 1.01 ) );
   
   std::set < std::string > critNames;
   
   
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

         
         critNames.insert( it->first ); //store the names of the values in the set critNames
         
      }
      
   }
   
   // Set up the root file with the tree and the branches
   _treeName = "values";
   setUpRootFile( _rootFileName, _treeName, critNames );      //prepare the root file.
   
   
   
   
   
   
 
   
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

   std::vector <Track*> trueTracks;
   
   // fill the vector with the true tracks
   for( int i=0; i < nMCTracks; i++){
      
      // get the monte carlo particel and the true track
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
//       MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      trueTracks.push_back( track );
  
      
   }
      
   streamlog_out( DEBUG3 ) << "\nTrue tracks: " << trueTracks.size() ;
 
 

   
   
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
      
      
      for(int i=0; i< nHits ; i++){ // for every hit
         
         
         TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         //Make an AutHit from the TrackerHit 
         AutHit* autHit = new AutHit ( trkHit );
         
         
         ftdRep.addHit( autHit );
         
         
      }
      
      streamlog_out( DEBUG3 ) << "\n " << nHits << " hits will be stored in the automaton" ;
      
      // Create a segmentbuilder
      SegmentBuilder segBuilder( &ftdRep );
      
      streamlog_out( DEBUG3 ) << "\nSegBuilder created.";
      
      // We load no criteria, as we want to check all connections
      
      // But a hitConnector is needed
      segBuilder.addHitConnector ( new HitCon( &autCode ) );
      
      streamlog_out( DEBUG3 ) << "\nCode added.";
      
      // Get out the 1-segments
      Automaton automaton = segBuilder.get1SegAutomaton();

      streamlog_out( DEBUG3 ) << "\nAutomaton created.";
      
      /**********************************************************************************************/
      /*                                                                                            */
      /*                Get the 1-segments from the Automaton and check them                        */
      /*                                                                                            */
      /**********************************************************************************************/
      
      
      std::vector <Segment*> segments = automaton.getSegments();
 
      streamlog_out( DEBUG3 ) << "\n 1-Segments to be checked = " << segments.size();
      
      
      for( unsigned i=0; i<segments.size(); i++ ){ // over all segments
         

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
            
            
            //Save it to a root file
            saveToRoot( _rootFileName, _treeName , rootData );  
      
         }
         
      }
 
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




