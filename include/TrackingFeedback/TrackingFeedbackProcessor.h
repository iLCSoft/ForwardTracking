#ifndef TrackingFeedbackProcessor_h
#define TrackingFeedbackProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include "FTDTrackFitter.h"
#include "ICriterion.h"


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;


/**  Example processor for marlin.
 * 
 * 
 * 
 * 
 */

class TrackingFeedbackProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new TrackingFeedbackProcessor ; }
  
  
    TrackingFeedbackProcessor() ;
  
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
   std::string _colNameMCTrueTracksRel;
   std::string _TrackCollection;

   std::string _rootFileName;
   std::string _treeName;

   int _nRun ;
   int _nEvt ;
   
   double _ptMin;
   double _distToIPMax;
   double _chi2ProbCut;
   int _nHitsMin;
   
   //For Fitting
   FTDTrackFitter _trackFitter;

   std::vector <ICriterion*> _crits2;
   std::vector <ICriterion*> _crits3;
   std::vector <ICriterion*> _crits4;

   
} ;

#endif



