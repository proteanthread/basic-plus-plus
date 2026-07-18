ARRAYS AND MATRICES IN BASIC++
================================
A Complete Tutorial on DIM Arrays, MAT Operations,
String Arrays, and Advanced Techniques

Version 6.4.0

TABLE OF CONTENTS
-----------------
  PART I  -- ARRAYS
  1.  What Is an Array?
  2.  Declaring Arrays with DIM
  3.  Array Indexing and OPTION BASE
  4.  Reading and Writing Array Elements
  5.  String Arrays
  6.  Two-Dimensional Arrays
  7.  REDIM -- Resizing Arrays
  8.  ERASE -- Deleting Arrays
  9.  Arrays and DATA / READ
  10. Passing Arrays to SUBs
  11. Array Functions: LBOUND, UBOUND
  12. The Element Pool: How Arrays Use Memory
  13. Common Array Patterns

  PART II -- MATRICES (MAT OPERATIONS)
  14. Introduction to MAT
  15. MAT READ and MAT PRINT
  16. MAT Assignment and Copy
  17. Matrix Arithmetic (Add, Subtract, Multiply)
  18. Scalar Multiplication
  19. Special Matrices (ZER, CON, IDN)
  20. Transpose (TRN) and Inverse (INV)
  21. Solving Linear Systems
  22. Determinants

  PART III -- ADVANCED TECHNIQUES
  23. Lookup Tables
  24. Sorting Arrays
  25. Searching Arrays
  26. Stacks and Queues with Arrays
  27. Sparse Data with Parallel Arrays
  28. Game Boards and Grids
  29. Histograms and Frequency Tables
  30. Performance Tips
  31. Limits and Troubleshooting


=====================================================================
                        PART I -- ARRAYS
=====================================================================


=====================================================================
1.  WHAT IS AN ARRAY?
=====================================================================

An array is a numbered collection of values stored under a
single name.  Instead of declaring 100 separate variables:

  SCORE1 = 0 : SCORE2 = 0 : SCORE3 = 0 : ... : SCORE100 = 0

You declare one array:

  DIM SCORE(100)

And access each element by number:

  SCORE(1) = 95
  SCORE(2) = 87
  PRINT SCORE(1) + SCORE(2)       ' 182

Arrays are the fundamental data structure in BASIC++.  Nearly
every non-trivial program uses them.


=====================================================================
2.  DECLARING ARRAYS WITH DIM
=====================================================================

2.1  One-Dimensional Arrays

  DIM arrayname(size)

  This creates an array with elements from 0 to size (inclusive),
  for a total of size+1 elements (with OPTION BASE 0, the default).

  Examples:
    DIM SCORES(10)            ' 11 elements: SCORES(0) to SCORES(10)
    DIM NAMES$(50)            ' 51 string elements
    DIM TEMPS(364)            ' 365 elements (one per day)

2.2  Two-Dimensional Arrays

  DIM arrayname(rows, cols)

  Creates a 2D array (a table or grid):

    DIM GRID(10, 10)          ' 11 x 11 = 121 elements
    DIM TABLE(5, 3)           ' 6 x 4 = 24 elements
    DIM BOARD$(8, 8)          ' 9 x 9 = 81 string elements

  2D arrays are stored internally in ROW-MAJOR order (row 0 is
  stored first, then row 1, etc.).

2.3  Multiple Declarations

  You can declare multiple arrays on one line:

    DIM A(10), B(20), C$(5)

2.4  Declaration Rules

  * DIM must appear before the array is used.
  * If you use an array name without DIM, BASIC++ auto-declares
    it with size 10 (i.e., DIM name(10) is implicit).
  * The maximum subscript value is 32767.
  * Maximum 64 arrays per program.
  * Maximum 2 dimensions per array.
  * Array names follow the same rules as variable names (up to
    31 characters, letters/digits/underscore).

2.5  DIM AS (Typed Arrays)

  In QBasic and BASIC++ extended dialects, you can declare arrays
  of user-defined types:

    TYPE StudentRec
      Name AS STRING
      Grade AS INTEGER
    END TYPE

    DIM Students(30) AS StudentRec

  See User_Defined_Types.txt for complete typed array coverage.

2.6  Typed Array Internals

  When you declare a typed array with DIM AS, the DimArray
  structure stores a type_index that links each array element
  to a UserTypeDef:

    typedef struct DimArray {
        char name[MAX_VAR_NAME_LEN + 1];
        int dims;
        int size[MAX_ARRAY_DIMS];
        BValue *elements;
        int total;
        int type_index;  /* -1=normal, >=0=typed (UserTypeDef index) */
    } DimArray;

  For typed arrays, elements are stored in field-stride layout:
  element[i * field_count + field_index].  Access a field with
  runtime_get_typed_array_field():

    BValue *runtime_get_typed_array_field(RuntimeState *rt,
        DimArray *arr, int elem_index, int field_index);

  A type_index of -1 means a normal (untyped) numeric or string
  array.  A value >= 0 indexes into user_types[] to find the
  type definition.


=====================================================================
3.  ARRAY INDEXING AND OPTION BASE
=====================================================================

3.1  Default: Base 0

  By default, arrays start at index 0:

    DIM A(5)
    ' Valid indices: A(0), A(1), A(2), A(3), A(4), A(5)
    ' Total: 6 elements

3.2  OPTION BASE 1

  To start arrays at index 1 (matching classic BASIC):

    OPTION BASE 1
    DIM A(5)
    ' Valid indices: A(1), A(2), A(3), A(4), A(5)
    ' Total: 5 elements
    ' A(0) is NOT valid!

  OPTION BASE must appear before any DIM statement.  It cannot
  be changed after arrays are declared.

