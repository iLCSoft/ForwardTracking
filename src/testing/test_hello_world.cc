////////////////////////
// hello_world test
////////////////////////

#include "ilctest/ILCTest.h"
#include <exception>
#include <iostream>
#include <cassert>

using namespace std ;

// this should be the first line in your test
static ILCTest ilctest = ILCTest( "hello_world" , std::cout );

const std::string hello()
{
  return "Hello World!" ;
}


//=============================================================================

int main(int argc, char** argv ){
    
    try{
    
        // ----- write your tests in here -------------------------------------

        ilctest.log( "hello world test" );

        string test_str = hello() ;
        string expected_str = "Hello World!" ;
        string test_case  = "comparing: [" + expected_str + "] == [ " + test_str + "]"  ;

        if( test_str.compare( expected_str ) != 0 )
        {
            ilctest.error( test_case );
        }
        else
        {
            ilctest.pass( test_case );
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

