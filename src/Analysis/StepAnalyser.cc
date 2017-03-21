#include "StepAnalyser.h"

#include <iostream>
#include <cmath>
#include <algorithm>

#include "marlin/VerbosityLevels.h"
#include "EVENT/LCCollection.h"
#include "EVENT/MCParticle.h"
#include "EVENT/LCRelation.h"
#include "EVENT/Track.h"
#include "IMPL/TrackerHitPlaneImpl.h"
#include "UTIL/LCTrackerConf.h"

#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"

#include "Tools/KiTrackMarlinTools.h"





using namespace lcio ;
using namespace marlin ;




StepAnalyser aStepAnalyser ;


StepAnalyser::StepAnalyser() : Processor("StepAnalyser") {
   
   // modify processor description
   _description = "StepAnalyser: Stores information about the path of a particle" ;
   
   
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
   

   branchNames.insert( "lastLayerBeforeIP" );
   branchNames.insert( "nHits" );
   branchNames.insert( "pt" );  
   
   // Set up the root file with the tree and the branches
   _treeName = "values";
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeName, branchNames );      //prepare the root file.
   
   
   branchNames.clear();
   branchNames.insert( "LayerA" );
   branchNames.insert( "LayerB" );
   branchNames.insert( "LayerDist" );
   branchNames.insert( "ModuleA" );
   branchNames.insert( "ModuleB" );
   branchNames.insert( "ModuleDist" );
   branchNames.insert( "SensorA" );
   branchNames.insert( "SensorB" );
   branchNames.insert( "SensorDist" );
   branchNames.insert("pt");
   
   _treeName2 = "hitPairs";
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeName2, branchNames , false );   
   
   
   
 
   
}


void StepAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void StepAnalyser::processEvent( LCEvent * evt ) { 
   
   
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   int nMCTracks = col->getNumberOfElements();

   std::vector < std::map < std::string , float > > rootDataVec;
   std::vector < std::map < std::string , float > > rootDataVec2;
   
   for( int i=0; i < nMCTracks; i++){ //over all tracks
      
      
      LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );     // the relation
      MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );             // the monte carlo particle
      Track*    track = dynamic_cast <Track*>      (rel->getFrom() );           // the track
      
      double pt = sqrt( mcp->getMomentum()[0]*mcp->getMomentum()[0] + mcp->getMomentum()[1]*mcp->getMomentum()[1] ); //transversal momentum
      
      std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
      
      //sort the hits
      sort( trackerHits.begin(), trackerHits.end(), KiTrackMarlin::compare_TrackerHit_z ); 
      

      int lastLayerBeforeIP = 0;
      int prevLayer = 0;
      int prevModule = 0;
      int prevSensor = 0;
      
      for( unsigned j = 0; j < trackerHits.size() ; j++ ){ // over all hits (start with the outer ones)
      
         
         BitField64  cellID( LCTrackerCellID::encoding_string() );
         
         cellID.setValue( trackerHits[j]->getCellID0() );
         
//          int detector = cellID[ LCTrackerCellID::subdet() ];
//          int side         = cellID[ LCTrackerCellID::side()   ];
         int layer        = cellID[ LCTrackerCellID::layer()  ];
         int module   = cellID[ LCTrackerCellID::module() ];
         int sensor   = cellID[ LCTrackerCellID::sensor() ];
         
         if (j == 0) lastLayerBeforeIP = layer;
         
         if( j >= 1){
            
            std::map < std::string , float > rootData;
            rootData["LayerA"] = prevLayer;
            rootData["LayerB"] = layer;
            rootData[ "LayerDist" ] = fabs( layer - prevLayer );
            rootData["ModuleA"] = prevModule;
            rootData["ModuleB"] = module;
            rootData[ "ModuleDist" ] = fabs( module - prevModule );
            rootData["SensorA"] = prevSensor;
            rootData["SensorB"] = sensor;
            rootData[ "SensorDist" ] = fabs( sensor - prevSensor );
            rootData["pt"] = pt;
            rootDataVec2.push_back( rootData );
         }
         
         prevLayer = layer;
         prevModule = module;
         prevSensor = sensor;
         
      }
      
      int nHits = trackerHits.size(); //Number of hits in the track
      
      
      
      
      // Store the data in a root file
      std::map < std::string , float > rootData;
      rootData["lastLayerBeforeIP"] = lastLayerBeforeIP;
      rootData["nHits"] = nHits;
      rootData["pt"] = pt;
      rootDataVec.push_back( rootData );
      



      
   }
      
 
   
   streamlog_out(DEBUG) << "Saving " << rootDataVec2.size() << "\n";

   KiTrackMarlin::saveToRoot( _rootFileName, _treeName , rootDataVec );
   KiTrackMarlin::saveToRoot( _rootFileName, _treeName2 , rootDataVec2 );


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




