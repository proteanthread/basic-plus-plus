THE BASIC++ USER-DEFINED TYPE SYSTEM
=======================================
Version 4.2.3

This manual explains how to define and use custom data
types in BASIC++ — structured records with named fields,
modeled after QBasic's TYPE...END TYPE.

Unlike a quick reference, this manual gives you deep
understanding.  By the time you finish reading it, you will
know:

  - How TYPE...END TYPE defines a new record structure
  - How to create typed variables and access their fields
  - How the type system is stored in the runtime
  - How numeric and string fields work together
  - How to combine types with arrays and subroutines
  - How user-defined types compare to classic BASIC data
  - How to build real programs with structured data
  - What the limits and design trade-offs are


TABLE OF CONTENTS
=================

  Part I:   Why User-Defined Types?
  1.  The Problem: Data Without Structure
  2.  The Solution: TYPE...END TYPE
  3.  Comparison With QBasic
  4.  Design Limits

  Part II:  Defining Types
  5.  TYPE Statement
  6.  Numeric Fields
  7.  String Fields
  8.  Mixed Fields
  9.  END TYPE
  10. Where TYPE Definitions Go in Your Program
  11. Type Naming Rules

  Part III: Using Typed Variables
  12. DIM ... AS TypeName
  13. Accessing Fields: The Dot Operator
  14. Setting Field Values
  15. Reading Field Values
  16. Printing Typed Variables
  17. Copying Typed Variables

  Part IV:  Types With Other Features
  18. Arrays of Typed Variables
  19. Typed Variables in SUB/FUNCTION
  20. Typed Variables With File I/O
  21. Typed Variables With DATA/READ
  22. Typed Variables and SELECT CASE
  23. Sorting Arrays of Records

  Part V:   How It Works Internally
  24. The UserTypeDef Structure
  25. The UserTypeField Structure
  26. The TypedVar Structure
  27. Storage Layout in RuntimeState
  28. Capacity Limits (config.h)
  29. Field Access: Name Lookup

  Part VI:  Practical Examples
  30. Example: Address Book
  31. Example: Inventory System
  32. Example: Student Grade Tracker
  33. Example: Game Entity System
  34. Example: Configuration File Parser

  Part VII: Error Reference and Best Practices
  35. Error Codes
  36. Common Mistakes
  37. Best Practices
  38. Quick Reference


======================================================================
Part I:  WHY USER-DEFINED TYPES?
======================================================================


1.  THE PROBLEM: DATA WITHOUT STRUCTURE
------------------------------------------

In classic BASIC, related data lives in separate
parallel arrays:

  100 REM -- An address book without types --
  110 DIM Name$(100)
  120 DIM Phone$(100)
  130 DIM Street$(100)
  140 DIM City$(100)
  150 DIM State$(100)
  160 DIM Zip$(100)
  170 DIM Age%(100)

This approach has serious problems:

  1. No logical grouping — the fields of one "record"
     are scattered across seven arrays
  2. Easy to desynchronize — deleting Name$(5) without
     deleting Phone$(5) corrupts the data
  3. Hard to pass around — you can't pass "one contact"
     to a subroutine; you pass 7 separate values
  4. Hard to read — code like Age%(I%) doesn't clearly
     say "the age of contact I"
  5. Hard to extend — adding a field means adding
     another parallel array everywhere


2.  THE SOLUTION: TYPE...END TYPE
------------------------------------

User-defined types solve all five problems:

  100 TYPE Contact
  110   Name AS STRING
  120   Phone AS STRING
  130   Street AS STRING
  140   City AS STRING
  150   State AS STRING
  160   Zip AS STRING
  170   Age AS INTEGER
  180 END TYPE

Now all seven fields belong to a single named structure.
You create variables of this type:

  200 DIM C AS Contact
  210 C.Name = "Alice Smith"
  220 C.Phone = "(555) 867-5309"
  230 C.Age = 30
  240 PRINT C.Name; " is "; C.Age; " years old"

And arrays of typed records:

  300 DIM Contacts(100) AS Contact
  310 Contacts(1).Name = "Bob"
  320 Contacts(1).Age = 25

The data is logically grouped, self-documenting, and
impossible to desynchronize.


3.  COMPARISON WITH QBASIC
-----------------------------

BASIC++ user-defined types are modeled after QBasic 4.5
with some simplifications:

  Feature              QBasic          BASIC++
  ───────────────────  ──────────────  ──────────────
  TYPE...END TYPE      Yes             Yes
  Numeric fields       INTEGER, LONG,  INTEGER
                       SINGLE, DOUBLE  (all numeric =
                                        long integer)
  String fields        STRING * N      STRING
                       (fixed-length)  (dynamic, max
                                        255 chars)
  Nested types         Yes             No (future)
  Arrays in types      Yes             No (future)
  Passing to SUB       By reference    By field copy
  Random file I/O      Yes (FIELD)     Future
  Max types            Unlimited       16
  Max fields/type      Unlimited       16
  Max typed vars       Unlimited       64

The simplifications keep the implementation small and
C89-compatible while covering the most common use cases.


4.  DESIGN LIMITS
-------------------

From config.h:

  Constant            Value   Meaning
  ──────────────────  ──────  ──────────────────────────
  MAX_USER_TYPES      64      Maximum TYPE definitions
  MAX_TYPE_FIELDS     64      Maximum fields per TYPE
  MAX_TYPED_VARS      512     Maximum typed variable
                              instances (total)
  MAX_VAR_NAME_LEN    31      Maximum name length for
                              types, fields, and vars

