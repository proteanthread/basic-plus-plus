#include "help.h"

const HelpEntry help_db_oper[] = {
  { "AND", "Logical/bitwise AND operator", "IF A>0 AND B>0 THEN ...", HCAT_OPER },
  { "EQV", "Logical equivalence operator", "PRINT 5 EQV 3", HCAT_OPER },
  { "IMP", "Logical implication operator", "PRINT 5 IMP 3", HCAT_OPER },
  { "MOD", "Modulo (remainder) operator", "PRINT 10 MOD 3 ' prints 1", HCAT_OPER },
  { "NOT", "Logical/bitwise NOT operator", "IF NOT A THEN ...", HCAT_OPER },
  { "OR", "Logical/bitwise OR operator", "IF A=0 OR B=0 THEN ...", HCAT_OPER },
  { "XOR", "Bitwise exclusive OR", "PRINT 5 XOR 3 ' prints 6", HCAT_OPER },
  { NULL, NULL, NULL, 0 }
};
