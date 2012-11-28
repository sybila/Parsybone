#ifndef PARSYBONE_SQLADAPTER_INCLUDED
#define PARSYBONE_SQLADAPTER_INCLUDED

#include <memory>

#include "../auxiliary/data_types.hpp"
#include "../sqlite/sqlite3.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Primitive interface to a database connection.
///
/// This class controls splitting of the parameter space both for independent rounds and for distributed synthesis.
///
/// @attention Holds only one statement at a time.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SQLAdapter {
   string file_name; ///< Name of the database file itself.
   sqlite3* database; ///< Database connection.
   sqlite3_stmt* statement; ///< Prepared statement in current use, if there is any such.

   /**
    *
    */
   void openDatabase() {
      int result = sqlite3_open(file_name.c_str(), &database);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_open \"" + file_name + "\" failed with: " + toString(result));
   }

public:
   /**
    * Constructor takes a name of the database one wants to use and creates a connection.
    */
   void setDatabase(const string & o_file_name) {
      file_name = o_file_name;
      database = nullptr;
      statement = nullptr;
      openDatabase();
   }

   size_t getColumnCount() const {
      if (statement == nullptr)
         throw runtime_error("invoked column count on a null statement");
      return sqlite3_column_count(statement);
   }

   /**
    * Adds correctness control to the sqlite3_exec function (executes all queries at once).
    *
    * @param query string with an SQLLite query
    */
   void safeExec(const string & query) {
      int result = sqlite3_exec(database, query.c_str(), 0, 0, 0);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_exec \"" + query + "\" failed with: " + toString(result));
   }

   /**
    * Adds correctness control to the sqlite3_prepare_v2 function (creates prepared statement). The statement is reffered to using the statement pointer.
    *
    * @param query string with an SQLLite query
    */
   void safePrepare(const string & query) {
      int result;
      if (statement != nullptr) {
         result = sqlite3_finalize(statement);
         statement = nullptr;
         if (result != SQLITE_OK)
             throw runtime_error("sqlite3_finalize failed with: " + toString(result));
      }

      result = sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_prepare_v2 \"" + query + "\" failed with: "+ toString(result));
   }

   /**
    *
    */
   void step() {
      if (statement == nullptr)
         return;

      int result = sqlite3_step(statement);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_step failed with: "+ toString(result));
   }

   /**
    *
    */
   ~SQLAdapter() {
      if (database != nullptr)
         sqlite3_close(database);
      database = nullptr;

      if (statement != nullptr)
         sqlite3_finalize(statement);
      statement = nullptr;
   }
} sql_adapter;

#endif // PARSYBONE_SQLADAPTER_INCLUDED