These limits are compile-time constants.  If you need
more, increase them in config.h and recompile.

Each typed variable stores its field values as an array
of BValue (tagged union) values.  A BValue can hold an
integer, a float, or a string.  This means:

  - Numeric fields use 12-16 bytes each (tag + long)
  - String fields use 12-16 bytes each (tag + pointer)
  - One typed variable ≈ MAX_TYPE_FIELDS × sizeof(BValue)
  - 64 typed variables ≈ 16 KB (well within budget)


======================================================================
Part II:  DEFINING TYPES
======================================================================


5.  TYPE STATEMENT
--------------------

Syntax:

  TYPE TypeName
    FieldName1 AS FieldType
    FieldName2 AS FieldType
    ...
  END TYPE

The TYPE block defines a new data structure.  Each field
declaration is on its own line.


6.  NUMERIC FIELDS
--------------------

  TYPE Point
    X AS INTEGER
    Y AS INTEGER
  END TYPE

Numeric fields store long integer values.  They default
to 0 when the variable is created.

"AS INTEGER" is the standard declaration.  Any type name
other than STRING is treated as numeric:

  TYPE Score
    Points AS INTEGER
    Level AS INTEGER
    Lives AS INTEGER
  END TYPE


7.  STRING FIELDS
-------------------

  TYPE Person
    Name AS STRING
    Email AS STRING
  END TYPE

String fields store dynamic-length strings up to 255
characters (MAX_STRING_LENGTH).  They default to ""
(empty string) when the variable is created.

Unlike QBasic's fixed-length strings (STRING * 20),
BASIC++ strings grow and shrink dynamically.  All string
data is allocated from the string pool.


8.  MIXED FIELDS
------------------

Types can mix numeric and string fields freely:

  TYPE Employee
    Name AS STRING
    Department AS STRING
    Salary AS INTEGER
    YearsWorked AS INTEGER
    Active AS INTEGER
  END TYPE

The is_string flag in UserTypeField determines how each
field is stored and accessed:

  Field Type   is_string   Default Value
  ───────────  ──────────  ──────────────
  INTEGER      0           0
  STRING       1           "" (empty)


9.  END TYPE
--------------

Every TYPE block must be closed with END TYPE:

  TYPE Rectangle
    X AS INTEGER
    Y AS INTEGER
    Width AS INTEGER
    Height AS INTEGER
  END TYPE

END TYPE tells the parser to stop scanning for fields.
If END TYPE is missing, the parser scans to the end of
the program, which is almost certainly not what you want.


10.  WHERE TYPE DEFINITIONS GO IN YOUR PROGRAM
-------------------------------------------------

TYPE definitions should be placed at the beginning of
your program, before any code that uses them:

  100 REM ==============================
  110 REM  Type definitions
  120 REM ==============================
  130 TYPE Enemy
  140   Name AS STRING
  150   HP AS INTEGER
  160   Attack AS INTEGER
  170   Defense AS INTEGER
  180 END TYPE
  190 REM
  200 REM ==============================
  210 REM  Main program starts here
  220 REM ==============================
  230 DIM Boss AS Enemy
  240 Boss.Name = "Dragon"
  250 Boss.HP = 500
  260 Boss.Attack = 40
  270 Boss.Defense = 25
  280 PRINT Boss.Name; " has "; Boss.HP; " HP"

TYPE definitions are processed at runtime when the line
containing TYPE is executed.  The parser scans forward
from the TYPE line, collecting field definitions, until
it finds END TYPE.  Execution then continues after the
END TYPE line.


11.  TYPE NAMING RULES
------------------------

Type names follow the same rules as variable names:

  - Up to 31 characters (MAX_VAR_NAME_LEN)
  - Letters and digits (A-Z, 0-9)
  - Case-insensitive (Contact = CONTACT = contact)
  - Must not conflict with BASIC keywords

Good type names:

  TYPE Contact      ' Person or organization
  TYPE GameState    ' Current game state
  TYPE MenuItem     ' Menu entry
  TYPE SensorData   ' Hardware sensor reading
  TYPE FileRecord   ' One record in a data file

Bad type names:

  TYPE FOR          ' Keyword collision!
  TYPE IF           ' Keyword collision!
  TYPE A            ' Too short, unclear


======================================================================
Part III:  USING TYPED VARIABLES
======================================================================


12.  DIM ... AS TypeName
--------------------------

Once a TYPE is defined, create variables of that type
using DIM ... AS:

  100 TYPE Color
  110   R AS INTEGER
  120   G AS INTEGER
  130   B AS INTEGER
  140 END TYPE
  150 REM
  160 DIM ForegroundColor AS Color
  170 DIM BackgroundColor AS Color

This allocates space for the typed variable and sets all
fields to their default values (0 for numeric, "" for
string).

You can also DIM arrays of typed variables:

  200 DIM Palette(256) AS Color

This creates 256 Color records, each with R, G, and B
fields.


13.  ACCESSING FIELDS: THE DOT OPERATOR
-------------------------------------------

Access individual fields using the dot (.) operator:

  100 TYPE Vec2
  110   X AS INTEGER
  120   Y AS INTEGER
  130 END TYPE
  140 DIM Pos AS Vec2
  150 REM -- Set fields --
  160 Pos.X = 100
  170 Pos.Y = 200
  180 REM -- Read fields --
  190 PRINT "Position: ("; Pos.X; ","; Pos.Y; ")"

The dot operator works like QBasic:

  variable.field

Field names are case-insensitive (Pos.X = Pos.x = POS.X).


14.  SETTING FIELD VALUES
----------------------------

