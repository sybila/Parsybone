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
#include "../model/model_translators.hpp"
#include <PunyHeaders/SQLAdapter.hpp>

class ExplicitFilter {
   set<ParamNo> allowed; ///< Numbers of all the parametrizations that are allowed.
   bool is_filter_active; ///< True iff there was an input mask.

   vector<size_t> getColumns(const Model & model, SQLAdapter & sql_adapter) {
      vector<size_t> locations;
      vector<string> names = sql_adapter.readColumnNames(PARAMETRIZATIONS_TABLE, regex(".*"));
      for (const SpecieID ID : scope(model.species)) {
         const Model::Parameters & params = model.getParameters(ID);
         for (const size_t kpar_no : scope(params)) {
            auto column_it = find(names.begin(), names.end(), ModelTranslators::makeConcise(params[kpar_no], model.getName(ID)));
            if (column_it == names.end())
               locations.push_back(INF);
            else
               locations.push_back(distance(names.begin(), column_it));
         }
      }
      return locations;
   }

public:
   ExplicitFilter() : is_filter_active(false) {}

   /**
    * @brief addAllowed add parametrizations that are allowed by given database
    */
   void addAllowed(const Model & model, SQLAdapter & sql_adapter) {
      vector<size_t> colum_match = getColumns(model, sql_adapter);
      sql_adapter.accessTable(PARAMETRIZATIONS_TABLE);
      Levels column_data = sql_adapter.getRow<ActLevel>(colum_match);
      while (!column_data.empty()) {
         set<ParamNo> matching = ModelTranslators::findMatching(model, column_data);
         allowed.insert(matching.begin(), matching.end());
         column_data = sql_adapter.getRow<ActLevel>(colum_match);
      }
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
