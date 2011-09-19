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



using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





CritAnalyser aCritAnalyser ;


CritAnalyser::CritAnalyser() : Processor("CritAnalyser") {
   
   // modify processor description
   _description = "CritAnalyser: Analysis of different criteria for hits on the FTD" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   
   
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
   
   
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();

   
   // fill the vector with the relations
   for( int i=0; i < nMCTracks; i++){
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
      
      
  
      
   }
      
 
   
   
   //Save it to a root file
   
   std::map < std::string , float > rootData;
  
   for ( unsigned i=0; i < _crits.size() ; i++ ){
      
      std::map < std::string , float > critData = _crits[i]->getMapOfValues();
      
      rootData.insert( critData.begin() , critData.end() );

   }
  
   
   saveToRoot( _rootFileName, _treeName , rootData );
               



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