Assign values to fields using the standard = operator:

  100 TYPE Config
  110   Title AS STRING
  120   Width AS INTEGER
  130   Height AS INTEGER
  140   Fullscreen AS INTEGER
  150 END TYPE
  160 DIM Cfg AS Config
  170 REM -- Set all fields --
  180 Cfg.Title = "My BASIC++ App"
  190 Cfg.Width = 800
  200 Cfg.Height = 600
  210 Cfg.Fullscreen = 0

You can use expressions:

  300 Cfg.Width = 640 + 160
  310 Cfg.Height = Cfg.Width * 3 / 4
  320 Cfg.Title = "App v" + STR$(VERSION%)


15.  READING FIELD VALUES
----------------------------

Fields can be used anywhere a value is expected:

  100 TYPE Player
  110   Name AS STRING
  120   HP AS INTEGER
  130   MaxHP AS INTEGER
  140   Level AS INTEGER
  150 END TYPE
  160 DIM P AS Player
  170 P.Name = "Hero"
  180 P.HP = 85
  190 P.MaxHP = 100
  200 P.Level = 5
  210 REM -- Use in expressions --
  220 PERCENT% = (P.HP * 100) / P.MaxHP
  230 PRINT P.Name; " (Lv."; P.Level; ")"
  240 PRINT "HP: "; P.HP; "/"; P.MaxHP;
  250 PRINT " ("; PERCENT%; "%)"
  260 REM -- Use in conditions --
  270 IF P.HP < P.MaxHP / 4 THEN
  280   PRINT "WARNING: Low health!"
  290 END IF


16.  PRINTING TYPED VARIABLES
--------------------------------

You cannot PRINT an entire typed variable at once.
Instead, print each field individually:

  100 REM -- Print a contact --
  110 SUB PrintContact(C AS Contact)
  120   PRINT "Name:    "; C.Name
  130   PRINT "Phone:   "; C.Phone
  140   PRINT "City:    "; C.City; ", "; C.State
  150   PRINT "Age:     "; C.Age
  160 END SUB

For formatted output, use PRINT USING with individual
fields:

  200 PRINT USING "Name: \                    \"; P.Name
  210 PRINT USING "HP:   ####/####"; P.HP; P.MaxHP


17.  COPYING TYPED VARIABLES
-------------------------------

Copy all fields from one typed variable to another:

  100 TYPE Card
  110   Suit AS STRING
  120   Value AS INTEGER
  130 END TYPE
  140 DIM A AS Card, B AS Card
  150 A.Suit = "Hearts"
  160 A.Value = 10
  170 REM -- Copy all fields --
  180 B.Suit = A.Suit
  190 B.Value = A.Value
  200 PRINT B.Suit; " "; B.Value

Note: BASIC++ does not currently support whole-record
assignment (B = A).  You must copy field by field.  A
helper SUB makes this cleaner:

  300 SUB CopyCard(Src AS Card, Dst AS Card)
  310   Dst.Suit = Src.Suit
  320   Dst.Value = Src.Value
  330 END SUB


======================================================================
Part IV:  TYPES WITH OTHER FEATURES
======================================================================


18.  ARRAYS OF TYPED VARIABLES
---------------------------------

The most powerful use of types is with arrays — you get
a database-like table of records:

  100 TYPE Item
  110   Name AS STRING
  120   Price AS INTEGER
  130   Qty AS INTEGER
  140 END TYPE
  150 DIM Inventory(50) AS Item
  160 REM
  170 REM -- Populate some items --
  180 Inventory(1).Name = "Health Potion"
  190 Inventory(1).Price = 50
  200 Inventory(1).Qty = 10
  210 Inventory(2).Name = "Mana Potion"
  220 Inventory(2).Price = 75
  230 Inventory(2).Qty = 5
  240 Inventory(3).Name = "Iron Sword"
  250 Inventory(3).Price = 200
  260 Inventory(3).Qty = 1
  270 REM
  280 REM -- List all items --
  290 PRINT "INVENTORY"
  300 PRINT "──────────────────────────────────"
  310 FOR I% = 1 TO 3
  320   PRINT Inventory(I%).Name;
  330   PRINT TAB(20); "$"; Inventory(I%).Price;
  340   PRINT TAB(28); "x"; Inventory(I%).Qty
  350 NEXT I%
  360 REM
  370 REM -- Calculate total value --
  380 TOTAL% = 0
  390 FOR I% = 1 TO 3
  400   TOTAL% = TOTAL% + (Inventory(I%).Price * _
                            Inventory(I%).Qty)
  410 NEXT I%
  420 PRINT "──────────────────────────────────"
  430 PRINT "Total value: $"; TOTAL%


19.  TYPED VARIABLES IN SUB/FUNCTION
---------------------------------------

Pass typed variables to subroutines and functions:

  100 TYPE Monster
  110   Name AS STRING
  120   HP AS INTEGER
  130   Attack AS INTEGER
  140 END TYPE
  150 REM
  160 SUB AttackMonster(M AS Monster, Damage AS INTEGER)
  170   M.HP = M.HP - Damage
  180   IF M.HP < 0 THEN M.HP = 0
  190   PRINT M.Name; " takes "; Damage; " damage!";
  200   PRINT "  ("; M.HP; " HP remaining)"
  210 END SUB
  220 REM
  230 FUNCTION IsAlive(M AS Monster) AS INTEGER
  240   IsAlive = (M.HP > 0)
  250 END FUNCTION
  260 REM
  270 DIM Goblin AS Monster
  280 Goblin.Name = "Goblin"
  290 Goblin.HP = 30
  300 Goblin.Attack = 5
  310 CALL AttackMonster(Goblin, 12)
  320 CALL AttackMonster(Goblin, 15)
  330 IF IsAlive(Goblin) THEN
  340   PRINT Goblin.Name; " is still alive!"
  350 ELSE
  360   PRINT Goblin.Name; " has been defeated!"
  370 END IF


