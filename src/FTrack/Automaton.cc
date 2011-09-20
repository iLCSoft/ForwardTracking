#include "Automaton.h"

#include "marlin/VerbosityLevels.h"

#include "IMPL/TrackImpl.h"

#include <MarlinCED.h>
#include "UTIL/ILDConf.h"

using namespace lcio;

using namespace FTrack;


void Automaton::addSegment ( Segment* segment ){


   if ( segment->getLayer() >= _segments.size() ) { //in case this layer is not included so far

      _segments.resize( segment->getLayer() + 1 ); //resize the vector, so that the layer of the segment is now included

   }

   _segments[ segment->getLayer() ].push_back ( segment );





}

void Automaton::lengthenSegments(){

   // Info A: On skipped layers
   // ^^^^^^^^^^^^^^^^^^^^^^^^^^
   //
   // (read this only if you are interested on how the number of skipped layers is determined)
   //
   // The skipped layers are always between the innermost two hits of a segment.
   // Why? Because the connection between those two hits is the thing that differs
   // from its parent. Let's assume two 5-segments like those:
   //           /                                                               //
   //           \\                                                              //
   //           //                                                              //
   //           \\      _layer 2                                                //
   //            /      _layer 1                                                //
   // You see how they overlap with all of their hits except the inner one of the child
   // and the outer one of the parent. As the layer they are on equals the layer of the
   // innermost point, the child will have layer 1 and the parent layer 2.
   //
   // Now suppose the child skips layer 1: ( in this example a kink in the segment means the layer is hit,
   // no kink means it is left out.)
   //
   //           /                                                               //
   //           \\                                                              //
   //           //                                                              //
   //           \\      _layer 2                                                //
   //            /      _layer 1                                                //
   //           /       _layer 0                                                //
   //
   // That means the child has now layer 0 and the parent layer 2.
   // This is no problem, the segment class has therefore an outer and an inner state (simulating skipped layers)
   // instead of just an int. (to be more precise it has a vector containing for the inner state and every layer left out)
   // Now when we want to make 6-segments (I know the numbers are high, but they help visualising), we would connect
   // parent and child to a new segment.
   //
   //
   //           /                                                               //
   //           \                                                               //
   //           /                                                               //
   //           \      _layer 2                                                 //
   //           /      _layer 1                                                 //
   //          /       _layer 0                                                 //
   //
   // So how many layers does this track skip: again 1 layer, so we need a state vector with 2 elements.
   // (one for layer 0, one for layer 1)
   //
   // So we don't care if there are any other skipped layers in the outer part of the segment, we only care about that
   // bit that won't overlap with parents.
   // So the easy recipe for the number of skipped layers after making a segment longer is:
   //   Compare the layer before ( 2 ) to the layer after ( 0 ). The skipped layers are the difference -1
   //   ( 2 - 0 - 1 = 1 --> segment->setSkippedLayers( 1 );




   //----------------------------------------------------------------------------------------------//
   //                                                                                              //
   // first: we create a new vector[][] to have somewhere we can put the longer segments           //
   //                                                                                              //
   //----------------------------------------------------------------------------------------------//

   std::vector < std::vector < Segment* > > longerSegments;
   longerSegments.resize ( _segments.size() -1 ); //This will have one layer less

   //----------------------------------------------------------------------------------------------//
   //                                                                                              //
   // next: find all the longer segments and store them in the new vector[][]                      //
   //                                                                                              //
   //----------------------------------------------------------------------------------------------//

   unsigned nLongerSegments=0;
   unsigned nShorterSegments= _segments[0].size();

   for (unsigned layer = 1; layer < _segments.size(); layer++){ //over all layer where there still can be something below

      for ( unsigned iSeg=0; iSeg < _segments[ layer ].size(); iSeg++){ //over all segments in this layer

         nShorterSegments++;

         Segment* segment = _segments[layer][iSeg];

         std::vector <Segment*> children = segment->getChildren();

         for ( unsigned iChild=0; iChild < children.size(); iChild++){ //over all children of this segment

            Segment* child = children[ iChild ];

            //Combine the segment and the child to form a new longer segment

            //take all the hits from the segment
            std::vector < AutHit* > autHits = segment->getAutHits();

            //and also add the last hit from the child
            autHits.push_back( child->getAutHits().back() );

            //make the new (longer) segment
            Segment* newSegment = new Segment ( autHits );
            nLongerSegments++;

            //set the layer to the layer of the childsegment. TODO: explain why we take the layer of the child.
            unsigned newLayer = child->getLayer();
            newSegment->setLayer ( newLayer );

            // Set the skipped layers.                  For an explanation see Info A above
            int skippedLayers = segment->getLayer() - child->getLayer() - 1;
            newSegment->setSkippedLayers( skippedLayers );      //

            streamlog_out( DEBUG1 ) << "\n Created longer segment: " << segment->getAutHits().size()
                                    << "-->" << newSegment->getAutHits().size()
                                    << " hits, layer = " << newLayer
                                    << ", skipped layers = " << skippedLayers;


            //Erase the connection from the child to the parent segment and replace it with a link to the new
            //(longer) segment. ( and vice versa ) This way we can connect the longer segments easily after.
            child->deleteParent( segment );
            child->addParent ( newSegment );
            segment->deleteChild ( child );
            segment->addChild ( newSegment );
            //So now the new longer segment is a child of the old segment and a parent of the childsegment.
            //TODO: really explain this better

            //Save the new segment in the new vector[][]
            longerSegments[newLayer].push_back( newSegment );

         }

      }

   }

   streamlog_out(DEBUG4) << "\n Made " << nLongerSegments << " longer segments from " << nShorterSegments << " shorter segments.\n";


   //----------------------------------------------------------------------------------------------//
   //                                                                                              //
   // Connect the new (longer) segments                                                            //
   //                                                                                              //
   //----------------------------------------------------------------------------------------------//

   unsigned nConnections=0;

   for ( unsigned layer = 1; layer < _segments.size()-1; layer++ ){ // over all layers (of course the first and the last ones are spared out because there is nothing more above or below

      for ( unsigned iSeg = 0; iSeg < _segments[layer].size(); iSeg++ ){ //over all (short) segments in this layer


         Segment* segment = _segments[layer][iSeg];

         for ( unsigned iParent=0; iParent < segment->getParents().size(); iParent++ ){ // over all parents of the segment


            Segment* parent = segment->getParents()[iParent];

            for ( unsigned iChild=0; iChild < segment->getChildren().size(); iChild++ ){ // over all children of the segment


               Segment* child = segment->getChildren()[iChild];

               //connect parent and child (i.e. connect the longer segments we previously created)
               child->addParent( parent );
               parent->addChild( child );

               nConnections++;

            }

         }

      }

   }

   streamlog_out (DEBUG4) << "\n Made Connections of Segments: " << nConnections << "\n";


   //----------------------------------------------------------------------------------------------//
   //                                                                                              //
   //   Finally: replace the vector[][] of the old segments with the new one                       //
   //                                                                                              //
   //----------------------------------------------------------------------------------------------//

   _segments = longerSegments;

}


