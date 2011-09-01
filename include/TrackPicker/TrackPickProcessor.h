#ifndef TrackPickProcessor_h
#define TrackPickProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>


using namespace lcio ;
using namespace marlin ;


/**  Processor to pick single or more tracks from those the track cheater finds.
 *    As this is just for testing, it is so far a hard coded stuff.
 * 
 * 
 * 
 */

class TrackPickProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new TrackPickProcessor ; }
  
  
    TrackPickProcessor() ;
  
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
    
   /**output collection
    */
   std::string _TrackPickCollection;

  int _nRun ;
  int _nEvt ;
} ;

#endif



