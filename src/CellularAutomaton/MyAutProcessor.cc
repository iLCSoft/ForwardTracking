#include "MyAutProcessor.h"
#include <iostream>

#include <EVENT/TrackerHit.h>
#include <IMPL/TrackerHitImpl.h>
#include <IMPL/TrackImpl.h>
#include <EVENT/Track.h>
#include <EVENT/LCCollection.h>
#include <IMPL/LCCollectionVec.h>

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

#include "Segment.h"
#include "TVector3.h"
#include <vector>
#include <algorithm>

#include "SimpleCircle.h"
#include <cmath>
#include <MarlinCED.h>

using namespace lcio ;
using namespace marlin ;
using namespace FTrack;




MyAutProcessor aMyAutProcessor ;


MyAutProcessor::MyAutProcessor() : Processor("MyAutProcessor") {

    // modify processor description
    _description = "MyAutProcessor tests the Cellular Automaton" ;
    

    // register steering parameters: name, description, class-variable, default value
    registerInputCollection(LCIO::TRACKERHIT,
                            "FTDHitCollectionName",
                            "FTD Hit Collection Name",
                            _FTDHitCollection,
                            std::string("FTDTrackerHits")); 


    registerOutputCollection(LCIO::TRACK,
                             "AutTrkCollection",
                             "Name of Cellular Automaton Track output collection",
                             _AutTrkCollection,
                             std::string("AutTracks"));
    
    registerProcessorParameter( "RdivZratioMax" ,
                                "Maximal ratio between distance of points divided by z distance"  ,
                                _ratioMax ,
                                2. ) ;

}




void MyAutProcessor::init() { 

    streamlog_out(DEBUG) << "   init called  " << std::endl ;

    // usually a good idea to
    printParameters() ;

    _nRun = 0 ;
    _nEvt = 0 ;
    
   
//     MarlinCED::init(this) ;    //CED

}


