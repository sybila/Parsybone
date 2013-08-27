#ifndef PARSYBONE_DATABASE_FILLER_INCLUDED
#define PARSYBONE_DATABASE_FILLER_INCLUDED

#include "../auxiliary/data_types.hpp"
#include "../auxiliary/user_options.hpp"
#include "../model/model_translators.hpp"
#include <PunyHeaders/SQLAdapter.hpp>

class DatabaseFiller {
   const Model & model;
   SQLAdapter sql_adapter;

   bool in_output; ///< True if there is currently transaction ongoing.

   void prepareTable(const string & name, const string & columns) {
      // Drop old tables if any.
      string drop_cmd = "DROP TABLE IF EXISTS " + name + "; ";
      string create_cmd = "CREATE TABLE " + name + " " + columns + ";\n";
      sql_adapter.safeExec(drop_cmd + create_cmd);
   }

   inline string makeInsert(const string & table) {
      return "INSERT INTO " + table + " VALUES ";
   }

   void fillComponents() {
      prepareTable(COMPONENTS_TABLE, "(Name TEXT, MaxActivity INTEGER)");

      string update = "";
      for(SpecieID target_ID:range(model.species.size())) {
         string values = "(\"" + model.getName(target_ID) + "\", " + toString(model.getMax(target_ID)) + "); \n";
         update += makeInsert(COMPONENTS_TABLE) + values;
      }
      sql_adapter.safeExec(update);
   }

   void fillRegulations() {
      prepareTable(REGULATIONS_TABLE, "(Regulator TEXT, Target TEXT, Thresholds TEXT)");
      string update = "";
      for(SpecieID target_ID:range(model.species.size())) {
         for(auto regul:ModelTranslators::getThresholds(model, target_ID)) {
            string values = "(\"" + model.getName(regul.first) + "\", ";
            values += "\"" + model.getName(target_ID) + "\", ";
            values += "\"";
            for (auto threshold:regul.second) {
               values += toString(threshold) + ",";
            }
            values.back() = '\"';
            values += "); \n";
            update += makeInsert(REGULATIONS_TABLE) + values;
         }
      }
      sql_adapter.safeExec(update);
   }

   string getContexts() const {
      string contexts = "";
      for(SpecieID target_ID:range(model.species.size()))
         for(auto param:model.getParameters(target_ID))
            contexts += ModelTranslators::makeConcise(param, model.getName(target_ID)) + " INTEGER, ";
      return contexts;
   }

   void fillParametrizations() {
      string columns = "(" + getContexts();
      columns += "Cost INTEGER,";
      if (user_options.compute_robustness)
         columns += "Robustness REAL,";
      if (user_options.compute_wintess)
         columns += "Witness_path TEXT,";
      columns += "Selection TEXT )";

      prepareTable(PARAMETRIZATIONS_TABLE, columns);
   }

public:
   DatabaseFiller(const Model & _model, const string & datafile_name, const bool create_database) : model(_model) {
      if (create_database)
         sql_adapter.setDatabase(datafile_name);
      in_output = false;
   }

   void creteTables() {
      sql_adapter.safeExec("BEGIN TRANSACTION;");
      fillComponents();
      fillRegulations();
      fillParametrizations();
      sql_adapter.safeExec("END;");
   }

   void dropTables() {
      const string DROP_CMD("DROP TABLE ");
      sql_adapter.safeExec(DROP_CMD + COMPONENTS_TABLE + ";");
      sql_adapter.safeExec(DROP_CMD + REGULATIONS_TABLE + ";");
      sql_adapter.safeExec(DROP_CMD + PARAMETRIZATIONS_TABLE + ";");
   }

   void addParametrization(string parametrization) {
      auto insert = makeInsert(PARAMETRIZATIONS_TABLE);
      sql_adapter.safeExec(insert + parametrization + " 1);");
   }

   void startOutput() {
      sql_adapter.safeExec("BEGIN TRANSACTION;");
      in_output = true;
   }

   void finishOutpout() {
      if (in_output)
         sql_adapter.safeExec("END;");
      in_output = false;
   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
