#ifndef TrueTrackCritAnalyser_h
#define TrueTrackCritAnalyser_h 1

#include "marlin/Processor.h"
#include "lcio.h"

#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include "Segment.h"
#include <EVENT/LCRelation.h>

#include "ICriterion.h"
#include <string>

using namespace lcio ;
using namespace marlin ;
using namespace FTrack;





/**  Processor to analyse what kind of criteria the the true tracks fulfill.
 * 
 * 
 *  <h4>Input - Prerequisites</h4>
 *  The cheated tracks in the FTDs
 *
 *  <h4>Output</h4> 
 *  A root file
 * 
 * @param MCTrueTrackRelCollectionName The true track <-> MCP relations in the FTDs
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
   
   const SectorSystemFTD* _sectorSystemFTD;
   
} ;

#endif



