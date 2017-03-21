#include "DDForwardTracking.h"

#include <algorithm>

#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "EVENT/LCCollection.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
#include "UTIL/LCTrackerConf.h"
#include <UTIL/ILDConf.h>

#include "marlin/VerbosityLevels.h"
#include "marlin/Exceptions.h"

#include "MarlinCED.h"

//----From gear------------
#include "gear/GEAR.h"
#include "gear/GearParameters.h"
#include "gear/BField.h"
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"


//----From DD4Hep-----------------------------
#include "DD4hep/LCDD.h"
#include "DD4hep/DD4hepUnits.h"


//----From KiTrack-----------------------------
#include "KiTrack/SubsetHopfieldNN.h"
#include "KiTrack/SubsetSimple.h"
#include "KiTrack/SegmentBuilder.h"
#include "KiTrack/Automaton.h"

//----From KiTrackMarlin-----------------------
#include "ILDImpl/FTDTrack.h"
#include "ILDImpl/FTDHit01.h"
#include "ILDImpl/FTDNeighborPetalSecCon.h"
#include "ILDImpl/FTDSectorConnector.h"
#include "Tools/KiTrackMarlinTools.h"
#include "Tools/KiTrackMarlinCEDTools.h"
#include "Tools/FTDHelixFitter.h"


#include "EndcapTrack.h"
#include "EndcapHit01.h"
#include "EndcapHitSimple.h"
// #include "EndcapNeighborSecCon.h" // FIXME: TO BE IMPLEMENTED!!
#include "EndcapSectorConnector.h"
#include "EndcapHelixFitter.h"


using namespace lcio ;
using namespace marlin ;
using namespace MarlinTrk ;

// Used to fedine the quality of the track output collection
const int DDForwardTracking::_output_track_col_quality_GOOD = 1;
const int DDForwardTracking::_output_track_col_quality_FAIR = 2;
const int DDForwardTracking::_output_track_col_quality_POOR = 3;


DDForwardTracking aDDForwardTracking ;


DDForwardTracking::DDForwardTracking() : Processor("DDForwardTracking") {

   _description = "DDForwardTracking reconstructs tracks through the FTD" ;





   ////////////////////////


   registerProcessorParameter("NDivisionsInPhi",
			      "Number of divisions in Phi",
			      _nDivisionsInPhi,
			      int(80));
  
  
   registerProcessorParameter("NDivisionsInTheta",
			      "Number of divisions in Theta",
			      _nDivisionsInTheta,
			      //int(80));
			      int(180));

   ////////////////////////



   std::vector< std::string > collections;
   collections.push_back( "FTDTrackerHits" );
   collections.push_back( "FTDSpacePoints" );
   
   registerProcessorParameter( "FTDHitCollections",
                               "FTD Hit Collections",
                               _FTDHitCollections,
                               collections); 
   

   registerOutputCollection(LCIO::TRACK,
                           "ForwardTrackCollection",
                           "Name of the Forward Tracking output collection",
                           _ForwardTrackCollection,
                           std::string("ForwardTracks"));

   
   registerProcessorParameter("Chi2ProbCut",
                              "Tracks with a chi2 probability below this will get sorted out",
                              _chi2ProbCut,
                              double(0.005));
   
   
   registerProcessorParameter("HelixFitMax",
                              "The maximum chi2/Ndf that is allowed as result of a helix fit",
                              _helixFitMax,
                              double( 500 ) );
   

   registerProcessorParameter("OverlappingHitsDistMax",
                              "The maximum distance of hits from overlapping petals belonging to one track",
                              _overlappingHitsDistMax,
                              //double(3.5));
                              double(4.0));
   
   
   registerProcessorParameter( "HitsPerTrackMin",
                               "The minimum number of hits to create a track",
                               _hitsPerTrackMin,
                               int( 4 ) );
   
   
   registerProcessorParameter( "BestSubsetFinder",
                               "The method used to find the best non overlapping subset of tracks. Available are: SubsetHopfieldNN, SubsetSimple and None",
                               _bestSubsetFinder,
                               std::string( "SubsetHopfieldNN" ) );
   
   
   registerProcessorParameter( "TakeBestVersionOfTrack",
                               "Whether when adding hits to a track only the track with highest quality should be further processed",
                               _takeBestVersionOfTrack,
                               bool( true ) );

   
   // Parameters for the Hopfield Neural Network
   
   registerProcessorParameter("HNN_Omega",
                              "Omega for the Hopfield Neural Network; the higher omega the higher the influence of the quality indicator",
                              _HNN_Omega,
                              double( 0.75 ) );
   
   registerProcessorParameter("HNN_Activation_Threshold",
                              "The activation threshold for the Hopfield Neural Network",
                              _HNN_ActivationThreshold,
                              double( 0.5 ) );
   
   registerProcessorParameter("HNN_TInf",
                              "The temperature limit of the Hopfield Neural Network",
                              _HNN_TInf,
                              double( 0.1 ) );
   
   
   
   // Security checks to prevent combinatorial disasters
   
   registerProcessorParameter( "MaxConnectionsAutomaton",
                               "If the automaton has more connections than this it will be redone with the next set of cut off parameters",
                               _maxConnectionsAutomaton,
                               //int( 100000 ) );
                               int( 920 ) );
   
   
   registerProcessorParameter("MaxHitsPerSector",
                              "Maximal number of hits allowed on a sector. More will cause drop of hits in sector",
                              _maxHitsPerSector,
                              int(1000));
   
   
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
   
   registerProcessorParameter( "TrackSystemName",
			       "Name of the track fitting system to be used (KalTest, DDKalTest, aidaTT, ... )",
			       _trkSystemName,
			       std::string("DDKalTest") );

   registerProcessorParameter("GetTrackStateAtCaloFace",
                              "Set to false if no track state at the calorimeter is needed",
                              _getTrackStateAtCaloFace,
                              bool(true));
  

   // The Criteria for the Cellular Automaton:
   
   std::vector< std::string > allCriteria = Criteria::getAllCriteriaNamesVec();
   
   
   registerProcessorParameter( "Criteria",
                               "A vector of the criteria that are going to be used. For every criterion a min and max needs to be set!!!",
                               _criteriaNames,
                               allCriteria);
   
   
   // Now set min and max values for all the criteria
   for( unsigned i=0; i < _criteriaNames.size(); i++ ){
    
      std::vector< float > emptyVec;
     
      std::string critMinString = _criteriaNames[i] + "_min";
      
      registerProcessorParameter( critMinString,
                                  "The minimum of " + _criteriaNames[i],
                                  _critMinima[ _criteriaNames[i] ],
                                  emptyVec);
      
      
      std::string critMaxString = _criteriaNames[i] + "_max";
      
      registerProcessorParameter( critMaxString,
                                  "The maximum of " + _criteriaNames[i],
                                  _critMaxima[ _criteriaNames[i] ],
                                  emptyVec);
      
      
   }
   

   
}




