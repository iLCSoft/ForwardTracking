#ifndef IHit_h
#define IHit_h

#include "ISectorSystem.h"

namespace FTrack{
   
   
   /** An Interface for hits as used in the FTrack package.
    * 
    * 
    */   
   class IHit{
      
      
   public:
      
      float getX() const { return _x; }
      float getY() const { return _y; }
      float getZ() const { return _z; }
      
      int getSector() const { return _sector; }
      
      virtual const ISectorSystem* getSectorSystem() const =0;
      unsigned getLayer() const {return getSectorSystem()->getLayer( _sector ); }
      
      bool isVirtual() const { return _isVirtual; };
      void setIsVirtual( bool isVirtual ){ _isVirtual = isVirtual; };
      
   protected:
      
            
      float _x;
      float _y;
      float _z;
      
      int _sector;
      
      
      bool _isVirtual;
      
      
   };
   
   
   
}


#endif


