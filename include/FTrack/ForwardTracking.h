#ifndef ForwardTracking_h
#define ForwardTracking_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include "Segment.h"

#include "FTDTrackFitter.h"


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





/**  Standallone Forward Tracking Processor for marlin.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The hits in the FTDs
 *
 *  <h4>Output</h4> 
 *  A collection of Tracks.
 * 
 * @param FTDHitCollection The collection of the FTD hits
 * 
 * @param AutTrkCollection The output collection
 * 
 * @param ptMin Minimal allowed transversal momentum. Should be a bit lower than the wanted value due to fluctuations. Measured in GeV
 * 
 * @param MultipleScatteringOn Whether to take multiple scattering into account when fitting the tracks
 * 
 * @param EnergyLossOn Whether to take energyloss into account when fitting the tracks
 * 
 * @param SmoothOn Whether to smooth all measurement sites in fit
 * 
 * @author R. Glattauer HEPHY, Wien
 *
 */



class ForwardTracking : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new ForwardTracking ; }
  
  
  ForwardTracking() ;
  
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
    

    
   /** Input collection name.
   */
   std::string _FTDHitCollection;
   
   /** Output collection name.
   */
   std::string _AutTrkCollection;


   int _nRun ;
   int _nEvt ;


   double _Bz; //B field in z direction


   //For Fitting
   FTDTrackFitter _trackFitter;

   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;

    
} ;

#endif



