#ifndef HopfieldNeuralNet_h
#define HopfieldNeuralNet_h
 
 
#include <vector>


namespace FTrack{

   /**
    * Represents a hopfield neuronal network
    * 
    * Author: Robin Glattauer
    */
   class HopfieldNeuralNet {


      public:
         
         /**
         * @param G a matrix of the correlations. True means two neurons are incompatible. False means, they are
         * compatible. (the diagonal elements are 0 by definition) 
         * 
         * @param QI a vector containing the qualtity indicators of the neurons (i.e. their power to amplify or
         * weaken other neurons)
         * 
         * @param states the states of the neurons. Should be between 0 and 1. TODO: check this, or use different method
         * 
         * @param omega controls the influence of the quality indicator on the  activation of the neuron
         */
         HopfieldNeuralNet( std::vector < std::vector <bool> > G , std::vector < double > QI , std::vector < double > states , double omega);
               
               
         /** Does one iteration of the neuronal network.
         * 
         * \f$ \vec{y} = W \times \vec{state} + \vec{w_0} \f$
         * 
         * \f$ \vec{state}_{new} = activationFunction(\vec{y}) \f$
         * 
         * @return Whether the neuronal network is considered as stable
         */
         bool doIteration();      
            
         /**
         * sets the temperature of the Neural Network
         */
         void setT    (double T)    { _T = T;};
         
         /**
         * Sets the temperature at infinity. The temperature will converge to this
         * value.
         */
         void setTInf (double TInf) {_TInf = TInf;};
         
         /**
         * Sets the value that changes of the states of the neurons must exceed in order to be counted as
         * "changed". When no state of a neuron changes more than this value, the Network is considered as stable.
         */
         void setLimitForStable (double limit) { _limitForStable = limit; };
         
         
         /** @return the vector of the states
         */
         std::vector <double> getStates(){ return _States; };
         

         
      protected:
         
         
            
         /** the matrix of the weights*/
         std::vector < std::vector <double> > _W;
         
         /** state describing how active a neuron is*/
         std::vector < double > _States;
         
         
         std::vector < double > _w0;
         
         /** temperatur */
         double _T;
         
         /** temperature after infinite iterations */
         double _TInf;

         /** indicates if the neuronal network is stable.
         * this is true when the change after one iteration 
         * of any neuron is not bigger than the value _limitForStable.
         */
         bool _isStable;   

         /** The upper limit for change of a neuron if it should be considered stabel.*/
         double _limitForStable;
         
         /** Omega controls the influence of the quality indicator on the activation of the neuron.
         */
         double _omega;

         /** the order of the neurons to be updated. So it should of course reach from 0 to the number of neurons -1.
         * (4 , 2, 0  1, 3) will for example mean: update first the neuron 4, then the neuron 2, then 0 and so on
         */
         std::vector <unsigned> _order;
         
         
         /** Calculates the activation function
         * 
         * @param state the state
         * @param T the temperature
         * @return the actication function corresponding to the input values: g(x) = 1/2* (1 + tanh( x/T ))
         */
         double activationFunction ( double state , double T );


   };


}


#endif

