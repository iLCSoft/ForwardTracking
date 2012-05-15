#include "TrueTrackCritAnalyser.h"

#include <algorithm>
#include <cmath>

#include "EVENT/LCCollection.h"
#include "EVENT/MCParticle.h"
#include "EVENT/Track.h"
#include "marlin/VerbosityLevels.h"
#include "marlin/Global.h"
#include "gear/FTDParameters.h"
#include "gear/FTDLayerLayout.h"

#include "Math/ProbFunc.h"

#include "Tools/KiTrackMarlinTools.h"
#include "Tools/FTDHelixFitter.h"
#include "Criteria/Criteria.h"
#include "ILDImpl/FTDHit01.h"


using namespace lcio;
using namespace marlin;
using namespace KiTrack;





TrueTrackCritAnalyser aTrueTrackCritAnalyser ;


TrueTrackCritAnalyser::TrueTrackCritAnalyser() : Processor("TrueTrackCritAnalyser") {
   
   // modify processor description
   _description = "TrueTrackCritAnalyser: Analysis of criteria for the Cellular Automaton" ;
   
   
   // register steering parameters: name, description, class-variable, default value
   
   registerInputCollection(LCIO::LCRELATION,
                           "MCTrueTrackRelCollectionName",
                           "Name of the TrueTrack MC Relation collection",
                           _colNameMCTrueTracksRel,
                           std::string("TruthTracksMCP"));
   
   
   
   registerProcessorParameter("RootFileName",
                              "Name of the root file for saving the results",
                              _rootFileName,
                              std::string("TrueTracksCritAnalysis.root") );
   
   registerProcessorParameter("WriteNewRootFile",
                              "What to do with older root file: true = rename it, false = leave it and append new one",
                              _writeNewRootFile,
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
   
   
   // The cuts
   registerProcessorParameter("CutChi2Prob",
                              "Tracks with a chi2 probability below this value won't be considered",
                              _chi2ProbCut,
                              double (0.005) ); 
      
   registerProcessorParameter("CutPtMin",
                              "The minimum transversal momentum pt above which tracks are of interest in GeV ",
                              _ptMin,
                              double (0.1)  );   
   
   registerProcessorParameter("CutDistToIPMax",
                              "The maximum distance from the origin of the MCP to the IP (0,0,0)",
                              _distToIPMax,
                              double (100. ) );   
   
   registerProcessorParameter("CutNumberOfHitsMin",
                              "The minimum number of hits a track must have",
                              _nHitsMin,
                              int (4)  );   
   
   
   
   registerProcessorParameter("OverlappingHitsDistMax",
                              "The maximum distance of hits from overlapping petals belonging to one track",
                              _overlappingHitsDistMax,
                              double(4));
   

   
}



void TrueTrackCritAnalyser::init() { 
   
   streamlog_out(DEBUG) << "   init called  " << std::endl ;
   
   // usually a good idea to
   printParameters() ;
   
   
   const gear::FTDParameters& ftdParams = Global::GEAR->getFTDParameters() ;
   const gear::FTDLayerLayout& ftdLayers = ftdParams.getFTDLayerLayout() ;
   int nLayers = ftdLayers.getNLayers() + 1;
   int nModules = ftdLayers.getNPetals(0);
   int nSensors = ftdLayers.getNSensors(0);
   
   for( int i=1; i<nLayers; i++){
      
      if( ftdLayers.getNPetals(i) > nModules ) nModules = ftdLayers.getNPetals(i); 
      if( ftdLayers.getNSensors(i) > nSensors ) nSensors = ftdLayers.getNSensors(i);
      
   }
   
   _sectorSystemFTD = new SectorSystemFTD( nLayers, nModules , nSensors );
   
   
   _nRun = 0 ;
   _nEvt = 0 ;
   
   std::set< std::string > critNames = Criteria::getAllCriteriaNames();
   std::set< std::string >::iterator it;
   
   for( it = critNames.begin(); it!= critNames.end(); it++ ){
      
      ICriterion* crit = Criteria::createCriterion( (*it) );
      
      if ( crit->getType() == "2Hit" ) _crits2.push_back( crit );
      else if ( crit->getType() == "3Hit" ) _crits3.push_back( crit );
      else if ( crit->getType() == "4Hit" ) _crits4.push_back( crit );
      else delete crit;
      
   }

   
   
   std::set < std::string > branchNames2; //branch names of the 2-hit criteria
   std::set < std::string > branchNames3;
   std::set < std::string > branchNames4;
   std::set < std::string > branchNamesKalman;
   std::set < std::string > branchNamesHitDist;
   
   
   // Set up the root file
   // Therefore first get all the possible names of the branches
   
   // create a virtual hit
   IHit* virtualIPHit = KiTrackMarlin::createVirtualIPHit(1 , _sectorSystemFTD );

   
   std::vector <IHit*> hitVec;
   hitVec.push_back( virtualIPHit );
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 1-segments (2 hit criteria)                         */
   /**********************************************************************************************/
   
   Segment virtual1Segment( hitVec );
   
   
   for ( unsigned int i=0; i < _crits2 .size() ; i++ ){ //for all criteria

      _crits2[i]->setSaveValues( true ); // so the calculated values won't just fade away, but are saved in a map
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
   branchNames2.insert( "layers" ); // a code for the layers the used hits had: 743 = layer 7, 4 and 3
   branchNames2.insert( "distance" ); // the distance between two hits
   branchNames2.insert( "chi2Prob" ); //the chi2 probability
   // Set up the root file with the tree and the branches
   _treeName2 = "2Hit";
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeName2, branchNames2, _writeNewRootFile );      //prepare the root file.
   
   
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 2-segments (3 hit criteria)                         */
   /**********************************************************************************************/
   
   hitVec.push_back( virtualIPHit );
   Segment virtual2Segment( hitVec );
   
   
   for ( unsigned int i=0; i < _crits3 .size() ; i++ ){ //for all criteria


      _crits3[i]->setSaveValues( true ); // so the calculated values won't just fade away, but are saved in a map

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
   branchNames3.insert( "chi2Prob" ); //the chi2 probability
   branchNames3.insert( "layers" ); // a code for the layers the used hits had: 743 = layer 7, 4 and 3
   
   // Set up the root file with the tree and the branches
   _treeName3 = "3Hit"; 
   
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeName3, branchNames3 , false );      //prepare the root file.
  
   
   
   /**********************************************************************************************/
   /*                Set up the tree for the 3-segments (4 hit criteria)                         */
   /**********************************************************************************************/
   
   hitVec.push_back( virtualIPHit );
   Segment virtual3Segment( hitVec );
   
   
   for ( unsigned int i=0; i < _crits4 .size() ; i++ ){ //for all criteria

      _crits4[i]->setSaveValues( true ); // so the calculated values won't just fade away, but are saved in a map

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
   branchNames4.insert( "chi2Prob" ); //the chi2 probability
   branchNames4.insert( "layers" ); // a code for the layers the used hits had: 743 = layer 7, 4 and 3
   
   // Set up the root file with the tree and the branches
   _treeName4 = "4Hit"; 
   
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeName4, branchNames4 , false );      //prepare the root file.
   
 
   delete virtualIPHit;
   
   
   
   /**********************************************************************************************/
   /*                Set up the tree for Kalman Fits                                             */
   /**********************************************************************************************/
   
   branchNamesKalman.insert( "helixChi2" );
   branchNamesKalman.insert( "helixNdf" );
   branchNamesKalman.insert( "helixChi2OverNdf" );
   branchNamesKalman.insert( "chi2Prob" );
   branchNamesKalman.insert( "chi2" );
   branchNamesKalman.insert( "Ndf" );
   branchNamesKalman.insert( "nHits" );
   branchNamesKalman.insert( "MCP_pt" ); //transversal momentum
   branchNamesKalman.insert( "MCP_distToIP" ); //the distance of the origin of the partivle to the IP
   
   // Set up the root file with the tree and the branches
   _treeNameKalman = "KalmanFit"; 
   
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeNameKalman, branchNamesKalman , false );      //prepare the root file.
   
 
   /**********************************************************************************************/
   /*                Set up the tree for the distance of hits                                    */
   /**********************************************************************************************/
   
   branchNamesHitDist.insert( "distToPrevHit" );
   branchNamesHitDist.insert( "MCP_pt" );
   
   _treeNameHitDist = "HitDist"; 
   KiTrackMarlin::setUpRootFile( _rootFileName, _treeNameHitDist, branchNamesHitDist , false );      //prepare the root file.
   
 
   
}


