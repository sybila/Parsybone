/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PRODUCT_INCLUDED
#define PARSYBONE_PRODUCT_INCLUDED

#include "../auxiliary/output_streamer.hpp"
#include "../construction/automaton_structure.hpp"
#include "../construction/unparametrized_structure.hpp"
#include "transition_system_interface.hpp"

/// Storing a single transition to neighbour state together with its transition function.
struct ProdTransitionion : public TransitionProperty {
	const TransConst & trans_const;

	ProdTransitionion(const StateID _target_ID, const TransConst & _trans_const)
		: TransitionProperty(_target_ID), trans_const(_trans_const) {}
};

/// State of the product - same as the state of UKS but put together with a BA state.
struct ProdState : public AutomatonStateProperty<ProdTransitionion> {
	const StateID KS_ID; ///< ID of an original KS state this one is built from
	const StateID BA_ID; ///< ID of an original BA state this one is built from
	const Levels & levels; ///< species_level[i] = activation level of specie i in this state
	vector<StateID> loops; ///< States with the Same KS ID, but different BA that are possible targets

	/// Simple filler, assigns values to all the variables
	ProdState(const StateID ID, const StateID _KS_ID, const StateID _BA_ID, const bool initial, const bool final, const Levels & _species_level)
		: AutomatonStateProperty<ProdTransitionion>(initial, final, ID), KS_ID(_KS_ID), BA_ID(_BA_ID), levels(_species_level) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Holds a product structure - the one that is used in coloring procedure.
///
/// This is the final step of construction - a structure that is acutally used during the computation. For simplicity, it copies data from its predecessors (BA and UKS).
/// @attention States of product are indexed as (BA_state_count * KS_state_ID + BA_state_ID) - e.g. if 3-state BA state ((1,0)x(1)) would be at position 3*1 + 1 = 4.
///
/// ProductStructure data can be set only from the ProductBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ProductStructure : public AutomatonInterface<ProdState>, public TSInterface<ProdState> {
	friend class ProductBuilder;
	UnparametrizedStructure structure;
	AutomatonStructure automaton;

	void createSubspace(const StateID BA_ID) {
		for (const StateID KS_ID : range(structure.getStateCount())) {
			StateID ID = getProductID(KS_ID, BA_ID);
			const Levels& levels = structure.getStateLevels(KS_ID);
			states.push_back(ProdState(ID, KS_ID, BA_ID, false, false, levels));
		}
	}

	// TODO remove labels from states that have no outgoing edges
	void relabel(const StateID BA_ID) {
		if (automaton.isInitial(BA_ID)) {
			for (const StateID KS_ID : range(structure.getStateCount())) {
				StateID ID = getProductID(KS_ID, BA_ID);
				// If there's a way to leave the state
				if ((states[ID].transitions.size() + states[ID].loops.size()) > 0) {
					initial_states.push_back(ID);
					states[ID].initial = true;
				}
			}
		}
		if (automaton.isFinal(BA_ID)) {
			for (const StateID KS_ID : range(structure.getStateCount())) {
				StateID ID = getProductID(KS_ID, BA_ID);
				// If there's a way to leave the state
				if ((states[ID].transitions.size() + states[ID].loops.size()) > 0 || (automaton.getMyType() == BA_finite)) {
					final_states.push_back(ID);
					states[ID].final = true;
				}
			}
		}
	}

	/**
	 * Add a new transition with all its values.
	 * @param ID	add data to the state with this IS
	 */
	inline void addTransition(const StateID ID, const StateID target_ID, const TransConst & constraints) {
		states[ID].transitions.push_back(ProdTransitionion(target_ID, constraints));
	}

	inline void addLoop(const StateID ID, const StateID loop) {
		states[ID].loops.push_back(loop);
	}

public:
	ProductStructure() = default;
	ProductStructure(UnparametrizedStructure _structure, AutomatonStructure _automaton) : structure(move(_structure)), automaton(move(_automaton)) {
		my_type = _automaton.getMyType();
	}
	ProductStructure(ProductStructure &&) = default;
	ProductStructure(const ProductStructure &) = delete;
	ProductStructure& operator=(const ProductStructure &) = delete;
	ProductStructure& operator=(ProductStructure && other) {
		structure = move(other.structure);
		automaton = move(other.automaton);
		states = move(other.states);
		my_type = other.my_type;
		initial_states = move(other.initial_states);
		final_states = move(other.final_states);
		return *this;
	}

	const inline UnparametrizedStructure & getStructure() const {
		return structure;
	}

	const inline AutomatonStructure & getAutomaton() const {
		return automaton;
	}

	inline StateID getProductID(const StateID KS_ID, const StateID BA_ID) const {
		return (BA_ID * structure.getStateCount() + KS_ID);
	}

	inline StateID getBAID(const StateID ID) const {
		return states[ID].BA_ID;
	}

	inline StateID getKSID(const StateID ID) const {
		return states[ID].KS_ID;
	}

	inline const vector<StateID> & getLoops(const StateID ID) const {
		return states[ID].loops;
	}

	const string getString(const StateID ID) const {
		string label = "(";

		for (const ActLevel lev : states[ID].levels)
			label += to_string(lev) + ",";

		label[label.length() - 1] = ';';
		label += to_string(getBAID(ID)) + ")";

		return label;
	}
};

#endif // PARSYBONE_PRODUCT_INCLUDED
