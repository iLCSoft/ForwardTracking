#include "Crit4_RChange.h"

#include <cmath>
#include "SimpleCircle.h"



using namespace FTrack;

Crit4_RChange::Crit4_RChange ( double changeMax ){
   
   
   _changeMax = changeMax;
   
   
   
}



bool Crit4_RChange::areCompatible( Segment* parent , Segment* child ){
    
   
   
   if (( parent->getAutHits().size() == 3 )&&( child->getAutHits().size() == 3 )){ //this is a criterion for 3-segments
      


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = parent->getAutHits()[2];
      AutHit* d = child-> getAutHits()[2];
      
      float ax = a->getX();
      float ay = a->getY();
      
      float bx = b->getX();
      float by = b->getY();
       
      float cx = c->getX();
      float cy = c->getY();
      
      float dx = d->getX();
      float dy = d->getY();
      
      try{
      
         SimpleCircle circle1 ( ax , ay , bx , by , cx , cy );
         SimpleCircle circle2 ( bx , by , cx , cy , dx , dy );
         
         float R1 = circle1.getRadius();
         float R2 = circle2.getRadius();
         
         float ratioOfR = R1/R2;
         
         _map_name_value["ratioOfR"] = ratioOfR;
         
         
            
         if ( ratioOfR > _changeMax ) return false;    
         if ( 1./ratioOfR > _changeMax ) return false;
         
      }
      catch ( InvalidParameter ){
         
         _map_name_value["ratioOfR"] = 0.;
         
      }
      
   }
   
   
   return true;
   
   
   
}

