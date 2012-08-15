#ifndef PARSYBONE_FORMULAE_PARSER_INCLUDED
#define PARSYBONE_FORMULAE_PARSER_INCLUDED

#include "../auxiliary/data_types.hpp"

class FormulaeParser {
   static const std::size_t err_parent = 1;
   static const std::size_t err_symbols = 2;
   static const std::size_t err_notfound = 3;

   static void throwException(const std::string & formula, const std::size_t error) {
      std::string err_str;
      switch (error) {
         case 1:
            err_str = "wrong parenthesis placement";
          case 2:
            err_str = "invalid symbol appereance";
          case 3:
            err_str = "specified variable was not found in the list";
          case 4:
            err_str = "unknown error";
      }
      throw std::runtime_error(std::string("error while parsing a formula \"").append(formula).append("\" ").append(err_str));
   }

public:
   static bool resolve (const std::map<std::string, bool> & valuation, std::string formula) {
      bool negate = false;
      if (formula.front() == '!') {
         negate = true;
         formula = formula.substr(1);
      }

      if (formula.find("(") == std::string::npos) {
         auto variable = valuation.find(formula);
         if (variable == valuation.end()) throwException(formula, err_notfound);
         return (negate ? !variable->second : variable->second);
      }

      std::size_t symbol = 0, division = 0;
      bool is_or = false;
      bool is_and = false;
      int parent_count = 0;
      while (symbol < formula.size()) {
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
               if (parent_count == 1) {
                  if (is_or || is_and)
                     throwException(formula, err_parent);
                  is_or = true;
                  division = symbol;
               }
               break;

            case '&':
               if (parent_count == 1) {
                  if (is_or || is_and)
                     throwException(formula, err_parent);
                  is_and = true;
                  division = symbol;
               }
         }

         symbol++;
      }
      if (parent_count != 0 || !(is_or || is_and))
           throwException(formula, err_parent);

      std::string first = formula.substr(1, division - 1);
      std::string second = formula.substr(division + 1, formula.size() - division - 2);
      bool result = is_or ? resolve(valuation, first) | resolve(valuation, second) : resolve(valuation, first) | resolve(valuation, second);
      return negate ? !result : result;
   }
};

#endif // PARSYBONE_FORMULAE_PARSER_INCLUDED
