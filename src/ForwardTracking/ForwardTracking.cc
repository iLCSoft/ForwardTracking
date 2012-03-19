#include "ForwardTracking.h"

#include <algorithm>

#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "EVENT/LCCollection.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
#include "UTIL/ILDConf.h"

#include "marlin/VerbosityLevels.h"

#include "MarlinCED.h"

#include "gear/GEAR.h"
#include "gear/GearParameters.h"
#include "gear/BField.h"
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"


//--------------------------------------------------------------
#include "FTDTrack.h"
#include "FTrackTools.h"
#include "FTrackILDTools.h"
#include "TrackSubsetHopfieldNN.h"
#include "TrackSubsetSimple.h"
#include "SegmentBuilder.h"
#include "Automaton.h"
#include "FTDHit01.h"
#include "FTDNeighborPetalSecCon.h"
#include "FTDSecCon01.h"
//--------------------------------------------------------------


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;
using namespace FTrackILD;
using namespace MarlinTrk ;





ForwardTracking aForwardTracking ;


ForwardTracking::ForwardTracking() : Processor("ForwardTracking") {

   // modify processor description
   _description = "ForwardTracking reconstructs tracks through the FTDs" ;


   // register steering parameters: name, description, class-variable, default value
   std::vector< std::string > collections;
   collections.push_back( "FTDTrackerHits" );
   collections.push_back( "FTDSpacePoints" );
   
   registerProcessorParameter( "FTDHitCollections",
                               "FTD Hit Collections",
                               _FTDHitCollections,
                               collections); 
   
//    registerInputCollection(LCIO::TRACKERHIT,
//                            "FTDHitCollections",
//                            "FTD Hit Collections",
//                            _FTDHitCollections,
//                            collections); 


   registerOutputCollection(LCIO::TRACK,
                           "ForwardTrackCollection",
                           "Name of the Forward Tracking output collection",
                           _ForwardTrackCollection,
                           std::string("ForwardTracks"));

   
   registerProcessorParameter("Chi2ProbCut",
                              "Tracks with a chi2 probability below this will get sorted out",
                              _chi2ProbCut,
                              double(0.005));
   

   registerProcessorParameter("OverlappingHitsDistMax",
                              "The maximum distance of hits from overlapping petals belonging to one track",
                              _overlappingHitsDistMax,
                              double(3.5));
   
   
   registerProcessorParameter( "HitsPerTrackMin",
                               "The minimum number of hits to create a track",
                               _hitsPerTrackMin,
                               int( 3 ) );
   
   
   registerProcessorParameter( "BestSubsetFinder",
                               "The method used to find the best non overlapping subset of tracks. Available are: TrackSubsetHopfieldNN and TrackSubsetSimple",
                               _bestSubsetFinder,
                               std::string( "TrackSubsetHopfieldNN" ) );
   
   registerProcessorParameter( "TakeBestVersionOfTrack",
                               "Whether when adding hits to a track only the track with highest quality should be further processed",
                               _takeBestVersionOfTrack,
                               bool( true ) );

   
   
   //For fitting:
   
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
   
   /////////////////////////////////////////////////////////////////////////////////
   // The Criteria for the Cellular Automaton
   
   std::vector< std::string > allCriteria = Criteria::getAllCriteriaNamesVec();
   
   
   registerProcessorParameter( "Criteria",
                               "A vector of the criteria that are going to be used. For every criterion a min and max needs to be set!!!",
                               _criteriaNames,
                               allCriteria);
   
   for( unsigned i=0; i < _criteriaNames.size(); i++ ){
    
      
      std::string critMinString = _criteriaNames[i] + "_min";
      
      registerProcessorParameter( critMinString,
                                  "The minimum of " + _criteriaNames[i],
                                  _critMinima[ _criteriaNames[i] ],
                                  float( 0. ));
      
      
      std::string critMaxString = _criteriaNames[i] + "_max";
      
      registerProcessorParameter( critMaxString,
                                  "The maximum of " + _criteriaNames[i],
                                  _critMaxima[ _criteriaNames[i] ],
                                  float( 0. ));
      
   
   }
   
   /////////////////////////////////////////////////////////////////////////////////
   
}




