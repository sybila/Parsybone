/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_USER_OPTIONS_INCLUDED
#define PARSYBONE_USER_OPTIONS_INCLUDED

#include "../auxiliary/data_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Storage of options obtained from execution arguments.
///
/// Class that stores options provided by the user on the input. Values can be set up only using the ArgumentParser object.
/// Further access to global object user_options is possible only due to constant getters.
/// Only a single object is intended to exist.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UserOptions {
public:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // OPTIONS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool be_verbose; ///< Display data from verbose stream (verbose_str), mainly progress information
   bool compute_robustness; ///< Should robustness value be computed and displyed?
   bool compute_wintess; ///< Should witnesses be computed and displayed?
   bool use_long_witnesses; ///< How witnesses should be displayed - complete state or only ID?
   bool use_in_mask; ///< Is there a paramset mask on the input?
   bool use_out_mask; ///< Should computed parametrizations be output in the form of mask?
   bool output_console;
   bool use_textfile;
   bool use_database;
   bool bounded_check;
   size_t bound_size;
   size_t process_number; ///< What is the ID of this process?
   size_t processes_count; ///< How many processes are included in the computation?
   string model_path;
   string model_name; ///< What is the name of the model?
   string database_file;
   string datatext_file;
   string in_mask_file;
   string out_mask_file;

   /**
    * @brief addDefaultFiles    Create default file names where there are none explicitly specified.
    */
   void addDefaultFiles() {
      if (database_file.empty())
         database_file = model_path + model_name + DATABASE_SUFFIX;
      if (datatext_file.empty())
         datatext_file = model_path + model_name + OUTPUT_SUFFIX;
      if (in_mask_file.empty())
         in_mask_file = model_path + model_name + MASK_SUFFIX;
      if (out_mask_file.empty())
         out_mask_file = model_path + model_name + MASK_SUFFIX;
   }

   /**
    * Constructor, sets up default values.
    */
   UserOptions() {
      compute_wintess = bounded_check = be_verbose = use_long_witnesses = compute_robustness = use_in_mask = use_out_mask = output_console = use_textfile = use_database = false;
      database_file = datatext_file = in_mask_file = out_mask_file = "";
      bound_size = INF;
      process_number = processes_count = 1;
      model_name = "";
   }

   inline bool robustness() const {
      return compute_robustness;
   }
   inline bool witnesses() const {
      return compute_wintess;
   }

   inline bool analysis() const {
      return (compute_robustness | compute_wintess);
   }

   inline bool longWit() const {
      return use_long_witnesses;
   }

   inline bool verbose() const {
      return be_verbose;
   }

   inline size_t procNum() const {
      return process_number;
   }

   inline size_t procCount() const {
      return processes_count;
   }

   inline string modelName() const {
      return model_name;
   }

   inline bool toConsole() const {
      return output_console;
   }

   inline bool toFile() const {
      return use_textfile;
   }

   inline bool toDatabase() const {
      return use_database;
   }

   inline bool isBounded() const {
      return bounded_check;
   }

   inline size_t getBoundSize() const {
      return bound_size;
   }

   inline size_t inputMask() const {
      return use_in_mask;
   }

   inline size_t outputMask() const {
      return use_out_mask;
   }

   inline const string & inMaskFile() const {
      return in_mask_file;
   }

   inline const string & outMaskFile() const {
      return out_mask_file;
   }

   inline const string & textFile() const {
      return datatext_file;
   }

   inline const string & dataFile() const {
      return database_file;
   }
} user_options; ///< Single program-shared user options object.

#endif // PARSYBONE_USER_OPTIONS_INCLUDED
