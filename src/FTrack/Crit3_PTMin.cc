#include "Crit3_PTMin.h"

#include "SimpleCircle.h"
#include <cmath>

using namespace FTrack;


Crit3_PTMin::Crit3_PTMin( double ptMin , double Bz){
   
   _ptMin  = ptMin;
   _Bz = Bz;
   
   _saveValues = false;
   
}



bool Crit3_PTMin::areCompatible( Segment* parent , Segment* child )throw( BadSegmentLength ){
   
   
   
   if (( parent->getAutHits().size() == 2 )&&( child->getAutHits().size() == 2 )){ //a criterion for 2-segments


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = child-> getAutHits()[1];
      
      float ax = a->getX();
      float ay = a->getY();
     
      float bx = b->getX();
      float by = b->getY();
      
      float cx = c->getX();
      float cy = c->getY();


      try{
      
         SimpleCircle circle ( ax , ay , bx , by , cx , cy );
      

         double R = circle.getRadius();
         
         
         // check if pt is bigger than _ptMin
         //
         // |omega| = K*Bz/pt
         // R = pt / (K*Bz)
         // pt = R * K *Bz
         //
               
         const double K= 0.00029979; //K depends on the used units
         
         double pt = R * K * _Bz;
            
         if (_saveValues) _map_name_value["PtMin_pt"] =  pt;
               
         if ( pt < _ptMin ) return false;

         
      }
      catch ( InvalidParameter ){
         
         if (_saveValues) _map_name_value["PtMin_pt"] =  0.;
         
      }



   }
   else{
      
      std::string s = "Crit3_PTMin::This criterion needs 2 segments with 2 hits each, passed was a "
      +  intToString( parent->getAutHits().size() ) + " hit segment (parent) and a "
      +  intToString( child->getAutHits().size() ) + " hit segment (child).";
      
      
      throw BadSegmentLength( s );
      
      
   }
   
   
   return true;
   
   
}