void MyAutProcessor::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void MyAutProcessor::processEvent( LCEvent * evt ) { 


//--CED---------------------------------------------------------------------
// Reset drawing buffer and START drawing collection
/*
  MarlinCED::newEvent(this , 0) ; 

  CEDPickingHandler &pHandler=CEDPickingHandler::getInstance();

  pHandler.update(evt); 
*/
//-----------------------------------------------------------------------
   

    // this gets called for every event 
    // usually the working horse ...

  

  LCCollection* col = evt->getCollection( _FTDHitCollection ) ;

  
  //this is a vector of a vector of TrackerHits. 
  std::vector< std::vector < Segment* > > hitsForward;
  hitsForward.resize(8);
  std::vector< std::vector < Segment* > > hitsBackward;
  hitsBackward.resize(8);

  // may sound odd, but: the first corresponds to the layer, with 0 being the interaction point and 1-7 being the ftd layers
  // in the vectors the hits are saved.
  // so hits[2][0].Z() will give the z coordinate of the first hit in the second layer.

  
  if( col != NULL ){

      int nHits = col->getNumberOfElements()  ;

      std::cout << std::endl << std::endl;
      std::cout << "Number of hits we deal with: " << nHits;
      std::cout << std::endl << std::endl;
      
      for(int i=0; i< nHits ; i++){

       
        TrackerHit* trkHit = dynamic_cast<TrackerHit*>( col->getElementAt( i ) );
       
         
        Segment* segment = new Segment(trkHit); 
        
        if (trkHit->getPosition()[2] > 0.){
           
           
            hitsForward[trkHit->getType()-200].push_back(segment);//Fill the array with the hits.
           
        }
        else 
           
           hitsBackward[trkHit->getType()-200].push_back(segment);
        
         /*
          //output the points and the layer
        std::cout << std::endl << "HitPosition: ( " 
        << trkHit->getPosition()[0] << " , " 
        << trkHit->getPosition()[1] << " , " 
        << trkHit->getPosition()[2] << " ) "
        << trkHit->getType()-201     <<  std::endl;
         */
      } 
  }
         

  std::vector < std::vector < std::vector <Segment* > > > allHits;
  allHits.push_back (hitsForward);
  allHits.push_back (hitsBackward);
  
  std::vector<Track*> trackCandidates;
  
  for (unsigned int hitCol=0; hitCol < allHits.size(); hitCol++){
  
      
      if ( hitCol == 0 )std::cout << "\n \t FORWARD: \n";
      else std::cout << "\n \t BACKWARD: \n";
     
      std::vector < std::vector <Segment* > > hits = allHits[hitCol];
     
      TrackerHitImpl* center = new TrackerHitImpl();
      double centerCoord[3] = {0.,0.,0.};  //This is the center of coordinates and equals roughly the ip.
      center->setPosition(centerCoord);    //that helps sorting out, BUT: it has to be removed later!!
      center->setType(200);
   
      Segment* centerSegment = new Segment(center);
      hits[0].push_back(centerSegment);
      
      
      
//       drawSegmentsInCed ( hits ); //CED

      
      std::cout << "\n---2-Segments---\n";
      // calculate all the 2-segments
      std::vector < std::vector <Segment* > > segments2 = getSegments2 ( hits );
      
       
      
//       drawSegmentsInCed ( segments2 ); //CED

      
      //Now do the cellular Automaton with the segments
      doAutomaton ( segments2 );   

      
      
//       drawSegmentsInCed ( segments2 ); //CED
     
      
      
      //count the track candidates. this is only for feedback and is not essentially needed
//       countTracks ( segments2 , 3 ); //start at layer 3 (== 4 hits or more)

      
      //erase wrong segments
      cleanSegments ( segments2 );
         


      
//       drawSegmentsInCed ( segments2 ); //CED
     
      
      
//       countTracks ( segments2 , 3);
      
      
      std::cout << "\n---3-Segments---\n";
      //calculate now the 3-segments
      std::vector < std::vector <Segment* > > segments3 = getSegments3 ( segments2 );
      

      //Now do the cellular Automaton with the 3-segments
      
      doAutomaton ( segments3 );
      
     
      
//       drawSegmentsInCed ( segments3 ); //CED 

      
      //erase wrong segments
      cleanSegments ( segments3 );
      cleanSinguletts ( segments3 );
         
      
      
//       drawSegmentsInCed ( segments3 ); //CED 
      

      //get the track candidates and save them. 
      
      unsigned int count = 0;
      

      
      for (unsigned int layer=2; layer < segments3.size(); layer++){ //over all layers bigger than 2 (= 4 hits or more)
         
         for (unsigned int iSeg=0; iSeg< segments3[layer].size(); iSeg++){ //over all segments in this layer
            
            if ( segments3[layer][iSeg]->_parents.size() == 0 ){ //only start tracks from where there are no parents
               
               std::vector <TrackerHit*> emptyHits;
                              
               std::vector <Track*> newTrackCands = getTrackCandidates( segments3[layer][iSeg] , emptyHits );
               
               for ( unsigned int i=0; i < newTrackCands.size(); i++){  //add the track candidates
                        
                  trackCandidates.push_back( newTrackCands[i] ); 
                        
                  count++;
                        
               }
               
            }
         }
      }
      
      std::cout << "\n Track Candidates=" << count << "\n";
      
     
       
  }
    
  std::cout << "\n Overall Track Candidates=" << trackCandidates.size() << "\n";
    
  
  
//   drawTracksInCed ( trackCandidates );   //CED begin
  

  
  
  //finally: save the tracks
  LCCollectionVec * trkCol = new LCCollectionVec(LCIO::TRACK);
  for (unsigned int i=0; i < trackCandidates.size(); i++) trkCol->addElement( trackCandidates[i] );
  evt->addCollection(trkCol,_AutTrkCollection.c_str());
  

  
 
  
  
//-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !

    streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
        << "   in run:  " << evt->getRunNumber() << std::endl ;

    
  
        
//    MarlinCED::draw(this);  //CED begin
   
   
        
    _nEvt ++ ;
}





void MyAutProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void MyAutProcessor::end(){ 

    //   std::cout << "MyProcessor::end()  " << name() 
    // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    // 	    << std::endl ;

}



