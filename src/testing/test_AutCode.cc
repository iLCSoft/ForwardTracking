////////////////////////
// AutCode test
////////////////////////

#include "ilctest/ILCTest.h"
#include <exception>
#include <iostream>
#include <cassert>
#include <cmath>

#include "AutCode.h"
#include "FTrackTools.h"

using namespace std ;
using namespace FTrack;

// this should be the first line in your test
static ILCTest ilctest = ILCTest( "AutCode" , std::cout );

//=============================================================================

int main(int argc, char** argv ){
   
   try{
      
      // ----- write your tests in here -------------------------------------
      
      ilctest.log( "testing class AutCode" );
      
      
      ilctest.log( "testing AutCode with 9 layers, 8 modules and 3 sensors" );
      ilctest.log( "checking, if input and output are consistent" );
      
      unsigned nLayer = 9;
      unsigned nModules = 8;
      unsigned nSensors = 3;
           
      
      FTrack::AutCode autCode( nLayer , nModules , nSensors );
      
      bool pass = true;
      
      for( int side = -1; side <= 1; side+=2){
         
         for( unsigned layer=0; layer < nLayer; layer++){
            
            for( unsigned module=0; module < nModules; module++){
               
               for( unsigned sensor=0; sensor < nSensors; sensor++){
                  
                  int code = autCode.getCode( side, layer , module , sensor );
                  
                  if(( autCode.getSide( code ) != side )||
                     ( autCode.getLayer( code ) != layer )||
                     ( autCode.getModule( code ) != module )||
                     ( autCode.getSensor( code ) != sensor ))
                  {
                    pass = false;
                    ilctest.error( "For side " + intToString(side) + ", layer " + intToString(layer)
                                +  ", module " + intToString(module) + ", sensor " + intToString(sensor)
                                + " we got the Code: " + intToString( code )
                                + "\nFor this code we got "
                                + "side " + intToString( autCode.getSide( code ) )
                                + ", layer " + intToString( autCode.getLayer( code ) )
                                + ", module " + intToString( autCode.getModule( code ) )
                                + ", sensor " + intToString( autCode.getSensor( code ) ) );
                    
                  }
                  
                  
               }
               
            }
            
         }
         
      }
      
      if( pass ) ilctest.pass( "For all possible numbers the code and the values matched." );
      
      
      

      
      // --------------------------------------------------------------------
      
      
      //} catch( ... ){
      } catch( exception &e ){
         ilctest.log( "exception caught" );
         ilctest.fatal_error( e.what() );
      }
      
      
      return 0;
   }
   
   //=============================================================================
   
   