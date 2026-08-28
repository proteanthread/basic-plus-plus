# BASIC++ v6.5.2 Build Your Own Language

## 1. OVERVIEW

BASIC++ is designed to support user-defined dialects and even non-BASIC languages through its virtual device abstractions and dialect configuration system. This guide explains how to create a custom language that runs on the BASIC++ VM.

## 2. DIALECT DEFINITION

The simplest approach is to define a custom dialect that selects which keywords are available and how they behave:

```basic
10 DIALECT DEFINE "LOGO"
20 DIALECT ALLOW "FORWARD", "BACK", "LEFT", "RIGHT", "PENUP", "PENDOWN"
30 DIALECT DENY "PRINT", "INPUT", "GOTO", "GOSUB"
40 DIALECT ACTIVATE "LOGO"
```

After activation, only the allowed keywords are recognized. Programs using denied keywords get syntax errors.

## 3. KEYWORD REGISTRATION

The KEYWORD statement registers new keywords that map to BASIC++ subroutines:

```basic
10 KEYWORD "FORWARD" AS 5000
20 KEYWORD "BACK" AS 5100
30 KEYWORD "LEFT" AS 5200
40 KEYWORD "RIGHT" AS 5300
50 KEYWORD "PENUP" AS 5400
60 KEYWORD "PENDOWN" AS 5500
```

Each keyword executes its mapped subroutine when used in a program. The subroutine receives arguments through the keyword parameter mechanism.

## 4. BUILDING A LOGO INTERPRETER

A complete Logo-like turtle graphics language on top of BASIC++:

```basic
' Turtle state
100 TurtleX = 160 : TurtleY = 100
110 TurtleAngle = 0 : PenDown = 1
120 SCREEN 12

5000 ' FORWARD distance
5010 Distance = KEYWORD.PARAM
5020 NewX = TurtleX + COS(TurtleAngle * PI / 180) * Distance
5030 NewY = TurtleY + SIN(TurtleAngle * PI / 180) * Distance
5040 IF PenDown THEN LINE (TurtleX,TurtleY)-(NewX,NewY), 15
5050 TurtleX = NewX : TurtleY = NewY
5060 RETURN

5100 ' BACK distance
5110 Distance = -KEYWORD.PARAM
5120 GOTO 5020    ' Reuse FORWARD logic

5200 ' LEFT degrees
5210 TurtleAngle = TurtleAngle - KEYWORD.PARAM
5220 RETURN

5300 ' RIGHT degrees
5310 TurtleAngle = TurtleAngle + KEYWORD.PARAM
5320 RETURN

5400 ' PENUP
5410 PenDown = 0
5420 RETURN

5500 ' PENDOWN
5510 PenDown = 1
5520 RETURN
```

Now a user can write Logo programs:

```basic
200 FORWARD 50
210 RIGHT 90
220 FORWARD 50
230 RIGHT 90
240 FORWARD 50
250 RIGHT 90
260 FORWARD 50
```

## 5. ALIAS-BASED LOCALIZATION

For creating a localized BASIC, use ALIAS to translate all keywords:

```basic
10 ALIAS "IMPRIMIR" AS "PRINT"
20 ALIAS "ENTRADA" AS "INPUT"
30 ALIAS "SE" AS "IF"
40 ALIAS "ENTAO" AS "THEN"
50 ALIAS "SENAO" AS "ELSE"
60 ALIAS "PARA" AS "FOR"
70 ALIAS "PROXIMO" AS "NEXT"
80 ALIAS "FIM" AS "END"
```

Save these aliases in a startup file and MERGE it before the main program.

## 6. OVERRIDE-BASED EXTENSIONS

OVERRIDE adds pre/post processing to existing keywords:

```basic
10 OVERRIDE "PRINT" WITH 9000
9000 ' Add timestamp to all PRINT output
9010 PRINT.ORIGINAL TIME$; " | "; OVERRIDE.PARAM$
9020 RETURN
```

## 7. MODULE-BASED EXTENSIONS

For heavy-duty language extensions, write a C17 module that registers keywords, functions, and devices through the module API. See the External Modules guide for details.

## 8. THE SPECIFICATION SYSTEM

Use the SPEC statement family to formally define and validate your custom language:

```basic
10 SPEC DEFINE "MyLanguage"
20 SPEC KEYWORD "FORWARD", "movement", "FORWARD distance"
30 SPEC KEYWORD "BACK", "movement", "BACK distance"
40 SPEC FUNCTION "HEADING", "()", "NUMBER", "Returns current angle"
50 SPEC VALIDATE
60 SPEC SAVE "mylanguage.spec"
```

The specification file can be distributed to other users who load it with SPEC LOAD.

## 9. LIMITATIONS

Custom languages built on BASIC++ are limited to the capabilities of the BASIC++ VM. They cannot introduce new data types, new operators, or new control flow constructs beyond what the VM supports. The expression evaluator, variable system, and execution model remain BASIC++. Custom keywords are dispatched as subroutine calls, not as native statement handlers.

For truly novel languages, use the bppc transpiler to compile your language to C17 code that links with the BASIC++ runtime libraries.
