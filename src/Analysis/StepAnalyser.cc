#include "StepAnalyser.h"
#include <iostream>

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>
#include <EVENT/MCParticle.h>
#include <cmath>
#include <algorithm>
#include "UTIL/ILDConf.h"

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





StepAnalyser aStepAnalyser ;


StepAnalyser::StepAnalyser() : Processor("StepAnalyser") {
   
   // modify processor description
   _description = "StepAnalyser: Analysis of different criteria for hits on the FTD" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TrueTracksMCP"));
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results",
                              _rootFileName,
                              std::string("StepAnalysis.root") );
   
   
   
   
}



void StepAnalyser::init() { 
   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;
   
   // usually a good idea to
   printParameters() ;
   

   
   _nRun = 0 ;
   _nEvt = 0 ;
   
   

      
   std::set < std::string > branchNames;
   
   
   // Set up the root file
   // Therefore first set all the possible names of the branches
   
   branchNames.insert( "path" );
   branchNames.insert( "lastLayerBeforeIP" );
   branchNames.insert( "nHits" );
   branchNames.insert( "pt" );  
   
   // Set up the root file with the tree and the branches
   _treeName = "values";
   setUpRootFile( _rootFileName, _treeName, branchNames );      //prepare the root file.
   
   
   
   
   
   
 
   
}


void StepAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void StepAnalyser::processEvent( LCEvent * evt ) { 
   
   
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();

   
   
   for( int i=0; i < nMCTracks; i++){ //over all tracks
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );     // the relation
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );             // the monte carlo particle
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );           // the track
      
      
      std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
      
      //sort the hits
      sort( trackerHits.begin(), trackerHits.end(), compare_z ); 
      
      int path = 0; // An integer encoding the path of the particle: 5432 = a hit moving from layer 2 to layer 3, 4 and then 5
                    // 542 is the same track, but skipping layer 3.
                    // Layers 0..6 are used at the moment in the FTD so those numbers will be contained in the int.
                    // For the case that the track cheater also takes tracks entering the TPC or other detectors,
                    // a hit which does not belong to the FTD is marked with the number 9.
                    // Additionally the integer gets a signs for its direction
                    
      int side = 0;
      int layer = 0;
      
      for( int j =  trackerHits.size() -1; j >= 0 ; j-- ){ // over all hits (start with the outer ones)
      
        
         
         //get the layer
         BitField64  cellID( ILDCellID0::encoder_string );
         
         cellID.setValue( trackerHits[j]->getCellID0() );
         
         int detector = cellID[ ILDCellID0::subdet ];
         side         = cellID[ ILDCellID0::side   ];
         layer        = cellID[ ILDCellID0::layer  ];
//          int module   = cellID[ ILDCellID0::module ];
//          int sensor   = cellID[ ILDCellID0::sensor ];
         
         if ( detector != ILDDetID::FTD ) layer=9; //a 9 marks a hit from a different detector
         
         
         path *= 10; 
         path += layer;
            

      }
      
      int lastLayerBeforeIP = layer; //The last layer before the IP
      int nHits = trackerHits.size(); //Number of hits in the track
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] ); //transversal momentum
      
      path *= side;
      
      
      // Store the data in a root file
      std::map < std::string , float > rootData;
      rootData.insert( std::pair< std::string , float >( "path" , path ) );
      rootData.insert( std::pair< std::string , float >( "lastLayerBeforeIP" , lastLayerBeforeIP ) );
      rootData.insert( std::pair< std::string , float >( "nHits" , nHits ) );
      rootData.insert( std::pair< std::string , float >( "pt" , pt ) );
      
      saveToRoot( _rootFileName, _treeName , rootData );


      
   }
      
 
   
 


   //-- note: this will not be printed if compiled w/o MARLINDEBUG4=1 !

   streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
   << "   in run:  " << evt->getRunNumber() << std::endl ;


   _nEvt ++ ;
   
   
}



void StepAnalyser::check( LCEvent * evt ) { 
   // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void StepAnalyser::end(){ 
   
   //   streamlog_out( DEBUG ) << "MyProcessor::end()  " << name() 
   //      << " processed " << _nEvt << " events in " << _nRun << " runs "
   //      << std::endl ;
   
}