20.  TYPED VARIABLES WITH FILE I/O
--------------------------------------

Save and load typed records to/from files:

  100 TYPE HighScore
  110   Name AS STRING
  120   Score AS INTEGER
  130   Level AS INTEGER
  140 END TYPE
  150 REM
  160 DIM Scores(10) AS HighScore
  170 REM -- ... populate scores ... --
  180 REM
  190 REM -- Save to file --
  200 OPEN "scores.dat" FOR OUTPUT AS #1
  210 FOR I% = 1 TO 10
  220   PRINT #1, Scores(I%).Name
  230   PRINT #1, Scores(I%).Score
  240   PRINT #1, Scores(I%).Level
  250 NEXT I%
  260 CLOSE #1
  270 REM
  280 REM -- Load from file --
  290 OPEN "scores.dat" FOR INPUT AS #1
  300 FOR I% = 1 TO 10
  310   LINE INPUT #1, Scores(I%).Name
  320   INPUT #1, Scores(I%).Score
  330   INPUT #1, Scores(I%).Level
  340 NEXT I%
  350 CLOSE #1


21.  TYPED VARIABLES WITH DATA/READ
---------------------------------------

Populate typed variables from DATA statements:

  100 TYPE Planet
  110   Name AS STRING
  120   Diameter AS INTEGER
  130   Moons AS INTEGER
  140 END TYPE
  150 DIM Planets(8) AS Planet
  160 FOR I% = 1 TO 8
  170   READ Planets(I%).Name
  180   READ Planets(I%).Diameter
  190   READ Planets(I%).Moons
  200 NEXT I%
  210 REM
  220 PRINT "THE SOLAR SYSTEM"
  230 PRINT "────────────────────────────────────"
  240 FOR I% = 1 TO 8
  250   PRINT Planets(I%).Name;
  260   PRINT TAB(12); Planets(I%).Diameter; " km";
  270   PRINT TAB(24); Planets(I%).Moons; " moons"
  280 NEXT I%
  290 REM
  900 DATA "Mercury", 4879, 0
  910 DATA "Venus", 12104, 0
  920 DATA "Earth", 12756, 1
  930 DATA "Mars", 6792, 2
  940 DATA "Jupiter", 142984, 95
  950 DATA "Saturn", 120536, 146
  960 DATA "Neptune", 49528, 16
  970 DATA "Uranus", 51118, 28


22.  TYPED VARIABLES AND SELECT CASE
----------------------------------------

Use fields in SELECT CASE statements:

  100 TYPE Command
  110   Name AS STRING
  120   Code AS INTEGER
  130 END TYPE
  140 DIM Cmd AS Command
  150 Cmd.Name = "SAVE"
  160 Cmd.Code = 3
  170 REM
  180 SELECT CASE Cmd.Code
  190   CASE 1
  200     PRINT "NEW"
  210   CASE 2
  220     PRINT "LOAD"
  230   CASE 3
  240     PRINT "SAVE"
  250   CASE ELSE
  260     PRINT "UNKNOWN"
  270 END SELECT


23.  SORTING ARRAYS OF RECORDS
---------------------------------

Sort an array of typed records using bubble sort:

  100 TYPE Student
  110   Name AS STRING
  120   Grade AS INTEGER
  130 END TYPE
  140 DIM Students(5) AS Student
  150 REM -- Populate --
  160 Students(1).Name = "Alice" : Students(1).Grade = 92
  170 Students(2).Name = "Bob"   : Students(2).Grade = 85
  180 Students(3).Name = "Carol" : Students(3).Grade = 97
  190 Students(4).Name = "Dave"  : Students(4).Grade = 78
  200 Students(5).Name = "Eve"   : Students(5).Grade = 91
  210 REM
  220 REM -- Sort by grade (descending) --
  230 FOR I% = 1 TO 4
  240   FOR J% = 1 TO 5 - I%
  250     IF Students(J%).Grade < Students(J%+1).Grade THEN
  260       REM Swap all fields
  270       TMPN$ = Students(J%).Name
  280       TMPG% = Students(J%).Grade
  290       Students(J%).Name = Students(J%+1).Name
  300       Students(J%).Grade = Students(J%+1).Grade
  310       Students(J%+1).Name = TMPN$
  320       Students(J%+1).Grade = TMPG%
  330     END IF
  340   NEXT J%
  350 NEXT I%
  360 REM
  370 REM -- Display sorted results --
  380 PRINT "GRADE REPORT (sorted)"
  390 PRINT "────────────────────────────────────"
  400 FOR I% = 1 TO 5
  410   PRINT I%; ". "; Students(I%).Name;
  420   PRINT TAB(16); Students(I%).Grade
  430 NEXT I%


======================================================================
Part V:  HOW IT WORKS INTERNALLY
======================================================================


24.  THE USERTYPEDEF STRUCTURE
---------------------------------

From runtime.h:

  typedef struct UserTypeDef {
      char name[MAX_VAR_NAME_LEN + 1];
      UserTypeField fields[MAX_TYPE_FIELDS];
      int  field_count;
  } UserTypeDef;

