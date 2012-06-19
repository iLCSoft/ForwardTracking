#ifndef ForwardTracking_h
#define ForwardTracking_h 1

#include <string>

#include "marlin/Processor.h"
#include "lcio.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "IMPL/TrackImpl.h"
#include "MarlinTrk/IMarlinTrkSystem.h"
#include "gear/BField.h"

#include "KiTrack/Segment.h"
#include "KiTrack/ITrack.h"
#include "Criteria/Criteria.h"
#include "ILDImpl/SectorSystemFTD.h"

using namespace lcio ;
using namespace marlin ;
using namespace KiTrack;
using namespace KiTrackMarlin;

/** a simple typedef, making writing shorter. And it makes sense: a track consists of hits. But as a real track
 * has more information, a vector of hits can be considered as a "raw track". */
typedef std::vector< IHit* > RawTrack;


/**  Standallone Forward Tracking Processor for Marlin.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The hits in the FTDs
 *
 *  <h4>Output</h4> 
 *  A collection of Tracks.
 * 
 * @param FTDHitCollections The collections containing the FTD hits <br>
 * (default value "FTDTrackerHits FTDSpacePoints" (string vector) )
 * 
 * @param ForwardTrackCollection Name of the Forward Tracking output collection<br>
 * (default value  "ForwardTracks" (output collection) )
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
 * @param Chi2ProbCut Tracks with a chi2 probability below this will get sorted out<br>
 * (default value 0.005 )
 * 
 * @param HelixFitMax the maximum chi2/Ndf that is allowed as result of a helix fit
 * (default value 500 )
 * 
 * @param OverlappingHitsDistMax The maximum distance of hits from overlapping petals belonging to one track<br>
 * (default value 3.5 )
 * 
 * @param HitsPerTrackMin The minimum number of hits to create a track<br>
 * (default value 3 )
 * 
 * @param BestSubsetFinder The method used to find the best non overlapping subset of tracks. Available are: SubsetHopfieldNN and SubsetSimple.
 * Any other value means, that no final search for the best subset is done and overlapping tracks are possible. (If you want that, don't
 * leave the string empty or the default value will be used! Just write something like "None")<br>
 * (default value TrackSubsetHopfieldNN )
 * 
 * @param Criteria A vector of the criteria that are going to be used by the Cellular Automaton. <br>
 * For every criterion a min and max needs to be set!!!<br>
 * (default value is defined in class Criteria )
 * 
 * @param NameOfACriterion_min/max For every used criterion a minimum and maximum value needs to be set. <br>
 * If a criterion is named "Crit_Example", then the min parameter would be: Crit_Example_min and the max parameter Crit_Example_max.<br>
 * You can enter more than one value!!! So for example you could write something like \<parameter name="Crit_Example_min" type="float">30 0.8\</parameter>.<br>
 * This means, that if the Cellular Automaton creates too many connections (how many is defined in "MaxConnectionsAutomaton" ) it reruns
 * it with the next set of parameters. Thus allowing to tighten the cuts, if there are too many connections and so preventing 
 * it from getting stuck in very bad combinatorial situations. So in this example the Automaton will first run with the Crit_Example_min of 30
 * and if that generates too many connections, it will rerun it with the value 0.8.<br>
 * If for a criterion no further parameters are specified, the first ones will be taken on reruns.
 * 
 * @param MaxConnectionsAutomaton If the automaton has more connections than this it will be redone with the next parameters for the criteria.<br>
 * If there are no further new parameters for the criteria, it will skip the event.<br>
 * (default value 100000 )
 * 
 * @param MaxHitsPerSector If on any single sector there are more hits than this, a marlin::SkipEventException will be thrown<br>
 * (default value 100)
 * 
 * @author Robin Glattauer HEPHY, Wien
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
    

    
   /** Input collection names.
   */
   std::vector<std::string> _FTDHitCollections;
   
   /** Output collection name.
   */
   std::string _ForwardTrackCollection;


   int _nRun ;
   int _nEvt ;


   double _Bz; //B field in z direction


   double _chi2ProbCut;
   double _helixFitMax;


   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   int _maxHitsPerSector;
      
   
   /** A map to store the hits according to their sectors */
   std::map< int , std::vector< IHit* > > _map_sector_hits;
   
   /**
    *  @return a map that links hits with overlapping hits on the petals behind
    */
   std::map< IHit* , std::vector< IHit* > > getOverlapConnectionMap( 
            std::map< int , std::vector< IHit* > > & map_sector_hits, 
            const SectorSystemFTD* secSysFTD,
            float distMax);

   /** Adds hits from overlapping areas to a RawTrack in every possible combination.
    *
    * @return all of the resulting RawTracks
    * 
    * @param rawTrack a RawTrack (vector of IHit* ), we want to add hits from overlapping regions
    * 
    * @param map_hitFront_hitsBack a map, where IHit* are the keys and the values are vectors of hits that
    * are in an overlapping region behind them. (TODO: what this function does is actually more general, maybe rename)
    */
   std::vector < RawTrack > getRawTracksPlusOverlappingHits( RawTrack rawTrack , std::map< IHit* , std::vector< IHit* > >& map_hitFront_hitsBack );
      
   /** Finalises the track: fits it and adds TrackStates at IP, Calorimeter Face, inner- and outermost hit.
    * Sets the subdetector hit numbers and the radius of the innermost hit.
    * Also sets chi2 and Ndf.
    */
   void finaliseTrack( TrackImpl* trackImpl );
      
   
   bool setCriteria( unsigned round );
   
   std::vector< std::string > _criteriaNames;
   std::map< std::string , std::vector<float> > _critMinima;
   std::map< std::string , std::vector<float> > _critMaxima;
   
   int _hitsPerTrackMin;
   
   std::vector <ICriterion*> _crit2Vec;
   std::vector <ICriterion*> _crit3Vec;
   std::vector <ICriterion*> _crit4Vec;
    
   const SectorSystemFTD* _sectorSystemFTD;
   
   bool _useCED;
   
   double _overlappingHitsDistMax;
   
   bool _takeBestVersionOfTrack;
   
   int _maxConnectionsAutomaton;
   
   std::string _bestSubsetFinder;
   
   
   /** @return Info on the content of _map_sector_hits */
   std::string getInfo_map_sector_hits();
   
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



