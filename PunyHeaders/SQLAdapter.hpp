/*
 * Copyright (C) 2012-2013 - Adam Streck
 * Independent headers for software development.
 */

#ifndef SQLADAPTER_HPP
#define SQLADAPTER_HPP

#include <sqlite3.h>
#include <regex>

#include "common_functions.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Primitive interface to a database connection. Behaves as a smart pointer i.e. connection is hold only for the time of its existence and then it is released.
///
/// Requires sqlite3 code or library and a header file to be present in the standard path.
/// Requires common_functions header.
/// @attention Holds only one statement at a time.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SQLAdapter {
   string file_name; ///< Name of the database file itself.
   sqlite3* database; ///< Database connection.
   sqlite3_stmt* statement; ///< Prepared statement in current use, if there is any such.

   /**
    * @brief openDatabase
    */
   void openDatabase() {
      int result = sqlite3_open(file_name.c_str(), &database);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_open \"" + file_name + "\" failed with: " + to_string(result));
   }

   /**
    * @brief closeDatabase
    */
   void closeDatabase() {
      if (statement != nullptr)
         sqlite3_finalize(statement);
      statement = nullptr;

      if (database != nullptr)
         sqlite3_close(database);
      database = nullptr;
   }

   /**
    * @brief nullify
    */
   void nullify() {
      file_name = "";
      database = nullptr;
      statement = nullptr;
   }

   /**
    * @brief copyFrom
    * @param other
    */
   void copyFrom(const SQLAdapter & other) {
      file_name = other.file_name;
      database = other.database;
      statement = other.statement;
   }