3.3  Which Should You Use?

  * OPTION BASE 0 -- matches C/C++, modern languages, internal
    storage.  Best for programmers comfortable with zero-based
    indexing.
  * OPTION BASE 1 -- matches mathematical notation, Fortran,
    and many classic BASIC programs.  Best for porting old code
    or when subscripts represent natural counts (student 1,
    student 2, etc.).


=====================================================================
4.  READING AND WRITING ARRAY ELEMENTS
=====================================================================

4.1  Assignment

    DIM PRICES(5)
    PRICES(1) = 19.99
    PRICES(2) = 24.50
    PRICES(3) = 9.95

4.2  Reading

    PRINT PRICES(1)             ' 19.99
    TOTAL = PRICES(1) + PRICES(2) + PRICES(3)
    PRINT "Total: "; TOTAL      ' 54.44

4.3  Computed Subscripts

  The subscript can be any numeric expression:

    DIM DATA(100)
    FOR I = 1 TO 100
      DATA(I) = I * I           ' squares
    NEXT I

    ' Random access
    INDEX = INT(RND * 100) + 1
    PRINT "Random value: "; DATA(INDEX)

4.4  Bounds Checking

  Accessing an element outside the declared range raises a
  runtime error:

    DIM A(5)
    PRINT A(10)                 ' Error: Subscript out of range

  Always validate subscripts when they come from user input:

    INPUT "Index (1-5): "; I$
    I = VAL(I$)
    IF I < 1 OR I > 5 THEN PRINT "Invalid index." : GOTO ...
    PRINT A(I)

4.5  Initializing All Elements

  New arrays are initialized to zero (numeric) or empty string
  (string arrays).  To fill with a specific value:

    DIM A(100)
    FOR I = 0 TO 100
      A(I) = -1                 ' initialize to -1
    NEXT I

  Or use MAT for 2D arrays (see Part II):

    DIM A(10, 10)
    MAT A = ZER                 ' fill with zeros
    MAT A = CON                 ' fill with ones


=====================================================================
5.  STRING ARRAYS
=====================================================================

5.1  Declaring String Arrays

  Append $ to the array name:

    DIM NAMES$(20)
    DIM WORDS$(100)
    DIM GRID$(8, 8)

5.2  Using String Arrays

    DIM MONTHS$(12)
    MONTHS$(1) = "January"
    MONTHS$(2) = "February"
    MONTHS$(3) = "March"
    ' ... etc.
    MONTHS$(12) = "December"

    INPUT "Month number (1-12): "; M
    IF M >= 1 AND M <= 12 THEN
      PRINT MONTHS$(M)
    END IF

5.3  String Pool Impact

  Each string stored in an array allocates space in the string
  pool.  Overwriting an element allocates NEW pool space; the
  old string data is not freed until pool reset (RUN/NEW/CLEAR).

  This means repeatedly updating string array elements in a
  loop consumes pool space:

    ' CAUTION: Each iteration consumes pool space
    DIM LOG$(1000)
    FOR I = 1 TO 1000
      LOG$(I) = "Entry " + STR$(I)
    NEXT I

  Monitor pool space with FRE("") if your program does heavy
  string array work.  See Secure_Coding.txt section 5 for
  detailed guidance.


=====================================================================
6.  TWO-DIMENSIONAL ARRAYS
=====================================================================

6.1  Declaring

    DIM TABLE(rows, cols)

    ' 5 students, 4 tests
    DIM GRADES(5, 4)

6.2  Accessing Elements

    ' Set grades for student 1
    GRADES(1, 1) = 92
    GRADES(1, 2) = 85
    GRADES(1, 3) = 88
    GRADES(1, 4) = 91

    ' Read grades for student 3, test 2
    PRINT GRADES(3, 2)

6.3  Nested Loops for 2D Arrays

  The standard pattern for processing every element:

    ' Print all grades in a table
    PRINT "Student", "T1", "T2", "T3", "T4", "Avg"
    FOR S = 1 TO 5
      PRINT S,
      TOTAL = 0
      FOR T = 1 TO 4
        PRINT GRADES(S, T),
        TOTAL = TOTAL + GRADES(S, T)
      NEXT T
      PRINT TOTAL / 4
    NEXT S

6.4  Row-Major Storage

  Elements are stored internally as:

    GRID(0,0), GRID(0,1), GRID(0,2), ...,   ' row 0
    GRID(1,0), GRID(1,1), GRID(1,2), ...,   ' row 1
    ...

  This matters for performance: accessing elements row by row
  is faster than column by column, because row-sequential access
  reads consecutive memory locations.

  FAST (row-major traversal):
    FOR R = 1 TO 100
      FOR C = 1 TO 100
        X = GRID(R, C)
      NEXT C
    NEXT R

  SLOW (column-major traversal):
    FOR C = 1 TO 100
      FOR R = 1 TO 100
        X = GRID(R, C)       ' jumps across rows
      NEXT R
    NEXT C


=====================================================================
7.  REDIM -- RESIZING ARRAYS
=====================================================================

REDIM changes the size of an existing array.  All existing data
is LOST (elements reset to zero or empty string).

  DIM DATA(10)
  DATA(1) = 42
  REDIM DATA(50)          ' DATA(1) is now 0 -- data is gone!

7.1  When to Use REDIM

  * When you don't know the required size at program start
  * When you need to grow an array after initial population
  * When recycling an array name for a different purpose

7.2  Preserving Data with REDIM

  BASIC++ does not support REDIM PRESERVE (QBasic does for the
  last dimension only).  To resize while keeping data, copy
  manually:

    ' Copy A() into a larger array
    DIM TEMP(100)
    FOR I = 0 TO 10
      TEMP(I) = A(I)
    NEXT I
    ERASE A
    DIM A(100)
    FOR I = 0 TO 10
      A(I) = TEMP(I)
    NEXT I
    ERASE TEMP


