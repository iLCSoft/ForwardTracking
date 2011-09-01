#ifndef MyAutProcessor_h
#define MyAutProcessor_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
#include <EVENT/TrackerHit.h>
#include <EVENT/Track.h>
#include "Segment.h"


using namespace lcio ;
using namespace marlin ;
using namespace FTrack;


/**  Cellular Automaton processor for marlin.
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
 * @param AutTrkCollection The output collection
 * 
 * @author R. Glattauer HEPHY, Wien
 *
 */




class MyAutProcessor : public Processor {
  
 public:
  
  virtual Processor*  newProcessor() { return new MyAutProcessor ; }
  
  
  MyAutProcessor() ;
  
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
    
   /** Connects the hits from 2 layers if they are not too far away. 
    * 
    * @param segments1 the hits to be connected represented as 1-segments in a vector of a vector
    * 
    * @return a vector of a vector of the calculated 2-segments
    * 
    */
    std::vector < std::vector <Segment* > > getSegments2 ( std::vector < std::vector <Segment* > > segments1 );
    
    /** Connects all 2-segments, that overlap
     */
    std::vector < std::vector <Segment* > > getSegments3( std::vector < std::vector <Segment* > > segments2 );
    
    /** Checks, if the segments are neighbors
     * 
     * Checks what the length of the segments is: if they are both 2-segments, then areNeighbors_2 is called,
     * if they are both 3-segments, then areNeighbors_3 is called.
     * If they are neither, they aren't neighbors (in the current definition, this might change however)
     * 
     * @return whether they are neighbors
     */
    bool areNeighbors ( Segment* parent , Segment* child );
    
    /** Checks if 2 2-segments are neighbors.
     * 
     * There are implemented two criterias at the moment:
     * 
     * - The angle between the segments
     * - The distance of (0,0) from the circle the 3 hits form
     */
    bool areNeighbors_2 ( Segment* parent , Segment* child );
    
    /** Checks, if 2 3-segments are neighbors
     * 
     * Those are the criteria:
     * 
     * - no zigzagging (or only when they are nearly a straight line)
     * - The angle between the 2segments of the 3- segments should not change a lot
     * - Each 3-segment defines a circle in the xy plane. The radius of the circle may only change a bit
     * - The delta phi angle (measured from the center of the circle) to two hits should be proportional to the z - distance of the hits
     * - The helix defined by 3 hits can be extrapolated to the z value of a 4th hit. The distance to this extrapolation
     * is also used as a criteria
     * 
     * @return whether the 3-segments are neighbors
     */
    bool areNeighbors_3 ( Segment* parent , Segment* child );
    
    /** Performs the Cellular Automaton.
     * 
     * In every iteration every segment is checked, whether it has a neighbor with the same state.
     * If it has one, its state gets +1. Otherwise it stays the same.
     * This is done until no more changes happen (which is equal to segments.size() - 1 times).
     */
    void doAutomaton( std::vector < std::vector <Segment* > > segments );
    
    /** Counts the tracks.
     * 
     * Uses getNumberOfTracks to get all tracks starting from a segment
     * 
     * @param segments here all segments are stored
     * @return Number of tracks in segments
     */
    void countTracks ( std::vector < std::vector <Segment* > > segments , unsigned int startLayer );
    
    /** 
     * 
     * @return number of tracks of the segment
     */
    int getNumberOfTracks ( Segment* segment );
    
    /** Cleans up after the automaton.
     * 
     * Checks if the segments have a state equal to their layer.
     * If not, the get erased and connections to them in their 
     * children and parents are erased as well. 
     * 
     */    
    void cleanSegments( std::vector < std::vector <Segment* > > & segments );
    
    /** Calculates all the track candidates that can be built from the children of a segment.
     * 
     * Works recursively. You pass it a segment and a vector of all the list the track contains so far, 
     * and it goes down all the children, adds the hits, and makes a seperate track candidate for every
     * unique path down. 
     * 
     */
    std::vector <Track*> getTrackCandidates ( Segment* segment, std::vector< TrackerHit*> hits );
   
   /** Input collection name.
   */
   std::string _FTDHitCollection;
   std::string _AutTrkCollection;
   
   double _ratioMax;

   int _nRun ;
   int _nEvt ;
} ;

#endif