public:
   SQLAdapter() {
      nullify();
   }
   SQLAdapter(const SQLAdapter & other) = delete;
   SQLAdapter & operator=(const SQLAdapter & other) = delete;
   SQLAdapter(SQLAdapter && other) {
      copyFrom(other);
      other.nullify();
   }
   SQLAdapter & operator=(SQLAdapter && other) {
      closeDatabase();
      nullify();
      copyFrom(other);
      other.nullify();
      return *this;
   }

   ~SQLAdapter() {
      closeDatabase();
   }

   /**
    * @brief getStatement
    * @return
    */
   sqlite3_stmt* getStatement() {
      return statement;
   }

   /**
    * @brief setDatabase
    * @param o_file_name
    */
   void setDatabase(const string & o_file_name) {
      closeDatabase();
      file_name = o_file_name;
      database = nullptr;
      statement = nullptr;
      openDatabase();
   }

   /**
    * @brief getColumnCount
    * @return
    */
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
         throw runtime_error("sqlite3_exec \"" + query + "\" failed with: " + to_string(result));
   }

   /**
    * @brief safeFinalize
    */
   void safeFinalize() {
      int result;
      if (statement != nullptr) {
         result = sqlite3_finalize(statement);
         statement = nullptr;
         if (result != SQLITE_OK)
            throw runtime_error("sqlite3_finalize failed with: " + to_string(result));
      }
   }

   /**
    * Adds correctness control to the sqlite3_prepare_v2 function (creates prepared statement). The statement is reffered to using the statement pointer.
    *
    * @param query string with an SQLLite query
    */
   void safePrepare(const string & query) {
      safeFinalize();

      int result;

      result = sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0);
      if (result != SQLITE_OK)
         throw runtime_error("sqlite3_prepare_v2 \"" + query + "\" failed with: "+ to_string(result));
   }

   /**
    * Controls existence of a column in the database.
    * @param table_name    name of the table to select from
    * @param column_name   a column to search for
    * @return  true if a colum of the given name exists
    */
   bool hasColumn(const string & table_name, const string & column_name) {
      bool column_present = false;

      safePrepare("SELECT * FROM " + table_name + ";");
      sqlite3_step(statement);

      for (int column_num = 0; column_num < sqlite3_column_count(statement); column_num++) {
         // Compare returns (0) false on hit, adding !false to the column_present variable causes it to be furter true.
         column_present |= ! column_name.compare(sqlite3_column_name(statement, column_num));
      }

      return column_present;
   }

   /**
    * @brief hasTable
    * @param table_name
    * @return
    */
   bool hasTable(const string & table_name) {
      safePrepare("SELECT * FROM sqlite_master WHERE name='" + table_name +"'");
      return sqlite3_step(statement) == SQLITE_ROW;
   }

   /**
    * Creates a vector of column names that correspond to the regular expression given.
    * @param table_name    name of the table to select from
    * @param control_expression    a regex a colmn's name has to abide by
    * @return  a vector of column names given as strings
    */
   vector<string> readColumnNames(const string & table_name, const regex & control_expression) {
      vector<string> columns;

      safePrepare("SELECT * FROM " + table_name + ";");
      sqlite3_step(statement);

      for (int column = 0; column < sqlite3_column_count(statement); column++) {
         if (regex_match(sqlite3_column_name(statement, column), control_expression))
            columns.push_back(sqlite3_column_name(statement, column));
      }
      return columns;
   }

   /**
    * @brief readColumn returns content of the column id the requested data type
    * @param table_name
    * @param column_name
    * @return
    */
   template <typename DataType>
   vector<DataType> readColumn(const string & table_name, const string & column_name) {
      vector<DataType> column_data;

      accessTable(table_name);

      size_t column_no = 0;
      while (column_name.compare(sqlite3_column_name(statement, column_no)) != 0) {
         column_no++;
         if (column_no >= getColumnCount())
            throw runtime_error("Column " + column_name + " not found where expected.");
      }

      // Cycle through all the rows available.
      while (sqlite3_step(statement) == SQLITE_ROW) {
         const unsigned char * entry  = (sqlite3_column_text(statement, column_no));

         try {
            if (entry != 0)
               column_data.push_back(boost::lexical_cast<DataType>(entry));
            else // Replace null value with zero.
               column_data.push_back(boost::lexical_cast<DataType>('0'));
         } catch(boost::bad_lexical_cast & e) {
            cerr << "Converting \"" << entry << "\" into " << typeid(DataType).name() << " has failed with " << e.what();
         }
      }

      return column_data;
   }

   /**
    * @brief accessTable Connect current statement to the given table.
    */
   inline void accessTable(const string & table_name) {
      safePrepare("SELECT * FROM " + table_name + ";");
   }

   /**
    * @return data from the current row
    */
   template <typename DataType>
   vector<DataType> getRow(const vector<size_t> & columns) {
      vector<DataType> data;
      if (statement == nullptr)
         return data;
      if (sqlite3_step(statement) == SQLITE_ROW) {
         for (const size_t column : columns) {
            const unsigned char * entry = 0;
            if (column < getColumnCount())
                entry = sqlite3_column_text(statement, column);
            try {
               if (entry != 0)
                  data.push_back(boost::lexical_cast<DataType>(entry));
               else // Replace null value with zero.
                  data.push_back(boost::lexical_cast<DataType>('0'));
            } catch(boost::bad_lexical_cast & e) {
               cerr << "Converting \"" << entry << "\" into " << typeid(DataType).name() << " has failed with " << e.what();
            }
         }
      }
      return data;
   }

   /**
    * @brief updateColumn  Function that ouputs values of a vector in their given order into a column of the database.
    * @attention outputs only if there are already rows present
    *
    * @param table_name    name of the table to select from
    * @param column_name  a name of the row to add
    * @param name  a vector of data to fill the row with
    */
   template<class ColumnType>
   void updateColumn(const string & table_name, const string & column_name, const vector<ColumnType> & column) {
      string query;

      for (const auto i:range(column.size())) {
         query += "UPDATE " + table_name + " SET " + column_name + "=" + boost::lexical_cast<string>(column[i])
                  + " WHERE ROWID=" + boost::lexical_cast<string>(i + 1) + "; ";
      }

      safeExec(query.c_str());
   }

   /**
    * @brief prepareTable
    * @param name
    * @param columns
    */
   void prepareTable(const string & name, const string & columns) {
      safeFinalize();
      string query;

      // Drop old tables if any.
      query += "DROP TABLE IF EXISTS " + name + ";";
      query += "CREATE TABLE " + name + " " + columns + ";";

      safeExec(query);
   }

   /**
    * Add a row to the table if it is not present or null its values if it is.
    *
    * @param table_name    name of the table to select from
    * @param column_name  a name of the row to add
    * @param type  a type of the row to add
    */
   void prepareColumn(const string & table_name, const string & column_name, const string & type) {
      string query;

      if (!hasColumn(table_name, column_name))
         query += "ALTER TABLE " + table_name + " ADD " + column_name + " " + type +";";
      else
         query += "UPDATE " + table_name + " SET " + column_name + "=NULL;";

      safeExec(query);
   }
};

#endif // SQLADAPTER_HPP