void Automaton::doAutomaton(){


   bool hasChanged = true;
   int nIterations = -1;

   while ( hasChanged == true ){ //repeat this until no more changes happen (this should always be equal or smaller to the number of layers - 1


      hasChanged = false;
      nIterations++;

      for ( int layer = _segments.size()-1; layer >= 0; layer--){ //for all layers from outside in

         for ( unsigned iSeg = 0; iSeg < _segments[layer].size(); iSeg++ ){ //for all segments in the layer


            Segment* parent= _segments[layer][iSeg];



            //Simulate skipped layers
            std::vector < int > state = parent->getState();

            for ( int j= state.size()-1; j>=1; j--){

               if ( state[j] == state[j-1] ){

                  state[j]++;
                  hasChanged = true; //something changed

               }
            }

            parent->setState( state );



            //Check if there is a neighbor
            std::vector <Segment*> children = parent->getChildren();
            for (unsigned iChild=0; iChild < children.size(); iChild++ ){// over all children



               Segment* child = children[iChild];

               if ( child->getOuterState() == parent->getInnerState() ){  //Only if they have the same state


                  bool areCompatible = true;

                  //check all criteria (or at least until one returns false
                  for ( unsigned iCrit = 0; iCrit < _criteria.size(); iCrit++ ){

                     if ( _criteria[iCrit]->areCompatible ( parent , child ) == false ){

                        areCompatible = false;
                        break;
                     }

                  }


                  if (  areCompatible ){ //they are compatible

                     parent->raiseState(); //So it has a neighbor --> raise the state

                     hasChanged = true; //something changed

                     break; //It has a neighbor, we raised the state, so we must not check again in this iteration


                  }

               }

            }




         }

      }

   }


   streamlog_out(DEBUG4) << "\n Automaton performed using " << nIterations << " iterations.\n";



}