void DDForwardTracking::init() { 

   streamlog_out( DEBUG3 ) << "   init called  " << std::endl ;

   // usually a good idea to
   printParameters() ;

   _nRun = 0 ;
   _nEvt = 0 ;

   _useCED = false; // Setting this to on will initialise CED in the processor and tracks or segments (from the CA)
                    // can be printed. As this is mainly used for debugging it is not a steerable parameter.
   if( _useCED )MarlinCED::init(this) ;    //CED
   
   

   /**********************************************************************************************/
   /*       Make a SectorSystemEndcap                                                             */
   /**********************************************************************************************/
  
   // The SectorSystemEndcap is the object translating the sectors of the hits into layers, modules etc. and vice versa

   // int nLayers = 7 + 6 + 1; // 7 ITE layers, 6 OTE layers, +1 virtual layer for the IP
   int nLayers = 6 + 7 + 5 + 1;// 6vtx, 7 ITE layers, 5 OTE layers, +1 virtual layer for the IP
    

   // double theta_min = 7.*M_PI/180.;
   // double theta_max = 55.;
   // //double theta_max = atan(1930./1450.);
   // _dPhi = 2.*M_PI/_nDivisionsInPhi;
   // _dTheta = (theta_max-theta_min)/_nDivisionsInTheta;

   streamlog_out( DEBUG2 ) << " nLayer = " << nLayers << " \n";
   streamlog_out( DEBUG2 ) << " nDivisionsInPhi = " << _nDivisionsInPhi << " \n";
   streamlog_out( DEBUG2 ) << " nDivisionsInTheta = " << _nDivisionsInTheta << " \n";

   _sectorSystemEndcap = new SectorSystemEndcap( nLayers, _nDivisionsInPhi , _nDivisionsInTheta );
 
   
   // Get the B Field in z direction
      //---------DD4Hep-------------  
   DD4hep::Geometry::LCDD& lcdd = DD4hep::Geometry::LCDD::getInstance();
   const double pos[3]={0,0,0}; 
   double magneticFieldVector[3]={0,0,0}; 
   lcdd.field().magneticField(pos,magneticFieldVector); // get the magnetic field vector from DD4hep
   _Bz = magneticFieldVector[2]/dd4hep::tesla;

   streamlog_out( DEBUG2 ) << " Bz = " << _Bz << " \n";



   /**********************************************************************************************/
   /*       Initialise the MarlinTrkSystem, needed by the tracks for fitting                     */
   /**********************************************************************************************/

  // set up the geometry needed by TrkSystem
  _trkSystem =  MarlinTrk::Factory::createMarlinTrkSystem( _trkSystemName , marlin::Global::GEAR , "" ) ;
  
  if( _trkSystem == 0 ){
    
    throw EVENT::Exception( std::string("  Cannot initialize MarlinTrkSystem of Type: ") + _trkSystemName  ) ;
    
  }
   
   // set the options   
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useQMS,        _MSOn ) ;       //multiple scattering
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::usedEdx,       _ElossOn) ;     //energy loss
   _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing,  _SmoothOn) ;    //smoothing

   // initialise the tracking system
   _trkSystem->init() ;
   
   
   
   /**********************************************************************************************/
   /*       Do a few checks, if the set parameters are right                                     */
   /**********************************************************************************************/

   
   // Only use allowed methods to find subsets. 
   assert( ( _bestSubsetFinder == "None" ) || ( _bestSubsetFinder == "SubsetHopfieldNN" ) || ( _bestSubsetFinder == "SubsetSimple" ) );
   
   // Use a sensible chi2prob cut. (chi squared probability, like any probability must range from 0 to 1)
   assert( _chi2ProbCut >= 0. );
   assert( _chi2ProbCut <= 1. );
   
   
   // Make sure, every used criterion exists and has at least one min and max set
   for( unsigned i=0; i<_criteriaNames.size(); i++ ){
      
      std::string critName = _criteriaNames[i];
      
      ICriterion* crit = Criteria::createCriterion( critName ); //throws an exception if the criterion is non existent
      delete crit;
      
      assert( !_critMinima[ critName ].empty() );
      assert( !_critMaxima[ critName ].empty() );
      
   }
   
   
   

}


void DDForwardTracking::processRunHeader( LCRunHeader* run) { 

   _nRun++ ;
} 



