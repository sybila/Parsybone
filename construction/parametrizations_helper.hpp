#ifndef PARAMETRIZATIONS_HELPER_HPP
#define PARAMETRIZATIONS_HELPER_HPP

#include "../parsing/model.hpp"
#include "../auxiliary/data_types.hpp"
#include <PunyHeaders/formulae_resolver.hpp>

class ParametrizationsHelper {

public:
    static bool isSubordinate(const Model & model, const Model::Parameter & current, const Model::Parameter & compare, const SpecieID target_ID, const SpecieID source_ID) {
        for (auto regul_ID:model.getRegulatorsIDs(target_ID)) {
            if (regul_ID != source_ID) {
                if (current.requirements.find(regul_ID)->second != compare.requirements.find(regul_ID)->second) {
                    return false;
                }
            }
            else if (current.requirements.find(regul_ID)->second.front() != compare.requirements.find(regul_ID)->second.back() + 1) {
                return false;
            }
        }
        return true;
    }


    /**
     * @param context regulatory context to test
     * @param ID	ID of the regulated specie
     *
     * @return true if the context denotes self-regulation
     */
    static bool isSelfRegulation(const Model & model, const vector<bool> & context, const StateID ID,  size_t & position) {
        position = INF;
        for (size_t reg_num = 0; reg_num < context.size(); reg_num++) {
            if (context[reg_num]) {
                if (position == INF) {
                    position = reg_num;
                }
                else {
                    position = INF;
                    return false;
                }
            }
        }

        if (position != INF && (model.getRegulations(ID))[ID].source == ID)
            return true;

        return false;
    }

    /**
     * Return true if the label (edge constrain) of the regulation is satisfied, false otherwise. All labels can be resolved based only on whether mon+ and mon- are true.
     * @param	activating	true if the parametrization satisfies +
     * @param	inhibiting	true if the parametrization satisfies -
     * @param	label	canonical form of edge label given as a string
     *
     * @return	true if the edge constrain is satisfied
     */
    static bool resolveLabel(const bool & activating, const bool & inhibiting, const string label) {
        // Fill the atomic propositions
        FormulaeResolver::Vals values;
        values.insert(FormulaeResolver::Val("+", activating));
        values.insert(FormulaeResolver::Val("-", inhibiting));

        string formula;

        // Find the constrain and return its valuation
        if (label.compare(Label::Activating) == 0)
            formula = "+";
        else if (label.compare(Label::ActivatingOnly) == 0)
            formula = "(+ & !-)";
        else if (label.compare(Label::Inhibiting) == 0)
            formula = "-";
        else if (label.compare(Label::InhibitingOnly) == 0)
            formula = "(- & !+)";
        else if (label.compare(Label::NotActivating) == 0)
            formula = "!+";
        else if (label.compare(Label::NotInhibiting) == 0)
            formula = "!-";
        else if (label.compare(Label::Observable) == 0)
            formula = "(+ | -)";
        else if (label.compare(Label::NotObservable) == 0)
            formula = "!(+ | -)";
        else
            formula = label;

        return (FormulaeResolver::resolve(values, formula));
    }
};

#endif // PARAMETRIZATIONS_HELPER_HPP