=====================================================================
8.  ERASE -- DELETING ARRAYS
=====================================================================

ERASE removes an array and frees its element pool space:

  ERASE A               ' delete array A
  ERASE A, B, C         ' delete multiple arrays

After ERASE:
  * The array name is undefined (using it is an error)
  * The element pool space is returned
  * The DIM count is decremented

Use ERASE when you're done with a large array and need pool
space for another:

  DIM BIGDATA(5000)
  ' ... process BIGDATA ...
  ERASE BIGDATA              ' free 5001 elements
  DIM RESULTS(3000)          ' reuse the pool space


=====================================================================
9.  ARRAYS AND DATA / READ
=====================================================================

9.1  Loading Arrays from DATA

    DIM PRIMES(10)
    FOR I = 1 TO 10
      READ PRIMES(I)
    NEXT I
    DATA 2, 3, 5, 7, 11, 13, 17, 19, 23, 29

9.2  Loading 2D Arrays from DATA

    DIM BOARD(3, 3)
    FOR R = 1 TO 3
      FOR C = 1 TO 3
        READ BOARD(R, C)
      NEXT C
    NEXT R
    DATA 1, 2, 3
    DATA 4, 5, 6
    DATA 7, 8, 9

  Or use MAT READ (see section 15):

    DIM BOARD(3, 3)
    MAT READ BOARD

9.3  Loading String Arrays from DATA

    DIM DAYS$(7)
    FOR I = 1 TO 7
      READ DAYS$(I)
    NEXT I
    DATA "Monday", "Tuesday", "Wednesday", "Thursday"
    DATA "Friday", "Saturday", "Sunday"


=====================================================================
10. PASSING ARRAYS TO SUBs
=====================================================================

In QBasic and BASIC++ extended dialects, arrays can be passed
to SUB and FUNCTION procedures:

  SUB PrintArray(A(), N)
    FOR I = 1 TO N
      PRINT A(I);
    NEXT I
    PRINT
  END SUB

  DIM SCORES(5)
  SCORES(1) = 10 : SCORES(2) = 20 : SCORES(3) = 30
  SCORES(4) = 40 : SCORES(5) = 50
  CALL PrintArray(SCORES(), 5)

The empty parentheses SCORES() indicate that the entire array
is being passed.  Arrays are passed BY REFERENCE -- changes
made inside the SUB affect the original array.

  SUB DoubleAll(A(), N)
    FOR I = 1 TO N
      A(I) = A(I) * 2          ' modifies the caller's array
    NEXT I
  END SUB


=====================================================================
11. ARRAY FUNCTIONS: LBOUND, UBOUND
=====================================================================

  LBOUND(array, dimension)   Lowest valid subscript
  UBOUND(array, dimension)   Highest valid subscript

  Example:
    OPTION BASE 1
    DIM SCORES(50)
    PRINT LBOUND(SCORES, 1)     ' 1
    PRINT UBOUND(SCORES, 1)     ' 50

  For 2D arrays:
    DIM GRID(10, 20)
    PRINT LBOUND(GRID, 1)       ' 0 (or 1 with OPTION BASE 1)
    PRINT UBOUND(GRID, 1)       ' 10
    PRINT LBOUND(GRID, 2)       ' 0
    PRINT UBOUND(GRID, 2)       ' 20

  Use LBOUND/UBOUND to write SUBs that work with any array size:

    SUB SumArray(A(), TOTAL)
      TOTAL = 0
      FOR I = LBOUND(A, 1) TO UBOUND(A, 1)
        TOTAL = TOTAL + A(I)
      NEXT I
    END SUB


=====================================================================
12. THE ELEMENT POOL: HOW ARRAYS USE MEMORY
=====================================================================

All DIM arrays share a single flat element pool of 8192 BValue
slots.  When you DIM an array, its elements are allocated from
this pool.

12.1  Pool Accounting

    DIM A(100)          ' uses 101 elements (0-100)
    DIM B(50, 50)       ' uses 51 * 51 = 2601 elements
    DIM C$(200)         ' uses 201 elements
    '                     Total: 2903 of 8192

  Check remaining pool space:
    PRINT SIZE("ARRAYS")      ' elements remaining

12.2  Pool Exhaustion

  If you try to DIM more elements than available:

    DIM BIG(8000)               ' 8001 elements -- OK
    DIM SMALL(200)              ' SORRY. Out of array space.

  Solutions:
    * ERASE arrays no longer needed before DIMming new ones
    * Reduce array sizes
    * Increase MAX_ARRAY_ELEMENTS in config.h and recompile

12.3  Planning Your Array Budget

  For complex programs, document element usage:

    ' ====== ARRAY BUDGET ======
    ' Players(4)          =     5 elements
    ' Board(8, 8)         =    81 elements
    ' Scores(4, 18)       =    95 elements
    ' MoveHistory(500)    =   501 elements
    ' Names$(4)           =     5 elements
    ' -------------------------
    ' Total               =   687 of 8192
    ' ==========================


=====================================================================
13. COMMON ARRAY PATTERNS
=====================================================================

13.1  Finding the Maximum

    MAX_VAL = A(1)
    MAX_IDX = 1
    FOR I = 2 TO N
      IF A(I) > MAX_VAL THEN
        MAX_VAL = A(I)
        MAX_IDX = I
      END IF
    NEXT I
    PRINT "Maximum: "; MAX_VAL; " at index "; MAX_IDX

13.2  Finding the Minimum

    MIN_VAL = A(1)
    FOR I = 2 TO N
      IF A(I) < MIN_VAL THEN MIN_VAL = A(I)
    NEXT I

13.3  Summing and Averaging

    TOTAL = 0
    FOR I = 1 TO N
      TOTAL = TOTAL + A(I)
    NEXT I
    AVERAGE = TOTAL / N

