#include "NeuronNet.h"
#include <cmath>
#include <iostream>


NeuronNet::NeuronNet( std::vector < std::vector <bool> > G , std::vector < double > QI , std::vector < double > states , double omega){

   unsigned int nNeurons = G.size();

   _omega = omega;
   _States = states;
   
   // resize the vectors. If they all have the sam size, as they should, this changes nothing
   _States.resize( nNeurons );
   _w0.resize( nNeurons );
   _W.resize( nNeurons );
   for ( unsigned int i =0; i < nNeurons; i++) _W[i].resize( nNeurons );
   
   //calculate _w0
   for (unsigned int i=0; i < QI.size(); i++) _w0[i] = omega * QI[i];
   
   //calculate _W
   
   double comp = (1. - omega) / double (nNeurons);
   
   for (unsigned int i=0; i< nNeurons ; i++)
      for (unsigned int j=0; j< nNeurons ; j++){
       
      if (i == j) _W[i][j] = 0.; //diagonal elements are 0
      
      else  
         if ( G[i][j] == 1 ) _W[i][j] = -1;   //Neurons are incompatible
      
         else _W[i][j] =  comp;   //Neurons are compatible
      
      
      }
   
   

   

      
      
      
   
   
    _T = 0;
    _TInf = 0;
   
   _isStable = false;
   _limitForStable = 0.01;
   


}



double NeuronNet::activationFunction ( double state , double T ){
   
       
    double y = 1;
     
    if (T > 0) y = 0.5 * ( 1 + tanh( state / T ) ); //if T==0  tanh( infinity ) gives 1.
  
   
    return y;
         
         
}




bool NeuronNet::doIteration(){
      
   _isStable = true;
      
   
   for (unsigned int i=0; i<_States.size() ; i++){ //for all entries of the vector
       
      double y;
      
      y = _w0[i];
        
      for (unsigned int j=0; j< _W[i].size(); j++) y 
               += _W[i][j] * _States[j]; //matrix vector multiplication (or one line of it to be precise)
      
       y = activationFunction ( y , _T );
       
       if ( fabs( _States[i] - y ) > _limitForStable ) _isStable = false;
       
       _States[i] = y;
      
   }
   
   
   
   // after the iteration, we need to calculate a new tempereatur _T
   
   _T = 0.5* ( _T + _TInf);
   
   
   return _isStable;
   
}


void NeuronNet::showStateInfo(){
   
   
   std::cout<< std::endl;
         
   std::cout << "( ";      
   
   for ( unsigned int i=0; i<_States.size(); i++) std::cout << _States[i] << " "; 
   
   std::cout << ")";
   
   std::cout<< std::endl;  
   
   
   std::cout<< std::endl;
         
   std::cout << "( ";      
   
   for ( unsigned int i=0; i<_W.size(); i++){
      
      for (unsigned int j=0; j<_W[i].size(); j++) 
         std::cout << _W[i][j] << " "; 
      
      std::cout << std::endl;
      
   }
   
   std::cout << ")";
   
   std::cout<< std::endl; 
   
   
}



