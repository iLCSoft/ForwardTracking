////////////////////////
// simple_circle test
////////////////////////

#include "ilctest/ILCTest.h"
#include <exception>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cstring>

#include "Criteria/SimpleCircle.h"

using namespace std ;

// this should be the first line in your test
static ILCTest ilctest = ILCTest( "simple_circle" , std::cout );

//=============================================================================

int main(int , char** ){
    
    try{
    
        // ----- write your tests in here -------------------------------------

        ilctest.log( "testing class SimpleCircle" );


        ilctest.log( "testing SimpleCircle with arguments 0.0, 1.0, 0.0, 1.0, 2.0, 2.0" );

        KiTrack::SimpleCircle s( 0.0, 1.0, 0.0, 1.0, 2.0, 2.0 );


        if( ! std::isinf( s.getRadius() ))
        {
            ilctest.pass( "getRadius() is not infinite" );
        }
        else
        {
            ilctest.error( "not expecting getRadius() to be infinite" );
        }    

      if( strcmp(ilctest.last_test_status(),"PASSED") ){

            // should actually be able to name the tests and check status by name
            ilctest.log( "testing something which only makes sense if getRadius() is not infinite" );

            if( std::isnan( s.getRadius() ))
            {
                ilctest.error( "not expecting getRadius() to be NAN" );
            }
            else
            {
                ilctest.pass( "getRadius() is not NAN" );
            }    
        }

        // --------------------------------------------------------------------


    //} catch( ... ){
    } catch( exception &e ){
        ilctest.log( "exception caught" );
        ilctest.fatal_error( e.what() );
    }


    return 0;
}

//=============================================================================