void TrueTrackCritAnalyser::processRunHeader( LCRunHeader* run) { 
   
   _nRun++ ;
} 



void TrueTrackCritAnalyser::processEvent( LCEvent * evt ) { 
   
   
   
   std::vector < std::map < std::string , float > > rootDataVec2;
   std::vector < std::map < std::string , float > > rootDataVec3;
   std::vector < std::map < std::string , float > > rootDataVec4;
   std::vector < std::map < std::string , float > > rootDataVecKalman;
   std::vector < std::map < std::string , float > > rootDataVecHitDist;
  
   // get the true tracks 
   LCCollection* col = evt->getCollection( _colNameMCTrueTracksRel ) ;
   
   
   
   if( col != NULL ){
      
      int nMCTracks = col->getNumberOfElements();

      
      unsigned nUsedRelations = 0;
      
      streamlog_out(DEBUG3) << "There are " << nMCTracks << " MCPTrackRelations in the collection " << _colNameMCTrueTracksRel << "\n";
      
      for( int i=0; i < nMCTracks; i++){ // for every true track
         
         
         LCRelation* rel = dynamic_cast <LCRelation*> (col->getElementAt(i) );
         MCParticle* mcp = dynamic_cast <MCParticle*> (rel->getTo() );
         Track*    track = dynamic_cast <Track*>      (rel->getFrom() );
         
         
         
         /**********************************************************************************************/
         /*               First: check if the track is of interest                                     */
         /**********************************************************************************************/
         // (we don't necessarily want or are able to reconstruct all tracks. Tracks with a really bad
         // multiple scattering and therefore bad chi2Prob are unlikely to be reconstructed.
         // Same goes for tracks with very low pt )
         
         
         
         //////////////////////////////////////////////////////////////////////////////////
         //If distance from origin is not too high   
         const double * vtx = mcp->getVertex();
         double distToIP = sqrt(vtx[0]*vtx[0] + vtx[1]*vtx[1] + vtx[2]*vtx[2] );
         
         // exclude vertices too far away from the origin. 
          
         if ( distToIP > _distToIPMax ){
            
            streamlog_out( DEBUG3 ) << "True track " << i << " is discarded because the distance of the vertex from the origin is too high: "
            << distToIP << " > _distToIPMax( " << _distToIPMax << ")\n";
            continue;   
            
         }
         
         //////////////////////////////////////////////////////////////////////////////////
         //If pt is not too low
         
         const double* p = mcp->getMomentum();
         
         double pt=  sqrt( p[0]*p[0]+p[1]*p[1] );
         
         
         if ( pt < _ptMin ){
            
            streamlog_out( DEBUG3 ) << "True track " << i << " is discarded because the pt is too low: "
            << pt << " < _ptMin( " << _ptMin << ")\n";
            continue;   
            
         }
         //
         //////////////////////////////////////////////////////////////////////////////////
         
         //////////////////////////////////////////////////////////////////////////////////
         //If there are enough hits in the track
         
         int nHits = track->getTrackerHits().size();
         if ( nHits < _nHitsMin ){
            
            streamlog_out( DEBUG3 ) << "True track " << i << " is discarded because there are too few hits in the track: "
            << nHits << " < _nHitsMin( " << _nHitsMin << ")\n";
            continue;   
            
         }
         //
         //////////////////////////////////////////////////////////////////////////////////
         
         
         //////////////////////////////////////////////////////////////////////////////////
         //If the chi2 probability is too low
         
         //Fit the track
         double chi2 = track->getChi2();
         double Ndf = track->getNdf();
         double chi2Prob = ROOT::Math::chisquared_cdf_c( chi2 , Ndf );
         
         
         if ( chi2Prob < _chi2ProbCut ){
            
            streamlog_out( DEBUG3 ) << "True track " << i << " is discarded because chi2 probability is too low: "
            << chi2Prob << " < _chi2ProbCut( " << _chi2ProbCut << ")\n";
            continue;   
            
         }
         //
         //////////////////////////////////////////////////////////////////////////////////
         
         
         /**********************************************************************************************/
         /*      If we reached this point the track is of interest  -> create FTDHits                  */
         /**********************************************************************************************/
         
         nUsedRelations++;
         
         std::vector <TrackerHit*> trackerHits = track->getTrackerHits();
         // sort the hits in the track
         sort( trackerHits.begin(), trackerHits.end(), KiTrackMarlin::compare_TrackerHit_z );
         // now at [0] is the hit with the smallest |z| and at [1] is the one with a bigger |z| and so on
        
         // make FTDHits from them (because Criteria need IHit pointers and FTDHits are derrived from IHit )
         std::vector <IHit*> hits;
         for ( unsigned j=0; j< trackerHits.size(); j++ ) hits.push_back( new FTDHit01( trackerHits[j] , _sectorSystemFTD ) );
         
         
         /**********************************************************************************************/
         /*     Store the distances of the hits                                                        */
         /**********************************************************************************************/
         
         for( unsigned j=0; j< hits.size()-1; j++ ){
            
            std::map < std::string , float > rootData;
            
            rootData[ "distToPrevHit" ] = hits[j]->distTo(hits[j+1]);
            rootData["MCP_pt"] = pt;
            
            rootDataVecHitDist.push_back( rootData );
            
         }         
         
         /**********************************************************************************************/
         /*                Manipulate the hits (for example erase some or add some)                    */
         /**********************************************************************************************/
         
         ///////////////////////////////////////////////////////////////////////////////////////////////
         // Add the IP as a hit
         IHit* virtualIPHit = KiTrackMarlin::createVirtualIPHit(1 , _sectorSystemFTD );
         
         hits.insert( hits.begin() , virtualIPHit );
         ///////////////////////////////////////////////////////////////////////////////////////////////
         
         
         ///////////////////////////////////////////////////////////////////////////////////////////////
         //Erase hits that are too close. For those will be from overlapping petals
         for ( unsigned j=1; j < hits.size() ; j++ ){
            
            IHit* hitA = hits[j-1];
            IHit* hitB = hits[j];
            
            float dist = hitA->distTo( hitB );
            
            if( dist < _overlappingHitsDistMax ){
               
               hits.erase( hits.begin() + j );
               j--;
               
            }               
            
         }
         ///////////////////////////////////////////////////////////////////////////////////////////////
         
         /**********************************************************************************************/
         /*                Build the segments                                                          */
         /**********************************************************************************************/
         
         // Now we have a vector of hits starting with the IP followed by all (or most) hits from the track.
         // So we now are able to build segments from them
         
         std::vector <Segment*> segments1; // 1-hit segments
         
         for ( unsigned j=0; j < hits.size(); j++ ){
            
            
            std::vector <IHit*> segHits;
            segHits.insert( segHits.begin() , hits.begin()+j , hits.begin()+j+1 );
            
            segments1.push_back( new Segment( segHits ) );
            
         }
         
         std::vector <Segment*> segments2; // 2-hit segments
         
         for ( unsigned j=0; j < hits.size()-1; j++ ){
            
            
            std::vector <IHit*> segHits;
            
            segHits.push_back( hits[j+1] );
            segHits.push_back( hits[j] );
            
            segments2.push_back( new Segment( segHits ) );
            
         }
         
         std::vector <Segment*> segments3; // 3-hit segments
         
         for ( unsigned j=0; j < hits.size()-2; j++ ){
            
            
            std::vector <IHit*> segHits;
            
            segHits.push_back( hits[j+2] );
            segHits.push_back( hits[j+1] );
            segHits.push_back( hits[j] );
            
            segments3.push_back( new Segment( segHits ) );
            
         }
         
         // Now we have the segments of the track (ordered) in the vector
         
         /**********************************************************************************************/
         /*                Use the criteria on the segments                                            */
         /**********************************************************************************************/
         
         
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
            rootData["chi2Prob"] = chi2Prob;
            rootData["layers"] = child->getHits()[0]->getLayer() *10 + parent->getHits()[0]->getLayer();
            
            IHit* childHit = child->getHits()[0];
            IHit* parentHit = parent->getHits()[0];
            float dx = childHit->getX() - parentHit->getX();
            float dy = childHit->getY() - parentHit->getY();
            float dz = childHit->getZ() - parentHit->getZ();
            rootData["distance"] = sqrt( dx*dx + dy*dy + dz*dz );
            
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
            rootData["chi2Prob"] = chi2Prob;
            rootData["layers"] = child->getHits()[1]->getLayer() *100 +
                                 child->getHits()[0]->getLayer() *10 + 
                                 parent->getHits()[0]->getLayer();
            
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
            rootData["chi2Prob"] = chi2Prob;
            rootData["layers"] = child->getHits()[2]->getLayer() *1000 +
                                 child->getHits()[1]->getLayer() *100 +
                                 child->getHits()[0]->getLayer() *10 + 
                                 parent->getHits()[0]->getLayer();
            
            rootDataVec4.push_back( rootData );
            
         }
         
         
         
         /**********************************************************************************************/
         /*                Save the fit of the track                                                   */
         /**********************************************************************************************/
         
         
         std::map < std::string , float > rootDataFit;
         
         
         rootDataFit[ "chi2" ]          = chi2;
         rootDataFit[ "Ndf" ]           = Ndf;
         rootDataFit[ "nHits" ]         = nHits;
         rootDataFit[ "chi2Prob" ]      = chi2Prob;
         
         rootDataFit["MCP_pt"] = pt;
         rootDataFit["MCP_distToIP"] = distToIP;
         
         FTDHelixFitter helixFitter( track );
         float helixChi2 = helixFitter.getChi2();
         float helixNdf  = helixFitter.getNdf();
         
         rootDataFit["helixChi2"] = helixChi2;
         rootDataFit["helixNdf"] = helixNdf;
         rootDataFit["helixChi2OverNdf"] = helixChi2 / helixNdf;
         
         
         rootDataVecKalman.push_back( rootDataFit );
         
         
         
         
         /**********************************************************************************************/
         /*                Clean up                                                                    */
         /**********************************************************************************************/
         
         for (unsigned i=0; i<segments1.size(); i++) delete segments1[i];
         segments1.clear();
         for (unsigned i=0; i<segments2.size(); i++) delete segments2[i];
         segments2.clear();
         for (unsigned i=0; i<segments3.size(); i++) delete segments3[i];
         segments3.clear();
         for (unsigned i=0; i<hits.size(); i++) delete hits[i];
         hits.clear();
         
         
       
      }
      
      
      
      /**********************************************************************************************/
      /*                Save all the data to ROOT                                                   */
      /**********************************************************************************************/
      
      
      KiTrackMarlin::saveToRoot( _rootFileName, _treeName2, rootDataVec2 );
      KiTrackMarlin::saveToRoot( _rootFileName, _treeName3, rootDataVec3 );
      KiTrackMarlin::saveToRoot( _rootFileName, _treeName4, rootDataVec4 );
      KiTrackMarlin::saveToRoot( _rootFileName, _treeNameKalman, rootDataVecKalman );
      KiTrackMarlin::saveToRoot( _rootFileName, _treeNameHitDist, rootDataVecHitDist );
      
      
      streamlog_out (DEBUG5) << "Number of used mcp-track relations: " << nUsedRelations <<"\n";
    
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
   
   delete _sectorSystemFTD;
   _sectorSystemFTD = NULL;
   
   
}






