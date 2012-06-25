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


/**  Standallone Forward Tracking Processor for Marlin.<br>
 * 
 * Reconstructs the tracks through the FTD <br>
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The hits in the Forward Tracking Detector FTD
 *
 *  <h4>Output</h4> 
 *  A collection of reconstructed Tracks.
 * 
 * @param FTDHitCollections The collections containing the FTD hits <br>
 * (default value "FTDTrackerHits FTDSpacePoints" (string vector) )
 * 
 * @param ForwardTrackCollection Name of the Forward Tracking output collection<br>
 * (default value  "ForwardTracks" )
 * 
 * @param MultipleScatteringOn Whether to take multiple scattering into account when fitting the tracks<br>
 * (default value true )
 * 
 * @param EnergyLossOn Whether to take energy loss into account when fitting the tracks<br>
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
 * @param MaxConnectionsAutomaton If the automaton has more connections than this it will be redone with the next cut off values for the criteria.<br>
 * If there are no further new values for the criteria, the event will be skipped.<br>
 * (default value 100000 )
 * 
 * @param MaxHitsPerSector If on any single sector there are more hits than this, all the hits in the sector get dropped.
 * This is to prevent combinatorial breakdown (It is a second safety mechanism, the first one being MaxConnectionsAutomaton.
 * But if there are soooo many hits, that already the first round of the Cellular Automaton would take forever, this mechanism
 * prevents it) <br>
 * (default value 1000)
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
   
   /**
   * @return a map that links hits with overlapping hits on the petals behind
   * 
   * @param map_sector_hits a map with first= the sector number. second = the hits in the sector. 
   * 
   * @param secSysFTD the SectorSystemFTD that is used
   * 
   * @param distMax the maximum distance of two hits. If two hits are on the right petals and their distance is smaller
   * than this, the connection will be saved in the returned map.
   */
   std::map< IHit* , std::vector< IHit* > > getOverlapConnectionMap( std::map< int , std::vector< IHit* > > & map_sector_hits, 
                                                                     const SectorSystemFTD* secSysFTD,
                                                                     float distMax);
   
   /** Adds hits from overlapping areas to a RawTrack in every possible combination.
   * 
   * @return all of the resulting RawTracks
   * 
   * @param rawTrack a RawTrack (vector of IHit* ), we want to add hits from overlapping regions
   * 
   * @param map_hitFront_hitsBack a map, where IHit* are the keys and the values are vectors of hits that
   * are in an overlapping region behind them.
   */
   std::vector < RawTrack > getRawTracksPlusOverlappingHits( RawTrack rawTrack , std::map< IHit* , std::vector< IHit* > >& map_hitFront_hitsBack );
   
   /** Finalises the track: fits it and adds TrackStates at IP, Calorimeter Face, inner- and outermost hit.
   * Sets the subdetector hit numbers and the radius of the innermost hit.
   * Also sets chi2 and Ndf.
   */
   void finaliseTrack( TrackImpl* trackImpl );
   
   /** Sets the cut off values for all the criteria
    * 
    * This method is necessary for cases where the CA just finds too much.
    * Therefore it is possible to enter a whole list of cut off values for every criterion (for every min and every max to be more precise),
    * that are then used one after the other. 
    * If the CA finds way too many connections, we can thus make the cuts tighter and rerun it. If there are still too many
    * connections, just tighten them again.
    * 
    * This method will set the according values. It will read the passed (as steering parameter) cut off values, create
    * criteria from them and store them in the corresponding vectors.
    * 
    * If there are no new cut off values for a criterion, the last one remains.
    * 
    * @return whether any new cut off value was set. false == there are no new cutoff values anymore
    * 
    * @param round The number of the round we are in. I.e. the nth time we run the Cellular Automaton.
    */
   bool setCriteria( unsigned round );
   
   
   /** @return Info on the content of _map_sector_hits. Says how many hits are in each sector */
   std::string getInfo_map_sector_hits();
   
   
   /** Input collection names */
   std::vector<std::string> _FTDHitCollections;
   
   /** Output collection name */
   std::string _ForwardTrackCollection;


   int _nRun ;
   int _nEvt ;

   /** B field in z direction */
   double _Bz;

   /** Cut for the Kalman Fit (the chi squared probability) */
   double _chi2ProbCut; 
   
   /** Cut for the Helix fit ( chi squared / degrees of freedom ) */
   double _helixFitMax; 

   // Properties of the Kalman Fit
   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   /** If this number of hits in a sector is surpassed for any sector, the hits in the sector will be dropped
    * and the quality of the output track collection will be set to poor */
   int _maxHitsPerSector;
   
   
   /** A map to store the hits according to their sectors */
   std::map< int , std::vector< IHit* > > _map_sector_hits;
   
   /** Names of the used criteria */
   std::vector< std::string > _criteriaNames;
   
   /** Map containing the name of a criterion and a vector of the minimum cut offs for it */
   std::map< std::string , std::vector<float> > _critMinima;
   
   /** Map containing the name of a criterion and a vector of the maximum cut offs for it */
   std::map< std::string , std::vector<float> > _critMaxima;
   
   /** Minimum number of hits a track has to have in order to be stored */
   int _hitsPerTrackMin;
   
   /** A vector of criteria for 2 hits (2 1-hit segments) */
   std::vector <ICriterion*> _crit2Vec;
   
   /** A vector of criteria for 3 hits (2 2-hit segments) */
   std::vector <ICriterion*> _crit3Vec;
   
   /** A vector of criteria for 4 hits (2 3-hit segments) */
   std::vector <ICriterion*> _crit4Vec;
   
   
   const SectorSystemFTD* _sectorSystemFTD;
   
   
   bool _useCED;
   
   /** the maximum distance of two hits from overlapping petals to be considered as possible part of one track */
   double _overlappingHitsDistMax;
   
   /** true = when adding hits from overlapping petals, store only the best track; <br>
    * false = store all tracksS
    */
   bool _takeBestVersionOfTrack;
   
   /** the maximum number of connections that are allowed in the automaton, if this value is surpassed, rerun
    * the automaton with tighter cuts or stop it entirely. */
   int _maxConnectionsAutomaton;
   
   /** The method used to find the best subset of tracks */
   std::string _bestSubsetFinder;
   
   

   
   MarlinTrk::IMarlinTrkSystem* _trkSystem;

   /** The quality of the output track collection */
   int _output_track_col_quality ; 
  
   static const int _output_track_col_quality_GOOD;
   static const int _output_track_col_quality_FAIR;
   static const int _output_track_col_quality_POOR;
  
   
} ;


/** A functor to return whether two tracks are compatible: The criterion is if they share a Hit or more */
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