std::vector < std::vector <Segment* > > MyAutProcessor::getSegments2 ( std::vector < std::vector <Segment* > > segments1 ){
   
   
      
   //***********************************//
   //***********************************//
   //Find all connected hits and set up the connection
      
   int count = 0;  
   

      
   std::vector< std::vector < Segment* > > hits = segments1;
   std::vector< std::vector < Segment* > > segments2; //segments with size 2. Segments always belong to the layer of their innermost point
   segments2.resize( hits.size() - 1 ); //one shorter than the segments which only contain 1 hit, of course
      
      //   std::cout << "\nhitsForward2.size()-1= " << hitsForward2.size()-1 ;
      
   for (unsigned int layer=hits.size()-1; layer>0; layer--){ //over all layers from outside to inside
        
      //decide on which steps (how far) to take
      std::vector < int > step;
      
      if ( layer >= 1 ) step.push_back (1); //that far we always want to go (except when we are at the end)
      if (( layer <= 4 )&&( layer >= 2 )) step.push_back( layer ); // allow jumping to 0 from layer 4 or lower (we don't add layer 1 because we allow jumpint 1 anyway. TODO: maybe this should better be a set than a vector?) 
      if ( layer >= 2) step.push_back ( 2 );
      
      //      std::cout << "\nhitsForward2[i].size()= " << hitsForward2[i].size() ;
      for (unsigned int iHit=0; iHit < hits[layer].size(); iHit++){ //over all hits in the layer
         
            
         for (unsigned int i=0; i < step.size(); i++) //over diverent ranges of step
                           
               for(unsigned int k=0; k< hits[layer-step[i] ].size(); k++){ //over all hits in the next inner layer
                     
                  const double* a =  hits[layer][iHit]->_trackerHits[0]->getPosition(); //the outer hit
                  const double* b =  hits[layer-step[i] ][k]->_trackerHits[0]->getPosition(); //the inner hit
                           
                           
                           
                  double ratio = sqrt( (a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]) ) / fabs ( a[2]-b[2] );
                           
                  //            std::cout << "\nratio=" << ratio ;
                           
                  if (ratio <= _ratioMax){ //ratio is good -> build segments. and store the links to them in the 1-segments
                              
                     std::vector <TrackerHit*> segHits;
                     segHits.push_back ( hits[layer][iHit]->_trackerHits[0] ); //the outer one
                     segHits.push_back ( hits[layer-step[i] ][k]->_trackerHits[0] ); //the inner one
                              
                     int layersSkipped = segHits[0]->getType() - segHits[1]->getType() -1; //number of skipped layers
                              
                              
                              //Create a new segment
                     Segment* newSeg= new Segment ( segHits );
                              
                     newSeg->_state.resize( layersSkipped +1);
                                 
                     segments2[layer-step[i] ].push_back( newSeg );  //Segments always belong to the layer of their innermost point
                              
                              //Store the pointer to the created 2-segments in these points, so we can easily connect the segments afterwards
                     hits[layer][iHit]->_children.push_back(  newSeg ); 
                     hits[layer-step [i] ][k]->_parents.push_back(  newSeg );
                     count++;
                  }
               
               }
            
      }
         
   }
      
      
      
   std::cout << "\n found 2-segments=" << count << "\n";
      
      
         
   //***********************************//
   //***********************************//
   //Connect all 2-segments, via searching all points for parents and children and connecting them.
      
   count= 0;
      
   for (unsigned int layer=0; layer<hits.size(); layer++){ //over all layers
         
      for (unsigned int hitnr=0; hitnr < hits[layer].size(); hitnr++){ //over all hits in layer
         
         for (unsigned int c=0; c <  hits[layer][hitnr]->_children.size() ; c++){ //connect all children
                  
            Segment* child = hits[layer][hitnr]->_children[c];         
         
            for (unsigned int p=0; p < hits[layer][hitnr]->_parents.size(); p++){ //to all parents
                        
               Segment* parent = hits[layer][hitnr]->_parents[p];
                     
               child->_parents.push_back( parent );
               parent->_children.push_back( child );
                     
               count++;
                     
            }
               
         }
            
      }
   }
      
   std::cout << "\n number of 2-segment-connections=" << count << "\n";  
   
   
   return segments2;
   
}