Each UserTypeDef represents one TYPE definition.  It
stores:

  - name: the type name (up to 31 chars, null-terminated)
  - fields[]: up to 64 field descriptors
  - field_count: how many fields are defined

The UserTypeDef array lives in RuntimeState:

  UserTypeDef user_types[MAX_USER_TYPES];  /* 64 slots */
  int         type_count;                  /* how many defined */


25.  THE USERTYPEFIELD STRUCTURE
-----------------------------------

From runtime.h:

  typedef struct UserTypeField {
      char name[MAX_VAR_NAME_LEN + 1];
      int  is_string;          /* 0=numeric, 1=string */
      int  nested_type_index;  /* -1=primitive, >=0=nested UserTypeDef */
  } UserTypeField;

Each field knows:

  - name: the field name (e.g., "Name", "HP", "X")
  - is_string: type discriminator
    - 0 = numeric (stored as BValue with VAL_INTEGER)
    - 1 = string (stored as BValue with VAL_STRING)
  - nested_type_index: index into user_types[] for nested types
    - -1 = primitive (numeric or string)
    - >=0 = this field is itself a user-defined type


26.  THE TYPEDVAR STRUCTURE
------------------------------

From runtime.h:

  typedef struct TypedVar {
      char name[MAX_VAR_NAME_LEN + 1];
      int  type_index;  /* index into user_types[] */
      BValue fields[MAX_TYPE_FIELDS];
  } TypedVar;

Each TypedVar is an instance — a variable of a user type.
It stores:

  - name: the variable name (e.g., "Player1", "Boss")
  - type_index: which UserTypeDef this variable uses
    (index into the user_types[] array)
  - fields[]: the actual field values, stored as BValues

Field values are indexed by position: fields[0] is the
first field, fields[1] is the second, etc.  The field
name lookup (player.HP -> index 2) happens at access time
by scanning the UserTypeDef's fields[] array.


26b.  TYPED VARIABLE COPYING
------------------------------

BASIC++ supports copying all field values from one typed
variable to another of the same type using LET assignment:

  DIM Player1 AS CharType
  DIM Player2 AS CharType
  Player1.Name = "Alice" : Player1.HP = 100
  LET Player2 = Player1     ' copy all fields

Internally, this calls runtime_copy_typed_var():

  int runtime_copy_typed_var(RuntimeState *rt,
      TypedVar *dst, TypedVar *src);

Both variables must have the same type_index (strict
same-type check).  Returns 0 on success, -1 on type
mismatch.


27.  STORAGE LAYOUT IN RUNTIMESTATE
---------------------------------------

The complete type system state in RuntimeState:

  /* User-defined types (TYPE...END TYPE) */
  UserTypeDef user_types[MAX_USER_TYPES];  /* 64 defs */
  int         type_count;                  /* # defined */

  /* Typed variable instances */
  TypedVar    typed_vars[MAX_TYPED_VARS];  /* 512 vars */
  int         typed_var_count;             /* # created */

Memory layout (approximate):

  user_types:
    64 × (32 name + 64 × (32 name + 4 is_string + 4 nested) + 4 count)
    ≈ 64 × 2596 = ~162 KB

  typed_vars:
    512 × (32 name + 4 index + 64 × sizeof(BValue))
    ≈ 512 × 1060 = ~530 KB

  Total: ~692 KB for the type system

This is allocated statically inside RuntimeState, so
there is no dynamic allocation for types.


28.  CAPACITY LIMITS (CONFIG.H)
----------------------------------

  Limit                Value   What Happens at Limit
  ───────────────────  ──────  ──────────────────────────
  MAX_USER_TYPES (64)  64      ERR_SORRY: "Out of memory"
  MAX_TYPE_FIELDS (64) 64      Extra fields silently
                               ignored (parser stops
                               collecting after 64)
  MAX_TYPED_VARS (512) 512     ERR_SORRY: "Out of memory"
  MAX_VAR_NAME_LEN     31      Names truncated to 31 chars

To increase these limits, edit config.h:

  #define MAX_USER_TYPES   128  /* was 64 */
  #define MAX_TYPE_FIELDS  128  /* was 64 */
  #define MAX_TYPED_VARS   1024 /* was 512 */

Then recompile.  The tradeoff is increased memory usage
in RuntimeState.


29.  FIELD ACCESS: NAME LOOKUP
---------------------------------

When the parser encounters "Player.HP", it:

  1. Looks up "Player" in the typed_vars[] table
     (linear scan, case-insensitive comparison)
  2. Finds the type_index (e.g., type_index = 0)
  3. Looks up "HP" in user_types[0].fields[]
     (linear scan, case-insensitive comparison)
  4. Gets the field index (e.g., field_index = 1)
  5. Reads/writes typed_vars[n].fields[1]

This is O(N×M) where N is the number of typed vars and
M is the number of fields.  With max 64 vars × 16 fields,
the worst case is 1024 comparisons — trivial for any
modern (or retro) CPU.


======================================================================
Part VI:  PRACTICAL EXAMPLES
======================================================================