void DDForwardTracking::processEvent( LCEvent * evt ) { 

   streamlog_out( DEBUG4 ) << "processing event number " << _nEvt << "\n";
   
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                                                                                              //
   //                                 DDForwardTracking                                                              //
   //                                                                                                              //
   //                            Track Reconstruction in the FTD                                                   //
   //                                                                                                              //
   //                                                                                                              //
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   

//--CED (only used for debugging )---------------------------------------
// Reset drawing buffer and START drawing collection

   if( _useCED ){
      
      MarlinCED::newEvent(this , 0) ; 
      
      CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();
      
      pHandler.update(evt); 
      
   }

//-----------------------------------------------------------------------
  
  
   // Reset the quality flag of the output track collection (we start with the assumption that our results are good.
   // If anything happens along the way, we modify this value )
   _output_track_col_quality = _output_track_col_quality_GOOD;

   std::vector< IHit* > hitsTBD; //Hits to be deleted at the end
   _map_sector_hits.clear();

   
   /**********************************************************************************************/
   /*    Read in the collections, create hits from the TrackerHits and store them in a map       */
   /**********************************************************************************************/
   
   streamlog_out( DEBUG4 ) << "\t\t---Reading in Collections---\n" ;
   
   
   for( unsigned iCol=0; iCol < _FTDHitCollections.size(); iCol++ ){ //read in all input collections
      
      
      LCCollection* col;
      
      
      try {
         
         col = evt->getCollection( _FTDHitCollections[iCol] ) ;
         
      }
      catch(DataNotAvailableException &e) {
         
         streamlog_out( DEBUG5 ) << "Collection " <<  _FTDHitCollections[iCol] <<  " is not available!\n";     
         continue;
         
      }
      
      unsigned nHits = col->getNumberOfElements();
      
      streamlog_out( DEBUG4 ) << "Number of hits in collection " << _FTDHitCollections[iCol] << ": " << nHits <<"\n";
      
      //just for debug
      //getCellID0AndPositionInfo( col );
      

      for(unsigned i=0; i< nHits ; i++){
                  
         TrackerHit* trackerHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
         
         if( trackerHit == NULL ){
            
            streamlog_out( DEBUG5 ) << "Cast to TrackerHit* was not possible, skipping element " << col->getElementAt(i) << "\n";
            continue;
            
         }       

	 //Make a EndcapHit01 from the TrackerHit
	 EndcapHit01* endcapHit = new EndcapHit01 ( trackerHit , _sectorSystemEndcap );
	 hitsTBD.push_back(endcapHit);
	 _map_sector_hits[ endcapHit->getSector() ].push_back( endcapHit );
	 
      }
      
   }
  

   //just for debug
   //std::string info = getInfo_map_sector_hits(); 
   //streamlog_out( DEBUG2 ) << info.c_str() << std::endl;
   
   
   if( !_map_sector_hits.empty() ){

      
      /**********************************************************************************************/
      /*                Check if no sector is overflowing with hits                                 */
      /**********************************************************************************************/
      
      
      std::map< int , std::vector< IHit* > >::iterator it;
      
      for( it=_map_sector_hits.begin(); it != _map_sector_hits.end(); it++ ){
       	  
	int nHits = it->second.size();
         streamlog_out( DEBUG2 ) << "Number of hits in sector " << it->first << " = " << nHits << "\n";
         
         if( nHits > _maxHitsPerSector ){
            
            it->second.clear(); //delete the hits in this sector, it will be dropped
            
            streamlog_out(ERROR)  << " ### EVENT " << evt->getEventNumber() << " :: RUN " << evt->getRunNumber() << " \n ### Number of Hits in FTD Sector " << it->first << ": " << nHits << " > " << _maxHitsPerSector << " (MaxHitsPerSector)\n : This sector will be dropped from track search, and QualityCode set to \"Poor\" " << std::endl;
           
            _output_track_col_quality = _output_track_col_quality_POOR; // We had to drop hits, so the quality of the result is decreased
            
         }
         
      }
      



      /**********************************************************************************************/
      /*                Check the possible connections of hits on overlapping petals                */
      /**********************************************************************************************/
      

      //ATT: at the moment overlap of hits in the same sector turned off to avoid bkg hits - to be investigated

      streamlog_out( DEBUG4 ) << "\t\t---Overlapping Hits---\n" ;
      
      std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack = getOverlapConnectionMap( _map_sector_hits, _sectorSystemEndcap, _overlappingHitsDistMax);
      
      
     

      /**********************************************************************************************/
      /*                Add the IP as virtual hit for forward and backward                          */
      /**********************************************************************************************/

      IHit* virtualIPHitForward = createVirtualIPHit( _sectorSystemEndcap );
      hitsTBD.push_back( virtualIPHitForward );
      _map_sector_hits[ virtualIPHitForward->getSector() ].push_back( virtualIPHitForward );
 
      
     
      
      /**********************************************************************************************/
      /*                SegmentBuilder and Cellular Automaton                                       */
      /**********************************************************************************************/
      
      unsigned round = 0; // the round we are in
      std::vector < RawTrack > rawTracks;
      
      // The following while loop ideally only runs once. (So we do round 0 and everything works)
      // It will repeat as long as the Automaton creates too many connections and as long as there are new criteria
      // parameters to use to cut down the problem.
      // Ideally already in round 0, there is a reasonable number of connections (not more than _maxConnectionsAutomaton), 
      // so the loop will be left. If however there are too many connections we stay in the loop and use 
      // (hopefully) tighter cut offs (if provided in the steering). This should prevent combinatorial breakdown
      // for very evil events.
      while( setCriteria( round ) ){
         
         
         round++; // count up the round we are in
         
         
         /**********************************************************************************************/
         /*                Build the segments                                                          */
         /**********************************************************************************************/
         
         streamlog_out( DEBUG4 ) << "\t\t---SegementBuilder---\n" ;
         
         //Create a segmentbuilder
         SegmentBuilder segBuilder( _map_sector_hits );
         
         segBuilder.addCriteria ( _crit2Vec ); // Add the criteria on when to connect two hits. The vector has been filled by the method setCriteria
         
         //Also load hit connectors
         unsigned layerStepMax = 1; // how many layers to go at max
         //unsigned layerStepMax = 2; // how many layers to go at max
         //unsigned lastLayerToIP = 9;// layer 1,2,3 and 4 get connected directly to the IP
         unsigned lastLayerToIP = 4;// layer 1,2,3 and 4 get connected directly to the IP
         EndcapSectorConnector secCon( _sectorSystemEndcap , layerStepMax, lastLayerToIP ) ;
         
         segBuilder.addSectorConnector ( & secCon ); // Add the sector connector (so the SegmentBuilder knows what hits from different sectors it is allowed to look for connections)
         
         
         // And get out the Cellular Automaton with the 1-segments 
         Automaton automaton = segBuilder.get1SegAutomaton();
         
         // Check if there are not too many connections
         if( automaton.getNumberOfConnections() > unsigned( _maxConnectionsAutomaton ) ){
            
            streamlog_out( DEBUG4 ) << "Redo the Automaton with different parameters, because there are too many connections:\n"
            << "\tconnections( " << automaton.getNumberOfConnections() << " ) > MaxConnectionsAutomaton( " << _maxConnectionsAutomaton << " )\n";
            continue;
            
         }
         
         
         
         /**********************************************************************************************/
         /*                Automaton                                                                   */
         /**********************************************************************************************/
         
         
         
         streamlog_out( DEBUG4 ) << "\t\t---Automaton---\n" ;
         
         if( _useCED ) KiTrackMarlin::drawAutomatonSegments( automaton ); // draws the 1-segments (i.e. hits)
         
         
         /*******************************/
         /*      2-hit segments         */
         /*******************************/
         
         streamlog_out( DEBUG4 ) << "\t\t--2-hit-Segments--\n" ;
         
         streamlog_out(DEBUG4) << "Automaton has " << automaton.getTracks( 3 ).size() << " track candidates\n"; //should be commented out, because it takes time
         
         automaton.clearCriteria();
         automaton.addCriteria( _crit3Vec );  // Add the criteria for 3 hits (i.e. 2 2-hit segments )
         
         
         // Let the automaton lengthen its 1-hit-segments to 2-hit-segments
         automaton.lengthenSegments();
        
	 
	 // std::vector<const KiTrack::Segment* > vec_seg_2hits = automaton.getSegments();
	 // for(size_t is=0; is<vec_seg_2hits.size(); is++){
	 //   streamlog_out( DEBUG2 ) << "-- segment " << is << " has nhits " << vec_seg_2hits.at(is)->getHits().size() << std::endl ;  
	 //   KiTrack::Segment* test_segment = const_cast<KiTrack::Segment* >(vec_seg_2hits.at(is));
	 //   streamlog_out( DEBUG2 ) << "-- segment " << is << " has nchildren " << test_segment->getChildren().size() << std::endl ;  
	 // }

         
         // So now we have 2-hit-segments and are ready to perform the Cellular Automaton.
         
         // Perform the automaton
         automaton.doAutomaton();
         
         
         // Clean segments with bad states
         automaton.cleanBadStates();
         
        
         // Reset the states of all segments
         automaton.resetStates();
        
         streamlog_out(DEBUG4) << "Automaton has " << automaton.getTracks( 3 ).size() << " track candidates\n"; //should be commented out, because it takes time
         
         
         // Check if there are not too many connections
         if( automaton.getNumberOfConnections() > unsigned( _maxConnectionsAutomaton ) ){
            
            streamlog_out( DEBUG4 ) << "Redo the Automaton with different parameters, because there are too many connections:\n"
            << "\tconnections( " << automaton.getNumberOfConnections() << " ) > MaxConnectionsAutomaton( " << _maxConnectionsAutomaton << " )\n";
            continue;
            
         }
         
         /*******************************/
         /*      3-hit segments         */
         /*******************************/
         streamlog_out( DEBUG4 ) << "\t\t--3-hit-Segments--\n" ;
         
         
         automaton.clearCriteria();
         automaton.addCriteria( _crit4Vec );      
         
         
         // Lengthen the 2-hit-segments to 3-hits-segments
         automaton.lengthenSegments();
 
	 
	 // std::vector<const KiTrack::Segment* > vec_seg_3hits = automaton.getSegments();
	 // for(size_t is=0; is<vec_seg_3hits.size(); is++){
	 //   streamlog_out( DEBUG2 ) << "-- segment " << is << " has nhits " << vec_seg_3hits.at(is)->getHits().size() << std::endl ;  
	 //   KiTrack::Segment* test_segment_3 = const_cast<KiTrack::Segment* >(vec_seg_3hits.at(is));
	 //   streamlog_out( DEBUG2 ) << "-- segment " << is << " has nchildren " << test_segment_3->getChildren().size() << std::endl ;  
	 //   std::string info_seg = test_segment_3->getInfo();
	 //   streamlog_out( DEBUG2 ) << "-- info segment = " << info_seg.c_str() << std::endl ; 
	 // }
	 // //std::vector < std::vector< IHit* > > test_tracks_segment = getTracksOfSegment();
        
         
         // Perform the Cellular Automaton
         automaton.doAutomaton();
         
         //Clean segments with bad states
         automaton.cleanBadStates();
         
         
         //Reset the states of all segments
         automaton.resetStates();
         


      
         streamlog_out(DEBUG4) << "Automaton has " << automaton.getTracks( 3 ).size() << " track candidates\n"; //should be commented out, because it takes time
         
         
         // Check if there are not too many connections
         if( automaton.getNumberOfConnections() > unsigned( _maxConnectionsAutomaton ) ){
            
            streamlog_out( DEBUG4 ) << "Redo the Automaton with different parameters, because there are too many connections:\n"
            << "\tconnections( " << automaton.getNumberOfConnections() << " ) > MaxConnectionsAutomaton( " << _maxConnectionsAutomaton << " )\n";
            continue;
            
         }
         
         // get the raw tracks (raw track = just a vector of hits, the most rudimentary form of a track)
         rawTracks = automaton.getTracks( 3 );
         
         break; // if we reached this place all went well and we don't need another round --> exit the loop
         
      }
      
      streamlog_out( DEBUG4 ) << "Automaton returned " << rawTracks.size() << " raw tracks \n";
      
      
      /**********************************************************************************************/
      /*                Add the overlapping hits                                                    */
      /**********************************************************************************************/
      
      
      streamlog_out( DEBUG4 ) << "\t\t---Add hits from overlapping petals + fit + helix and Kalman cuts---\n" ;
      
      
      std::vector <ITrack*> trackCandidates;
      
      
      // for all raw tracks we got from the automaton
      for( unsigned i=0; i < rawTracks.size(); i++){
         
         
         RawTrack rawTrack = rawTracks[i];
         
         _nTrackCandidates++;
         
         
	 // for not breaking the code put something dummy - rawtracksplus are excatly the rawtracks no additional tracks are added
         // // get all versions of the track plus hits from overlapping petals
         std::vector < RawTrack > rawTracksPlus = getRawTracksPlusOverlappingHits( rawTrack, map_hitFront_hitsBack );
         
         streamlog_out( DEBUG2 ) << "For raw track number " << i << " there are " << rawTracksPlus.size() << " versions\n";
         
         
         /**********************************************************************************************/
         /*                Make track candidates, fit them and throw away bad ones                     */
         /**********************************************************************************************/
         


         std::vector< ITrack* > overlappingTrackCands;
         

         for( unsigned j=0; j < rawTracksPlus.size(); j++ ){
            
            _nTrackCandidatesPlus++;
            
            RawTrack rawTrackPlus = rawTracksPlus[j];
            
            if( rawTrackPlus.size() < unsigned( _hitsPerTrackMin ) ){
               
               streamlog_out( DEBUG2 ) << "Trackversion discarded, too few hits: only " << rawTrackPlus.size() << " < " << _hitsPerTrackMin << "(hitsPerTrackMin)\n";
               continue;
               
            }
            

            EndcapTrack* trackCand = new EndcapTrack( _trkSystem );
            
            // add the hits to the track
            for( unsigned k=0; k<rawTrackPlus.size(); k++ ){
               
               IEndcapHit* endcapHit = dynamic_cast< IEndcapHit* >( rawTrackPlus[k] ); // cast to IEndcapHits, as needed for an EndcapTrack
               if( endcapHit != NULL ) trackCand->addHit( endcapHit );
               else streamlog_out( DEBUG4 ) << "Hit " << rawTrackPlus[k] << " could not be casted to IEndcapHit\n";
               
            }

            
            std::vector< IHit* > trackCandHits = trackCand->getHits();
            streamlog_out( DEBUG2 ) << "-- Evt " << _nEvt <<" -- Fitting track candidate with " << trackCandHits.size() << " hits\n";
            
            for( unsigned k=0; k < trackCandHits.size(); k++ ) streamlog_out( DEBUG1 ) << trackCandHits[k]->getPositionInfo();
            streamlog_out( DEBUG1 ) << "\n";
            
            /*-----------------------------------------------*/
            /*                Helix Fit                      */
            /*-----------------------------------------------*/
            
            streamlog_out( DEBUG2 ) << "Fitting with Helix Fit\n";
            try{
               
               EndcapHelixFitter helixFitter( trackCand->getLcioTrack() );
               float chi2OverNdf = helixFitter.getChi2() / float( helixFitter.getNdf() );
               streamlog_out( DEBUG2 ) << "chi2OverNdf = " << chi2OverNdf << "\n";
               
               if( chi2OverNdf > _helixFitMax ){
                  
                  streamlog_out( DEBUG2 ) << "Discarding track because of bad helix fit: chi2/ndf = " << chi2OverNdf << "\n";
                  delete trackCand;
                  continue;
                  
               }
               else streamlog_out( DEBUG2 ) << "Keeping track because of good helix fit: chi2/ndf = " << chi2OverNdf << "\n";
               
            }
            catch( EndcapHelixFitterException e ){
               
               
               streamlog_out( DEBUG3 ) << "Track rejected, because fit failed: " <<  e.what() << "\n";
               delete trackCand;
               continue;
               
            }
            
            /*-----------------------------------------------*/
            /*                Kalman Fit                      */
            /*-----------------------------------------------*/
            
            streamlog_out( DEBUG2 ) << "Fitting with Kalman Filter\n";
            try{
                  
               trackCand->fit();
                  
               streamlog_out( DEBUG2 ) << " Track " << trackCand 
                                       << " chi2Prob = " << trackCand->getChi2Prob() 
                                       << "( chi2=" << trackCand->getChi2() 
                                       <<", Ndf=" << trackCand->getNdf() << " )\n";
                  
                  
               if ( trackCand->getChi2Prob() >= _chi2ProbCut ){
                  
                  streamlog_out( DEBUG2 ) << "Track accepted (chi2prob " << trackCand->getChi2Prob() << " >= " << _chi2ProbCut << "\n";
                  
               }
               else{
                  
                  streamlog_out( DEBUG2 ) << "Track rejected (chi2prob " << trackCand->getChi2Prob() << " < " << _chi2ProbCut << "\n";
                  delete trackCand;
                  
                  continue;
                  
               }
               
               
            }
            catch( FitterException e ){
               
               
               streamlog_out( DEBUG3 ) << "Track rejected, because fit failed: " <<  e.what() << "\n";
               delete trackCand;
               continue;
               
            }
            
            // If we reach this point than the track got accepted by all cuts
            overlappingTrackCands.push_back( trackCand );
            
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
                  
		 
		 //if( overlappingTrackCands[j]->getChi2Prob() > bestTrack->getChi2Prob() ){
		 if( overlappingTrackCands[j]->getHits().size() > bestTrack->getHits().size() ){ // ATM NO VERY IMPORTANT WITH CRITERIA BECAUSE I AM NOT CONSIDERING OVERLAPPING HITS FOR DIFFERENT VERSION OF THE SAME TRACK
		 //double diffChi2 = overlappingTrackCands[j]->getChi2Prob() - bestTrack->getChi2Prob();
		 //bool muchBetterChi2 = (diffChi2<-0.1);
		 //bool moreHits = (overlappingTrackCands[j]->getHits().size() > bestTrack->getHits().size());
		 //if (muchBetterChi2 || moreHits){
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
      
      if( _useCED ){
//          for( unsigned i=0; i < trackCandidates.size(); i++ ) KiTrackMarlin::drawTrackRandColor( trackCandidates[i] );
      }
      
      /**********************************************************************************************/
      /*               Get the best subset of tracks                                                */
      /**********************************************************************************************/
      
      streamlog_out(DEBUG3) << "The track candidates so far: \n";
      for( unsigned iTrack=0; iTrack < trackCandidates.size(); iTrack++ ){
         
         streamlog_out(DEBUG3) << "track " << iTrack << ": " << trackCandidates[iTrack] << "\t" << KiTrackMarlin::getTrackHitInfo( trackCandidates[iTrack] ) << "\n";
         
      }
      
      streamlog_out( DEBUG4 ) << "\t\t---Get best subset of tracks---\n" ;
      
      std::vector< ITrack* > tracks;
      std::vector< ITrack* > rejected;
      
      TrackCompatibilityShare1SP comp;
      // TrackQIChi2Prob trackQI;
      // TrackQIChi2ProbSpecial trackQIChi2ProbSpecial;
      TrackNHits trackNHits;
      
      
      
      if( _bestSubsetFinder == "SubsetHopfieldNN" ){
         
         streamlog_out( DEBUG3 ) << "Use SubsetHopfieldNN for getting the best subset\n" ;
         
         SubsetHopfieldNN< ITrack* > subset;
         subset.setOmega( _HNN_Omega );
         subset.setActivationThreshold( _HNN_ActivationThreshold );
         subset.setTInf( _HNN_TInf );
         subset.add( trackCandidates );
         
         
         //subset.calculateBestSet( comp, trackQIChi2ProbSpecial );
         subset.calculateBestSet( comp, trackNHits );
         
         tracks = subset.getAccepted();
         rejected = subset.getRejected();
         
      }
      else if( _bestSubsetFinder == "SubsetSimple" ){
         
         streamlog_out( DEBUG3 ) << "Use SubsetSimple for getting the best subset\n" ;
         
         SubsetSimple< ITrack* > subset;
         subset.add( trackCandidates );
         //subset.calculateBestSet( comp, trackQIChi2ProbSpecial );
         subset.calculateBestSet( comp, trackNHits );
         tracks = subset.getAccepted();
         rejected = subset.getRejected();
         
      }
      else { // in any other case take all tracks
         
         streamlog_out( DEBUG3 ) << "Input for subset = \"" << _bestSubsetFinder << "\". All tracks are kept\n" ;
         
         tracks = trackCandidates;
         
      }
      
      
      if( _useCED ){
//          for( unsigned i=0; i < tracks.size(); i++ ) KiTrackMarlin::drawTrack( tracks[i] , 0x00ff00 );
//          for( unsigned i=0; i < rejected.size(); i++ ) KiTrackMarlin::drawTrack( rejected[i] , 0xff0000 );
      }
      
      
      for ( unsigned i=0; i<rejected.size(); i++){
         
         delete rejected[i];
         
      }
      
      
      
      /**********************************************************************************************/
      /*               Finally: Finalise and save the tracks                                        */
      /**********************************************************************************************/
      
      streamlog_out( DEBUG4 ) << "\t\t---Save Tracks---\n" ;
      
      LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
      
      // Set the flags
      LCFlagImpl hitFlag(0) ;
      hitFlag.setBit( LCIO::TRBIT_HITS ) ;
      trkCol->setFlag( hitFlag.getFlag()  ) ;
      
      
      for (unsigned int i=0; i < tracks.size(); i++){
         
	//FTDTrack* myTrack = dynamic_cast< FTDTrack* >( tracks[i] );
         EndcapTrack* myTrack = dynamic_cast< EndcapTrack* >( tracks[i] );
         
         if( myTrack != NULL ){
            
            
            TrackImpl* trackImpl = new TrackImpl( *(myTrack->getLcioTrack()) );
            
            try{
               
               finaliseTrack( trackImpl );
               trkCol->addElement( trackImpl );
               
            }
            catch( FitterException e ){
               
               streamlog_out( DEBUG4 ) << "DDForwardTracking: track couldn't be finalized due to fitter error: " << e.what() << "\n";
               delete trackImpl;
            }
            
            
         }
         
         
      }
     
      // set the quality of the output collection
      switch (_output_track_col_quality) {
         
         case _output_track_col_quality_FAIR:
            trkCol->parameters().setValue( "QualityCode" , "Fair"  ) ;
            break;
            
         case _output_track_col_quality_POOR:
            trkCol->parameters().setValue( "QualityCode" , "Poor"  ) ;
            break;
            
         default:
            trkCol->parameters().setValue( "QualityCode" , "Good"  ) ;
            break;
      }

      evt->addCollection(trkCol,_ForwardTrackCollection.c_str());
      
      
      
      streamlog_out (DEBUG5) << "Forward Tracking found and saved " << tracks.size() << " tracks in event " << _nEvt << "\n"; 
      for (size_t itrack=0; itrack<tracks.size(); itrack++){
	streamlog_out (DEBUG5) << " track " << itrack << " has nhits " << tracks.at(itrack)->getHits().size() << "\n";
	for (size_t ihit=0; ihit<tracks.at(itrack)->getHits().size(); ihit++){
	  streamlog_out (DEBUG5) << " hit z " << tracks.at(itrack)->getHits().at(ihit)->getZ() << "\n"; 
	}
      }
      streamlog_out (DEBUG5) << "\n"; 
      
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





void DDForwardTracking::check( LCEvent * evt ) {}


void DDForwardTracking::end(){
   
 
   for ( unsigned i=0; i< _crit2Vec.size(); i++) delete _crit2Vec[i];
   for ( unsigned i=0; i< _crit3Vec.size(); i++) delete _crit3Vec[i];
   for ( unsigned i=0; i< _crit4Vec.size(); i++) delete _crit4Vec[i];
   _crit2Vec.clear();
   _crit3Vec.clear();
   _crit4Vec.clear();
   
   delete _sectorSystemEndcap;
   _sectorSystemEndcap = NULL;

   // delete _sectorSystemFTD;
   // _sectorSystemFTD = NULL;
   
   // streamlog_out( DEBUG3 ) << "There are " << _nTrackCandidates << "track candidates from CA and "<<  _nTrackCandidatesPlus
   //    << " track Candidates with hits from overlapping hits\n"
   //    << "The ratio is " << float( _nTrackCandidatesPlus )/_nTrackCandidates;

   
}




std::map< IHit* , std::vector< IHit* > > DDForwardTracking::getOverlapConnectionMap( 
            const std::map< int , std::vector< IHit* > > & map_sector_hits, 
            const SectorSystemEndcap* secSysEndcap,
            float distMax){
   
   
   unsigned nConnections=0;

   
   std::map< IHit* , std::vector< IHit* > > map_hitFront_hitsBack;
   std::map< int , std::vector< IHit* > >::const_iterator it;
   

   //for every sector
   for ( it= map_sector_hits.begin() ; it != map_sector_hits.end(); it++ ){
           
     std::vector< IHit* > hitVecA = it->second;
     int sector = it->first;

     for ( unsigned j=0; j < hitVecA.size(); j++ ){
       for ( unsigned k=j+1; k < hitVecA.size(); k++ ){
	 IHit* hitA = hitVecA[j];
	 IHit* hitB = hitVecA[k];

	 // float dx = hitA->getX() - hitB->getX();
	 // float dy = hitA->getY() - hitB->getY();
	 // float dz = hitA->getZ() - hitB->getZ();
	 // float dist = sqrt( dx*dx + dy*dy + dz*dz );
	 float dist = hitA->distTo(hitB);
	 
	 bool closeHits = dist < distMax;


	 ///////ATT: TURNED OFF to avoid background hits (to be investigated)
	 closeHits=false;

	 // bool closeBySensors = false;
	 
	 // UTIL::BitField64  cellid_decoder( LCTrackerCellID::encoding_string() );
	 // cellid_decoder.setValue( hitA->getCellID0() );

	 // int moduleA = cellid_decoder["module"].value(); //ring
	 // int sensorA = cellid_decoder["sensor"].value();


	 // cellid_decoder.setValue( hitB->getCellID0() );

	 // int moduleB = cellid_decoder["module"].value(); //ring
	 // int sensorB = cellid_decoder["sensor"].value();

	 // if ( moduleA==moduleB && fabs(sensorA-sensorB)==1) closeBySensors=true;
	 // else if (fabs(moduleA-moduleB)==1) closeBySensors=true;

	 // if ( closeHits && closeBySensors ){ 
	 streamlog_out( DEBUG2 ) << "--- dist: " << dist << "\n";
	 streamlog_out( DEBUG2 ) << "--- closeHits: " << closeHits << "\n";
 
	 if ( closeHits ){ 
                                   
	   streamlog_out( DEBUG2 ) << "Connected: (" << hitA->getX() << "," << hitA->getY() << "," << hitA->getZ() << ")-->("
				   << hitB->getX() << "," << hitB->getY() << "," << hitB->getZ() << ")\n";
                  
	   map_hitFront_hitsBack[ hitA ].push_back( hitB );
	   nConnections++;
	 }
	 
       }
     }

   }


   // //for every sector
   // for ( it= map_sector_hits.begin() ; it != map_sector_hits.end(); it++ ){
      
     
   //    std::vector< IHit* > hitVecA = it->second;
   //    int sector = it->first;
      
   //    // get the neighbouring petals
   //    FTDNeighborPetalSecCon secCon( secSysFTD );
   //    std::set< int > targetSectors = secCon.getTargetSectors( sector );
      
      
   //    //for all neighbouring petals
   //    for ( std::set<int>::iterator itTarg = targetSectors.begin(); itTarg!=targetSectors.end(); itTarg++ ){
         
         
   //      //fg: this blows up the map with empty vectors ! 
   // 	//	std::vector< IHit* > hitVecB = map_sector_hits[ *itTarg ];
         
   // 	 std::map< int , std::vector< IHit* > >::const_iterator itB = map_sector_hits.find( *itTarg ) ; 

   // 	 if( itB == map_sector_hits.end() ){
   // 	   continue ;
   // 	 }
   // 	 std::vector< IHit* > hitVecB = itB->second ;
	 

   //       for ( unsigned j=0; j < hitVecA.size(); j++ ){
            
            
   //          IHit* hitA = hitVecA[j];
            
   //          for ( unsigned k=0; k < hitVecB.size(); k++ ){
               
               
   //             IHit* hitB = hitVecB[k];
               
               
   //             float dx = hitA->getX() - hitB->getX();
   //             float dy = hitA->getY() - hitB->getY();
   //             float dz = hitA->getZ() - hitB->getZ();
   //             float dist = sqrt( dx*dx + dy*dy + dz*dz );
               
   //             if (( dist < distMax )&& ( fabs( hitB->getZ() ) > fabs( hitA->getZ() ) )  ){ // if they are close enough and B is behind A
                  
                  
   //                streamlog_out( DEBUG2 ) << "Connected: (" << hitA->getX() << "," << hitA->getY() << "," << hitA->getZ() << ")-->("
   //                                        << hitB->getX() << "," << hitB->getY() << "," << hitB->getZ() << ")\n";
                  
   //                map_hitFront_hitsBack[ hitA ].push_back( hitB );
   //                nConnections++;
                  
   //             }
               
   //          }
            
   //       } 
         
   //    }
     
   // }
   
   streamlog_out( DEBUG3 ) << "Connected " << map_hitFront_hitsBack.size() << " hits with " << nConnections << " possible overlapping hits\n";
   
   
   return map_hitFront_hitsBack;
   
   
   
}


std::string DDForwardTracking::getInfo_map_sector_hits(){
   
   
   std::stringstream s;
   
   std::map< int , std::vector< IHit* > >::iterator it;
   
   for( it = _map_sector_hits.begin(); it != _map_sector_hits.end(); it++ ){
      
      
      std::vector<IHit*> hits = it->second;
      int sector = it->first;
      
      //int side = _sectorSystemEndcap->getSide( sector );
      unsigned layer = _sectorSystemEndcap->getLayer( sector );
      unsigned theta = _sectorSystemEndcap->getTheta( sector );
      unsigned phi = _sectorSystemEndcap->getPhi( sector );
      
      s << " getInfo - sector " << sector  << " (layer "
      << layer << ", theta "
      << theta << ", phi "
      << phi << ") has "
      << hits.size() << " hits\n";  
      
   }  
   
   
   return s.str();   
   
}


std::vector < RawTrack > DDForwardTracking::getRawTracksPlusOverlappingHits( RawTrack rawTrack , std::map< IHit* , std::vector< IHit* > >& map_hitFront_hitsBack ){
   
   
   
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
   
   // // for every hit in the original track
   // for( unsigned i=0; i < rawTrack.size(); i++ ){
      
     
   //    IHit* frontHit = rawTrack[i];
      
   //    // get the hits that are behind frontHit
   //    std::map< IHit* , std::vector< IHit* > >::iterator it;
   //    it = map_hitFront_hitsBack.find( frontHit );
   //    if( it == map_hitFront_hitsBack.end() ) continue; // if there are no hits on the back skip this one
   //    std::vector< IHit* > backHits = it->second; 
      
      
   //    // Create the different versions of the tracks so far with the hits from the back
      
   //    std::vector< RawTrack > newVersions; //here we store all the versions with overlapping hits from the different back hits at this frontHit
      
   //    // for every hit in back of the frontHit
   //    for( unsigned j=0; j<backHits.size(); j++ ){
         
         
   //       IHit* backHit = backHits[j];
         
   //       // for all tracks we have so far
   //       for( unsigned k=0; k<rawTracksPlus.size(); k++ ){
            
            
   //          RawTrack newVersion = rawTracksPlus[k];     // exact copy of the track
   //          newVersion.push_back( backHit );          // add the backHit to it   
   //          newVersions.push_back( newVersion );         // store it
            
   //       }
         
   //    }
      
   //    // Now put all the new versions of the tracks into the rawTracksPlus vector before we go on to the next
   //    // hit of the original track
   //    rawTracksPlus.insert( rawTracksPlus.end(), newVersions.begin(), newVersions.end() );
      
      
   // }
   
   
   
   return rawTracksPlus;
   
}


bool DDForwardTracking::setCriteria( unsigned round ){
 
   // delete the old ones
   for ( unsigned i=0; i< _crit2Vec.size(); i++) delete _crit2Vec[i];
   for ( unsigned i=0; i< _crit3Vec.size(); i++) delete _crit3Vec[i];
   for ( unsigned i=0; i< _crit4Vec.size(); i++) delete _crit4Vec[i];
   _crit2Vec.clear();
   _crit3Vec.clear();
   _crit4Vec.clear();
   
   
   
   bool newValuesGotUsed = false; // if new values are used
   
   for( unsigned i=0; i<_criteriaNames.size(); i++ ){
      
      std::string critName = _criteriaNames[i];
      
      
      float min = _critMinima[critName].back();
      float max = _critMaxima[critName].back();
      
      
      
      // use the value corresponding to the round, if there are no new ones for this criterion, just do nothing (the previous value stays in place)
      if( round + 1 <= _critMinima[critName].size() ){
         
         min =  _critMinima[critName][round];
         newValuesGotUsed = true;
         
      }
      
      if( round + 1 <= _critMaxima[critName].size() ){
         
         max =  _critMaxima[critName][round];
         newValuesGotUsed = true;
         
      }
      
      ICriterion* crit = Criteria::createCriterion( critName, min , max );
      
      // Some debug output about the created criterion
      std::string type = crit->getType();
      
      streamlog_out( DEBUG3 ) <<  "Added: Criterion " << critName << " (type =  " << type 
      << " ). Min = " << min
      << ", Max = " << max
      << ", round " << round << "\n";
      
      
      // Add the new criterion to the corresponding vector
      if( type == "2Hit" ){
         
         _crit2Vec.push_back( crit );
         
      }
      else if( type == "3Hit" ){
         
         _crit3Vec.push_back( crit );
         
      }
      else if( type == "4Hit" ){
         
         _crit4Vec.push_back( crit );
         
      }
      else delete crit;
      
      
   }
   
   return newValuesGotUsed;
   
   
}


void DDForwardTracking::finaliseTrack( TrackImpl* trackImpl ){
   
   
   Fitter fitter( trackImpl , _trkSystem );
   
   trackImpl->trackStates().clear();
   

   TrackStateImpl* trkStateIP = new TrackStateImpl( *fitter.getTrackState( lcio::TrackState::AtIP ) ) ;
   trkStateIP->setLocation( TrackState::AtIP );
   trackImpl->addTrackState( trkStateIP );
   
   TrackStateImpl* trkStateFirstHit = new TrackStateImpl( *fitter.getTrackState( TrackState::AtFirstHit ) ) ;
   trkStateFirstHit->setLocation( TrackState::AtFirstHit );
   trackImpl->addTrackState( trkStateFirstHit );
   
   TrackStateImpl* trkStateLastHit = new TrackStateImpl( *fitter.getTrackState( TrackState::AtLastHit ) ) ;
   trkStateLastHit->setLocation( TrackState::AtLastHit );
   trackImpl->addTrackState( trkStateLastHit );
   
   if( _getTrackStateAtCaloFace ) {
     TrackStateImpl* trkStateAtCalo = new TrackStateImpl( *fitter.getTrackState( TrackState::AtCalorimeter ) ) ;
     trkStateAtCalo->setLocation( TrackState::AtCalorimeter );
     trackImpl->addTrackState( trkStateAtCalo );
   }

   trackImpl->setChi2( fitter.getChi2( TrackState::AtIP ) );
   trackImpl->setNdf(  fitter.getNdf ( TrackState::AtIP ) );
   
   const float* p = trkStateFirstHit->getReferencePoint();
   trackImpl->setRadiusOfInnermostHit( sqrt( p[0]*p[0] + p[1]*p[1] + p[2]*p[2] ) );
   
   std::map<int, int> hitNumbers; 
   
   hitNumbers[lcio::ILDDetID::VXD] = 0;
   hitNumbers[lcio::ILDDetID::SIT] = 0;
   hitNumbers[lcio::ILDDetID::FTD] = 0;
   hitNumbers[lcio::ILDDetID::TPC] = 0;
   hitNumbers[lcio::ILDDetID::SET] = 0;
   hitNumbers[lcio::ILDDetID::ETD] = 0;
   
   std::vector< TrackerHit* > trackerHits = trackImpl->getTrackerHits();
   for( unsigned j=0; j < trackerHits.size(); j++ ){
      
      UTIL::BitField64 encoder( LCTrackerCellID::encoding_string() );
      encoder.setValue( trackerHits[j]->getCellID0() );
      int subdet =  encoder[lcio::LCTrackerCellID::subdet()];
     
      
      ++hitNumbers[ subdet ];
      
   }
   
   trackImpl->subdetectorHitNumbers().resize(2 * lcio::ILDDetID::ETD);
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::VXD - 2 ] = hitNumbers[lcio::ILDDetID::VXD];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::FTD - 2 ] = hitNumbers[lcio::ILDDetID::FTD];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::SIT - 2 ] = hitNumbers[lcio::ILDDetID::SIT];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::TPC - 2 ] = hitNumbers[lcio::ILDDetID::TPC];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::SET - 2 ] = hitNumbers[lcio::ILDDetID::SET];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::ETD - 2 ] = hitNumbers[lcio::ILDDetID::ETD];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::VXD - 1 ] = hitNumbers[lcio::ILDDetID::VXD];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::FTD - 1 ] = hitNumbers[lcio::ILDDetID::FTD];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::SIT - 1 ] = hitNumbers[lcio::ILDDetID::SIT];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::TPC - 1 ] = hitNumbers[lcio::ILDDetID::TPC];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::SET - 1 ] = hitNumbers[lcio::ILDDetID::SET];
   trackImpl->subdetectorHitNumbers()[ 2 * lcio::ILDDetID::ETD - 1 ] = hitNumbers[lcio::ILDDetID::ETD];
   
   
   return;
   
   
}