std::vector < std::vector <Segment* > > MyAutProcessor::getSegments3( std::vector < std::vector <Segment* > > segments2 ) {
   
   
   unsigned int count=0;
      
       
   std::vector < std::vector < Segment* > > segments3;
   segments3.resize( segments2.size() -1 ); //of course this has one layer less than the 2-segments had
      
   //find all the 3-segments
   for (int layer = segments2.size() - 1; layer > 0; layer--){ //over all layers (as we always need two layers, we won't need layer 0)
                  
      for (unsigned int iSeg = 0; iSeg < segments2[layer].size(); iSeg++){ //over all 2-segments
            
         Segment* seg_2 = segments2[layer][iSeg];
         unsigned int nChildren = seg_2->_children.size(); //we need this cause, we are going to add new children and thus change the length
            
         for (unsigned int iChild=0; iChild < nChildren; iChild++){ //combine this 2-segment with all children
               
            Segment* child = seg_2->_children[iChild];
               
            std::vector <TrackerHit*> segHits;
               
            segHits.push_back( seg_2->_trackerHits[0] ); //the outer hit of the outer 2-segment
            segHits.push_back( child->_trackerHits[0] ); //the outer hit of the inner 2-segment (= the inner hit of the outer 2-segment)
            segHits.push_back( child->_trackerHits[1] ); //the inner hit of the inner 2-segment
               
               //now we make a fresh new 3-segment:
            Segment* newSeg = new Segment( segHits );
      
            int newlayer = child->_trackerHits[1]->getType()-200; // the layer always euqals the layer of the innermost hit
            int layersSkipped = child->_trackerHits[0]->getType() - child->_trackerHits[1]->getType() - 1;
            newSeg->_state.resize( layersSkipped + 1 );
               
            segments3[newlayer].push_back( newSeg );  // add it to the vector<vector>
               
               
               //And of course save its presence 
            seg_2->_children.push_back( newSeg ); //We can stuff it in there, because later we simply check the number of
            child->_parents.push_back( newSeg );  // tracker hits the children have and thus find the 3-segments
               
            count++;
         }
            
      }
         
   }
   std::cout << "\n built 3-segments=" << count << "\n";
      
      
   count = 0;
      
      
      
      //link them together
      
   for ( unsigned int layer = 1; layer < segments2.size()-1; layer++){ //the outermost layer we can skip
         
      for ( unsigned int iSeg = 0; iSeg < segments2[layer].size(); iSeg++){ // over all 2-segments in the layer
            
         Segment* seg = segments2[layer][iSeg];
            
         for ( unsigned int iParent = 0; iParent < seg->_parents.size(); iParent++ ){
               
            Segment* parent = seg->_parents[iParent];
               
            if ( parent->_trackerHits.size() == 3 ){ //if it is a 3-segment
                  
               for ( unsigned int iChild = 0; iChild < seg->_children.size(); iChild++){
                     
                  Segment* child = seg->_children[iChild];
                     
                  if ( child->_trackerHits.size() == 3){ //now everything fits --> connect them
                        
                     parent->_children.push_back( child );
                     child->_parents.push_back( parent );
                        
                     count++;
                        
                  }
               }
            }
         }
      }
   }
   std::cout << "\n linked together 3-segments=" << count << "\n";         
      
       
      
   return segments3;
   
}



