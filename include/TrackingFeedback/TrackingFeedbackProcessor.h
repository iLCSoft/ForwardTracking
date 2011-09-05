#ifndef TrackingFeedbackProcessor_h
#define TrackingFeedbackProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>


using namespace lcio ;
using namespace marlin ;


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
    std::string _AutTrkCollection;
    
    std::string _rootFileName;
    
    std::string _treeName;

  int _nRun ;
  int _nEvt ;
} ;

#endif