void ForwardTracking::init() { 

   streamlog_out(DEBUG) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

   _nRun = 0 ;
   _nEvt = 0 ;

   _useCED = false;
   if( _useCED )MarlinCED::init(this) ;    //CED
   
    
    
   const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
   const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
   int nLayers = ftdLayers.getNLayers() + 1;
   int nModules = ftdLayers.getNPetals(0);
   int nSensors = ftdLayers.getNSensors(0);
   
   for( int i=1; i<nLayers; i++){
     
      if( ftdLayers.getNPetals(i) > nModules ) nModules = ftdLayers.getNPetals(i); 
      if( ftdLayers.getNSensors(i) > nSensors ) nSensors = ftdLayers.getNSensors(i);
     
   }
   
   streamlog_out( DEBUG4 ) << "using " << nLayers - 1 << " layers, " << nModules << " petals and " << nSensors << " sensors.\n";
   
   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );
   
   
   _Bz = Global::GEAR->getBField().at( gear::Vector3D(0., 0., 0.) ).z();    //The B field in z direction
   
   //Initialise the TrackFitter of the tracks:
   FTDTrack::initialiseFitter( "KalTest" , marlin::Global::GEAR , "" , _MSOn , _ElossOn , _SmoothOn  );



   // store the criteria 
   for( unsigned i=0; i<_criteriaNames.size(); i++ ){
      
      std::string critName = _criteriaNames[i];
      
      ICriterion* crit = Criteria::createCriterion( critName, _critMinima[critName] , _critMaxima[critName] );
      
      std::string type = crit->getType();
      
      streamlog_out( DEBUG4 ) <<  "\nAdded: Criterion " << critName << " (type =  " << type 
                              << " ). Min = " << _critMinima[critName]
                              << ", Max = " << _critMaxima[critName];
      
      if( type == "2Hit" ){
         
         _crit2Vec.push_back( crit );
         
      }
      else 
      if( type == "3Hit" ){
         
         _crit3Vec.push_back( crit );
         
      }
      else 
      if( type == "4Hit" ){
         
         _crit4Vec.push_back( crit );
         
      }
      else delete crit;


   }



}


