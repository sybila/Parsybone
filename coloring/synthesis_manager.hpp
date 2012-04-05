/*
 * Copyright (C) 2012 - Adam Streck
 *
 * This file is part of ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool
 *
 * ParSyBoNe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 *
 * This software has been created as a part of a research conducted in the Systems Biology Laboratory of Masaryk University Brno.
 * See http://sybila.fi.muni.cz/ .
 */

#ifndef PARSYBONE_SYNTHESIS_MANAGER_INCLUDED
#define PARSYBONE_SYNTHESIS_MANAGER_INCLUDED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class that shelters all of the synthesis and output of the results
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/product.hpp"
#include "../reforging/functions_structure.hpp"
#include "parameters_functions.hpp"
#include "../results/results.hpp"
#include "../results/output_manager.hpp"
#include "../auxiliary/output_streamer.hpp"

class SynthesisManager {
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const UserOptions & user_options; // Values provided as parameters
	const ParametrizedStructure & structure; // Stores info about KS states
	const AutomatonStructure & automaton; // Stores info about BA states
	const FunctionsStructure & functions;
	Product & product;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CREATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SynthesisManager(const SynthesisManager & other);            // Forbidden copy constructor.
	SynthesisManager& operator=(const SynthesisManager & other); // Forbidden assignment operator.

public:
	SynthesisManager(const UserOptions & _user_options, const ParametrizedStructure & _structure, const AutomatonStructure & _automaton, const FunctionsStructure & _functions,
		             Product & _product)
		            : user_options(_user_options), structure(_structure), automaton(_automaton), product(_product), functions(_functions) { }

	/**
	 * Main synthesis function that iterates through all the rounds of the synthesis
	 */
	void doSynthesis() {
		SplitManager split_manager(user_options.process_number, user_options.processes_count, structure.getParametersCount());
		Results results(structure, automaton, split_manager);
		ModelChecker model_checker(user_options, split_manager, structure, automaton, results, product);
		model_checker.computeResults();
		 // ModelChecker model_checker(product);

		/*while (true) {
			// Synthetize round
			doRound();
			// Get colors

			// Synthetize witnesses

			// Ouput params

			// Increase round
			if (!split_manager.lastRound())
				split_manager.increaseRound();
			else 
				break;
		}*/

		output_streamer.output(verbose, "Output started", OutputStreamer::important);
		OutputManager output_manager(user_options, results, functions, split_manager);
		output_manager.basicOutput();
	}

private:
	void doRound() {

	}
};

#endif