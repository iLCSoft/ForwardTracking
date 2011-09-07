#ifndef FTDGhostProcessor_h
#define FTDGhostProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <vector>


// STUFF needed for GEAR
#include <marlin/Global.h>
#include <gear/GEAR.h>

#include <EVENT/TrackerHit.h>


using namespace lcio ;
using namespace marlin ;


/** Generates Ghost Hits in the FTD detector.
 * 
 * 
 * @author Robin Glattauer, HEPHY
 */
class FTDGhostProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new FTDGhostProcessor ; }
  
  
    FTDGhostProcessor() ;
  
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

   float _pointReso;

   int _nRun ;
   int _nEvt ;

   std::vector <double> _diskPositionZ;
   std::vector <double> _diskInnerRadius;
   std::vector <double> _diskOuterRadius; 
   
   int _nPetalsPerDisk;
   int _nSensorsPerPetal;
   int _nStripsPerSensor;
    

   
   


} ;

#endif



