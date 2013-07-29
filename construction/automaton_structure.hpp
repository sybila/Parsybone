/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED
#define PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED

#include "PunyHeaders/common_functions.hpp"

#include "../auxiliary/output_streamer.hpp"
#include "automaton_interface.hpp"

/// Single labelled transition from one state to another.
struct AutTransitionion : public TransitionProperty {
    Configurations allowed_values; ///< Allowed values of species for this transition.

    AutTransitionion(const StateID target_ID, Configurations _allowed_values)
        : TransitionProperty(target_ID), allowed_values(_allowed_values) {}  ///< Simple filler, assigns values to all the variables.
};

/// Storing a single state of the Buchi automaton. This state is extended with a value saying wheter the states is final.
struct AutState : public AutomatonStateProperty<AutTransitionion> {

    /// Fills data and checks if the state has value  -> is initial
    AutState(const StateID ID, const bool final, string && label)
        : AutomatonStateProperty<AutTransitionion>((ID == 0), final, ID, move(label)) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief A Buchi automaton designed to control some \f$\omega\f$-regular property.
///
/// AutomatonStructure stores Buchi automaton with edges labelled by values the KS can be in for the transition to be allowed.
/// AutomatonStructure data can be set only from the AutomatonStructureBuilder object.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AutomatonStructure : public AutomatonInterface<AutState> {
    friend class AutomatonBuilder;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FILLING METHODS (can be used only from AutomatonStructureBuilder)
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * Add a new transition - having a source, target and permitted values for each specie
     */
    inline void addTransition(const StateID source_state, const StateID target_state, Configurations & allowed_values) {
        states[source_state].transitions.push_back(AutTransitionion(target_state, allowed_values));
    }

    /**
     * @param final	if true than state with index equal to the one of this vector is final
     */
    inline void addState(const StateID ID, const bool final) {
        string label("(");
        label.append(toString(ID)).append(")");
        states.push_back(move(AutState(ID, final, move(label))));
        if (ID == 0)
            initial_states.push_back(ID);
        if (final)
            final_states.push_back(ID);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OTHER METHODS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /**
     * Checks if a transition of the BA is possible in the current state of a KS.
     *
     * @param ID	source state of the transition
     * @param transition_num	ordinal number of the transition
     * @param levels	current levels of species i.e. the state of the KS
     *
     * @return	true if the transition is feasible
     */
    bool isTransitionFeasible(const StateID ID, const size_t transition_num, const Levels & levels) const {
        const AutTransitionion & transition = states[ID].transitions[transition_num];

        for (size_t clause_num = 0; clause_num < transition.allowed_values.size(); clause_num++) {
            // Cycle through the sates
            for (size_t specie_num = 0; specie_num < transition.allowed_values[clause_num].size(); specie_num++) {
                // If you do not find current specie level between allowed, return false
                if (transition.allowed_values[clause_num][specie_num] != levels[specie_num])
                    break;
                else if (specie_num == (transition.allowed_values[clause_num].size() - 1))
                    return true;
            }
        }
        return false;
    }
};

#endif // PARSYBONE_AUTOMATON_STRUCTURE_INCLUDED