void Automaton::cleanBadStates(){



   unsigned nErasedSegments = 0;
   unsigned nKeptSegments = 0;

   for( unsigned layer=0; layer < _segments.size(); layer++ ){//for every layer

      for( unsigned iSeg=0; iSeg < _segments[layer].size(); iSeg++){//over every segment


            Segment* segment = _segments[layer][iSeg];

            if( segment->getInnerState() == (int) layer ){ //the state is alright (equals the layer), this segment is good

            nKeptSegments++;

         }
         else { //state is wrong, delete the segment


            nErasedSegments++;

            //erase it from all its children
            std::vector <Segment*> children = segment->getChildren();

            for (unsigned int i=0; i < children.size(); i++){

               children[i]->deleteParent ( segment );

            }

            //erase it from all its parents
            std::vector <Segment*> parents = segment->getParents();

            for (unsigned int i=0; i < parents.size(); i++){

               parents[i]->deleteChild ( segment );

            }

            //erase from the automaton
            _segments[layer].erase( _segments[layer].begin() + iSeg );

            //set iSeg back by one. Why: because we changed the vector: if we checked entry number 4 and now deleted it,
            //the former entry number 5 will now be on place number 4. So if we just continue (as usual) with iSeg = 5,
            //we skip this one! That's why iSeg needs to be decremented.
            iSeg--;

         }

      }

   }


   streamlog_out( DEBUG4 ) << "\n Erased segments because of bad states= " << nErasedSegments << "\n";
   streamlog_out( DEBUG4 ) << "\n Kept segments because of good states= " << nKeptSegments << "\n";





}


void Automaton::resetStates(){


   for ( unsigned layer = 0; layer < _segments.size(); layer++ ){ //over all layers

      for (unsigned iSeg = 0; iSeg < _segments[layer].size(); iSeg++ ){ //over all segments in the layer

         _segments[layer][iSeg]->resetState();

      }

   }



}


void Automaton::cleanBadConnections(){


   unsigned nConnectionsKept = 0;
   unsigned nConnectionsErased = 0;


   for ( int layer = _segments.size()-1 ; layer >= 1 ; layer-- ){ //over all layers from outside in. And there's no need to check layer 0, as it has no children.

      for ( unsigned iSeg=0; iSeg < _segments[layer].size(); iSeg++ ){ // over all segments in the layer

         Segment* parent = _segments[layer][iSeg];
         std::vector < Segment* > children = parent->getChildren();

         for ( unsigned iChild=0; iChild < children.size(); iChild++ ){ //over all children the segment has got


            Segment* child = children[iChild];

            bool areCompatible = true; //whether segment and child are compatible

            //check all criteria (or at least until the first false pops up)
            for ( unsigned iCrit=0; iCrit < _criteria.size() ; iCrit++ ){


               if ( _criteria[iCrit]->areCompatible( parent , child ) == false ){


                  areCompatible = false;
                  break; //no need to continue, now that we know, they're not compatible

               }

            }


            if ( areCompatible == false ){ // they are not compatible --> erase the connection

               nConnectionsErased++;

               //erase the connection:
               parent->deleteChild ( child );
               child->deleteParent ( parent );

               //A small note here: although we deleted a child from the vector, this doesn't mean we have to do iSeg--!
               //Because we copied the value of segment->getChildren to the vector children. And this one doesn't change!



            }
            else{

               nConnectionsKept++;

            }

         }

      }

   }


   streamlog_out( DEBUG4 ) << "\n Erased bad connections= " << nConnectionsErased << "\n";
   streamlog_out( DEBUG4 ) << "\n Kept good connections= " << nConnectionsKept << "\n";



}