void ForwardTracking::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void ForwardTracking::processEvent( LCEvent * evt ) { 

   
   
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                                                                                              //
   //                                                                                                              //
   //                            Track Reconstruction in the FTD                                                   //
   //                                                                                                              //
   //                                                                                                              //
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   

//--CED---------------------------------------------------------------------
// Reset drawing buffer and START drawing collection

   if( _useCED ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
      
   }

//-----------------------------------------------------------------------
  


   std::vector< IHit* > hitsTBD; //Hits to be deleted at the end
   _map_sector_hits.clear();

   
   /**********************************************************************************************/
   /*    Read in the collections, create hits from the TrackerHits and store them in a map       */
   /**********************************************************************************************/
   
   streamlog_out( DEBUG4 ) << "\t\t---Reading in Collections---\n" ;
   
   
   for( unsigned iCol=0; iCol < _FTDHitCollections.size(); iCol++ ){
      
      
      LCCollection* col = evt->getCollection( _FTDHitCollections[iCol] ) ;
      unsigned nHits = col->getNumberOfElements();
      
      streamlog_out( DEBUG4 ) << "Number of hits in collection " << _FTDHitCollections[iCol] << ": " << nHits <<"\n";
      
      
      for(unsigned i=0; i< nHits ; i++){
         
         
         TrackerHit* trackerHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         streamlog_out(DEBUG1) << "hit" << i << " " << FTrackILD::getCellID0Info( trackerHit->getCellID0() );
         
         //Make an FTDHit01 from the TrackerHit 
         
         FTDHit01* ftdHit = new FTDHit01 ( trackerHit , _sectorSystemFTD );
         hitsTBD.push_back(ftdHit); //so we can easily delete every created hit afterwards
         
         _map_sector_hits[ ftdHit->getSector() ].push_back( ftdHit );         
         
      }
      
   }
  


   
   if( !_map_sector_hits.empty() ){
      
     
      /**********************************************************************************************/
      /*                Add the IP as virtual hit for forward and backward                          */
      /**********************************************************************************************/
      
      IHit* virtualIPHitForward = createVirtualIPHit(1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitForward );
      _map_sector_hits[ virtualIPHitForward->getSector() ].push_back( virtualIPHitForward );
      
      IHit* virtualIPHitBackward = createVirtualIPHit(-1 , _sectorSystemFTD );
      hitsTBD.push_back( virtualIPHitBackward );
      _map_sector_hits[ virtualIPHitBackward->getSector() ].push_back( virtualIPHitBackward );
      
      std::string inf = getInfo_map_sector_hits(); 
      streamlog_out( DEBUG2 ) << inf;
      
      /**********************************************************************************************/
      /*                Check the possible connections of hits on overlapping petals                */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG4 ) << "\t\t---Overlapping Hits---\n" ;
      
      std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack = getOverlapConnectionMap( _map_sector_hits, _sectorSystemFTD, _overlappingHitsDistMax);
      
      /**********************************************************************************************/
      /*                Build the segments                                                          */
      /**********************************************************************************************/
      
      
      streamlog_out( DEBUG4 ) << "\t\t---SegementBuilder---\n" ;
      
      //Create a segmentbuilder
      SegmentBuilder segBuilder( _map_sector_hits );
      
      
      segBuilder.addCriteria ( _crit2Vec );
      
      //Also load hit connectors
      unsigned layerStepMax = 2; // how many layers to go at max
      unsigned petalStepMax = 1; // how many petals to go at max
      unsigned lastLayerToIP = 4;// layer 1,2...4 get connected directly to the IP
      FTDSecCon01 secCon( _sectorSystemFTD , layerStepMax , petalStepMax , lastLayerToIP );
      
      
      segBuilder.addSectorConnector ( & secCon );
      
      
      // And get out the 1-segments 
      Automaton automaton = segBuilder.get1SegAutomaton();
      
      
      /**********************************************************************************************/
      /*                Automaton                                                                   */
      /**********************************************************************************************/
      
      
      
      streamlog_out( DEBUG4 ) << "\t\t---Automaton---\n" ;
      
      if( _useCED ) automaton.drawSegments();
      
      /*******************************/
      /*      2-hit segments         */
      /*******************************/
      streamlog_out( DEBUG4 ) << "\t\t--2-hit-Segments--\n" ;
      
      automaton.clearCriteria();
      automaton.addCriteria( _crit3Vec );  
      
      
      // Let the automaton lengthen its 1-segments to 2-segments
      automaton.lengthenSegments();
      
      // So now we have 2-segments and are ready to perform the cellular automaton.
      
      // Perform the automaton
      automaton.doAutomaton();
      
      //Clean segments with bad states
      automaton.cleanBadStates();
      
      //Clean connections of segments (this uses the same criteria again as before)
      automaton.cleanBadConnections();
      
      //Reset the states of all segments
      automaton.resetStates();
      
      
      /*******************************/
      /*      3-hit segments         */
      /*******************************/
      streamlog_out( DEBUG4 ) << "\t\t--3-hit-Segments--\n" ;
      
      
      automaton.clearCriteria();
      automaton.addCriteria( _crit4Vec );      
      
      automaton.lengthenSegments();
      // So now we have 3 hit segments 
      
      // Perform the automaton
      automaton.doAutomaton();

      //Clean segments with bad states
      automaton.cleanBadStates();
       
      //Clean connections of segments (this uses the same criteria again as before)
      automaton.cleanBadConnections();
      
      //Reset the states of all segments
      automaton.resetStates();
      
      std::vector < RawTrack > autRawTracks = automaton.getTracks( _hitsPerTrackMin );
      streamlog_out( DEBUG3 ) << " Automaton returned " << autRawTracks.size() << " tracks \n";
      
      
      
      /**********************************************************************************************/
      /*                Add the overlapping hits                                                    */
      /**********************************************************************************************/
      
      
      streamlog_out( DEBUG4 ) << "\t\t---Add hits from overlapping petals + fit + chi2prob Cuts---\n" ;
      
      
      std::vector <ITrack*> trackCandidates;
      
      
      // for all raw tracks
      for( unsigned i=0; i < autRawTracks.size(); i++){
         
         
         // get all versions of the track with hits from overlapping petals
         std::vector < RawTrack > autRawTracksPlus = getRawTracksPlusOverlappingHits( autRawTracks[i], map_hitFront_hitsBack );
         
         
         /**********************************************************************************************/
         /*                Make track candidates, fit them and throw away bad ones                     */
         /**********************************************************************************************/
         
         std::vector< ITrack* > overlappingTrackCands;
         
         for( unsigned j=0; j < autRawTracksPlus.size(); j++ ){
            
            
            ITrack* trackCand = new FTDTrack( autRawTracksPlus[j] );
            streamlog_out( DEBUG2 ) << "Fitting track candidate with " << trackCand->getHits().size() << " hits\n";
            
//             std::vector< IHit* > testHits = trackCand->getHits();
//             for( unsigned k=0; k < testHits.size(); k++ ){
//                
//                std::string inf = _sectorSystemFTD->getInfoOnSector( testHits[k]->getSector() );
//                streamlog_out( DEBUG2) << "(" << testHits[k]->getX() << "," << testHits[k]->getY() << "," << testHits[k]->getZ() << ")" << inf;
//             }
//             streamlog_out(DEBUG2) << "\n";
            
            trackCand->fit();
            
            streamlog_out( DEBUG2 ) << " Track " << trackCand 
                                    << " chi2Prob = " << trackCand->getChi2Prob() 
                                    << "( chi2=" << trackCand->getChi2() 
                                    <<", Ndf=" << trackCand->getNdf() << " )\n";
            
            
            if ( trackCand->getChi2Prob() > _chi2ProbCut ){
               
               overlappingTrackCands.push_back( trackCand );
               streamlog_out( DEBUG2 ) << "Track accepted (chi2prob " << trackCand->getChi2Prob() << " > " << _chi2ProbCut << "\n";
               
            }
            else{
               
               streamlog_out( DEBUG2 ) << "Track rejected (chi2prob " << trackCand->getChi2Prob() << " <= " << _chi2ProbCut << "\n";
               delete trackCand;
               
            }
            
         }
         
         /**********************************************************************************************/
         /*                Take the best version of the track                                          */
         /**********************************************************************************************/
        // Now we have all versions of one track, coming from adding possible hits from overlapping petals.
         
         if( _takeBestVersionOfTrack ){ // we want to take only the best version
            
            
            streamlog_out( DEBUG2 ) << "Take the version of the track with best quality from " << overlappingTrackCands.size() << " track candidates\n";
            
            if( !overlappingTrackCands.empty() ){
               
               ITrack* bestTrack = overlappingTrackCands[0];
               
               for( unsigned j=1; j < overlappingTrackCands.size(); j++ ){
                  
                  if( overlappingTrackCands[j]->getQI() > bestTrack->getQI() ){
                     
                     delete bestTrack; //delete the old one, not needed anymore
                     bestTrack = overlappingTrackCands[j];
                  }
                  else{
                     
                     delete overlappingTrackCands[j]; //delete this one
                     
                  }
                  
               }
               streamlog_out( DEBUG2 ) << "Adding best track candidate with " << bestTrack->getHits().size() << " hits\n";
               
               trackCandidates.push_back( bestTrack );
               
            }
            
         }
         else{ // we take all versions
            
            streamlog_out( DEBUG2 ) << "Taking all " << overlappingTrackCands.size() << " versions of the track\n";
            trackCandidates.insert( trackCandidates.end(), overlappingTrackCands.begin(), overlappingTrackCands.end() );
            
         }
         
      }
      
      
      /**********************************************************************************************/
      /*               Get the best subset of tracks                                                */
      /**********************************************************************************************/
      streamlog_out( DEBUG4 ) << "\t\t---Get best subset of tracks---\n" ;
      
      std::vector< ITrack* > tracks;
      
      ITrackSubset* subset = NULL;
      
      // Make a TrackSubset
      if( _bestSubsetFinder == "TrackSubsetHopfieldNN" ) subset = new TrackSubsetHopfieldNN();
      if( _bestSubsetFinder == "TrackSubsetSimple" ) subset = new TrackSubsetSimple();
      
      
      if( subset != NULL ){
        
         subset->addTracks( trackCandidates ); 
         
         //Calculate the best subset:
         subset->calculateBestSet();
         
         std::vector< ITrack* > tracks = subset->getAcceptedTracks();
         std::vector< ITrack* > rejectedTracks = subset->getRejectedTracks();
         
         // immediately delete the rejected ones
         for ( unsigned i=0; i<rejectedTracks.size(); i++){
            
            delete rejectedTracks[i];
            
         }
         
         delete subset;
         
      }
      else tracks = trackCandidates;
      
      
      
      /**********************************************************************************************/
      /*               Finally: save the tracks                                                     */
      /**********************************************************************************************/
      streamlog_out( DEBUG4 ) << "\t\t---Save Tracks---\n" ;
      
      LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
      
      LCFlagImpl hitFlag(0) ;
      hitFlag.setBit( LCIO::TRBIT_HITS ) ;
      trkCol->setFlag( hitFlag.getFlag()  ) ;

      for (unsigned int i=0; i < tracks.size(); i++){
         
         FTDTrack* myTrack = dynamic_cast< FTDTrack* >( tracks[i] );
         
         if( myTrack != NULL ) trkCol->addElement( myTrack->getLcioTrack() );
         
         
      }
      evt->addCollection(trkCol,_ForwardTrackCollection.c_str());
      
      
      
      streamlog_out (MESSAGE0) << "\nForward Tracking found and saved " << tracks.size() << " tracks.\n"; 
      
      
      /**********************************************************************************************/
      /*                Clean up                                                                    */
      /**********************************************************************************************/
      
      // delete all the created IHits
      for ( unsigned i=0; i<hitsTBD.size(); i++ )  delete hitsTBD[i];
      
      // delete the FTracks
      for (unsigned int i=0; i < tracks.size(); i++){ delete tracks[i];}
      
      
      
      
   }




   if( _useCED ) MarlinCED::draw(this);


   _nEvt ++ ;
   
}