13.4  Counting Occurrences

    TARGET = 42
    COUNT = 0
    FOR I = 1 TO N
      IF A(I) = TARGET THEN COUNT = COUNT + 1
    NEXT I
    PRINT TARGET; " appears "; COUNT; " times"

13.5  Reversing an Array

    FOR I = 1 TO N / 2
      SWAP A(I), A(N - I + 1)
    NEXT I

13.6  Copying an Array

    DIM B(N)
    FOR I = 0 TO N
      B(I) = A(I)
    NEXT I

13.7  Shifting Elements

    ' Shift left (delete element at position P)
    FOR I = P TO N - 1
      A(I) = A(I + 1)
    NEXT I
    N = N - 1

    ' Shift right (insert at position P)
    FOR I = N TO P STEP -1
      A(I + 1) = A(I)
    NEXT I
    A(P) = NEW_VALUE
    N = N + 1


=====================================================================
                    PART II -- MATRICES (MAT OPERATIONS)
=====================================================================


=====================================================================
14. INTRODUCTION TO MAT
=====================================================================

BASIC++ supports classic Dartmouth BASIC MAT (matrix) operations.
These operate on entire 2D arrays with a single statement, making
matrix math concise and readable.

MAT operations require:
  * QBasic, BASIC++, or Dartmouth dialect (not Tiny BASIC)
  * 2D arrays for most operations (some work on 1D)
  * Numeric arrays only (not string arrays)

Why use MAT?
  * One-line matrix addition instead of nested loops
  * Built-in matrix multiplication with dimension checking
  * Transpose and inverse without writing the algorithms
  * Clean, mathematical notation


=====================================================================
15. MAT READ AND MAT PRINT
=====================================================================

15.1  MAT READ

  Fill a matrix from DATA statements:

    10 DIM A(3, 3)
    20 MAT READ A
    30 DATA 1, 2, 3
    40 DATA 4, 5, 6
    50 DATA 7, 8, 9

  MAT READ fills the matrix row by row, reading values from
  the DATA pool in order.  The matrix must have exactly as many
  elements as there are remaining DATA values (or fewer).

15.2  MAT PRINT

  Display a matrix in formatted rows:

    MAT PRINT A;         ' compact format (semicolon)
    MAT PRINT A,         ' tabbed columns (comma)
    MAT PRINT A          ' default (tabbed)

  Output of MAT PRINT A; (compact):
     1  2  3
     4  5  6
     7  8  9

  Output of MAT PRINT A, (tabbed):
     1            2            3
     4            5            6
     7            8            9

15.3  MAT READ with 1D Arrays

  MAT READ also works with 1D arrays:

    DIM V(5)
    MAT READ V
    DATA 10, 20, 30, 40, 50

  This is equivalent to reading V(0) through V(5), or V(1)
  through V(5) with OPTION BASE 1.


=====================================================================
16. MAT ASSIGNMENT AND COPY
=====================================================================

16.1  Copying a Matrix

    DIM A(3, 3), B(3, 3)
    MAT READ A
    MAT B = A             ' B is now a copy of A

  Both matrices must have the same dimensions.

16.2  In-Place vs. Copy

  MAT operations create results in the target matrix.  The
  source and target can be different arrays:

    MAT C = A + B         ' C = A + B (C is different from A, B)

  But you can also overwrite a source:

    MAT A = A + B         ' A = A + B (A is modified in place)


=====================================================================
17. MATRIX ARITHMETIC (ADD, SUBTRACT, MULTIPLY)
=====================================================================

17.1  Addition

  Element-wise addition.  Both matrices must have the same
  dimensions.

    DIM A(3, 3), B(3, 3), C(3, 3)
    MAT READ A
    MAT READ B
    MAT C = A + B

    ' If A = [[1,2,3],[4,5,6],[7,8,9]]
    ' and B = [[9,8,7],[6,5,4],[3,2,1]]
    ' then C = [[10,10,10],[10,10,10],[10,10,10]]

17.2  Subtraction

    MAT C = A - B

    ' C(i,j) = A(i,j) - B(i,j) for all i, j

17.3  Matrix Multiplication

  The inner dimensions must match.  If A is m x n and B is
  n x p, then C = A * B is m x p.

    DIM A(2, 3), B(3, 2), C(2, 2)
    MAT READ A
    MAT READ B
    MAT C = A * B

    ' A = [[1,2,3],    B = [[7,8],     C = [[58,64],
    '      [4,5,6]]         [9,10],         [139,154]]
    '                       [11,12]]

  The calculation:
    C(1,1) = 1*7 + 2*9 + 3*11 = 58
    C(1,2) = 1*8 + 2*10 + 3*12 = 64
    C(2,1) = 4*7 + 5*9 + 6*11 = 139
    C(2,2) = 4*8 + 5*10 + 6*12 = 154

17.4  Dimension Errors

  If dimensions don't match, BASIC++ raises a runtime error:

    DIM A(2, 3), B(2, 3), C(2, 2)
    MAT C = A * B              ' HOW? Inner dimensions 3 and 2
                               ' don't match (need B to be 3xN)


=====================================================================
18. SCALAR MULTIPLICATION
=====================================================================

Multiply every element of a matrix by a constant:

  K = 2.5
  MAT B = (K) * A

  The scalar MUST be in parentheses.

  Example:
    DIM A(2, 2), B(2, 2)
    A(1,1) = 2 : A(1,2) = 4
    A(2,1) = 6 : A(2,2) = 8
    K = 3
    MAT B = (K) * A
    MAT PRINT B
    '  6  12
    ' 18  24

  Use scalar multiplication for:
    * Unit conversion (multiply all temperatures by 1.8)
    * Scaling (multiply all coordinates by a zoom factor)
    * Normalization (divide by sum: K = 1/SUM)


