#ifndef AutHit_h
#define AutHit_h

#include "EVENT/TrackerHit.h"
#include "lcio.h"

using namespace lcio;

namespace FTrack{
   
   
   /** a class representing a hit as used by the cellular automaton
    */   
   class AutHit{
      
      
   public:
      
      AutHit( TrackerHit* trackerHit );
      
      TrackerHit* getTrackerHit(){ return _trackerHit; };
      
      float getX() { return _x; };
      float getY() { return _y; };
      float getZ() { return _z; };
      
      int getSide() { return _side; };
      unsigned getLayer() { return _layer; };
      unsigned getModule() { return _module; };
      unsigned getSensor() { return _sensor; };
      
      void setSide( int side ){ _side = side; };
      void setLayer( unsigned layer ){ _layer = layer; };
      void setModule( unsigned module ){ _module = module; };
      void setSensor( unsigned sensor ){ _layer = sensor; };
      
      void setIsVirtual( bool isVirtual ){ _isVirtual = isVirtual; };
      bool isVirtual(){ return _isVirtual; };
      
      
   private:
      
      TrackerHit* _trackerHit;
      
      float _x;
      float _y;
      float _z;
      
      int _side;
      unsigned _layer;
      unsigned _module;
      unsigned _sensor;
      
      bool _isVirtual;
      
      
   };
   
   
   
}


#endif

