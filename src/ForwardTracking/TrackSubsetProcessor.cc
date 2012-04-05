#include "TrackSubsetProcessor.h"


#include <EVENT/LCCollection.h>


#include "marlin/VerbosityLevels.h"
#include "marlin/Global.h"

#include "KiTrack/SubsetSimple.h"
#include "KiTrack/SubsetHopfieldNN.h"

using namespace lcio ;
using namespace marlin ;
using namespace KiTrack;









TrackSubsetProcessor aTrackSubsetProcessor ;


TrackSubsetProcessor::TrackSubsetProcessor() : Processor("TrackSubsetProcessor") {

  // modify processor description
  _description = "TrackSubsetProcessor takes tracks from multiple sources and outputs them (or modified versions, or a subset of them) as one track collection." ;


  // register steering parameters: name, description, class-variable, default value
  
  
  std::vector< std::string > trackInputColNamesDefault;
  trackInputColNamesDefault.push_back( "ForwardTracks" );
  trackInputColNamesDefault.push_back( "SiTracks" );
  
  registerProcessorParameter( "TrackInputCollections" , 
                              "A vector of the input track collections"  ,
                              _trackInputColNames ,
                              trackInputColNamesDefault );
  
  
  registerOutputCollection(LCIO::TRACK,
                           "TrackOutputCollection",
                           "Name of the output track collection",
                           _trackOutputColName,
                           std::string("SubsetTracks"));
  
  
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
  
  
}



void TrackSubsetProcessor::init() { 

  streamlog_out(DEBUG) << "   init called  " << std::endl ;

  // usually a good idea to
  printParameters() ;

  _nRun = 0 ;
  _nEvt = 0 ;
  
  
  /**********************************************************************************************/
  /*       Initialise the MarlinTrkSystem, needed by the tracks for fitting                     */
  /**********************************************************************************************/
  
  // set upt the geometry
  _trkSystem =  MarlinTrk::Factory::createMarlinTrkSystem( "KalTest" , marlin::Global::GEAR , "" ) ;
  
  if( _trkSystem == 0 ) throw EVENT::Exception( std::string("  Cannot initialize MarlinTrkSystem of Type: ") + std::string("KalTest" )  ) ;
  
  
  // set the options   
  _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useQMS,        _MSOn ) ;       //multiple scattering
  _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::usedEdx,       _ElossOn) ;     //energy loss
  _trkSystem->setOption( MarlinTrk::IMarlinTrkSystem::CFG::useSmoothing,  _SmoothOn) ;    //smoothing
  
  // initialise the tracking system
  _trkSystem->init() ;
    

}


void TrackSubsetProcessor::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void TrackSubsetProcessor::processEvent( LCEvent * evt ) { 

   
   
  std::vector< Track* > tracks;
  
  /**********************************************************************************************/
  /*       Read in the collections                                                              */
  /**********************************************************************************************/
  
  streamlog_out( DEBUG4 ) << "Try to load " << _trackInputColNames.size() << " input track collections\n";
  
  unsigned nTrackLoaded = 0;
  
  for( unsigned i=0; i < _trackInputColNames.size(); i++ ){
    
    
    LCCollection* col = evt->getCollection( _trackInputColNames[i] );
    
    
    if( col != NULL ){
      
      int nTracks = col->getNumberOfElements();
      streamlog_out( DEBUG4 ) << "Load track input collection " << _trackInputColNames[i] << " with " << nTracks << " tracks\n";
      
      
      for(int j=0; j< nTracks ; j++){
        
        Track* track = dynamic_cast<Track*>( col->getElementAt( j ) ) ;
        
        if( track == NULL ){
          
          streamlog_out( ERROR ) << "Entry " << j << " from collection " << _trackInputColNames[i] << " couldn't get casted to a Track*\n";
          
        }
        else{
          
          tracks.push_back( track );
          nTrackLoaded++;
          
        }        
        
      } 
      
    }
    else streamlog_out( ERROR ) << "track input collection " << _trackInputColNames[i] << " could not be found!!!\n";
    
  }
  
  
  streamlog_out( DEBUG4 ) << "Loaded all in all " << nTrackLoaded << " tracks, which will now get further processed\n";
  
  

  
  
  
  /**********************************************************************************************/
  /*       Make sure that all tracks are compatible: find the best subset                       */
  /**********************************************************************************************/
  
  TrackQI trackQI( _trkSystem );
  
  streamlog_out( DEBUG3 ) << "The tracks and their qualities (and their hits ): \n";
  for( unsigned i=0; i < tracks.size(); i++ ){
    
    double qi = trackQI( tracks[i] );
    streamlog_out( DEBUG3 ) << tracks[i] << "\t" << qi << "( ";
    std::vector< TrackerHit* > hits = tracks[i]->getTrackerHits();
    
    for( unsigned j=0; j<hits.size(); j++ ) streamlog_out( DEBUG3 ) << hits[j] << " ";
    
    streamlog_out( DEBUG3 ) << ")\n";
    
  }
  
  TrackCompatibility comp;
  
  
  SubsetHopfieldNN< Track* > subset;
  subset.add( tracks );
  subset.calculateBestSet( comp, trackQI );
  
  std::vector< Track* > accepted = subset.getAccepted();
  std::vector< Track* > rejected = subset.getRejected();
  
  streamlog_out( DEBUG3 ) << "\tThe accepted tracks: \n";
  for( unsigned i=0; i < accepted.size(); i++ ){
    
    streamlog_out( DEBUG3 ) << accepted[i] <<  "\n";
    
  }
  
  streamlog_out( DEBUG3 ) << "\tThe rejected tracks: \n";
  for( unsigned i=0; i < rejected.size(); i++ ){
    
    streamlog_out( DEBUG3 ) << rejected[i] <<  "\n";
    
  }
  
  /**********************************************************************************************/
  /*                            */
  /**********************************************************************************************/
  
  



  _nEvt ++ ;
  
}



void TrackSubsetProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void TrackSubsetProcessor::end(){ 

    //   std::cout << "TrackSubsetProcessor::end()  " << name() 
    // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    // 	    << std::endl ;

}