void ForwardTracking::check( LCEvent * evt ) {}


void ForwardTracking::end(){
   
   for ( unsigned i=0; i< _crit2Vec.size(); i++) delete _crit2Vec[i];
   for ( unsigned i=0; i< _crit3Vec.size(); i++) delete _crit3Vec[i];
   for ( unsigned i=0; i< _crit4Vec.size(); i++) delete _crit4Vec[i];
   _crit2Vec.clear();
   _crit3Vec.clear();
   _crit4Vec.clear();
   
   delete _sectorSystemFTD;
   _sectorSystemFTD = NULL;
   
}




std::map< IHit* , std::vector< IHit* > > ForwardTracking::getOverlapConnectionMap( 
            std::map< int , std::vector< IHit* > > & map_sector_hits, 
            const SectorSystemFTD* secSysFTD,
            float distMax){
   
   
   unsigned nConnections=0;

   
   std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack;
   std::map< int , std::vector< IHit* > >::iterator it;
   
   //for every sector
   for ( it= map_sector_hits.begin() ; it != map_sector_hits.end(); it++ ){
      
     
      std::vector< IHit* > hitVecA = it->second;
      int sector = it->first;
      // get the neighbouring petals
      FTDNeighborPetalSecCon secCon( secSysFTD );
      std::set< int > targetSectors = secCon.getTargetSectors( sector );
      
      
      //for all neighbouring petals
      for ( std::set<int>::iterator itTarg = targetSectors.begin(); itTarg!=targetSectors.end(); itTarg++ ){
         
         
         std::vector< IHit* > hitVecB = map_sector_hits[ *itTarg ];
         
         for ( unsigned j=0; j < hitVecA.size(); j++ ){
            
            
            IHit* hitA = hitVecA[j];
            
            for ( unsigned k=0; k < hitVecB.size(); k++ ){
               
               
               IHit* hitB = hitVecB[k];
               
               
               float dx = hitA->getX() - hitB->getX();
               float dy = hitA->getY() - hitB->getY();
               float dz = hitA->getZ() - hitB->getZ();
               float dist = sqrt( dx*dx + dy*dy + dz*dz );
               
               if (( dist < distMax )&& ( fabs( hitB->getZ() ) > fabs( hitA->getZ() ) )  ){ // if they are close enough and B is behind A
                  
                  
                  streamlog_out( DEBUG2 ) << "Connected: (" << hitA->getX() << "," << hitA->getY() << "," << hitA->getZ() << ")-->("
                                          << hitB->getX() << "," << hitB->getY() << "," << hitB->getZ() << ")\n";
                  
                  map_hitFront_hitsBack[ hitA ].push_back( hitB );
                  nConnections++;
                  
               }
               
            }
            
         } 
         
      }
     
   }
   
   streamlog_out( DEBUG3 ) << "Connected " << map_hitFront_hitsBack.size() << " hits with " << nConnections << " possible overlapping hits\n";
   
   
   return map_hitFront_hitsBack;
   
   
   
}


