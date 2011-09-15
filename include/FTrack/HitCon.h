#ifndef HitCon_h
#define HitCon_h

#include "IHitConnector.h"

#include "AutCode.h"
//TODO: maybe an interface for the code-class would be useful?
// TODO: split this up into several hitconnectors: one for a fixed step size. one for hopping to layer 0 and so on.


namespace FTrack{
   
   /**
    * TODO: document this! 
    * 
    * Gets a code and returns a vector of them too. All the returned ones represent possible connections
    */   
   class HitCon : public IHitConnector{
      
      
   public:
      
      HitCon ( AutCode* autCode );
      
      virtual std::set <int>  getTargetCode ( int code );
      
      
   private:
      
      AutCode* _autCode;
      
   };
   
   
}


#endif