=====================================================================
19. SPECIAL MATRICES (ZER, CON, IDN)
=====================================================================

19.1  Zero Matrix (ZER)

  Set all elements to 0:

    DIM A(4, 4)
    MAT A = ZER

    ' A = [[0,0,0,0],
    '      [0,0,0,0],
    '      [0,0,0,0],
    '      [0,0,0,0]]

  Use for initialization and clearing.

19.2  Constant Matrix (CON)

  Set all elements to 1:

    MAT A = CON

    ' A = [[1,1,1,1],
    '      [1,1,1,1],
    '      [1,1,1,1],
    '      [1,1,1,1]]

  Combine with scalar multiplication for any constant:

    K = 5
    MAT A = CON
    MAT A = (K) * A         ' fill with 5

19.3  Identity Matrix (IDN)

  Diagonal = 1, all others = 0.  Matrix must be square.

    DIM I(3, 3)
    MAT I = IDN

    ' I = [[1,0,0],
    '      [0,1,0],
    '      [0,0,1]]

  The identity matrix is the multiplicative identity:
    MAT B = A * I            ' B = A (A is unchanged)
    MAT B = I * A            ' B = A (same result)


=====================================================================
20. TRANSPOSE (TRN) AND INVERSE (INV)
=====================================================================

20.1  Transpose

  Swap rows and columns:

    DIM A(2, 3), B(3, 2)
    MAT READ A
    MAT B = TRN(A)

    ' If A = [[1,2,3],     then B = [[1,4],
    '         [4,5,6]]                [2,5],
    '                                 [3,6]]

  The target matrix must have swapped dimensions: if A is
  m x n, then B must be n x m.

  Transpose of a square matrix can be done in place:
    DIM A(3, 3), B(3, 3)
    MAT B = TRN(A)           ' B is the transpose of A

20.2  Inverse

  The matrix inverse (square matrices only):

    DIM A(3, 3), B(3, 3)
    MAT READ A
    MAT B = INV(A)

  The inverse exists only if the determinant is non-zero.
  If the matrix is singular (det = 0), BASIC++ raises a
  runtime error.

  To check for singularity before inverting:

    ON ERROR GOTO 500
    MAT B = INV(A)
    ON ERROR GOTO 0
    PRINT "Inverse computed successfully."
    GOTO 600
    500 PRINT "Matrix is singular -- no inverse exists."
    510 RESUME 600
    600 ' continue...

  Properties of the inverse:
    MAT C = A * B            ' C = I (identity) if B = INV(A)
    MAT C = B * A            ' C = I (same result)


=====================================================================
21. SOLVING LINEAR SYSTEMS
=====================================================================

The classic application of matrix operations: solving Ax = b.

21.1  The Problem

  Given a system of equations:
     2x + 3y +  z = 1
     4x +  y + 2z = 2
      x + 2y + 3z = 3

  This is Ax = b where:
     A = [[2,3,1],[4,1,2],[1,2,3]]
     b = [[1],[2],[3]]

21.2  The Solution: x = A^(-1) * b

    10 DIM A(3, 3), B(3, 3), X(3, 1), C(3, 1)
    20 '
    30 ' Read matrix A (coefficients)
    40 MAT READ A
    50 DATA 2, 3, 1
    60 DATA 4, 1, 2
    70 DATA 1, 2, 3
    80 '
    90 ' Read vector b (constants)
    100 MAT READ C
    110 DATA 1
    120 DATA 2
    130 DATA 3
    140 '
    150 ' Compute inverse
    160 ON ERROR GOTO 500
    170 MAT B = INV(A)
    180 ON ERROR GOTO 0
    190 '
    200 ' Multiply: x = A^(-1) * b
    210 MAT X = B * C
    220 '
    230 PRINT "Solution:"
    240 PRINT "x ="; X(1, 1)
    250 PRINT "y ="; X(2, 1)
    260 PRINT "z ="; X(3, 1)
    270 END
    500 PRINT "System has no unique solution."
    510 END

21.3  Verification

  To verify the solution, compute A * x and check that it
  equals b:

    DIM CHECK(3, 1)
    MAT CHECK = A * X
    PRINT "Verification (should equal b):"
    MAT PRINT CHECK


=====================================================================
22. DETERMINANTS
=====================================================================

BASIC++ does not have a built-in DET function, but you can
compute the determinant using the inverse operation.

22.1  2x2 Determinant

    ' det(A) = a*d - b*c for [[a,b],[c,d]]
    FUNCTION Det2(A())
      Det2 = A(1,1) * A(2,2) - A(1,2) * A(2,1)
    END FUNCTION

