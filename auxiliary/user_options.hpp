/*
 * Copyright (C) 2012 - Adam Streck
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 * ParSyBoNe is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno. See http://sybila.fi.muni.cz/ .
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
   friend class ArgumentParser;
   friend class ModelParser;
   friend class ParsingManager;
   friend class OutputStreamer;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // OPTIONS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool be_verbose; ///< Display data from verbose stream (verbose_str), mainly progress information
   bool display_stats; ///< Display data from statistics stream (stats_str), mainly states and transition counts
   bool time_series; ///< Work with the property as with a time series (meaning check only reachability property)
   bool compute_robustness; ///< Should robustness value be computed and displyed?
   bool compute_wintess; ///< Should witnesses be computed and displayed?
   bool use_long_witnesses; ///< How witnesses should be displayed - complete state or only ID?
   bool use_in_mask; ///< Is there a paramset mask on the input?
   bool use_out_mask; ///< Should computed parametrizations be output in the form of mask?
   bool output_console;
   bool output_file;
   bool output_database;
   size_t process_number; ///< What is the ID of this process?
   size_t processes_count; ///< How many processes are included in the computation?
   string model_path;
   string model_name; ///< What is the name of the model?
   string database_file;
   string datatext_file;
   string in_mask_file;
   string out_mask_file;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CONSTRUCTION METHODS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   UserOptions(const UserOptions & other); ///< Forbidden copy constructor.
   UserOptions& operator=(const UserOptions & other); ///< Forbidden assignment operator.

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

public:
   /**
    * Constructor, sets up default values.
    */
   UserOptions() {
      compute_wintess = be_verbose = display_stats = time_series = use_long_witnesses = compute_robustness = use_in_mask = use_out_mask = output_console = output_file = output_database = false;
      database_file = datatext_file = in_mask_file = out_mask_file = "";
      process_number = processes_count = 1;
      model_name = "";
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // CONSTANT GETTERS
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /**
    * @return	true if compute_robustness (robustness output is requested)
    */
   inline bool robustness() const {
      return compute_robustness;
   }

   /**
    * @return	true if witnesses are to be computed
    */
   inline bool witnesses() const {
      return compute_wintess;
   }

   /**
    * @return	true if additional analysis will be computed (witnesses/robustness)
    */
   inline bool analysis() const {
      return (compute_robustness | compute_wintess);
   }

   /**
    * @return	true if use_long_witnesses is set (display state levels instead of just a number)
    */
   inline bool longWit() const {
      return use_long_witnesses;
   }

   /**
    * @return	true if verbose is set (displaying additional information during computation)
    */
   inline bool verbose() const {
      return be_verbose;
   }

   /**
    * @return	true if display_stats is set (displaying statistics of the model)
    */
   inline bool stats() const {
      return display_stats;
   }

   /**
    * @return	true if property is a time series
    */
   inline bool timeSeries() const {
      return time_series;
   }

   /**
    * @return	number of this process in distributed computation (indexed from 1)
    */
   inline size_t procNum() const {
      return process_number;
   }

   /**
    * @return	total number of processes in distributed computation
    */
   inline size_t procCount() const {
      return processes_count;
   }

   /**
    * @return  name of the model being used
    */
   inline string modelName() const {
      return model_name;
   }

   /**
    * @return  true if results are sent on-screen;
    */
   inline bool toConsole() const {
      return output_console;
   }

   /**
    * @return  true if results are sent on-screen;
    */
   inline bool toFile() const {
      return output_file;
   }

   /**
    * @return  true if results are sent on-screen;
    */
   inline bool toDatabase() const {
      return output_database;
   }

   /**
    * @return	true if the input mask was provided
    */
   inline size_t inputMask() const {
      return use_in_mask;
   }

   /**
    * @return	true if the mask of computation should be printed
    */
   inline size_t outputMask() const {
      return use_out_mask;
   }
} user_options; ///< Single program-shared user options object.

#endif // PARSYBONE_USER_OPTIONS_INCLUDED
