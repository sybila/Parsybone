#ifndef PARSYBONE_FORMULAE_PARSER_INCLUDED
#define PARSYBONE_FORMULAE_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"

#include "../boost/algorithm/string.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Class able to resolve any logical function in propositional logic.
///
/// This is a static helper class able of resolving any preposition logic formula.
/// Formula construction:
///   -# \f$tt\f$ (true) and \f$ff\f$ (false) are formulas representing true and false respectively,
///	-# any variable is a formula,
///   -# for \f$\varphi\f$ formula is \f$!\varphi\f$ formula,
///   -# for \f$\psi, \varphi\f$ formulas are \f$(\psi|\varphi)\f$, \f$(\psi\&\varphi)\f$ formulas representing logical disjunction and conjunction respectively,
///   -# nothing else is a formula.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FormulaeParser {
   enum error_val {err_parent, err_symbols, err_notfound}; ///< Enumeration denoting error type that occured during evaluation / parsing.

   /**
    * If an exceptional state is reached, this function is used to throw an exception with a text given by the formula and error type .
    *
    * @param error   numerical constant defining string to throw
    */
   static void throwException(const std::string & formula, const error_val error) {
      std::string err_str;
      switch (error) {
         case err_parent:
            err_str = "wrong parenthesis placement";
            break;
          case err_symbols:
            err_str = "invalid symbol appereance";
            break;
          case err_notfound:
            err_str = "specified variable was not found in the list";
            break;
          default:
            err_str = "unknown error";
            break;
      }
      throw std::runtime_error(std::string("Error while parsing a formula \"").append(formula).append("\": ").append(err_str));
   }

   /**
    * Reads the current formula and finds position of current outmost operator and its kind.
    */
   static void readFormula(const std::string & formula, bool & is_or, std::size_t & divisor_pos) {
      // Control variables
      bool is_set = false;
      int parent_count = 0;

      // Cycle through symbols in the formula
      for (std::size_t symbol = 0; symbol < formula.size(); symbol++) {
         switch (formula[symbol]) {
            case '(':
               parent_count++;
               break;

            case ')':
               parent_count--;
               if (parent_count < 0)
                  throwException(formula, err_parent);
               break;

            case '|':
               if (parent_count == 1) { // When parent count is equal to 1, operator that is found is used (see (A|B), (((A|B)&C)|D)..)
                  if (is_set)
                     throwException(formula, err_parent);
                  is_or = true; is_set = true;
                  divisor_pos = symbol;
               }
               break;

            case '&':
               if (parent_count == 1) { // When parent count is equal to 1, operator that is found is used (see (A|B), (((A|B)&C)|D)..)
                  if (is_set)
                     throwException(formula, err_parent);
                  is_or = false; is_set = true;
                  divisor_pos = symbol;
               }
               break;
         }
      }
      // Check if the operator was found and the parenthesis parity is ok
      if (parent_count != 0 || !is_set)
           throwException(formula, err_parent);
   }

public:
   /**
    * Function that returns valuation of the formula based on valuation of its variables.
    *
    * @param[in] valuation  map of variable valuations in the form (name, value)
    * @param[in] formula formula to resolve
    *
    * @return true iff valuation of the formula is true
    */
   static bool resolve (const std::map<std::string, bool> & valuation, std::string formula) {
      // If there is a ! symbol, negate the formula and remove it
      bool negate = false; bool result;

      boost::trim(formula);
      if (formula[0] == '!') {
         negate = true;
         formula = formula.substr(1);
      }
      boost::trim(formula);

      // Search for any operator, if not found, assume formula to be an atom and return its valuation
      if (formula.find("|") == std::string::npos && formula.find("&") == std::string::npos) {

         if (formula.compare("tt") == 0) {
            result = true;
         }
         else if (formula.compare("ff") == 0) {
            result = false;
         }
         else {
            auto variable = valuation.find(formula);
            if (variable == valuation.end())
               throwException(formula, err_notfound);
            result = variable->second;
         }
      }
      else {
         // Find position of the operator and its kind (or/and)
         bool is_or; std::size_t division_pos;
         readFormula(formula, is_or, division_pos);

         // Divide formula by the operator and remove its outer parenthesis, then descend recursivelly
         std::string first = formula.substr(1, division_pos - 1);
         std::string second = formula.substr(division_pos + 1, formula.size() - division_pos - 2);
         result = is_or ? resolve(valuation, first) | resolve(valuation, second) : resolve(valuation, first) & resolve(valuation, second);
      }
      // Return the value based on valuations of its parts
      return negate ? !result : result;
   }
};

#endif // PARSYBONE_FORMULAE_PARSER_INCLUDED