30.  EXAMPLE: ADDRESS BOOK
----------------------------

  100 REM ======================================
  110 REM  Address Book
  120 REM ======================================
  130 TYPE Contact
  140   Name AS STRING
  150   Phone AS STRING
  160   Email AS STRING
  170   City AS STRING
  180 END TYPE
  190 REM
  200 CONST MAX_CONTACTS% = 50
  210 DIM Book(MAX_CONTACTS%) AS Contact
  220 COUNT% = 0
  230 REM
  240 DO
  250   CLS
  260   PRINT "╔══════════════════════════════╗"
  270   PRINT "║     Address Book v1.0        ║"
  280   PRINT "╠══════════════════════════════╣"
  290   PRINT "║  1. Add Contact              ║"
  300   PRINT "║  2. List Contacts            ║"
  310   PRINT "║  3. Search by Name           ║"
  320   PRINT "║  4. Delete Contact           ║"
  330   PRINT "║  0. Exit                     ║"
  340   PRINT "╚══════════════════════════════╝"
  350   PRINT
  360   INPUT "Choice: ", CH%
  370   SELECT CASE CH%
  380     CASE 1 : GOSUB 1000  : REM Add
  390     CASE 2 : GOSUB 2000  : REM List
  400     CASE 3 : GOSUB 3000  : REM Search
  410     CASE 4 : GOSUB 4000  : REM Delete
  420     CASE 0 : EXIT DO
  430   END SELECT
  440 LOOP
  450 END
  460 REM
  1000 REM -- Add Contact --
  1010 IF COUNT% >= MAX_CONTACTS% THEN
  1020   PRINT "Book is full!" : RETURN
  1030 END IF
  1040 COUNT% = COUNT% + 1
  1050 INPUT "Name:  ", Book(COUNT%).Name
  1060 INPUT "Phone: ", Book(COUNT%).Phone
  1070 INPUT "Email: ", Book(COUNT%).Email
  1080 INPUT "City:  ", Book(COUNT%).City
  1090 PRINT "Contact added."
  1100 SLEEP 1000
  1110 RETURN
  1120 REM
  2000 REM -- List Contacts --
  2010 PRINT "ALL CONTACTS ("; COUNT%; " total)"
  2020 PRINT "────────────────────────────────────"
  2030 FOR I% = 1 TO COUNT%
  2040   PRINT I%; ". "; Book(I%).Name;
  2050   PRINT " - "; Book(I%).Phone
  2060 NEXT I%
  2070 PRINT
  2080 INPUT "Press Enter...", DUMMY$
  2090 RETURN
  2100 REM
  3000 REM -- Search --
  3010 INPUT "Search name: ", SEARCH$
  3020 FOUND% = 0
  3030 FOR I% = 1 TO COUNT%
  3040   IF INSTR(UCASE$(Book(I%).Name), _
                  UCASE$(SEARCH$)) > 0 THEN
  3050     PRINT Book(I%).Name; " - "; Book(I%).Phone
  3060     PRINT "  Email: "; Book(I%).Email
  3070     PRINT "  City:  "; Book(I%).City
  3080     FOUND% = FOUND% + 1
  3090   END IF
  3100 NEXT I%
  3110 IF FOUND% = 0 THEN PRINT "No matches."
  3120 INPUT "Press Enter...", DUMMY$
  3130 RETURN
  3140 REM
  4000 REM -- Delete --
  4010 INPUT "Delete entry #: ", D%
  4020 IF D% < 1 OR D% > COUNT% THEN
  4030   PRINT "Invalid." : RETURN
  4040 END IF
  4050 REM Shift remaining entries down
  4060 FOR I% = D% TO COUNT% - 1
  4070   Book(I%).Name = Book(I%+1).Name
  4080   Book(I%).Phone = Book(I%+1).Phone
  4090   Book(I%).Email = Book(I%+1).Email
  4100   Book(I%).City = Book(I%+1).City
  4110 NEXT I%
  4120 COUNT% = COUNT% - 1
  4130 PRINT "Deleted."
  4140 SLEEP 1000
  4150 RETURN


31.  EXAMPLE: INVENTORY SYSTEM
---------------------------------

  100 REM ======================================
  110 REM  Shop Inventory Manager
  120 REM ======================================
  130 TYPE Product
  140   SKU AS STRING
  150   Name AS STRING
  160   Price AS INTEGER
  170   Stock AS INTEGER
  180   MinStock AS INTEGER
  190 END TYPE
  200 DIM Items(30) AS Product
  210 NITEM% = 0
  220 REM -- Seed with sample data --
  230 RESTORE 9000
  240 READ NITEM%
  250 FOR I% = 1 TO NITEM%
  260   READ Items(I%).SKU
  270   READ Items(I%).Name
  280   READ Items(I%).Price
  290   READ Items(I%).Stock
  300   READ Items(I%).MinStock
  310 NEXT I%
  320 REM -- Display with low-stock warnings --
  330 PRINT "INVENTORY REPORT"
  340 PRINT "══════════════════════════════════════"
  350 FOR I% = 1 TO NITEM%
  360   PRINT Items(I%).SKU; " ";
  370   PRINT Items(I%).Name;
  380   PRINT TAB(22); "$"; Items(I%).Price;
  390   PRINT TAB(30); "Qty:"; Items(I%).Stock;
  400   IF Items(I%).Stock < Items(I%).MinStock THEN
  410     PRINT " ** LOW **";
  420   END IF
  430   PRINT
  440 NEXT I%
  450 END
  460 REM
  9000 DATA 5
  9010 DATA "W001", "Widget", 10, 150, 50
  9020 DATA "G002", "Gadget", 25, 30, 40
  9030 DATA "S003", "Sprocket", 5, 200, 100
  9040 DATA "B004", "Bolt Pack", 3, 15, 25
  9050 DATA "N005", "Nut Pack", 2, 500, 100


