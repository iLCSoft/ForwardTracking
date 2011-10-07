#include "Crit4_distOfCircleCenters.h"

#include <cmath>
#include "SimpleCircle.h"

// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"

using namespace FTrack;

Crit4_distOfCircleCenters::Crit4_distOfCircleCenters ( double distMax ){
   
   
   _distMax = distMax;
   
   
   
}



bool Crit4_distOfCircleCenters::areCompatible( Segment* parent , Segment* child ){
    
   
   
   if (( parent->getAutHits().size() == 3 )&&( child->getAutHits().size() == 3 )){ //this is a criterion for 3-segments
      


      AutHit* a = parent->getAutHits()[0];
      AutHit* b = parent->getAutHits()[1];
      AutHit* c = parent->getAutHits()[2];
      AutHit* d = child-> getAutHits()[2];

      
      
      float ax = a->getX();
      float ay = a->getY();
//       float az = a->getZ();
      
      float bx = b->getX();
      float by = b->getY();
//       float bz = b->getZ();
      
      float cx = c->getX();
      float cy = c->getY();
//       float cz = c->getZ();
      
      float dx = d->getX();
      float dy = d->getY();
//       float dz = d->getZ();
      
/*      
      streamlog_out( DEBUG4 ) << "\nax " << ax;
      streamlog_out( DEBUG4 ) << "\nay " << ay;
      streamlog_out( DEBUG4 ) << "\naz " << az;
      streamlog_out( DEBUG4 ) << "\nbx " << bx;
      streamlog_out( DEBUG4 ) << "\nby " << by;
      streamlog_out( DEBUG4 ) << "\nbz " << bz;
      streamlog_out( DEBUG4 ) << "\ncx " << cx;
      streamlog_out( DEBUG4 ) << "\ncy " << cy;
      streamlog_out( DEBUG4 ) << "\ncz " << cz;
      streamlog_out( DEBUG4 ) << "\ndx " << dx;
      streamlog_out( DEBUG4 ) << "\ndy " << dy;
      streamlog_out( DEBUG4 ) << "\ndz " << dz;*/

      
      
      try{
      
         SimpleCircle circle1 ( ax , ay , bx , by , cx , cy );
         SimpleCircle circle2 ( bx , by , cx , cy , dx , dy );

         
         float X1 = circle1.getCenterX();
         float Y1 = circle1.getCenterY();

         float X2 = circle2.getCenterX();
         float Y2 = circle2.getCenterY();
         
         
         float distOfCircleCenters = sqrt( (X2-X1)*(X2-X1) + (Y2-Y1)*(Y2-Y1) );
         
         _map_name_value["distOfCircleCenters"] = distOfCircleCenters;
         
//          streamlog_out( DEBUG4 ) << "\n" << distOfCircleCenters;
//          streamlog_out( DEBUG4 ) << "\nx1 " << X1;
//          streamlog_out( DEBUG4 ) << "\ny1 " << Y1;
//          streamlog_out( DEBUG4 ) << "\nx2 " << X2;
//          streamlog_out( DEBUG4 ) << "\ny2 " << Y2;

         
         if ( distOfCircleCenters > _distMax ) return false;
      
      }
      catch ( InvalidParameter ){
         
         _map_name_value["distOfCircleCenters"] = -1.;
         
         
      }
      
      
   }
   
   
   return true;
   
   
   
}

