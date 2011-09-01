#ifndef FTDNoiseProcessor_h
#define FTDNoiseProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <vector>


// STUFF needed for GEAR
#include <marlin/Global.h>
#include <gear/GEAR.h>
#include <gear/VXDParameters.h>
#include <gear/VXDLayerLayout.h>


using namespace lcio ;
using namespace marlin ;


/** Generates Noise Hits in the FTD detector.
 * The number of noise hits are given by the parameter nNoiseHits.
 * 
 * @author Robin Glattauer, HEPHY
 */
class FTDNoiseProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new FTDNoiseProcessor ; }
  
  
    FTDNoiseProcessor() ;
  
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

  std::string _colNameFTD ;
  int _nNoiseHits;


  int _nRun ;
  int _nEvt ;

  std::vector <double> _diskPositionZ;
  std::vector <double> _diskInnerRadius;
  std::vector <double> _diskOuterRadius; 


} ;

#endif