32.  EXAMPLE: STUDENT GRADE TRACKER
---------------------------------------

  100 REM ======================================
  110 REM  Student Grade Tracker
  120 REM ======================================
  130 TYPE Student
  140   Name AS STRING
  150   ID AS INTEGER
  160   Test1 AS INTEGER
  170   Test2 AS INTEGER
  180   Test3 AS INTEGER
  190   Average AS INTEGER
  200   Grade AS STRING
  210 END TYPE
  220 DIM Class(30) AS Student
  230 NS% = 5
  240 REM -- Sample data --
  250 Class(1).Name = "Alice"   : Class(1).ID = 1001
  260 Class(1).Test1 = 92 : Class(1).Test2 = 88 : Class(1).Test3 = 95
  270 Class(2).Name = "Bob"     : Class(2).ID = 1002
  280 Class(2).Test1 = 78 : Class(2).Test2 = 82 : Class(2).Test3 = 80
  290 Class(3).Name = "Carol"   : Class(3).ID = 1003
  300 Class(3).Test1 = 95 : Class(3).Test2 = 97 : Class(3).Test3 = 99
  310 Class(4).Name = "Dave"    : Class(4).ID = 1004
  320 Class(4).Test1 = 65 : Class(4).Test2 = 70 : Class(4).Test3 = 68
  330 Class(5).Name = "Eve"     : Class(5).ID = 1005
  340 Class(5).Test1 = 88 : Class(5).Test2 = 91 : Class(5).Test3 = 85
  350 REM -- Calculate averages and letter grades --
  360 FOR I% = 1 TO NS%
  370   Class(I%).Average = (Class(I%).Test1 + _
                             Class(I%).Test2 + _
                             Class(I%).Test3) / 3
  380   A% = Class(I%).Average
  390   IF A% >= 90 THEN
  400     Class(I%).Grade = "A"
  410   ELSEIF A% >= 80 THEN
  420     Class(I%).Grade = "B"
  430   ELSEIF A% >= 70 THEN
  440     Class(I%).Grade = "C"
  450   ELSEIF A% >= 60 THEN
  460     Class(I%).Grade = "D"
  470   ELSE
  480     Class(I%).Grade = "F"
  490   END IF
  500 NEXT I%
  510 REM -- Display report --
  520 PRINT "STUDENT GRADE REPORT"
  530 PRINT "══════════════════════════════════════"
  540 PRINT "Name     ID    T1  T2  T3  Avg  Grade"
  550 PRINT "──────────────────────────────────────"
  560 FOR I% = 1 TO NS%
  570   PRINT Class(I%).Name;
  580   PRINT TAB(9); Class(I%).ID;
  590   PRINT TAB(15); Class(I%).Test1;
  600   PRINT TAB(19); Class(I%).Test2;
  610   PRINT TAB(23); Class(I%).Test3;
  620   PRINT TAB(27); Class(I%).Average;
  630   PRINT TAB(33); Class(I%).Grade
  640 NEXT I%
  650 END


33.  EXAMPLE: GAME ENTITY SYSTEM
-----------------------------------

  100 REM ======================================
  110 REM  Simple RPG Entity System
  120 REM ======================================
  130 TYPE Entity
  140   Name AS STRING
  150   HP AS INTEGER
  160   MaxHP AS INTEGER
  170   ATK AS INTEGER
  180   DEF AS INTEGER
  190   X AS INTEGER
  200   Y AS INTEGER
  210 END TYPE
  220 REM
  230 DIM Hero AS Entity
  240 DIM Enemies(5) AS Entity
  250 REM -- Set up hero --
  260 Hero.Name = "Warrior"
  270 Hero.HP = 100 : Hero.MaxHP = 100
  280 Hero.ATK = 15 : Hero.DEF = 10
  290 Hero.X = 5 : Hero.Y = 5
  300 REM -- Set up enemies --
  310 Enemies(1).Name = "Slime"
  320 Enemies(1).HP = 20 : Enemies(1).MaxHP = 20
  330 Enemies(1).ATK = 3 : Enemies(1).DEF = 1
  340 Enemies(2).Name = "Goblin"
  350 Enemies(2).HP = 35 : Enemies(2).MaxHP = 35
  360 Enemies(2).ATK = 8 : Enemies(2).DEF = 4
  370 Enemies(3).Name = "Dragon"
  380 Enemies(3).HP = 200 : Enemies(3).MaxHP = 200
  390 Enemies(3).ATK = 30 : Enemies(3).DEF = 20
  400 REM
  410 REM -- Battle simulation --
  420 FOR E% = 1 TO 3
  430   PRINT "═══════════════════════════════"
  440   PRINT Hero.Name; " vs "; Enemies(E%).Name
  450   PRINT "═══════════════════════════════"
  460   DO
  470     REM Hero attacks
  480     DMG% = Hero.ATK - Enemies(E%).DEF
  490     IF DMG% < 1 THEN DMG% = 1
  500     Enemies(E%).HP = Enemies(E%).HP - DMG%
  510     IF Enemies(E%).HP < 0 THEN Enemies(E%).HP = 0
  520     PRINT Hero.Name; " deals "; DMG%; " damage!";
  530     PRINT " ("; Enemies(E%).Name; ": ";
  540     PRINT Enemies(E%).HP; "/";
  550     PRINT Enemies(E%).MaxHP; " HP)"
  560     IF Enemies(E%).HP <= 0 THEN
  570       PRINT Enemies(E%).Name; " defeated!"
  580       EXIT DO
  590     END IF
  600     REM Enemy attacks
  610     DMG% = Enemies(E%).ATK - Hero.DEF
  620     IF DMG% < 1 THEN DMG% = 1
  630     Hero.HP = Hero.HP - DMG%
  640     IF Hero.HP < 0 THEN Hero.HP = 0
  650     PRINT Enemies(E%).Name; " deals "; DMG%;
  660     PRINT " damage! (Hero: "; Hero.HP; "/";
  670     PRINT Hero.MaxHP; " HP)"
  680     IF Hero.HP <= 0 THEN
  690       PRINT "Hero has fallen!"
  700       END
  710     END IF
  720   LOOP
  730   PRINT
  740 NEXT E%
  750 PRINT "All enemies defeated! Victory!"
  760 END


