#ifndef FTrackExceptions_h
#define FTrackExceptions_h

#include <string>
#include <exception> 

//Exceptions for the FTrackNamespace

namespace FTrack {

  /**Base exception class for FTrack - all other exceptions extend this.
   * @author R. Glattauer, HEPHY
   * 
   */

  class FTrackException : public std::exception {

    
  protected:
    std::string message ;
    
    FTrackException(){  /*no_op*/ ; } 
    
  public: 
     virtual ~FTrackException() throw() { /*no_op*/; } 
    
    FTrackException( const std::string& text ){
      message = "FTrack::Exception: " + text ;
    }

    virtual const char* what() const  throw() { return  message.c_str() ; } 

  };

  
  
  /**Out of range exception, used when the user tries to access layers oder sensors, that are not implemented
   * @author R. Glattauer, HEPHY
   */
  class OutOfRange : public FTrackException{
    
  protected:
    OutOfRange() {  /*no_op*/ ; } 
  public: 
    virtual ~OutOfRange() throw() { /*no_op*/; } 

    OutOfRange( std::string text ){
      message = "FTrack::OutOfRange: " + text ;
    }
  }; 


} 

#endif


