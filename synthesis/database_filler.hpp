#ifndef PARSYBONE_DATABASE_FILLER_INCLUDED
#define PARSYBONE_DATABASE_FILLER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"
#include "../construction/construction_holder.hpp"
#include "SQLAdapter.hpp"

class DatabaseFiller {
   const string COMPONENTS_TABLE;
   const string REGULATIONS_TABLE;
   const string PARAMETRIZATIONS_TABLE;

   const Model & model;

   unique_ptr<SQLAdapter> base;

public:
   DatabaseFiller(const ConstructionHolder & holder)
      : COMPONENTS_TABLE("components"), REGULATIONS_TABLE("regulations"), PARAMETRIZATIONS_TABLE("parametrizations"),
      model(holder.getModel()) {
   }

   void connect() {
      string database_name = user_options.modelName() + DATABASE_SUFFIX;
      base.reset(new SQLAdapter(database_name));
      base->openDatabase();
   }

   void creteTables() {
      string create_cmd = "CREATE TABLE " + COMPONENTS_TABLE + "(Name TEXT, MaxActivity INTEGER)";
      base->safeExec(create_cmd);
   }

   void fillComponents() {

   }

   void fillInteractions() {

   }

   void fillParametrizations() {

   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