std::vector <Track*> Automaton::getTracksOfSegment ( Segment* segment, const std::vector< TrackerHit*> hits , unsigned minHits ){


   std::vector <Track*> tracks; //the vector of the tracks to be returned

   std::vector <AutHit*> autHits = segment->getAutHits(); // the autHits of the segment

   std::vector <TrackerHit*> newHits= hits;

   //add the outer hit
   if ( autHits[0]->isVirtual() == false ) newHits.push_back ( autHits[0]->getTrackerHit() );  //Of course add only real hits to the track


   std::vector <Segment*> children = segment->getChildren();

   if ( children.size() == 0){ //No more children --> we are at the bottom --> start a new Track here

      //add the rest of the hits to the vector
      for ( unsigned int i = 1 ; i < autHits.size(); i++){

         if ( autHits[i]->isVirtual() == false ) newHits.push_back ( autHits[i]->getTrackerHit() );

      }

      
      if ( newHits.size() >= minHits ){

         //make a new track
         TrackImpl* newTrack = new TrackImpl();


         //Store all the hits in the track
         for (unsigned int i=0; i< newHits.size(); i++){

            newTrack->addHit( newHits[i] );

         }

         tracks.push_back ( newTrack );

      }


   }
   else{// there are still children below --> so just take all their tracks and add the current segments outer hit



      for (unsigned int i=0; i < children.size(); i++){ //for all children


         std::vector <Track*> newTracks = getTracksOfSegment( children[i] , newHits );

         for (unsigned int j=0; j < newTracks.size(); j++){//for all the tracks of the child


            tracks.push_back ( newTracks[j] );

         }

      }

   }


   return tracks;



}


std::vector <Track*> Automaton::getTracks( unsigned minHits ){



   std::vector <Track*> tracks;

   std::vector <TrackerHit*> emptyHitVec;


   for ( unsigned layer = 0 ; layer < _segments.size() ; layer++ ){ //over all layers

      for ( unsigned iSeg = 0; iSeg < _segments[layer].size() ; iSeg++ ){ //over all segments


         if ( _segments[layer][iSeg]->getParents().size() == 0 ){ // if it has no parents it is the end of a possible track


            // get the tracks from the segment
            std::vector <Track*> newTracks = getTracksOfSegment( _segments[layer][iSeg] , emptyHitVec , minHits );

            // and add them to the vector of all tracks
            tracks.insert( tracks.end() , newTracks.begin() , newTracks.end() );


         }

      }

   }



   streamlog_out( DEBUG4 ) << "\n Created " << tracks.size() << " tracks.\n";

   return tracks;

}


std::vector <Segment*> Automaton::getSegments(){
   
   
   
   std::vector <Segment*> segments;
   
   for( unsigned layer=0; layer < _segments.size(); layer++ ){
      
      
      segments.insert( segments.end() , _segments[layer].begin() , _segments[layer].end() );
      
   }
   
   
   
   return segments; 
         
         
   
   
}


void Automaton::drawSegments(){
   
   
   
   for( unsigned int layer=0 ; layer < _segments.size(); layer++ ){ //over all layers
      


      for( unsigned int iSeg=0; iSeg < _segments[layer].size(); iSeg++ ){ //over all segments in the layer
         
         Segment* segment = _segments[layer][iSeg];
         std::vector <AutHit*> autHits = segment->getAutHits();         
         
         if ( autHits.size() == 1){ //exactly one hit, so draw a point
         
                  
            AutHit* a = autHits[0];
            ced_hit( a->getX() ,a->getY() , a->getZ() , 0 , 3 ,0xff0000 );
            
            
         }
         else //more than one point or no points
            for( unsigned i=1 ; i< autHits.size() ; i++ ){ // over all hits in the segment (as we connect it with the previous we start with hit 1)
      
               AutHit* a = autHits[i];
               AutHit* b = autHits[i-1];
               
               
               unsigned int color=0;
               unsigned int red=0;
               unsigned int blue=0;
               unsigned int green=0;
               
               float p =  sqrt ((float)  segment->getInnerState() / (float) ( _segments.size()) );
               
               green = ceil ( (1.-p) * 255 );
               red = floor( 255*p );
               blue = ceil ( (1.-p) * 255 );
               
               color = red * 256*256 + green * 256 + blue;
               
               ced_line_ID( a->getX() ,a->getY() , a->getZ() , b->getX() ,b->getY() , b->getZ() , 2 , segment->getInnerState()+1 , color, 0);
               
            }
            
      }
      
   }
   

   
   
   
   
}








