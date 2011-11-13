#ifndef ForwardTracking_h
#define ForwardTracking_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include "Segment.h"

#include <gear/BField.h>
#include "Criteria.h"
#include "SectorSystemFTD.h"

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
 * @param ForwardTrackCollection Name of the Forward Tracking output collection
 * 
 * @param ptMin Minimal allowed transversal momentum. Should be a bit lower than the wanted value due to fluctuations. Measured in GeV
 * 
 * @param MultipleScatteringOn Whether to take multiple scattering into account when fitting the tracks
 * 
 * @param EnergyLossOn Whether to take energyloss into account when fitting the tracks
 * 
 * @param SmoothOn Whether to smooth all measurement sites in fit
 * 
 * @param Chi2ProbCut Tracks with a chi2 probability below this will get sorted out
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
   std::string _ForwardTrackCollection;


   int _nRun ;
   int _nEvt ;


   double _Bz; //B field in z direction


   double _chi2ProbCut;



   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   
   /** Draws the FTD sensors in CED
    */
   void drawFTDSensors ( const gear::GearParameters& paramFTD , unsigned nPetalsPerDisk , unsigned nSensorsPerPetal);
   


   std::vector< std::string > _criteriaNames;
   std::map< std::string , float > _critMinima;
   std::map< std::string , float > _critMaxima;
   
   
   std::vector <ICriterion*> _crit2Vec;
   std::vector <ICriterion*> _crit3Vec;
   std::vector <ICriterion*> _crit4Vec;
    
   const SectorSystemFTD* _sectorSystemFTD;
   
} ;

#endif



