#ifndef THOMAS_NETWORK_HPP
#define THOMAS_NETWORK_HPP

struct ThomasNetwork {
   bool bound_loops; ///< Should loop parameters that are not
   bool force_extremal_targets; ///< Sets parameters in the extremal conditions alway to max/min.

   struct Regulation {
      string source;
      size_t threshold;
   };

   struct Parameter {
      string context;
      size_t target_val;
   };

   struct Formula {
      string condition;
      size_t targe_val;
   };

   struct Component {
      string name;
      size_t max_val;
      size_t basal_val;

      vector<Regulation> regulations;
      vector<Parameter> parameters;
      vector<Formula> formulae;
   };

   vector<component> components;

   ThomasNetwork() {
      bound_loops = force_extremal_targets = false;
   }
};

#endif // THOMAS_NETWORK_HPP
