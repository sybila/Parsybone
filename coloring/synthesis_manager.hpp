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

#include "../auxiliary/output_streamer.hpp"
#include "../auxiliary/time_manager.hpp"
#include "parameters_functions.hpp"
#include "model_checker.hpp"
#include "../reforging/parametrized_structure.hpp"
#include "../reforging/automaton_structure.hpp"
#include "../reforging/product.hpp"
#include "../reforging/functions_structure.hpp"
#include "../results/results.hpp"
#include "../results/output_manager.hpp"


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
	SynthesisManager(const UserOptions & _user_options, const FunctionsStructure & _functions, Product & _product)
		            : user_options(_user_options), functions(_functions), structure(_product.getKS()), automaton(_product.getBA()), product(_product) { }

	/**
	 * Main synthesis function that iterates through all the rounds of the synthesis
	 */
	void doSynthesis() {
		// Pre-create data
		SplitManager split_manager(user_options.process_number, user_options.processes_count, structure.getParametersCount());
		Results results(product, split_manager);
		ModelChecker model_checker(user_options, split_manager, results, product);
		
		time_manager.startClock("coloring runtime");
		model_checker.computeResults();
		time_manager.ouputClock("coloring runtime");
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

		// Do output
		OutputManager output_manager(user_options, results, functions, split_manager);
		output_manager.basicOutput();
	}

private:
	void doRound() {

	}
};

#endif