std::string ForwardTracking::getInfo_map_sector_hits(){
   
   
   std::stringstream s;
   
   std::map< int , std::vector< IHit* > >::iterator it;
   
   for( it = _map_sector_hits.begin(); it != _map_sector_hits.end(); it++ ){
      
      
      std::vector<IHit*> hits = it->second;
      int sector = it->first;
      
      int side = _sectorSystemFTD->getSide( sector );
      unsigned layer = _sectorSystemFTD->getLayer( sector );
      unsigned module = _sectorSystemFTD->getModule( sector );
      unsigned sensor = _sectorSystemFTD->getSensor( sector );
      
      s << "sector " << sector  << " (si"
      << side << ",la"
      << layer << ",mo"
      << module << "se,"
      << sensor << ") has "
      << hits.size() << " hits\n";
      
      
   }  
   
   
   return s.str();   
   
}

std::vector < RawTrack > ForwardTracking::getRawTracksPlusOverlappingHits( RawTrack rawTrack , std::map< IHit* , std::vector< IHit* > >& map_hitFront_hitsBack ){
   
   
   
   // So we have a raw track (a vector of hits, that is) and a map, that tells us
   // for every hit, if there is another hit in the overlapping region behind it very close,
   // so that it could be part of the same track.
   //
   // We now want to find for a given track all possible tracks, when hits from the overlapping regions are added
   //
   // The method is this: start with pure track.
   // Make a vector of rawTracks and fill in the pure track.
   // For every hit on the original track do the following:
   // Check if there are overlapping hits.
   // For every overlapping hit take all the created tracks so far and make another version
   // with the overlapping hit added to it and add them to the vector of rawTracks.
   //
   //
   // Let's do an example: 
   // the original hits in the track are calles A,B and C.
   // A has one overlapping hit A1
   // and B has two overlapping hits B1 and B2.
   //
   // So we start with a vector containing only the original track: {(A,B,C)}
   //
   // We start with the first hit: A. It has one overlapping hit A1.
   // We take all tracks (which is the original one so far) and make another version containing A1 as well.
   // Then we add it to the vector of tracks:
   //
   // {(A,B,C)(A,A1,B,C)}
   //
   // On to the next hit from the original track: B. Here we have overlapping hits B1 and B2.
   // We take all the tracks so far and add versions with B1: (A,B,B1,C) and (A,A1,B,B1,C)
   // We don't immediately add them or otherwise, we would create a track containing B1 as well as B2, which is plainly wrong
   //
   // So instead we make the combinations with B2: (A,B,B2,C) and (A,A1,B,B2,C)
   // And now having gone through all overlapping hits of B, we add all the new versions to the vector:
   //
   // {(A,B,C)(A,A1,B,C)(A,B,B1,C)(A,A1,B,B1,C)(A,B,B2,C)(A,A1,B,B2,C)}
   //
   // So now we have all possible versions of the track with overlapping hits
   
   std::vector < RawTrack > rawTracksPlus;
   
   rawTracksPlus.push_back( rawTrack ); //add the original one
   
   // for every hit in the original track
   for( unsigned i=0; i < rawTrack.size(); i++ ){
      
     
      IHit* frontHit = rawTrack[i];
      
      // get the hits that are behind frontHit
      std::map< IHit* , std::vector< IHit* > >::iterator it;
      it = map_hitFront_hitsBack.find( frontHit );
      if( it == map_hitFront_hitsBack.end() ) continue; // if there are no hits on the back skip this one
      std::vector< IHit* > backHits = it->second; 
      
      
      // Create the different versions of the tracks so far with the hits from the back
      
      std::vector< RawTrack > newVersions; //here we store all the versions with overlapping hits from the different back hits at this frontHit
      
      // for every hit in back of the frontHit
      for( unsigned j=0; j<backHits.size(); j++ ){
         
         
         IHit* backHit = backHits[j];
         
         // for all tracks we have so far
         for( unsigned k=0; k<rawTracksPlus.size(); k++ ){
            
            
            RawTrack newVersion = rawTracksPlus[k];     // exact copy of the track
            newVersion.push_back( backHit );          // add the backHit to it   
            newVersions.push_back( newVersion );         // store it
            
         }
         
      }
      
      // Now put all the new versions of the tracks into the rawTracksPlus vector before we go on to the next
      // hit of the original track
      rawTracksPlus.insert( rawTracksPlus.end(), newVersions.begin(), newVersions.end() );
      
      
   }
   
   
   
   return rawTracksPlus;
   
}



