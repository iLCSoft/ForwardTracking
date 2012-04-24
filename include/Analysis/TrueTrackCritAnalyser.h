#ifndef TrueTrackCritAnalyser_h
#define TrueTrackCritAnalyser_h

#include <string>

#include "marlin/Processor.h"
#include "lcio.h"
#include "EVENT/TrackerHit.h"
#include "EVENT/Track.h"
#include "EVENT/LCRelation.h"
#include "MarlinTrk/IMarlinTrkSystem.h"

#include "KiTrack/Segment.h"
#include "Criteria/ICriterion.h"

#include "ILDImpl/SectorSystemFTD.h"

using namespace lcio ;
using namespace marlin ;
using namespace KiTrackMarlin;





/**  Processor to analyse Criteria of true tracks.
 * 
 * Criteria are classes derived from ICriterion in KiTrack. 
 * They are used by the Cellular Automaton to determine what tracksegments
 * might belong together. 
 * (For more information on Criteria and the Cellular Automaton see the KiTrack package.)
 * 
 * This processor analyses the values of these criteria, that true tracks produce.
 * The results are stored in the trees of a root file.
 * Additional to information about the criteria, fit information and the distances of
 * hits in a track are stored.
 * 
 * It is intended as a tool to finde appropriate values for the steering of the Cellular Automaton.
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The true tracks to be analysed
 *
 *  <h4>Output</h4> 
 *  A root file containing information on the criteria
 * 
 * @param MCTrueTrackRelCollectionName Name of the TrueTrack MC Relation collection <br>
 * (default value TruthTracksMCP )
 * 
 * @param RootFileName Name of the root file for saving the results <br>
 * (default value TrueTracksCritAnalysis.root )
 * 
 * @param WriteNewRootFile What to do with older root file: true = rename it, false = leave it and append new one <br>
 * (default value true )
 * 
 * @param CutChi2Prob Tracks with a chi2 probability below this value won't be considered <br>
 * (default value 0.005 )
 * 
 *  @param CutPtMin The minimum transversal momentum pt above which tracks are of interest in GeV <br>
 * (default value 0.1 )
 * 
 *  @param CutDistToIPMax The maximum distance from the origin of the MCP to the IP (0,0,0)<br>
 * (default value 100 )
 * 
 *  @param CutNumberOfHitsMin The minimum number of hits a track must have <br>
 * (default value 4 )
 * 
 *  @param OverlappingHitsDistMax The maximum distance of hits from overlapping petals belonging to one track <br>
 * (default value 4 )
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
 * @author R. Glattauer HEPHY, Wien
 *
 */
class TrueTrackCritAnalyser : public Processor {
   
public:
   
   virtual Processor*  newProcessor() { return new TrueTrackCritAnalyser ; }
   
   
   TrueTrackCritAnalyser() ;
   
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
   std::string _FTDTrueTracks;
   
   
   
   int _nRun ;
   int _nEvt ;
   
   
   double _Bz; //B field in z direction
   
   std::string _rootFileName;
   std::string _treeName2;
   std::string _treeName3;
   std::string _treeName4;
   std::string _treeNameKalman;
   std::string _treeNameHitDist;
   
   std::string _colNameMCTrueTracksRel;
   
   std::vector <ICriterion*> _crits2;
   std::vector <ICriterion*> _crits3;
   std::vector <ICriterion*> _crits4;

   
   
   bool _MSOn ;
   bool _ElossOn ;
   bool _SmoothOn ;
   
   
   double _ptMin;
   double _distToIPMax;
   double _chi2ProbCut;
   int _nHitsMin;
   double _overlappingHitsDistMax;
   bool _writeNewRootFile;
   
   const SectorSystemFTD* _sectorSystemFTD;
   
   
   
} ;

#endif



