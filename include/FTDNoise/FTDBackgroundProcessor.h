#ifndef FTDBackgroundProcessor_h
#define FTDBackgroundProcessor_h 1

#include <string>
#include <vector>

#include <CLHEP/Vector/ThreeVector.h>

#include "marlin/Processor.h"
#include "lcio.h"


using namespace lcio ;
using namespace marlin ;


/** Generates background hits in the FTD detector.
 * 
 * @param FTDPixelTrackerHitCollectionName Name of the FTD Pixel TrackerHit collection where the background hits will be added.<br>
 * (default value FTDPixelTrackerHits)
 * 
 * @param FTDStripTrackerHitCollectionName Name of the FTD Strip TrackerHit collection where the background hits will be added.<br>
 * (default value FTDStripTrackerHits)
 * 
 * @param ResolutionU resolution in direction of u (in mm) <br>
 * (default value 0.004)
 * 
 * @param ResolutionV Resolution in direction of v (in mm) <br>
 * (default value 0.004)
 * 
 * @param BackgroundHitDensity the densities of the background hits measured in hits / cm^2 /BX  (BX= bunchcrossing) for
 * the different layers.<br>
 * These units are chosen because they are identical with those in the LOI.<br>
 * (default values 0.013 0.008 0.002 0.002 0.001 0.001 0.001 )
 * 
 * @param BackgroundHitDensitySigma the sigmas corresponding to the BackgroundHitDensity. Also in hits / cm^2 /BX.<br>
 * The actual number of created background hits will be smeared gaussian aroung the BackgroundHitDensity with these values.<br>
 * (default values 0.005 0.003 0.001 0.001 0.001 0.001 0.001 ) 
 * 
 * @param IntegratedBX the number of integrations of bunchcrossings the FTDs do before readout. For strip detectors this
 * is usually 1 and for Pixels a lot more.<br>
 * (default values 100 100 1 1 1 1 1 )
 * 
 * @param DensityRegulator Regulates all densities. This can be used to dim or amplify all the background. <br>
 * 1 means no change at all, 2 means background is doubled, 0.7 means only 70 percent of the background and so on. <br>
 * (default value 1. )
 * 
 * @author Robin Glattauer, HEPHY
 */
class FTDBackgroundProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new FTDBackgroundProcessor ; }
  
  
    FTDBackgroundProcessor() ;
  
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
   
   
    CLHEP::Hep3Vector getRandPosition( double rMin, double lengthMin, double lengthMax, double width, double phi, double z );
   

   std::string _colNameFTDStripTrackerHit;
   std::string _colNameFTDPixelTrackerHit;

   float _resU ;
   float _resV ;

   int _nRun ;
   int _nEvt ;

   
   float _densityRegulator;
   
   std::vector < float > _backgroundDensity;
   std::vector < float > _backgroundDensitySigma;
   std::vector < int >   _integratedBX;


} ;

#endif



