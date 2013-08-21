#ifndef TRANSITION_SYSTEM_INTERFACE_HPP
#define TRANSITION_SYSTEM_INTERFACE_HPP

#include "graph_interface.hpp"

struct TransConst {
   ParamNo step_size; ///< How many bits of a parameter space bitset is needed to get from one targe value to another.
   bool req_dir; ///<
   ActLevel comp_value; ///<
   const Levels & targets; ///<
};

#endif // TRANSITION_SYSTEM_INTERFACE_HPP
