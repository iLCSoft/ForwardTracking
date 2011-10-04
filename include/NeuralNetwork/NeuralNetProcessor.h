#ifndef NeuralNetProcessor_h
#define NeuralNetProcessor_h 1

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

class NeuralNetProcessor : public Processor {
  
 public:
  
    virtual Processor*  newProcessor() { return new NeuralNetProcessor ; }
  
  
    NeuralNetProcessor() ;
  
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


  int _nRun ;
  int _nEvt ;
} ;

#endif



