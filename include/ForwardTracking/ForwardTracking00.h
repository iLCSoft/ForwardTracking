#ifndef ForwardTracking00_h
#define ForwardTracking00_h 1

#include <string>

#include "marlin/Processor.h"
#include "lcio.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "gear/BField.h"
#include "MarlinTrk/IMarlinTrkSystem.h"

#include "KiTrack/Segment.h"
#include "KiTrack/ITrack.h"
#include "Criteria/Criteria.h"
#include "ILDImpl/SectorSystemFTD.h"

using namespace lcio ;
using namespace marlin ;
using namespace KiTrack;
using namespace KiTrackMarlin;





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



class ForwardTracking00 : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new ForwardTracking00 ; }
  
  
  ForwardTracking00() ;
  
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
   
   void drawTrack( ITrack* track, unsigned color , unsigned width = 1);

   std::vector< std::string > _criteriaNames;
   std::map< std::string , float > _critMinima;
   std::map< std::string , float > _critMaxima;
   
   
   std::vector <ICriterion*> _crit2Vec;
   std::vector <ICriterion*> _crit3Vec;
   std::vector <ICriterion*> _crit4Vec;
    
   const SectorSystemFTD* _sectorSystemFTD;
   
   bool _useCED;
   
   MarlinTrk::IMarlinTrkSystem* _trkSystem;
   
} ;


/** A functor to return whether two tracks are compatible: The criterion is if the share a TrackerHit or more */
class TrackCompatibilityShare1SP{
   
public:
   
   inline bool operator()( ITrack* trackA, ITrack* trackB ){
      
      
      std::vector< IHit* > hitsA = trackA->getHits();
      std::vector< IHit* > hitsB = trackB->getHits();
      
      
      for( unsigned i=0; i < hitsA.size(); i++){
         
         for( unsigned j=0; j < hitsB.size(); j++){
            
            if ( hitsA[i] == hitsB[j] ) return false;      // a hit is shared -> incompatible
            
         }
         
      }
      
      return true;      
      
   }
   
};


/** A functor to return the quality of a track, which is currently the chi2 probability. */
class TrackQIChi2Prob{
   
public:
   
   inline double operator()( ITrack* track ){ return track->getChi2Prob(); }
   
   
};

#endif