34.  EXAMPLE: CONFIGURATION FILE PARSER
-------------------------------------------

  100 REM ======================================
  110 REM  Config File Parser
  120 REM ======================================
  130 TYPE ConfigEntry
  140   Key AS STRING
  150   Value AS STRING
  160 END TYPE
  170 DIM Config(32) AS ConfigEntry
  180 NCFG% = 0
  190 REM
  200 REM -- Load config file --
  210 ON ERROR GOTO 400
  220 OPEN "app.cfg" FOR INPUT AS #1
  230 DO WHILE NOT EOF(1)
  240   LINE INPUT #1, L$
  250   REM Skip comments and blank lines
  260   IF LEN(L$) = 0 THEN GOTO 340
  270   IF LEFT$(L$, 1) = "#" THEN GOTO 340
  280   REM Parse "key = value"
  290   EQ% = INSTR(L$, "=")
  300   IF EQ% > 0 THEN
  310     NCFG% = NCFG% + 1
  320     Config(NCFG%).Key = LTRIM$(RTRIM$( _
                              LEFT$(L$, EQ% - 1)))
  330     Config(NCFG%).Value = LTRIM$(RTRIM$( _
                               MID$(L$, EQ% + 1)))
  340   END IF
  350 LOOP
  360 CLOSE #1
  370 REM
  380 REM -- Display loaded config --
  390 PRINT "Loaded "; NCFG%; " config entries:"
  392 FOR I% = 1 TO NCFG%
  394   PRINT "  "; Config(I%).Key; " = "; Config(I%).Value
  396 NEXT I%
  398 END
  400 PRINT "Could not open app.cfg"
  410 END


======================================================================
Part VII:  ERROR REFERENCE AND BEST PRACTICES
======================================================================


35.  ERROR CODES
------------------

  Code  Constant     When It Occurs
  ────  ───────────  ──────────────────────────────────
  ERR_SORRY          Too many TYPE definitions
                     (type_count >= MAX_USER_TYPES)
  ERR_SORRY          Too many typed variables
                     (typed_var_count >= MAX_TYPED_VARS)
  ERR_WHAT           Invalid TYPE name (not a variable)
  ERR_WHAT           Unknown field name in dot access
  ERR_WHAT           Variable is not a typed variable
  ERR_HOW            Type mismatch in field assignment
                     (assigning string to numeric field
                      or vice versa)


36.  COMMON MISTAKES
-----------------------

  Mistake 1: Forgetting END TYPE

    100 TYPE Player
    110   Name AS STRING
    120   HP AS INTEGER
    130 REM -- Oops! No END TYPE! --
    140 DIM P AS Player

  The parser will scan forward looking for END TYPE and
  may consume program lines as field definitions.


  Mistake 2: Using a type before defining it

    100 DIM P AS Player    : REM Error! Player not defined
    200 TYPE Player
    210   Name AS STRING
    220 END TYPE

  Move TYPE definitions before their first use.


  Mistake 3: Exceeding limits silently

    100 TYPE BigRecord
    110   Field1 AS STRING
    120   Field2 AS STRING
    ...
    270   Field17 AS STRING   : REM 17th field - IGNORED!
    280 END TYPE

  Only the first 16 fields are stored.  The 17th and
  beyond are silently ignored.


  Mistake 4: Trying to PRINT a whole record

    100 PRINT P              : REM Doesn't print fields!

  Print each field individually instead.


37.  BEST PRACTICES
---------------------

  1. Define all TYPEs at the top of your program, before
     any executable code.

  2. Use descriptive type names (Contact, not C; Player,
     not P; GameState, not GS).

  3. Use descriptive field names (HitPoints, not HP;
     FullName, not N) when clarity matters.

  4. Keep types small (5-10 fields).  If you need more
     than 16 fields, consider splitting into two types.

  5. Write helper SUBs for common operations on typed
     records (PrintContact, SaveScore, etc.).

  6. Initialize all fields after DIM.  Don't assume
     previous values.

  7. When sorting arrays of records, swap ALL fields
     (not just the sort key).

  8. When deleting from an array, shift all remaining
     elements and decrement the count.


38.  QUICK REFERENCE
----------------------

  Statement/Syntax                 Description
  ───────────────────────────────  ────────────────────────
  TYPE TypeName                    Begin type definition
    FieldName AS INTEGER           Define numeric field
    FieldName AS STRING            Define string field
  END TYPE                         End type definition
  DIM VarName AS TypeName          Create typed variable
  DIM Array(N) AS TypeName         Create typed array
  VarName.FieldName = value        Set a field
  value = VarName.FieldName        Read a field
  Array(I).FieldName = value       Set array element field
  value = Array(I).FieldName       Read array element field

  Limits:
    Max type definitions:          64 (MAX_USER_TYPES)
    Max fields per type:           64 (MAX_TYPE_FIELDS)
    Max typed variable instances:  512 (MAX_TYPED_VARS)
    Max name length:               31 (MAX_VAR_NAME_LEN)
    Numeric field default:         0
    String field default:          "" (empty)


======================================================================
  END OF TUTORIAL: USER-DEFINED TYPES
======================================================================
