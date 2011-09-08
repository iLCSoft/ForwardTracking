#ifndef FTDNoiseProcessor_h
#define FTDNoiseProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <vector>


// STUFF needed for GEAR
#include <marlin/Global.h>
#include <gear/GEAR.h>




using namespace lcio ;
using namespace marlin ;


/** Generates Noise Hits in the FTD detector.
 * The number of noise hits are given by the parameter nNoiseHits.
 * 
 * @param FTDCollectionName Name of the FTD TrackerHit collection where the background hits will be added.
 * (default name FTDTrackerHits)
 * 
 * @param PointResolution the resolution of a hit on the FTD. 
 * As the simulated background hits are placed at random no smearing is necessary. However, as the resolution in the plane
 * of the detector is saved for a TrackerHitPlaneImpl this value will get stored.
 * (default value 0.010)
 * 
 * @param BackgroundHitDensity the densities of the background hits measured in hits / cm^2 /BX  (BX= bunchcrossing) for
 * the different layers.
 * These units are chosen because they are identical with those in the LOI.
 * (default values 0.013 0.008 0.002 0.002 0.001 0.001 0.001 )
 * 
 * @param BackgroundHitDensitySigma the sigmas corresponding to the BackgroundHitDensity. Also in hits / cm^2 /BX.
 * The actual number of created background hits will be smeared gaussian aroung the BackgroundHitDensity with these values.
 * (default values 0.005 0.003 0.001 0.001 0.001 0.001 0.001 ) 
 * 
 * @param IntegratedBX the number of integrations of bunchcrossings the FTDs do before readout. For strip detectors this
 * is usually 1 and for Pixels a lot more.
 * (default values 100 100 1 1 1 1 1 )
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

  float _pointReso;

  int _nRun ;
  int _nEvt ;

  std::vector <double> _diskPositionZ;
  std::vector <double> _diskInnerRadius;
  std::vector <double> _diskOuterRadius; 
  
  
   std::vector < float > _backgroundDensity;
   std::vector < float > _backgroundDensitySigma;
   std::vector < int >   _integratedBX;


} ;

#endif



