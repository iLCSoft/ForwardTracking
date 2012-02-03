#include "HopfieldNeuralNet.h"
#include <cmath>
#include <iostream>
#include <algorithm>

using namespace FTrack;

HopfieldNeuralNet::HopfieldNeuralNet( std::vector < std::vector <bool> > G , std::vector < double > QI , std::vector < double > states , double omega){

   unsigned int nNeurons = G.size();

   _omega = omega;
   _States = states;
   
   // resize the vectors. If they all have the sam size, as they should, this changes nothing
   _States.resize( nNeurons );
   _w0.resize( nNeurons );
   _W.resize( nNeurons );
   _order.resize( nNeurons);
   
   for ( unsigned int i =0; i < nNeurons; i++){
      
      // resize the vectors of the matrix W
      _W[i].resize( nNeurons );
      
      
      // initialise the order vector
      _order[i]=i;                      //the order now is 0,1,2,3... (will be changed to a random sequence in the iteration)
      
   }
   
   
   //calculate _w0
   for (unsigned int i=0; i < QI.size(); i++) _w0[i] = omega * QI[i];
   
   
   
   // Build the W matrix. (the matrix of the influences including their force)
   
   double comp = (1. - omega) / double (nNeurons);
   

   for (unsigned int i=0; i< nNeurons ; i++){ 

      for (unsigned int j=0; j< nNeurons ; j++){
       
      if (i == j) _W[i][j] = 0.; //diagonal elements are 0 --> whatever the matrix G says here is ignored.
      
      else  
         if ( G[i][j] == 1 ) _W[i][j] = -1;   //Neurons are incompatible
      
         else _W[i][j] =  comp;   //Neurons are compatible
      
      
      }
   
   }
   

   

      
      
      
   
   
    _T = 0;
    _TInf = 0;
   
   _isStable = false;
   _limitForStable = 0.01;
   


}



double HopfieldNeuralNet::activationFunction ( double state , double T ){
   
       
    double y = 1;
     
    if (T > 0) y = 0.5 * ( 1 + tanh( state / T ) ); //if T==0  tanh( infinity ) gives 1.
  
   
    return y;
         
         
}




bool HopfieldNeuralNet::doIteration(){
      
   _isStable = true;
      
   
   random_shuffle ( _order.begin() , _order.end() ); //shuffle the order
   
   for (unsigned int i=0; i<_States.size() ; i++){ //for all entries of the vector
       
      unsigned iNeuron = _order[i];


      double y;
      
      y = _w0[iNeuron];
      
      //matrix vector multiplication (or one line of it to be precise)  
      for (unsigned int j=0; j< _W[iNeuron].size(); j++){ 
       
         y  += _W[iNeuron][j] * _States[j]; 
      
      }
      
      y = activationFunction ( y , _T );
       
      // check if the change was big enough that the Network is not stable
      if ( fabs( _States[iNeuron] - y ) > _limitForStable ) _isStable = false;
       
      // update the state
      _States[iNeuron] = y;
      
   }
   
   
   
   // after the iteration, we need to calculate a new tempereatur _T
   
   _T = 0.5* ( _T + _TInf);
   
   
   return _isStable;
   
}






