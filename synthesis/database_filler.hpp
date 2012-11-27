#ifndef PARSYBONE_DATABASE_FILLER_INCLUDED
#define PARSYBONE_DATABASE_FILLER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"
#include "SQLAdapter.hpp"

class DatabaseFiller {
   const Model & model;
   SQLAdapter & adapter;
public:

   DatabaseFiller(const Model & _model) : model(_model) {

   }

   openDatabase() {
      string database_name = user_options.modelName();
      adapter = SQLAdapter();
   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
