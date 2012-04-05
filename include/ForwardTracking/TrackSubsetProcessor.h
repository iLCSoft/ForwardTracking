#ifndef TrackSubsetProcessor_h
#define TrackSubsetProcessor_h 1


#include "marlin/Processor.h"
#include "lcio.h"
#include "EVENT/Track.h"
#include "MarlinTrk/IMarlinTrkSystem.h"

#include "Tools/Fitter.h"




using namespace lcio ;
using namespace marlin ;


/**  Processor that takes tracks from multiple sources and outputs them (or modified versions, or a subset of them)
 * as one track collection.
 * 
 *  <h4>Input - Prerequisites</h4>
 *  Track collections
 *
 *  <h4>Output</h4> 
 *  A single track collection
 * 
 * @param TrackInputCollections A vector of the input track collections <br>
 * (default value ForwardTracks SiTracks )
 * 
 * @param TrackOutputCollection Name of the output track collection <br>
 * (default value SubsetTracks )
 * 
 * @param MultipleScatteringOn Whether to take multiple scattering into account when fitting the tracks<br>
 * (default value true )
 *  
 * @param EnergyLossOn Whether to take energyloss into account when fitting the tracks<br>
 * (default value true )
 * 
 * @param SmoothOn Whether to smooth all measurement sites in fit<br>
 * (default value false )
 * 
 * @author Robin Glattauer, HEPHY
 * 
 */



class TrackSubsetProcessor : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new TrackSubsetProcessor ; }
  
  
  TrackSubsetProcessor() ;
  
  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual void init() ;
  
  /** Called for every run.
   */
  virtual void processRunHeader( LCRunHeader* run ) ;
  
  /** Called for every event - the working horse.
   */
  virtual void processEvent( LCEvent * evt ) ; 
  
  
  virtual void check( LCEvent * evt ) ; 
  
  
  /** Called after data processing for clean up.
   */
  virtual void end() ;
  
  
 protected:


 
  
  /** Input collection names */
  std::vector< std::string > _trackInputColNames;
  
  /** Output collection name */
  std::string _trackOutputColName;
  
  MarlinTrk::IMarlinTrkSystem* _trkSystem;
  
  bool _MSOn ;
  bool _ElossOn ;
  bool _SmoothOn ;

  int _nRun ;
  int _nEvt ;
} ;


/** A functor to return whether two tracks are compatible: The criterion is if the share a TrackerHit or more */
class TrackCompatibility{
  
  
public:
  
  
  inline bool operator()( Track* trackA, Track* trackB ){
    
    
    std::vector< TrackerHit* > hitsA = trackA->getTrackerHits();
    std::vector< TrackerHit* > hitsB = trackB->getTrackerHits();
    
    
    for( unsigned i=0; i < hitsA.size(); i++){
      
      for( unsigned j=0; j < hitsB.size(); j++){
        
        if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
      
      }
      
    }
    
    return true;      
    
    
  }
  
  
};


/** A functor to return the quality of a track, which is currently the chi2 probability. */
class TrackQI{
  
public:
  
  /** @param trkSystem a pointer to an IMarlinTrkSystem, needed for fitting of tracks */
  TrackQI( MarlinTrk::IMarlinTrkSystem* trkSystem ): _trkSystem(trkSystem){}
  
  inline double operator()( Track* track ){
    
    try{
      
      Fitter fitter( track , _trkSystem );
      return fitter.getChi2Prob( lcio::TrackState::AtIP );
      
    }
    catch( FitterException e ){
      
      
      streamlog_out( ERROR ) << "TrackSubsetProcessor: getQI(): Track " << track << "couldn't be fitted: " <<  e.what() << "\n";
      
    }
    
    return 0.;
    
    
  }
  
protected:
  
  MarlinTrk::IMarlinTrkSystem* _trkSystem;
  
  
};




#endif



