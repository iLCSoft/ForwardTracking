/**
 * Represents a neuronal network
 * 
 * Author: Robin Glattauer
 */
 
#ifndef NeuronNet_h
#define NeuronNet_h
 
 
#include <vector>


class NeuronNet {


   public:
      
      NeuronNet( std::vector < std::vector <bool> > G , std::vector < double > QI , std::vector < double > states , double omega);
              
      /** Does one iteration of the neuronal network.
       * 
       * \f$ \vec{y} = W \times \vec{state} + \vec{w_0} \f$
       * 
       * \f$ \vec{state}_{new} = activationFunction(\vec{y}) \f$
       * 
       * @return Whether the neuronal network is considered as stable
       */
      bool doIteration();      
         

      void setT    (double T)    { _T = T;};
      void setTInf (double TInf) {_TInf = TInf;};
      void setLimitForStable (double limit) { _limitForStable = limit; };
      
      
      void showStateInfo();
      
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
      
      /** Omega controls the influence of the quality indicator on the  activation of the neuron.
       */
      double _omega;

      
      
      /** Calculates the activation function
       * 
       * @param state the state
       * @param T the temperature
       * @return the actication function corresponding to the input values: g(x) = 1/2* (1 + tanh( x/T ))
       */
      double activationFunction ( double state , double T );


};





#endif