void DDForwardTracking::getCellID0AndPositionInfo(LCCollection*& col ){


  std::string cellIDEcoding = col->getParameters().getStringVal("CellIDEncoding") ;  
  UTIL::BitField64 cellid_decoder( cellIDEcoding ) ;

  // std::string TRICK = "system:8,barrel:3,layer:4,module:14,sensor:2,side:32:-2,strip:20";
  // UTIL::BitField64 cellid_decoder( TRICK ) ;
  

  for (int i=0; i<col->getNumberOfElements(); i++){    
    TrackerHitPlane* trackerHit = dynamic_cast<TrackerHitPlane*>( col->getElementAt(i) ) ;

    DD4hep::long64 id = trackerHit->getCellID0() ;
    cellid_decoder.setValue( id ) ;

    int layer = cellid_decoder["layer"].value();
    //int subdet = cellid_decoder["system"].value();
    int subdet = cellid_decoder["subdet"].value();
    int side = cellid_decoder["side"].value();
    int module = cellid_decoder["module"].value();
    int sensor = cellid_decoder["sensor"].value();

    //if (subdet==2) layer = layer+0;
    if (subdet==4) layer = layer+6;
    else if (subdet==6) layer = layer+6+2;
    else if (subdet==3) layer = layer+6;
    else if (subdet==5) layer = layer+6+2;

    streamlog_out(DEBUG2) << " hit" << i
    			   << " ( subdetector: " << subdet
    			   <<", side: " << side
    			   <<", layer: " << layer
    			   <<", module: " << module
    			   <<", sensor: " << sensor
    			   <<" ) ( r: " << sqrt(pow(trackerHit->getPosition()[0],2)+pow(trackerHit->getPosition()[1],2))
    			   <<" , phi: " << atan(trackerHit->getPosition()[1]/trackerHit->getPosition()[0])
    			   <<" , z: " << trackerHit->getPosition()[2] 
    			   <<" ) \n";
  }

 
  return;
}







EndcapHitSimple* DDForwardTracking::createVirtualIPHit( const SectorSystemEndcap* sectorSystemEndcap ){
   
   int layer = 0 ;
   int phi = 0 ;
   int theta = 0 ;

   EndcapHitSimple* virtualIPHit = new EndcapHitSimple( 0.,0.,0., layer, phi, theta, sectorSystemEndcap );

   virtualIPHit->setIsVirtual ( true );
   
   return virtualIPHit;
   
}