void MyAutProcessor::doAutomaton( std::vector < std::vector <Segment* > > segments ) {
   
   
   
   for( unsigned int i= 0; i < segments.size()-1 ; i++){ //more iterations don't make sense. After this it should be done
         
//          std::cout<< "\n\n";

         
      for( int layer=segments.size()-1; layer>-1 ; layer--){ //over all layers, but work from outside in. So we can raise the state on the go.
                                                                           //and of course don't check layer 0, there can't be any further 2-segments, right?
//             std::cout<< "\nlayer" << layer <<": ";
            
         for( unsigned int segnr=0; segnr< segments[layer].size(); segnr++){ //over all 2-segments in this layer
               
            Segment* parent= segments[layer][segnr];
               
      
            for ( int j= parent->_state.size()-1; j>0; j--) //Simulate skipped layers
                  
               if ( parent->_state[j] == parent->_state[j-1] )
                  parent->_state[j]++;
               
            for (unsigned int childnr=0; childnr< parent->_children.size(); childnr++){// over all children
                  
                  
                  
               Segment* child = parent->_children[childnr];
                  
               if ( child->_state[ child->_state.size()-1 ] == parent->_state[0] ){  //Only if they have the same state
                     
                  if (  areNeighbors( parent , child ) ){ //they are compatible
                        
                     parent->_state[0]++; //So it has a neighbor --> raise the state (at the bottom
                                       
                     break; //It has a neighbor, we raised the state, so we must not check again in this iteration 
                           
                        
                  }
                     
               }
                  
                  
            }
//                std::cout<< parent->_state[ parent->_state.size()-1 ];   
                           
               
         }
         
      }
      
   }
      
   
   
   
}



void MyAutProcessor::countTracks ( std::vector < std::vector <Segment* > > segments , unsigned int startLayer ){
   
   
   unsigned int count = 0;
      

      
   for (unsigned int layer=startLayer; layer < segments.size(); layer++){ //over all layers bigger than 3 (= 4 hits or more)
         
      for (unsigned int iSeg=0; iSeg< segments[layer].size(); iSeg++){ //over all segments in this layer
            
         if ( segments[layer][iSeg]->_parents.size() == 0 ){ //only start tracks from where there are no parents
               
               
            count += getNumberOfTracks( segments[layer][iSeg] );
                        
               
               
         }
      }
   }
      
   std::cout << "\n Track Candidates=" << count << "\n";
   
}



int MyAutProcessor::getNumberOfTracks ( Segment* segment ){
   
      
   int n=0;
   
   if ( segment->_children.size() == 0){ //No more children --> this is the beginning of a track
      
      return 1;
      
   }
   else{// there are still children below
      

      
      for (unsigned int i=0; i < segment->_children.size(); i++){ //for all children
         
         
         n += getNumberOfTracks ( segment->_children[i] );
         
      
      }
      
   }
   
   
   return n;
   
}




void MyAutProcessor::cleanSegments( std::vector < std::vector <Segment* > > & segments ){
   
   
   unsigned int count = 0;
      
   for( unsigned int layer=0; layer < segments.size(); layer++ ){//for every layer
            
      for( unsigned int iSeg=0; iSeg < segments[layer].size(); iSeg++){//over every segment
         
         Segment* seg = segments[layer][iSeg];            
            
         if( seg->_state[0] == (int) layer ){ //the state is alright, this segment is good
            
            count++;
               
         }
         else { //state is wrong, delete the segment
               
               //erase it from all its children
            for (unsigned int i=0; i < seg->_children.size(); i++){
                  
               seg->_children[i]->deleteParent ( seg );  
                  
            }
               
               //erase it from all its parents
            for (unsigned int i=0; i < seg->_parents.size(); i++){
                  
               seg->_parents[i]->deleteChild ( seg );
                  
            }
               
               //erase from vector
            segments[layer].erase( segments[layer].begin() + iSeg );
            iSeg--;
               
         }
               
         
      }
         
   }
      
      
   std::cout << "\n reamining segments=" << count << "\n";
      
      
      
            
   //***********************************//
   //***********************************//
   //erase all connections, that don't fit together
   for (unsigned int layer = 1; layer < segments.size(); layer++){ //over all layers
          
      for (unsigned int iSeg = 0; iSeg < segments[layer].size(); iSeg++){ // over all segments in the layer
            
         Segment* parent = segments[layer][iSeg];
            
         for (unsigned int iChild = 0; iChild < parent->_children.size(); iChild++){ //over all children of the segment
               
            Segment* child = parent->_children[iChild];
            if ( areNeighbors( parent , child ) == false ){ // if they don't fit together
                   
                  // erase their memories
               parent->deleteChild(child);
               child->deleteParent(parent);
                  
            }
               
         }
            
      }
         
   }
   
}


