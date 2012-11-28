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

   void prepareTable(const string & name, const string & columns) {
      // Drop old tables if any.
      string drop_cmd = "DROP TABLE IF EXISTS " + name + "; ";
      string create_cmd = "CREATE TABLE " + name + " " + columns + ";\n";
      database_output.safeExec(drop_cmd + create_cmd);
   }

   inline string makeInsert(const string & table) {
      return "INSERT INTO " + table + " VALUES ";
   }

   void fillComponents() {
      prepareTable(COMPONENTS_TABLE, "(Name TEXT, MaxActivity INTEGER)");

      string update = "";
      for(SpecieID ID: ::range(model.getSpeciesCount())) {
         string values = "(\"" + model.getName(ID) + "\", " + toString(model.getMax(ID)) + "); \n";
         update += makeInsert(COMPONENTS_TABLE) + values;
      }
      database_output.safeExec(update);
   }

   void fillInteractions() {
      prepareTable(REGULATIONS_TABLE, "(Regulator TEXT, Target TEXT, Threshold INTEGER)");
      string update = "";
      for(SpecieID ID: ::range(model.getSpeciesCount())) {
         for(auto regul:model.getRegulations(ID)) {
            string values = "(\"" + model.getName(regul.source) + "\", ";
            values += "\"" + model.getName(ID) + "\", ";
            values += toString(regul.threshold) + "); \n";
            update += makeInsert(REGULATIONS_TABLE) + values;
         }
      }
      database_output.safeExec(update);
   }

   string getContexts() const {
      string contexts = "";
      for(SpecieID ID: ::range(model.getSpeciesCount())) {
         for(auto param:model.getParameters(ID)) {
            string context = "K_" + model.getName(ID) + "_";
            for (auto present:param.first) {
               context += toString(present);
            }
            context += " TEXT,";
            contexts += move(context);
         }
      }
      return contexts;
   }

   void fillParametrizations() {
      string columns = "(" + getContexts();
      if (user_options.timeSeries())
         columns += "Cost INTEGER,";
      if (user_options.robustness())
         columns += "Robustness REAL,";
      if (user_options.witnesses())
         columns += "Witness_path TEXT,";
      columns.back() = ')';

      prepareTable(PARAMETRIZATIONS_TABLE, columns);
   }

public:
   DatabaseFiller(const Model & _model): COMPONENTS_TABLE("components"), REGULATIONS_TABLE("regulations"), PARAMETRIZATIONS_TABLE("parametrizations"),
      model(holder.getModel()) {
      creteTables();
   }

   void creteTables() {
      database_output.safeExec("BEGIN TRANSACTION;");
      fillComponents();
      fillInteractions();
      fillParametrizations();
      database_output.safeExec("END;");
   }

   void addParametrization(string parametrization) {
      auto insert = makeInsert(PARAMETRIZATIONS_TABLE);
      database_output.safeExec("BEGIN TRANSACTION;");
      database_output.safeExec(insert + parametrization + ";");
      database_output.safeExec("END;");
   }
};

#endif // PARSYBONE_DATABASE_FILLER_INCLUDED
