/*
 * Copyright (C) 2012-2013 - Adam Streck
 * This file is a part of the ParSyBoNe (Parameter Synthetizer for Boolean Networks) verification tool.
 * ParSyBoNe is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License version 3.
 * ParSyBoNe is released without any warrany. See the GNU General Public License for more details. <http://www.gnu.org/licenses/>.
 * For affiliations see <http://www.mi.fu-berlin.de/en/math/groups/dibimath> and <http://sybila.fi.muni.cz/>.
 */

#ifndef PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
#define PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED

#include "PunyHeaders/common_functions.hpp"
#include "PunyHeaders/formulae_resolver.hpp"

#include "../auxiliary/data_types.hpp"
#include "../parsing/model.hpp"
#include "parametrizations_holder.hpp"
#include "parametrizations_helper.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class that computes feasible parametrizations for each specie from edge constrains and stores them in a ParametrizationHolder object.
///
/// Currently does not include constraint on self-loops.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParametrizationsBuilder {
    /**
     * Test all possible subcolors and saves valid.
     *
     * @param valid	data storage to save in
     * @param ID	ID of currently used specie
     * @param colors_num	how many colors are there together
     * @param bottom_color	low bound on possible contexts
     * @param top_color	top bound on possible contexts
     */
    static void testColors(const Model & model, const SpecieID ID, const Levels & bottom_color, const Levels & top_color,
                           ParametrizationsHolder & params, ParametrizationsHolder::SpecieColors & valid) {
        // Cycle through all possible subcolors for this specie
        Levels subcolor(bottom_color);

        // Cycle through all colors
        do {
            // Test if the parametrization satisfies constraints.
            if (!testSubparametrization(model, ID, subcolor))
                continue;
            valid.push_back(subcolor);
        } while (iterate(top_color, bottom_color, subcolor));

        if (valid.subcolors.empty())
            throw runtime_error(string("No valid parametrization found for the specie ").append(toString(ID)));

        // Add computed subcolors
        params.colors.push_back(valid);
    }

    /**
     * Test specific constrain on given color - this function checks both observability and the edge constrain.
     *
     * @param activating	a reference to the variable storing true iff the regulation has observable activating effect
     * @param inhibiting a reference to the variable storing true iff the regulation has observable inhibiting effect
     * @param ID	ID of the specie that undergoes the test
     * @param param_num	index of tested parameter
     * @param regul_num	index of regulation whose constrains are tested
     * @param subcolor	coloring for this specie that is tested
     *
     * @return	true if constrains are satisfied
     */
    static void testConstrains(const Model & model, const SpecieID target_ID, const size_t & param_num, const Model::Regulation & regul,
                               const vector<size_t> & subparam, bool & activating, bool & inhibiting ) {
        // Get reference data
        const auto & parameters = model.getParameters(target_ID);
        const StateID source_ID = regul.source;
        ActLevel threshold = parameters[param_num].requirements.find(source_ID)->second.front();
        if (threshold == 0)
            return;
        else
            threshold--;

        size_t compare_num = 0;
        while(compare_num < parameters.size()) {
            auto compare = parameters[compare_num];
            if (ParametrizationsHelper::isSubordinate(model, parameters[param_num], compare, target_ID, source_ID))
                break;
            else
                compare_num++;
        }

        // Assign regulation aspects
        activating |= subparam[param_num] > subparam[compare_num];
        inhibiting |= subparam[param_num] < subparam[compare_num];
    }

    /**
     * Tests if given subparametrization on a given specie can satisfy given requirements.
     *
     * @param ID	ID of the specie to test contexts in
     * @param subcolor	unique valuation of all regulatory contexts
     *
     * @return	true if the subparametrization is feasible
     */
    static bool testSubparametrization (const Model & model, const SpecieID ID, const vector<size_t> & subparam) {
        // get referecnces to Specie data
        const auto & regulations = model.getRegulations(ID);
        const auto & parameters = model.getParameters(ID);

        // Cycle through all species's regulators
        for (auto regul:regulations) {
            // Skip if there are no requirements (free label)
            if (regul.label.compare(Label::Free) == 0)
                continue;

            // Prepare variables storing info about observable effects of this component
            bool activating = false, inhibiting = false;
            // For each parameter containing the reugulator in parametrization control its satisfaction
            for (auto param_num:range(parameters.size())) {
                testConstrains(model, ID, param_num, regul, subparam, activating, inhibiting);
            }

            // Test obtained knowledge agains the label itself - return false if the label is not satisfied
            if (!ParametrizationsHelper::resolveLabel(activating, inhibiting, regul.label))
                return false;
        }

        // If everything has passed, return true
        return true;
    }

    /**
     * Compute and store boundaries on possible context values - used for iterations.
     *
     * @param ID	ID of currently used specie
     * @param bottom_color	low bound on possible contexts
     * @param top_color	top bound on possible contexts
     *
     * @return	how many colors are there together
     */
    static ParamNum getBoundaries(const Model & model, const SpecieID ID, vector<size_t> & bottom_color, vector<size_t> & top_color) {
        // Obtain all regulations
        auto parameters = model.getParameters(ID);
        ParamNum colors_num = 1;

        // Cycle through regulations
        for (auto param:parameters) {
            bottom_color.push_back(param.targets.front());
            top_color.push_back(param.targets.back());
            colors_num *= param.targets.size();
        }

        return colors_num;
    }

    /**
     * For this specie, test all possible subcolors (all valuations of this specie contexts) and store those that satisfy edge labels.
     *
     * @param specie used in this round
     */
    static void createKinetics(const Model & model, const SpecieID ID, ParametrizationsHolder & params) {
        // Data to fill
        ParametrizationsHolder::SpecieColors valid;
        valid.ID = ID;

        // Create boundaries for iteration
        vector<size_t> bottom_color, top_color;
        valid.possible_count = getBoundaries(model, ID, bottom_color, top_color);

        // Test all the subcolors and save feasible
        testColors(model, ID, bottom_color, top_color, params, valid);
    }

public:
    /**
     * Entry function of parsing, tests and stores subcolors for all the species.
     */
    static ParametrizationsHolder buildParametrizations(const Model & model) {
        ParametrizationsHolder params;

        // Cycle through species
        for (SpecieID ID = 0; ID < model.getSpeciesCount(); ID++)
            createKinetics(model, ID, params);

        return params;
    }
};

#endif // PARSYBONE_PARAMETRIZATIONS_BUILDER_INCLUDED
