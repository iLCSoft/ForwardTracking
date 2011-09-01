#include "NeuronNetProcessor.h"
#include "NeuronNet.h"
#include <iostream>



// ----- include for verbosity dependend logging ---------
#include "marlin/VerbosityLevels.h"



using namespace lcio ;
using namespace marlin ;





NeuronNetProcessor aNeuronNetProcessor ;


NeuronNetProcessor::NeuronNetProcessor() : Processor("NeuronNetProcessor") {

    // modify processor description
   _description = "NeuronNetProcessor" ;


    // register steering parameters: name, description, class-variable, default value

   
   
     

}



void NeuronNetProcessor::init() { 

    streamlog_out(DEBUG) << "   init called  " << std::endl ;

    // usually a good idea to
    printParameters() ;

    _nRun = 0 ;
    _nEvt = 0 ;

}


void NeuronNetProcessor::processRunHeader( LCRunHeader* run) { 

    _nRun++ ;
} 



void NeuronNetProcessor::processEvent( LCEvent * evt ) { 


   std::vector < std::vector <bool> > G;
   
   std::vector <bool> temp(3);
   
   temp[0]= 0;
   temp[1]= 1;
   temp[2]= 0;
   
   G.push_back( temp );
   
   temp[0]= 1;
   temp[1]= 0;
   temp[2]= 0;
   
   G.push_back( temp );
   
   temp[0]= 0;
   temp[1]= 0;
   temp[2]= 0;
   
   G.push_back( temp );
   
   
   std::vector < double > QI ;
   
   QI.push_back(0.2);
   QI.push_back(0.8);
   QI.push_back(0.7);
  
   std::vector < double > states;
   
   states.push_back(0.02);
   states.push_back(0.07);
   states.push_back(0.04);
   
   double omega = 0.5;
   
   
   
   NeuronNet Net( G , QI , states , omega);
   
   Net.setT (2.1);
   Net.setTInf(0.1);
   Net.setLimitForStable(0.01);
         
   Net.showStateInfo();
   
   while ( !Net.doIteration() ) Net.showStateInfo();
   
   Net.showStateInfo();
   
   
   
   
   
    //-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !

    streamlog_out(DEBUG) << "   processing event: " << evt->getEventNumber() 
        << "   in run:  " << evt->getRunNumber() << std::endl ;



    _nEvt ++ ;
}



void NeuronNetProcessor::check( LCEvent * evt ) { 
    // nothing to check here - could be used to fill checkplots in reconstruction processor
}


void NeuronNetProcessor::end(){ 

    //   std::cout << "MyProcessor::end()  " << name() 
    // 	    << " processed " << _nEvt << " events in " << _nRun << " runs "
    // 	    << std::endl ;

}

