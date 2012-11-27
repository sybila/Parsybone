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

   string parametrizationsQuery() const {
      return "";
   }

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
      fillComponents();
      fillInteractions();
      fillParametrizations();
   }

   void fillComponents() {
      // Drop old tables if any.
      string drop_cmd = "DROP TABLE IF EXISTS " + COMPONENTS_TABLE + "; ";
      base->safeExec(drop_cmd);
      string create_cmd = "CREATE TABLE " + COMPONENTS_TABLE + " (Name TEXT, MaxActivity INTEGER);";
      base->safeExec(create_cmd);
   }

   void fillInteractions() {
      string drop_cmd = "DROP TABLE IF EXISTS " + REGULATIONS_TABLE + "; ";
      base->safeExec(drop_cmd);
      string create_cmd = " CREATE TABLE " + REGULATIONS_TABLE + " (Regulator TEXT, Target TEXT, Threshold INTEGER);";
      base->safeExec(create_cmd);;
      // Create new tables.
   }

   void fillParametrizations() {
      string drop_cmd = "DROP TABLE IF EXISTS " + PARAMETRIZATIONS_TABLE + "; ";
      base->safeExec(drop_cmd);
   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