void MyAutProcessor::cleanSinguletts( std::vector < std::vector <Segment* > > & segments ){
   
   
   for( unsigned int layer=0; layer < segments.size(); layer++ ){//for every layer
            
      for( unsigned int iSeg=0; iSeg < segments[layer].size(); iSeg++){//over every segment
         
         Segment* seg = segments[layer][iSeg];            
            
          
            if ( ( seg->_children.size() == 0) && (seg->_parents.size() == 0) ){ //state is wrong, delete the segment
               
            
               segments[layer].erase( segments[layer].begin() + iSeg );
               
               iSeg--;
               
            }
               
         
      }
         
   }
   
   
   
   
   
}



bool MyAutProcessor::areNeighbors ( Segment* parent , Segment* child ){
   
   
   if (( parent->_trackerHits.size() == 2 )&&( child->_trackerHits.size() == 2 ))
      return areNeighbors_2 (parent , child);
   
   if (( parent->_trackerHits.size() == 3 )&&( child->_trackerHits.size() == 3 ))
      return areNeighbors_3 (parent , child); 
   
   
   return false;
   
}


bool MyAutProcessor::areNeighbors_2( Segment* parent , Segment* child){
   

   
   //check the angle
   
   const double* a = parent->_trackerHits[0]->getPosition();
   const double* b = parent->_trackerHits[1]->getPosition();
   const double* c = child->_trackerHits[1]->getPosition();
      
   double zaehler=0.;
   double uBetrag=0.;
   double vBetrag=0.;
   
   for (int i=0; i <= 2; i++){
      
      double u = b[i] - a[i];
      double v = c[i] - b[i];
      
      zaehler+= u*v;
      uBetrag +=  u*u;
      vBetrag += v*v;
      
   }
   
   double nenner = sqrt ( uBetrag * vBetrag );
   
   if (nenner > 0){
   
      double cosTheta = fabs(zaehler / nenner );
      if (cosTheta < 0.9962) return false; // = 5degree, TODO: calculate this at another point!  
   
   }
   
   
   
   //check the distance of a circle defined by the two 2-segments to the origin in xy
   
   SimpleCircle circle ( a[0] , a[1] , b[0] , b[1] , c[0] , c[1] );
   
   double x = circle.getCenterX();
   double y = circle.getCenterY();
   double R = circle.getRadius();
   
   double distTo0 = fabs ( R - sqrt ( x*x + y*y ) );
   
   if ( distTo0 > 10. ) return false;
   
   return true;
   
   
}


 
bool MyAutProcessor::areNeighbors_3( Segment* parent , Segment* child){
  
 
   TVector3 parentOuter  ( parent->_trackerHits[0]->getPosition() ); // Outer hit of first 3-segment
   TVector3 parentMiddle ( parent->_trackerHits[1]->getPosition() ); // Middle hit of first 3-segment
   TVector3 parentInner  ( parent->_trackerHits[2]->getPosition() ); // Inner hit of first 3-segment
   TVector3 childOuter   ( child->_trackerHits[0]->getPosition() ); 
   TVector3 childMiddle  ( child->_trackerHits[1]->getPosition() ); 
   TVector3 childInner   ( child->_trackerHits[2]->getPosition() );
        
   TVector3 outerVec  = parentOuter - parentMiddle;
   TVector3 middleVec = parentMiddle - parentInner;
   TVector3 innerVec  = childMiddle - childInner;
  
   /*****************************************************/
  //forbid zigzagging
  
  
     
   double angleXY1 = outerVec.Phi()-middleVec.Phi(); //the angle between 2-segments in the xy plane
   double angleXY2 = middleVec.Phi()-innerVec.Phi();
   
   angleXY1 -= 2*M_PI*floor( angleXY1 /2. /M_PI );    //to the range from 0 to 2pi 
   if (angleXY1 > M_PI) angleXY1 -= 2*M_PI;           //to the range from -pi to pi

   angleXY2 -= 2*M_PI*floor( angleXY2 /2. /M_PI );    //to the range from 0 to 2pi 
   if (angleXY2 > M_PI) angleXY2 -= 2*M_PI;           //to the range from -pi to pi
  
   if ( angleXY1 *180./M_PI * angleXY2 *180./M_PI < -1.)  // if the angles don't change signs or are around 0. So if they are + and -1°
      return false;                                       // that's still okay +1*(-1)= -1 >= -1. 
                                                          // if they don't change sign the product is positive anyway 

   /*****************************************************/
  
  
   /*****************************************************/
  //check change in xyAngle
  
  
   if ( fabs(angleXY2) > 2.5 * fabs( angleXY1) ) return false;
   if ( fabs(angleXY2) < 0.4 * fabs( angleXY1) ) return false; 
   
  
   /*****************************************************/
  
   
   
   /*****************************************************/
  //check change in radius
  
     
   SimpleCircle circleParent( parentOuter.X() , parentOuter.Y() , parentMiddle.X() , parentMiddle.Y() , parentInner.X() , parentInner.Y() );
   SimpleCircle circleChild( childOuter.X() , childOuter.Y() , childMiddle.X() , childMiddle.Y() , childInner.X() , childInner.Y() );

   if ( circleParent.getRadius() > 2.* circleChild.getRadius() ) return false;
   if ( circleChild.getRadius() > 2.* circleParent.getRadius() ) return false;
   
  
   /*****************************************************/
  
   
   
   /*****************************************************/
  //check deltaPhi/z
   
  
   TVector3 u ( parentMiddle.X() - circleParent.getCenterX(), parentMiddle.Y() - circleParent.getCenterY() , 0.); //vector from center of circle to point
   TVector3 v ( parentInner.X() - circleParent.getCenterX(), parentInner.Y() - circleParent.getCenterY() , 0.);
               
   double zDist = fabs( parentMiddle.Z() - parentInner.Z() );
   double parentPhiOverDeltaZ = u.Angle(v) / zDist;
  
  
   TVector3 s ( childMiddle.X() - circleChild.getCenterX(), childMiddle.Y() - circleChild.getCenterY() , 0.); //vector from center of circle to point
   TVector3 t ( childInner.X() - circleChild.getCenterX(), childInner.Y() - circleChild.getCenterY() , 0.);
               
   zDist = fabs( childMiddle.Z() - childInner.Z() );
   double childPhiOverDeltaZ = s.Angle(t) / zDist;
  
  
   if ( childPhiOverDeltaZ > 2. * parentPhiOverDeltaZ ) return false;
   if ( parentPhiOverDeltaZ > 2. * childPhiOverDeltaZ ) return false;
  
   
   /*****************************************************/
   
   
   /*****************************************************/
   //check distance in xy from prediction
   
   SimpleCircle circle ( parentOuter.X() , parentOuter.Y() , parentMiddle.X() , parentMiddle.Y() , parentInner.X() , parentInner.Y() );
               
   double centerX = circle.getCenterX();
   double centerY = circle.getCenterY();
   double R = circle.getRadius();
               
   TVector3 a ( parentMiddle.X() - centerX , parentMiddle.Y() - centerY , parentMiddle.Z() );
   TVector3 b ( parentInner.X() - centerX , parentInner.Y() - centerY , parentInner.Z() );
               
                  
   double deltaPhiParent = b.Phi() - a.Phi(); //angle in xy plane from center of circle, between point 2 and 3
                  
   // use this angle and the distance to the next layer to extrapolate
   double zDistParent = fabs( parentMiddle.Z() - parentInner.Z() );
   double zDistChild  = fabs( parentInner.Z() - childInner.Z() );
                  
   double deltaPhiChild = deltaPhiParent * zDistChild / zDistParent ;
                  
   double phiChild = b.Phi() + deltaPhiChild;
                  
   double xChildPred = centerX + R* cos(phiChild);
   double yChildPred = centerY + R* sin(phiChild);
               
   double xChild = childInner.X();
   double yChild = childInner.Y();
               
   double distToPrediction = sqrt ( ( xChildPred- xChild )*( xChildPred- xChild ) + ( yChildPred- yChild )*( yChildPred- yChild ) );
   double distNormed = distToPrediction / zDistChild;   
   
   if ( distNormed > 0.02 ) return false;
   
   
   /*****************************************************/
 
  
  
   return true;
  
}
           
               
std::vector <Track*> MyAutProcessor::getTrackCandidates ( Segment* segment, std::vector< TrackerHit*> hits ){

   std::vector <Track*> tracks;

   std::vector <TrackerHit*> newHits= hits;
   newHits.push_back ( segment->_trackerHits[0] ); //add the outer hit


   if ( segment->_children.size() == 0){ //No more children --> start a new Track here

      TrackImpl* newTrack = new TrackImpl(); //make a new track

      for ( unsigned int i = 1 ; i < segment->_trackerHits.size()-1; i++)
         newHits.push_back ( segment->_trackerHits[i] ); //add all the hits at the end (except the innermost one: this is our virtual hit)
      
      for (unsigned int i=0; i< newHits.size(); i++)  //add the other hits
         newTrack->addHit( newHits[i] ); 

      tracks.push_back ( newTrack );

   }
   else{// there are still children below



      for (unsigned int i=0; i < segment->_children.size(); i++){ //for all children

         std::vector <Track*> newTracks = getTrackCandidates( segment->_children[i] , newHits );

         for (unsigned int j=0; j < newTracks.size(); j++)
            tracks.push_back ( newTracks[j] );

      }

   }


   return tracks;

}


