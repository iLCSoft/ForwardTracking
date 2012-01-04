#ifndef IHit_h
#define IHit_h

#include "ISectorSystem.h"

namespace FTrack{
   
   
   /** An Interface for hits.
    * 
    * They have of course a position, accessible with getX(), getY() and getZ().
    * 
    * Also a hit has a sector. A sector is just an integer, telling where the hit lies. For example
    * a sector 141 could mean 1st layer, 4th module, 1st sensor. But how information is encoded is dealt
    * with in the SectorSystem classes (see ISectorSystem for the ABC).
    *
    * 
    */   
   class IHit{
      
      
   public:
      
      /** @return the x position */
      float getX() const { return _x; }
      
      /** @return the y position */
      float getY() const { return _y; }
      
      /** @return the z position */
      float getZ() const { return _z; }
      
      int getSector() const { return _sector; }
      
      /** @return the used SectorSystem */
      virtual const ISectorSystem* getSectorSystem() const =0;
      
      unsigned getLayer() const {return getSectorSystem()->getLayer( _sector ); }
      
      /** @return whether the hit is virtual (not a real hit). */ 
      bool isVirtual() const { return _isVirtual; };
      void setIsVirtual( bool isVirtual ){ _isVirtual = isVirtual; };
      
      /** @return the distance to an other hit */
      float distTo( IHit* otherHit );
      
      virtual ~IHit(){}
      
   protected:
      
            
      float _x;
      float _y;
      float _z;
      
      int _sector;
      
      
      bool _isVirtual;
      
      
   };
   
   
      
}


#endif




