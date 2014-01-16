/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PRODUCT_BUILDER_INCLUDED
#define PARSYBONE_PRODUCT_BUILDER_INCLUDED

#include "product_structure.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a final ProductStructure that is used as a template for the synthesis procedure, as a product of ParametrizedStructure and AutomatonStructure.
///
/// ProductBuilder creates the an automaton corresponding to the synchronous product of BA and KS.
/// @attention States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProductBuilder {

	/**
	 * Create state of the product as a combination of a single BA and a single UKS state
	 * @param BA_ID	source in the BA
	 * @param transition_count	value which counts the transition for the whole product, will be filled
	 */
	void addSubspaceTransitions(const StateID BA_ID, ProductStructure & product) const {
		const UnparametrizedStructure & structure = product.getStructure();
		const AutomatonStructure & automaton = product.getAutomaton();

		for (const size_t trans_no : range(automaton.getTransitionCount(BA_ID))) {
			StateID BA_target = automaton.getTargetID(BA_ID, trans_no);
			DFS<ConstraintParser> search(automaton.getTransitionConstraint(BA_ID, trans_no));

			// List through the states that are allowed by the constraint
			while (ConstraintParser *result = search.next()) {
				StateID KS_ID = structure.getID(result->getSolution());
				StateID ID = product.getProductID(KS_ID, BA_ID);
				delete result;

				// Add all the trasient combinations for the kripke structure
				if (!automaton.isStableRequired(BA_ID, trans_no)) {
					for (const size_t trans_no : range(structure.getTransitionCount(KS_ID))) {
						const StateID KS_target = product.getStructure().getTargetID(KS_ID, trans_no);
						const TransConst & trans_const = product.getStructure().getTransitionConst(KS_ID, trans_no);
						product.addTransition(ID, product.getProductID(KS_target, BA_target), trans_const);
					}
				}
				// Add a self-loop
				if (!automaton.isTransientRequired(BA_ID, trans_no)) {
					product.addLoop(ID, product.getProductID(KS_ID, BA_target));
				}
			}
		}
	}

public:
	/**
	 * Create the the synchronous product of the provided BA and UKS.
	 */
	ProductStructure buildProduct(UnparametrizedStructure  _structure, AutomatonStructure  _automaton) const {
		ProductStructure product(move(_structure), move(_automaton));

		// Creates states and their transitions
		for (size_t BA_ID = 0; BA_ID < product.getAutomaton().getStateCount(); BA_ID++) {
			output_streamer.output(verbose_str, "Building product subspace: " + to_string(BA_ID + 1) + "/" + to_string(product.getAutomaton().getStateCount()) + ".",
				OutputStreamer::no_newl | OutputStreamer::rewrite_ln);
			product.createSubspace(BA_ID);
			addSubspaceTransitions(BA_ID, product);
			product.relabel(BA_ID);
		}

		output_streamer.clear_line(verbose_str);

		return product;
	}
};

#endif // PARSYBONE_PRODUCT_BUILDER_INCLUDED
