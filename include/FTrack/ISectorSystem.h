#ifndef ISectorSystem_h
#define ISectorSystem_h


#include "FTrackExceptions.h"

namespace FTrack{
   
   
   /** An interface for Sector Systems. 
    * 
    * A sector system is able to take an integer that represents a sector and gives back information
    * about it. This can be things like the number of the sensor or the rough distance from the IP or such
    * things. Or even neighbouring sectors. 
    * But this is all dependent on the circumstances of the detectors and their representation. In the interface
    * only what is really needed is defined and that are the layers. So any sector system must be able to
    * return the layer of a sector and how many layers there are all in all. 
    * 
    * 
    */  
   class ISectorSystem{
      
      
   public:
      
      
      virtual unsigned getLayer( int sector ) const throw ( OutOfRange ) =0;
      
      
      unsigned getNumberOfLayers() const { return _nLayers; };
      
   protected:
      
      
      unsigned _nLayers;
      
      
      
   };
   
   
}


#endif

