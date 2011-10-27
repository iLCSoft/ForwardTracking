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
  
  
  /**Invalid Parameter exception.
   * @author R. Glattauer, HEPHY
   */
  class InvalidParameter : public FTrackException{
     
  protected:
     InvalidParameter() {  /*no_op*/ ; } 
  public: 
     virtual ~InvalidParameter() throw() { /*no_op*/; } 
     
     InvalidParameter( std::string text ){
        message = "FTrack::InvalidParameter: " + text ;
     }
  }; 
  
  
  /**Wrong segment length exception.
   * @author R. Glattauer, HEPHY
   */
  class BadSegmentLength : public FTrackException{
     
  protected:
     BadSegmentLength() {  /*no_op*/ ; } 
  public: 
     virtual ~BadSegmentLength() throw() { /*no_op*/; } 
     
     BadSegmentLength( std::string text ){
        message = "FTrack::BadSegmentLength: " + text ;
     }
  }; 

  
  
  /**Unknown criterion exception.
   * @author R. Glattauer, HEPHY
   */
  class UnknownCriterion : public FTrackException{
     
  protected:
     UnknownCriterion() {  /*no_op*/ ; } 
  public: 
     virtual ~UnknownCriterion() throw() { /*no_op*/; } 
     
     UnknownCriterion( std::string text ){
        message = "FTrack::UnknownCriterion: " + text ;
     }
  }; 

} 

#endif


