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
 * For this the hits in the strip detectors of the FTDs are taken and sorted according to the
 * pair of strip sensors where they caused a hit.
 * Then all the hits for a pair of sensors are taken and a strip activation is simulated for each of them.
 * the strips are combined to get all possible spacepoints from that certain strip-activation and finally the
 * new hits are stored in the FTD collection.
 * 
 * @param FTDCollectionName the name of the TrackerHit Collection of the FTD.\ For those hits the ghost hits are calculated
 * and in there they also get stored.
 * 
 * @param PointResolution the resolution of a hit on the FTD. 
 * The ghost hits will be smeared gaussian around their simulated position with this value as standard deviation.
 * 
 * @param nPetalsPerDisk how many petals there are for one FTD disk. 
 * (default value 16)
 * 
 * @param nSensorsPerPetal how many sensors there are on one petal. They are assumed to have the same delta R and to be
 * alligend in radial direction.
 * (default value 2)
 * 
 * @param nStripsPerSensor number of strips (same number for radial and for angular strips!) in a sensor.
 * (default value 1000)
 * 
 * @param IsStrip a vector of ints that tells, whether a disk is a strip detector (1) or not (0)
 * (default values 0 0 1 1 1 1 1)
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
    

   std::vector <int> _isStrip;
   


} ;

#endif