void MyAutProcessor::drawSegmentsInCed( std::vector < std::vector <Segment* > >  segments ){
   
     
   
   for( unsigned int layer=0 ; layer < segments.size(); layer++ ){ //over all layers
      
      for( unsigned int iSeg=0; iSeg < segments[layer].size(); iSeg++ ){ //over all segments in the layer
         
         Segment* segment = segments[layer][iSeg];
         
         
         if (segment->_trackerHits.size() == 1){ //exactly one hit, so draw a point
         
                  
            const double* a = segment->_trackerHits[0]->getPosition();
            ced_hit( a[0] ,a[1] , a[2] , 0 , 3 ,0xff0000 );

            
         }
         else //more than one point or no points
            for( unsigned int i=1 ; i< segment->_trackerHits.size() ; i++ ){ // over all hits in the segment (as we connect it with the previous we start with hit 1)
      
               const double* a = segment->_trackerHits[i]->getPosition();
               const double* b = segment->_trackerHits[i-1]->getPosition();
               
               
               unsigned int color=0;
               unsigned int red=0;
               unsigned int blue=0;
               unsigned int green=0;
               
               float p =  sqrt ((float)  segment->_state[0] / (float) ( segments.size()) );
               
               green = ceil ( (1.-p) * 255 );
               red = floor( 255*p );
               blue = ceil ( (1.-p) * 255 );
               
               color = red * 256*256 + green * 256 + blue;
      
               ced_line_ID( a[0], a[1], a[2], b[0], b[1], b[2] , 2 , segment->_state[0]+1 , color, 0);
               
            }
      }
   }
   
   
   
}


void MyAutProcessor::drawTracksInCed ( std::vector<Track*> tracks ){
   
   
   
   for ( unsigned iTrack = 0; iTrack < tracks.size(); iTrack++){ //over all tracks

      for ( unsigned iHit = 0; iHit < tracks[iTrack]->getTrackerHits().size() -1 ; iHit++ ){
         
                  
         const double* a = tracks[iTrack]->getTrackerHits()[iHit]->getPosition();
         const double* b = tracks[iTrack]->getTrackerHits()[iHit+1]->getPosition();

         ced_line_ID( a[0], a[1], a[2], b[0], b[1], b[2] , 2 , 2, 0x00ff00, 0);
         
      }
      
   }

   
   
}
   
