/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */


#ifndef EXPLICIT_FILTER_HPP
#define EXPLICIT_FILTER_HPP

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief This class takes parametrizations from the database and rememberes those that are allowed by the database.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../auxiliary/data_types.hpp"
#include <PunyHeaders/SQLAdapter.hpp>

class ExplicitFilter {
   set<ParamNo> allowed; ///< Numbers of all the parametrizations that are allowed.
   bool is_filter_active; ///< True iff there was an input mask.



public:
   ExplicitFilter() : is_filter_active(false) {}

   void addAllowed(const Model & model, const SQLAdapter & sql_adapter) {

   }

   /**
    * @return true iff the parametrization is not filtered out.
    */
   inline bool isAllowed(ParamNo param_no) {
      if (is_filter_active)
         return (allowed.find(param_no) != allowed.end());
      else
         return true;
   }
};

#endif // EXPLICIT_FILTER_HPP
