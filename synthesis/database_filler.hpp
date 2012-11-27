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
      base->safeExec("BEGIN TRANSACTION;");
      fillComponents();
      fillInteractions();
      fillParametrizations();
      base->safeExec("END;");
   }

   void prepareTable(const string & name, const string & columns) {
      // Drop old tables if any.
      string drop_cmd = "DROP TABLE IF EXISTS " + name + "; ";
      string create_cmd = "CREATE TABLE " + name + " " + columns + ";\n";
      base->safeExec(drop_cmd + create_cmd);
   }

   inline string makeInsert(const string & table) {
      return "INSERT INTO " + table + " VALUES (";
   }

   void fillComponents() {
      prepareTable(COMPONENTS_TABLE, "(Name TEXT, MaxActivity INTEGER)");

      string update = "";
      for(SpecieID ID: ::range(model.getSpeciesCount())) {
         string values = "\"" + model.getName(ID) + "\", " + toString(model.getMax(ID)) + "); \n";
         update += makeInsert(COMPONENTS_TABLE) + values;
      }
      base->safeExec(update);
   }

   void fillInteractions() {
      prepareTable(REGULATIONS_TABLE, "(Regulator TEXT, Target TEXT, Threshold INTEGER)");
      string update = "";
      for(SpecieID ID: ::range(model.getSpeciesCount())) {
         for(auto regul:model.getRegulations(ID)) {
            string values = "\"" + model.getName(regul.source) + "\", ";
            values += "\"" + model.getName(ID) + "\", ";
            values += toString(regul.threshold) + "); \n";
            update += makeInsert(REGULATIONS_TABLE) + values;
         }
      }
      base->safeExec(update);
   }

   void fillParametrizations() {
      prepareTable(PARAMETRIZATIONS_TABLE, "(Test TEXT)");
   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