22.2  3x3 Determinant (Sarrus' Rule)

    FUNCTION Det3(A())
      D = A(1,1) * (A(2,2)*A(3,3) - A(2,3)*A(3,2))
      D = D - A(1,2) * (A(2,1)*A(3,3) - A(2,3)*A(3,1))
      D = D + A(1,3) * (A(2,1)*A(3,2) - A(2,2)*A(3,1))
      Det3 = D
    END FUNCTION

22.3  Singularity Check

    D = Det3(A())
    IF ABS(D) < 0.00001 THEN
      PRINT "Matrix is singular (det ≈ 0)"
    ELSE
      PRINT "Determinant = "; D
      MAT B = INV(A)
    END IF


=====================================================================
                   PART III -- ADVANCED TECHNIQUES
=====================================================================


=====================================================================
23. LOOKUP TABLES
=====================================================================

Arrays make excellent lookup tables that replace IF/THEN chains:

23.1  Day-of-Week Lookup

    DIM DAYS$(7)
    DAYS$(1) = "Sun" : DAYS$(2) = "Mon" : DAYS$(3) = "Tue"
    DAYS$(4) = "Wed" : DAYS$(5) = "Thu" : DAYS$(6) = "Fri"
    DAYS$(7) = "Sat"

    ' Instead of:  IF D=1 THEN P$="Sun" ELSE IF D=2 THEN ...
    ' Just:
    P$ = DAYS$(D)

23.2  Days-in-Month Lookup

    DIM MDAYS(12)
    FOR I = 1 TO 12 : READ MDAYS(I) : NEXT I
    DATA 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31

    INPUT "Month: "; M
    PRINT "Days: "; MDAYS(M)

23.3  Character Classification Table

    DIM IS_VOWEL(255)
    MAT IS_VOWEL = ZER         ' wait -- 1D, so use a loop:
    FOR I = 0 TO 255 : IS_VOWEL(I) = 0 : NEXT I
    IS_VOWEL(ASC("A")) = 1 : IS_VOWEL(ASC("E")) = 1
    IS_VOWEL(ASC("I")) = 1 : IS_VOWEL(ASC("O")) = 1
    IS_VOWEL(ASC("U")) = 1
    IS_VOWEL(ASC("a")) = 1 : IS_VOWEL(ASC("e")) = 1
    IS_VOWEL(ASC("i")) = 1 : IS_VOWEL(ASC("o")) = 1
    IS_VOWEL(ASC("u")) = 1

    ' Now checking is O(1):
    IF IS_VOWEL(ASC(C$)) THEN PRINT C$; " is a vowel"


=====================================================================
24. SORTING ARRAYS
=====================================================================

24.1  Bubble Sort (Simple)

    ' Sort A(1) through A(N) in ascending order
    FOR PASS = 1 TO N - 1
      FOR I = 1 TO N - PASS
        IF A(I) > A(I + 1) THEN
          SWAP A(I), A(I + 1)
        END IF
      NEXT I
    NEXT PASS

24.2  Selection Sort (Fewer Swaps)

    FOR I = 1 TO N - 1
      MIN_IDX = I
      FOR J = I + 1 TO N
        IF A(J) < A(MIN_IDX) THEN MIN_IDX = J
      NEXT J
      IF MIN_IDX <> I THEN SWAP A(I), A(MIN_IDX)
    NEXT I

24.3  Insertion Sort (Good for Nearly-Sorted Data)

    FOR I = 2 TO N
      KEY_VAL = A(I)
      J = I - 1
      WHILE J >= 1 AND A(J) > KEY_VAL
        A(J + 1) = A(J)
        J = J - 1
      WEND
      A(J + 1) = KEY_VAL
    NEXT I

24.4  Sorting String Arrays

  The same algorithms work with string arrays -- BASIC++
  compares strings alphabetically:

    DIM NAMES$(100)
    ' ... fill NAMES$ ...
    ' Bubble sort:
    FOR PASS = 1 TO N - 1
      FOR I = 1 TO N - PASS
        IF NAMES$(I) > NAMES$(I + 1) THEN
          SWAP NAMES$(I), NAMES$(I + 1)
        END IF
      NEXT I
    NEXT PASS

24.5  Sorting with a Key (Parallel Arrays)

  Sort one array while keeping a second array in sync:

    ' Sort SCORES() descending, keep NAMES$() aligned
    FOR I = 1 TO N - 1
      FOR J = I + 1 TO N
        IF SCORES(J) > SCORES(I) THEN
          SWAP SCORES(I), SCORES(J)
          SWAP NAMES$(I), NAMES$(J)
        END IF
      NEXT J
    NEXT I


=====================================================================
25. SEARCHING ARRAYS
=====================================================================

25.1  Linear Search

    FUNCTION LinearSearch(A(), N, TARGET)
      FOR I = 1 TO N
        IF A(I) = TARGET THEN
          LinearSearch = I     ' found at index I
          EXIT FUNCTION
        END IF
      NEXT I
      LinearSearch = -1         ' not found
    END FUNCTION

25.2  Binary Search (Sorted Array)

  Much faster for large sorted arrays (O(log N) vs O(N)):

    FUNCTION BinarySearch(A(), N, TARGET)
      LO = 1 : HI = N
      WHILE LO <= HI
        MID_PT = INT((LO + HI) / 2)
        IF A(MID_PT) = TARGET THEN
          BinarySearch = MID_PT
          EXIT FUNCTION
        ELSE IF A(MID_PT) < TARGET THEN
          LO = MID_PT + 1
        ELSE
          HI = MID_PT - 1
        END IF
      WEND
      BinarySearch = -1         ' not found
    END FUNCTION

25.3  Searching String Arrays

    FUNCTION FindName$(NAMES$(), N, SEARCH$)
      S$ = UCASE$(SEARCH$)
      FOR I = 1 TO N
        IF UCASE$(NAMES$(I)) = S$ THEN
          FindName$ = STR$(I)
          EXIT FUNCTION
        END IF
      NEXT I
      FindName$ = ""            ' not found
    END FUNCTION


=====================================================================
26. STACKS AND QUEUES WITH ARRAYS
=====================================================================

26.1  Stack (LIFO: Last In, First Out)

    DIM STACK(100)
    TOP = 0

    ' Push
    SUB Push(VALUE)
      IF TOP >= 100 THEN PRINT "Stack overflow!" : EXIT SUB
      TOP = TOP + 1
      STACK(TOP) = VALUE
    END SUB

    ' Pop
    FUNCTION Pop()
      IF TOP <= 0 THEN PRINT "Stack underflow!" : Pop = -1 : EXIT FUNCTION
      Pop = STACK(TOP)
      TOP = TOP - 1
    END FUNCTION

    ' Usage
    CALL Push(10)
    CALL Push(20)
    CALL Push(30)
    PRINT Pop()               ' 30
    PRINT Pop()               ' 20

26.2  Queue (FIFO: First In, First Out)

    DIM QUEUE(100)
    FRONT = 1 : BACK = 0 : QSIZE = 0

    SUB Enqueue(VALUE)
      IF QSIZE >= 100 THEN PRINT "Queue full!" : EXIT SUB
      BACK = BACK + 1
      IF BACK > 100 THEN BACK = 1     ' wrap around
      QUEUE(BACK) = VALUE
      QSIZE = QSIZE + 1
    END SUB

    FUNCTION Dequeue()
      IF QSIZE <= 0 THEN PRINT "Queue empty!" : Dequeue = -1 : EXIT FUNCTION
      Dequeue = QUEUE(FRONT)
      FRONT = FRONT + 1
      IF FRONT > 100 THEN FRONT = 1   ' wrap around
      QSIZE = QSIZE - 1
    END FUNCTION


=====================================================================
27. SPARSE DATA WITH PARALLEL ARRAYS
=====================================================================

When most elements would be zero or empty, use parallel arrays
instead of a large 2D array:

  ' Instead of DIM SPARSE(1000, 1000) -- impossible!
  ' Store only non-zero values:

  DIM SP_ROW(500)          ' row index of each value
  DIM SP_COL(500)          ' column index
  DIM SP_VAL(500)          ' the value
  SP_COUNT = 0

  SUB SparseSet(R, C, V)
    ' Check if entry exists
    FOR I = 1 TO SP_COUNT
      IF SP_ROW(I) = R AND SP_COL(I) = C THEN
        SP_VAL(I) = V
        EXIT SUB
      END IF
    NEXT I
    ' Add new entry
    SP_COUNT = SP_COUNT + 1
    SP_ROW(SP_COUNT) = R
    SP_COL(SP_COUNT) = C
    SP_VAL(SP_COUNT) = V
  END SUB

  FUNCTION SparseGet(R, C)
    FOR I = 1 TO SP_COUNT
      IF SP_ROW(I) = R AND SP_COL(I) = C THEN
        SparseGet = SP_VAL(I)
        EXIT FUNCTION
      END IF
    NEXT I
    SparseGet = 0              ' default: zero
  END FUNCTION


=====================================================================
28. GAME BOARDS AND GRIDS
=====================================================================

28.1  Tic-Tac-Toe Board

    DIM BOARD(3, 3)

    ' 0 = empty, 1 = X, 2 = O
    SUB DrawBoard()
      FOR R = 1 TO 3
        FOR C = 1 TO 3
          IF BOARD(R, C) = 0 THEN PRINT ".";
          IF BOARD(R, C) = 1 THEN PRINT "X";
          IF BOARD(R, C) = 2 THEN PRINT "O";
          IF C < 3 THEN PRINT "|";
        NEXT C
        PRINT
        IF R < 3 THEN PRINT "-----"
      NEXT R
    END SUB

    ' Check for winner
    FUNCTION CheckWin(PLAYER)
      ' Check rows
      FOR R = 1 TO 3
        IF BOARD(R,1)=PLAYER AND BOARD(R,2)=PLAYER AND BOARD(R,3)=PLAYER THEN
          CheckWin = 1 : EXIT FUNCTION
        END IF
      NEXT R
      ' Check columns
      FOR C = 1 TO 3
        IF BOARD(1,C)=PLAYER AND BOARD(2,C)=PLAYER AND BOARD(3,C)=PLAYER THEN
          CheckWin = 1 : EXIT FUNCTION
        END IF
      NEXT C
      ' Check diagonals
      IF BOARD(1,1)=PLAYER AND BOARD(2,2)=PLAYER AND BOARD(3,3)=PLAYER THEN
        CheckWin = 1 : EXIT FUNCTION
      END IF
      IF BOARD(1,3)=PLAYER AND BOARD(2,2)=PLAYER AND BOARD(3,1)=PLAYER THEN
        CheckWin = 1 : EXIT FUNCTION
      END IF
      CheckWin = 0
    END FUNCTION

28.2  Maze/Map Grid

    DIM MAP(20, 40)
    ' 0=floor, 1=wall, 2=player, 3=exit

    SUB DrawMap()
      FOR R = 1 TO 20
        FOR C = 1 TO 40
          IF MAP(R, C) = 0 THEN PRINT " ";
          IF MAP(R, C) = 1 THEN PRINT "#";
          IF MAP(R, C) = 2 THEN PRINT "@";
          IF MAP(R, C) = 3 THEN PRINT "E";
        NEXT C
        PRINT
      NEXT R
    END SUB

28.3  Conway's Game of Life

    DIM CELLS(20, 40), NEXT_GEN(20, 40)

    FUNCTION CountNeighbors(R, C)
      N = 0
      FOR DR = -1 TO 1
        FOR DC = -1 TO 1
          IF DR = 0 AND DC = 0 THEN GOTO SKIP_SELF
          NR = R + DR : NC = C + DC
          IF NR >= 1 AND NR <= 20 AND NC >= 1 AND NC <= 40 THEN
            N = N + CELLS(NR, NC)
          END IF
          SKIP_SELF:
        NEXT DC
      NEXT DR
      CountNeighbors = N
    END FUNCTION

    SUB NextGeneration()
      FOR R = 1 TO 20
        FOR C = 1 TO 40
          N = CountNeighbors(R, C)
          IF CELLS(R, C) = 1 THEN
            ' Live cell survives with 2 or 3 neighbors
            IF N = 2 OR N = 3 THEN NEXT_GEN(R,C) = 1 ELSE NEXT_GEN(R,C) = 0
          ELSE
            ' Dead cell becomes alive with exactly 3 neighbors
            IF N = 3 THEN NEXT_GEN(R,C) = 1 ELSE NEXT_GEN(R,C) = 0
          END IF
        NEXT C
      NEXT R
      ' Copy next generation back
      FOR R = 1 TO 20
        FOR C = 1 TO 40
          CELLS(R, C) = NEXT_GEN(R, C)
        NEXT C
      NEXT R
    END SUB


=====================================================================
29. HISTOGRAMS AND FREQUENCY TABLES
=====================================================================

29.1  Counting Letter Frequencies

    DIM FREQ(25)              ' A=0, B=1, ..., Z=25

    INPUT "Enter text: "; T$
    T$ = UCASE$(T$)
    FOR I = 1 TO LEN(T$)
      C = ASC(MID$(T$, I, 1))
      IF C >= 65 AND C <= 90 THEN
        FREQ(C - 65) = FREQ(C - 65) + 1
      END IF
    NEXT I

    PRINT "Letter frequencies:"
    FOR I = 0 TO 25
      IF FREQ(I) > 0 THEN
        PRINT CHR$(I + 65); ": "; FREQ(I)
      END IF
    NEXT I

29.2  Horizontal Bar Chart

    ' Display FREQ() as a bar chart
    MAX_F = 0
    FOR I = 0 TO 25
      IF FREQ(I) > MAX_F THEN MAX_F = FREQ(I)
    NEXT I

    FOR I = 0 TO 25
      IF FREQ(I) > 0 THEN
        PRINT CHR$(I + 65); " ";
        BAR_LEN = INT(FREQ(I) * 40 / MAX_F)
        PRINT STRING$(BAR_LEN, "#");
        PRINT " "; FREQ(I)
      END IF
    NEXT I

29.3  Numeric Histogram (Bucketing)

    ' Group values 0-99 into 10 buckets (0-9, 10-19, etc.)
    DIM BUCKETS(9)
    FOR I = 1 TO N
      BUCKET = INT(VALUES(I) / 10)
      IF BUCKET > 9 THEN BUCKET = 9
      BUCKETS(BUCKET) = BUCKETS(BUCKET) + 1
    NEXT I

    FOR I = 0 TO 9
      PRINT I * 10; "-"; I * 10 + 9; ": ";
      PRINT STRING$(BUCKETS(I), "*")
    NEXT I


=====================================================================
30. PERFORMANCE TIPS
=====================================================================

30.1  Pre-compute Array Bounds

  BAD (recomputes UBOUND every iteration):
    FOR I = LBOUND(A,1) TO UBOUND(A,1)
      ...
    NEXT I

  BETTER (compute once):
    LO = LBOUND(A, 1) : HI = UBOUND(A, 1)
    FOR I = LO TO HI
      ...
    NEXT I

30.2  Minimize Array Accesses in Inner Loops

  BAD (accesses A(I) multiple times):
    FOR I = 1 TO N
      IF A(I) > MAX_VAL THEN MAX_VAL = A(I)
      TOTAL = TOTAL + A(I)
    NEXT I

  BETTER (cache in a local variable):
    FOR I = 1 TO N
      V = A(I)
      IF V > MAX_VAL THEN MAX_VAL = V
      TOTAL = TOTAL + V
    NEXT I

30.3  Use MAT Operations Instead of Loops

  MAT operations are implemented in C and are faster than
  equivalent BASIC loops:

  SLOW:
    FOR R = 1 TO N
      FOR C = 1 TO N
        C(R, C) = A(R, C) + B(R, C)
      NEXT C
    NEXT R

  FAST:
    MAT C = A + B

30.4  Use 1D Arrays When Possible

  1D arrays are slightly faster than 2D arrays because the
  element address calculation is simpler (base + offset vs.
  base + row * cols + col).

30.5  Avoid Unnecessary DIM/ERASE Cycles

  If you can reuse an array by zeroing it, do so:

    ' Instead of ERASE A : DIM A(100)
    FOR I = 0 TO 100 : A(I) = 0 : NEXT I


=====================================================================
31. LIMITS AND TROUBLESHOOTING
=====================================================================

31.1  Hard Limits

  Limit                    Value       Notes
  -----                    -----       -----
  Max arrays               1024        Increase MAX_DIM_ARRAYS
  Max dimensions           3           Max dimensions allowed
  Max elements (total)     4194304     Shared across all arrays
  Max subscript value      65529       Same as max line number
  Max array name length    31 chars    Same as variable names

31.2  Common Errors

  "Subscript out of range"
    Cause: Index outside declared bounds
    Fix:   Check subscript against DIM size
           Verify OPTION BASE setting

  "SORRY. Out of array space"
    Cause: Element pool exhausted
    Fix:   ERASE unneeded arrays
           Reduce array sizes
           Increase MAX_ARRAY_ELEMENTS in config.h

  "Duplicate definition"
    Cause: DIM array that was already DIMmed
    Fix:   Use ERASE before re-DIMming, or use REDIM

  "Type mismatch"
    Cause: Using numeric operations on string array, or
           assigning string to numeric array
    Fix:   Match variable types ($ suffix for strings)

  "Dimension mismatch" (MAT operations)
    Cause: Matrix dimensions don't match for the operation
    Fix:   Verify m x n requirements for multiply, add, etc.

31.3  Debugging Tips

  * Use VARS to dump all defined arrays and their sizes
  * Use PRINT SIZE("ARRAYS") to check element pool usage
  * Use TRON to trace which lines access arrays
  * Add temporary PRINT statements to display subscripts:
      PRINT "Accessing A("; I; ","; J; ")"

31.4  Portability Notes

  * Array element storage uses BValue (tagged union), which
    means each element is approximately 12-16 bytes depending
    on platform (sizeof(BValue)).
  * On 16-bit platforms (FreeDOS), keep total element counts
    small to fit within conventional memory.
  * MAT operations require a dialect that supports them
    (QBasic, BASIC++, or Dartmouth).


=====================================================================
END OF DOCUMENT
=====================================================================
