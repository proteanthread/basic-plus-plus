<!--
Title:        catalog
Tier:         4
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, detok, trans)
Authority:    engine/include/runtime/language_descriptor.h
Generated:    yes, from LanguageDescriptor metadata
Status:       Active
-->

# BASIC++ v6.5.2 Master Inventory Catalog

This catalog provides an organized directory of all active built-in statements, intrinsic functions, and special variables in BASIC++ v6.5.2. Each item links directly to its authoritative Tier 3 reference documentation under `docs/keywords/`.

## Control Flow & Looping (41 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [break](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/break.md) | Statement | `BREAK` |
| [CALL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CALL.md) | Statement | `CALL name [(argument_list)]` |
| [CAUSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CAUSE.md) | Statement | `CAUSE ERROR error_code` |
| [CONTINUE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CONTINUE.md) | Statement | `CONTINUE` |
| [DECLARE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DECLARE.md) | Statement | `DECLARE {SUB \| FUNCTION} name [ALIAS "aliasname"] [(params)]` |
| [DO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DO.md) | Statement | `DO [{WHILE\|UNTIL} condition]` |
| [DOEVENTS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DOEVENTS.md) | Function | `numForms% = DOEVENTS()` |
| [END](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/END.md) | Statement | `END [IF \| SUB \| FUNCTION \| SELECT \| STRUCT]` |
| [ENDLOOP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ENDLOOP.md) | Statement | `ENDLOOP` |
| [EXIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXIT.md) | Statement | `EXIT {FOR\|DO\|WHILE\|SUB\|FUNCTION}` |
| [EXTERN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXTERN.md) | Statement | `EXTERN {SUB \| FUNCTION} name [ALIAS "aliasname"] [(params)]` |
| [EXTERNAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXTERNAL.md) | Statement | `EXTERNAL SUB\|FUNCTION name [(parameter_list)]` |
| [FOR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FOR.md) | Statement | `FOR var = start TO end [STEP step]` |
| [FUNCTION](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FUNCTION.md) | Statement | `FUNCTION name [(parameter_list)] ... END FUNCTION` |
| [GOSUB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GOSUB.md) | Statement | `GOSUB line_num \| expr \| label` |
| [GOTO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GOTO.md) | Statement | `GOTO line_num \| expr \| label` |
| [HANDLER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/HANDLER.md) | Statement | `HANDLER name$ ... END HANDLER` |
| [IF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/IF.md) | Statement | `IF expr THEN stmt/line [ELSE stmt/line]` |
| [IF_POSTFIX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/IF_POSTFIX.md) | Statement | `statement IF condition [ELSE statement]` |
| [LOOP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOOP.md) | Statement | `LOOP [{WHILE\|UNTIL} condition] \| LOOP (BASIC09 block opener)` |
| [NEXT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NEXT.md) | Statement | `NEXT [var1[, var2...]] \| NEXT [var1 [var2...]]` |
| [ON_GOSUB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_GOSUB.md) | Statement | `ON expr GOSUB line1 [, line2, ...]` |
| [ON_GOTO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_GOTO.md) | Statement | `ON expr GOTO line1 [, line2, ...]` |
| [PERFORM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PERFORM.md) | Statement | `PERFORM routine_name [(arg1, arg2, ...)]` |
| [REM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REM.md) | Statement | `REM [comment text] or ' [comment text]` |
| [REPEAT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REPEAT.md) | Statement | `REPEAT` |
| [RETRY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RETRY.md) | Statement | `RETRY` |
| [RETURN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RETURN.md) | Statement | `RETURN [line_num]` |
| [SELECT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SELECT.md) | Statement | `SELECT CASE test_expression ... CASE expression_list ... END SELECT` |
| [SLEEP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SLEEP.md) | Statement | `SLEEP [seconds]` |
| [STOP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/STOP.md) | Statement | `STOP` |
| [SUSPEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SUSPEND.md) | Statement | `SUSPEND [TIMER \| KEY \| COM \| TASK id \| EVENT name$ \| expr] [, timeout]` |
| [TASK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TASK.md) | Statement | `TASK [filename_expr$ \| ::label \| LIST \| WAIT task_id \| KILL task_id]` |
| [UNLESS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNLESS.md) | Statement | `UNLESS expr [THEN] stmt/line [ELSE stmt/line]` |
| [UNTIL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNTIL.md) | Statement | `UNTIL condition` |
| [VOID](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VOID.md) | Statement | `VOID expression` |
| [WAIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WAIT.md) | Statement | `WAIT seconds \| WAIT port, and_mask [, xor_mask] \| WAIT #channel, seconds` |
| [WEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WEND.md) | Statement | `WEND` |
| [WHEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WHEN.md) | Statement | `WHEN ERROR IN ... USE ... END WHEN` |
| [WHILE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WHILE.md) | Statement | `WHILE condition` |
| [YIELD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/YIELD.md) | Statement | `YIELD` |

## Event Trapping & Exceptions (15 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ERR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ERR$.md) | Function | `ERR$ [(error_code%)]` |
| [ERROR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ERROR.md) | Statement | `ERROR error_code%` |
| [KEY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/KEY.md) | Statement | `KEY ON \| KEY OFF \| KEY LIST \| KEY n, string$ \| KEY(n) {ON\|OFF\|STOP}` |
| [ON COM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON COM.md) | Statement | `ON COM(n) GOSUB line_num` |
| [ON ERROR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON ERROR.md) | Statement | `ON ERROR GOTO {line_label \| 0}` |
| [ON KEY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON KEY.md) | Statement | `ON KEY(n) GOSUB line_label \| KEY(n) {ON\|OFF\|STOP} \| KEY ON \| KEY OFF \| KEY n, string` |
| [ON TIMER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON TIMER.md) | Statement | `ON TIMER(seconds) GOSUB line_label \| TIMER {ON\|OFF\|STOP}` |
| [ONKEY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ONKEY$.md) | Statement | `ONKEY$(key_code$) GOSUB line` |
| [ON_ERROR_GOTO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_ERROR_GOTO.md) | Statement | `ON ERROR GOTO {line_number \| label \| 0}` |
| [ON_KEY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_KEY.md) | Statement | `ON KEY(n) GOSUB line \| KEY(n) {ON\|OFF\|STOP}` |
| [ON_PLAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_PLAY.md) | Statement | `ON PLAY(n) GOSUB line \| PLAY {ON\|OFF\|STOP}` |
| [ON_TIMER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ON_TIMER.md) | Statement | `ON TIMER(seconds) GOSUB line \| TIMER {ON\|OFF\|STOP}` |
| [RESUME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RESUME.md) | Statement | `RESUME [0 \| NEXT \| line_label \| TASK id \| EVENT name$]` |
| [TRY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TRY.md) | Statement | `TRY ... CATCH err_var ... FINALLY ... END TRY` |
| [WHENEVER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WHENEVER.md) | Statement | `WHENEVER {ERROR \| [NOT] EOF #channel} THEN {GOTO line \| statement}` |

## Filesystem & Stream I/O (52 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ACCESS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ACCESS.md) | Statement | `ACCESS filepath$ [, mode%]` |
| [ADVANCE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ADVANCE.md) | Statement | `ASSIGN #channel TO "filespec$" [, BUFFER n]` |
| [APPEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/APPEND.md) | Statement | `APPEND [#]channel, "filespec$"` |
| [ASSIGN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ASSIGN.md) | Statement | `ASSIGN #channel TO "filespec$" [, BUFFER n]` |
| [BACKSPACE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BACKSPACE.md) | Statement | `BACKSPACE [#]channel` |
| [BLOAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BLOAD.md) | Statement | `BLOAD filename$ [, offset%]` |
| [BRUN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BRUN.md) | Statement | `BRUN filename$ [, address%]` |
| [BSAVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BSAVE.md) | Statement | `BSAVE filename$, offset%, length%` |
| [CHDIR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CHDIR.md) | Statement | `CHDIR pathname$` |
| [CLOSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CLOSE.md) | Statement | `CLOSE [[#]file_num1[, [#]file_num2...]]` |
| [CREATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CREATE.md) | Statement | `CREATE filename$ [, type] \| CREATE #ch, filename$ [: mode]` |
| [CURDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CURDIR$.md) | Function | `CURDIR$([drive$])` |
| [DESTROY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DESTROY.md) | Statement | `DESTROY filename$` |
| [DIR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DIR.md) | Statement | `DIR [filespec$] [, attributes%]` |
| [DIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DIR$.md) | Function | `DIR$([filespec$])` |
| [EOF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EOF.md) | Function | `flag% = EOF(file_num%)` |
| [EXISTS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXISTS.md) | Statement | `EXISTS(path$) \| EXISTS path$` |
| [FILEMOD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FILEMOD.md) | Statement | `FILEMOD(path$)` |
| [FILEMOD$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FILEMOD$.md) | Statement | `FILEMOD$(path$)` |
| [FILES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FILES.md) | Statement | `FILES [filespec]` |
| [FILESIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FILESIZE.md) | Statement | `FILESIZE(path$)` |
| [FIND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FIND.md) | Statement | `FIND [#]channel [, RECORD record_number]` |
| [GET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GET.md) | Statement | `GET [#]file_num [, record_number] \| GET (x1, y1)-(x2, y2), array_name` |
| [INPUT#](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INPUT#.md) | Statement | `INPUT #file_num, var1 [, var2...] \| LINE INPUT [#file_num,] string_var` |
| [KILL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/KILL.md) | Statement | `KILL filespec` |
| [LOC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOC.md) | Function | `pos& = LOC(file_num%)` |
| [LOCK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOCK.md) | Statement | `LOCK filepath$ \| LOCK [#]file_num [, [record_start] [TO record_end]]` |
| [LOF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOF.md) | Function | `length& = LOF(file_num%)` |
| [MKDIR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MKDIR.md) | Statement | `MKDIR pathname$` |
| [MODIFY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MODIFY.md) | Statement | `MODIFY #channel, "field_spec"` |
| [MOUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MOUNT.md) | Statement | `MOUNT drive_letter$, device_uri$` |
| [MOUNTS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MOUNTS.md) | Statement | `MOUNTS` |
| [NAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NAME.md) | Statement | `NAME oldspec AS newspec` |
| [OPEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OPEN.md) | Statement | `OPEN filespec [FOR mode] AS [#]file_num [LEN=reclen]` |
| [PREFIX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PREFIX.md) | Statement | `PREFIX dir_path$` |
| [PRINT#](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PRINT#.md) | Statement | `PRINT #file_num, expression_list` |
| [PUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PUT.md) | Statement | `PUT [#]file_num [, record_number] \| PUT (x, y), array_name [, action]` |
| [PWD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PWD.md) | Statement | `PWD` |
| [QLOAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/QLOAD.md) | Statement | `QLOAD filename$ [, address%]` |
| [QRUN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/QRUN.md) | Statement | `QRUN filename$ [, param$]` |
| [QSAVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/QSAVE.md) | Statement | `QSAVE filename$, offset [, length [, payload_type%]] [, {properties}]` |
| [RECORD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RECORD.md) | Statement | `RECORD [#]channel, record_number \| RECORD record_name ... END RECORD` |
| [RESET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RESET.md) | Statement | `RESET` |
| [REWIND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REWIND.md) | Statement | `REWIND [#]channel` |
| [RMDIR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RMDIR.md) | Statement | `RMDIR pathname$` |
| [SEEK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEEK.md) | Statement | `SEEK [#]file_num, position` |
| [TEXT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TEXT.md) | Statement | `TEXT #channel, "encoding_spec"` |
| [TYP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TYP.md) | Function | `type_code% = TYP(#channel) \| TYP(channel)` |
| [UMOUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UMOUNT.md) | Statement | `UMOUNT drive_letter$` |
| [UNLOCK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNLOCK.md) | Statement | `UNLOCK [#]file_num [, [record_start] [TO record_end]]` |
| [VPATH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VPATH.md) | Statement | `VPATH [search_path$]` |
| [WRITE#](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WRITE#.md) | Statement | `WRITE #file_num, expression_list` |

## General & Extended (240 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [AND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/AND.md) | Function | `AND(val1, val2 [, ...]) or val1 AND val2` |
| [ARRAY EXT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ARRAY EXT.md) | Statement | `ARRAY.SORT / ARRAY.REVERSE / ARRAY.FILL` |
| [ARRAYFILL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ARRAYFILL.md) | Statement | `ARRAYFILL array_name(), fill_value` |
| [ASK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ASK.md) | Statement | `ASK property variable [, ...]` |
| [assert](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/assert.md) | Statement | `ASSERT condition [, message$]` |
| [AT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/AT.md) | Statement | `PRINT AT x, y [, fg [, bg]] \| AT(x, y [, fg [, bg]]) \| AT[px, py] \| AT{prop: val, ...}` |
| [AVG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/AVG.md) | Function | `AVG(val1, val2 [, ...]) or AVG(arr) or AVG{...} or AVG[arr]` |
| [BIOS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/BIOS.md) | Function | `BIOS(int_num [, ax, bx, cx, dx]) \| BIOS(op$, ...)` |
| [BIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/BIT.md) | Function | `BIT(val, bit)` |
| [BITCOUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/BITCOUNT.md) | Function | `BITCOUNT(val)` |
| [BITFIELD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/BITFIELD.md) | Function | `BITFIELD(val, start, len) or BITFIELD[val, start, len]` |
| [BUTTON](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BUTTON.md) | Statement | `BUTTON caption$ [, col%, row%, width%, height%]` |
| [CBYTE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CBYTE.md) | Function | `b = CBYTE(x) \| BYTE(x)` |
| [CDWORD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CDWORD.md) | Function | `dw = CDWORD(x) \| DWORD(x)` |
| [CHECKBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CHECKBOX.md) | Statement | `CHECKBOX caption$ [, checked% [, col%, row%, width%]]` |
| [CLASS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CLASS.md) | Statement | `CLASS class_name [EXTENDS parent]` |
| [CLK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CLK.md) | Variable | `CLK or CLK()` |
| [CLK$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CLK$.md) | Variable | `CLK$ or CLK$()` |
| [CLOCK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CLOCK.md) | Variable | `CLOCK or CLOCK()` |
| [CLOCK$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CLOCK$.md) | Variable | `CLOCK$ or CLOCK$()` |
| [CLRBIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CLRBIT.md) | Function | `CLRBIT(val, bit)` |
| [COMBOBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/COMBOBOX.md) | Statement | `COMBOBOX items$ [, sel% [, col%, row%, width%]]` |
| [CONSOLE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CONSOLE.md) | Statement | `CONSOLE [device$ \| mode%]` |
| [COUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COUNT.md) | Function | `COUNT(arr_or_str [, match])` |
| [COUNT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COUNT$.md) | Function | `COUNT$(arr$ [, match$]) \| COUNT$(dyn$ [, delim$])` |
| [CPU](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CPU.md) | Function | `CPU(op$, ...) \| CPU86(op$, ...)` |
| [CREATEINDEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CREATEINDEX.md) | Statement | `CREATEINDEX #ch, idx$, len [, dups]` |
| [CRYPTO.DECRYPT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CRYPTO.DECRYPT$.md) | Function | `CRYPTO.DECRYPT$(key$, ciphertext_hex$)` |
| [CRYPTO.ENCRYPT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CRYPTO.ENCRYPT$.md) | Function | `CRYPTO.ENCRYPT$(key$, plaintext$)` |
| [CRYPTO.HASH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CRYPTO.HASH$.md) | Function | `CRYPTO.HASH$(data$)` |
| [CRYPTO.HMAC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CRYPTO.HMAC$.md) | Function | `CRYPTO.HMAC$(key$, data$)` |
| [CRYPTO.KEY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CRYPTO.KEY$.md) | Function | `CRYPTO.KEY$([bits%])` |
| [CSRLIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CSRLIN.md) | Variable | `CSRLIN or CSRLIN()` |
| [CVD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVD.md) | Function | `CVD(8byte_str$)` |
| [CVI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVI.md) | Function | `CVI(2byte_str$)` |
| [CVS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVS.md) | Function | `CVS(4byte_str$)` |
| [CWORD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CWORD.md) | Function | `w = CWORD(x) \| WORD(x)` |
| [DEF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEF.md) | Statement | `DEF FNname[(args)] = expr` |
| [DEF USR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEF USR.md) | Statement | `DEF USR[digit%] = address%` |
| [DEF_FN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEF_FN.md) | Statement | `DEF FNname[(args)] = expr` |
| [DELAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DELAY.md) | Statement | `DELAY ms \| DELAY.MS ms \| DELAY.US us` |
| [DELETE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DELETE.md) | Statement | `DELETE #ch` |
| [DELETE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DELETE$.md) | Function | `DELETE$(target$, start, count) \| DELETE(arr, pos [, count]) \| DELETE$(dyn$, attr)` |
| [DELETEINDEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DELETEINDEX.md) | Statement | `DELETEINDEX #ch, idx$` |
| [DEMAND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEMAND.md) | Statement | `DEMAND var1 [, var2 ...] [IN min TO max] [DEFAULT def_val]` |
| [DEREF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEREF.md) | Function | `val = DEREF(ptr)` |
| [DEVCAPS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEVCAPS.md) | Function | `DEVCAPS(dev_name$)` |
| [DEVCTL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEVCTL.md) | Function | `DEVCTL(dev$, cmd[, arg1, arg2])` |
| [DEVCTL$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEVCTL$.md) | Function | `DEVCTL$(dev$, cmd[, arg1, arg2])` |
| [DEVICE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEVICE.md) | Statement | `DEVICE "dev_name:", "config_param=val"` |
| [DEVICES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEVICES.md) | Statement | `DEVICE "dev_name:", "config_param=val"` |
| [DEVINFO$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEVINFO$.md) | Function | `DEVINFO$(dev_name$, property$)` |
| [DIM #](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DIM #.md) | Statement | `DIM #channel, array_name(bounds)` |
| [DIRLISTBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DIRLISTBOX.md) | Statement | `DIRLISTBOX path$, col%, row%, width%, height%` |
| [DISPLAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DISPLAY.md) | Statement | `DISPLAY [exprlist]` |
| [DRIVELISTBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DRIVELISTBOX.md) | Statement | `DRIVELISTBOX col%, row%, width%` |
| [DUMP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DUMP.md) | Statement | `DUMP [var_or_address [, length%]]` |
| [DYNARRAY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DYNARRAY$.md) | Function | `DYNARRAY$(set_or_group)` |
| [ECHO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ECHO.md) | Statement | `ECHO [ON \| OFF]` |
| [ENTER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ENTER.md) | Statement | `ENTER [#channel,] timeout_sec, status_var, string_var` |
| [EPOCH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EPOCH.md) | Variable | `EPOCH or EPOCH()` |
| [EPOCH.MS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EPOCH.MS.md) | Variable | `EPOCH.MS` |
| [EPS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EPS.md) | Variable | `EPS or EPS()` |
| [EQV](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EQV.md) | Function | `EQV(val1, val2) or val1 EQV val2` |
| [EVERY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EVERY.md) | Statement | `EVERY ms GOSUB line_num` |
| [EXTEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXTEND.md) | Statement | `EXTEND` |
| [EXTRACT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EXTRACT.md) | Function | `EXTRACT(dyn_arr, attr [, val [, subval]])` |
| [FALSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/FALSE.md) | Variable | `FALSE or FALSE()` |
| [FID](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FID.md) | Function | `FID(channel) / FIN(channel)` |
| [FIELD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FIELD.md) | Function | `FIELD(str_expr, delim_expr, instance_expr [, count_expr])` |
| [FIFO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FIFO.md) | Function | `FIFO(op$, [val]) \| FIFO.PUSH(val) \| FIFO.POP()` |
| [FILELISTBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FILELISTBOX.md) | Statement | `FILELISTBOX filter$, col%, row%, width%, height%` |
| [FILEOPENBOX$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FILEOPENBOX$.md) | Function | `f$ = FILEOPENBOX$([pattern$ [, title$ [, default$]]])` |
| [FILESAVEBOX$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FILESAVEBOX$.md) | Function | `f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]])` |
| [FORM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FORM.md) | Statement | `FORM format_specifier_list` |
| [FRAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FRAME.md) | Statement | `FRAME title$ [, col%, row%, width%, height%]` |
| [FUJI.IP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FUJI.IP$.md) | Function | `FUJI.IP$()` |
| [FUJI.JSON.GET$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FUJI.JSON.GET$.md) | Function | `FUJI.JSON.GET$(url$, json_path$)` |
| [FUJI.SSID$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FUJI.SSID$.md) | Function | `FUJI.SSID$()` |
| [FUJI.STATUS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FUJI.STATUS$.md) | Function | `FUJI.STATUS$()` |
| [GMST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/GMST.md) | Variable | `GMST or GMST()` |
| [HEBREW$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HEBREW$.md) | Variable | `HEBREW$` |
| [HI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HI.md) | Function | `HI(val%)` |
| [HSCROLLBAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/HSCROLLBAR.md) | Statement | `HSCROLLBAR min%, max%, val%, col%, row%, width%` |
| [IMAGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/IMAGE.md) | Statement | `IMAGE: format_template_specifiers` |
| [IMP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IMP.md) | Function | `IMP(val1, val2) or val1 IMP val2` |
| [IMPORT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/IMPORT.md) | Statement | `MODULE "ModuleName" [EXPORTS sym1, sym2, ...]` |
| [IN#](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/IN#.md) | Statement | `IN# slot_number` |
| [INDEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INDEX.md) | Function | `INDEX(arr_or_str, target [, start_pos])` |
| [INDEX$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INDEX$.md) | Function | `INDEX$(arr_or_str, target [, start_pos])` |
| [INF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/INF.md) | Variable | `INF or INF()` |
| [INKEY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INKEY.md) | Function | `INKEY [(timeout_ms)]` |
| [INKEY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/INKEY$.md) | Variable | `INKEY$ or INKEY$()` |
| [INPUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INPUT.md) | Statement | `INPUT [;] ["prompt";] variable[, ...]` |
| [INPUT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INPUT$.md) | Function | `INPUT$(n% [, [#]channel%])` |
| [INPUTBOX$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INPUTBOX$.md) | Function | `res$ = INPUTBOX$(prompt$ [, title$ [, default$]])` |
| [INSERT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INSERT.md) | Statement | `INSERT #ch [, record]` |
| [INSERT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INSERT$.md) | Function | `INSERT$(target$, source$, pos) \| INSERT(arr, pos, val) \| INSERT$(dyn$, attr, val$)` |
| [INTERRUPT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INTERRUPT.md) | Statement | `CALL INTERRUPT(int_num%, inregs, outregs) / INTERRUPT int_num%, inregs, outregs` |
| [INVOKE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INVOKE.md) | Statement | `INVOKE driver_path$` |
| [ISAM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ISAM.md) | Function | `is_isam = ISAM(ch) \| KEYED(ch)` |
| [JD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/JD.md) | Variable | `JD or JD()` |
| [JIFFIES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/JIFFIES.md) | Variable | `JIFFIES or JIFFIES()` |
| [JULIAN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/JULIAN.md) | Variable | `JULIAN or JULIAN()` |
| [JULIAN$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/JULIAN$.md) | Variable | `JULIAN$ or JULIAN$()` |
| [KEY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/KEY$.md) | Function | `k$ = KEY$(ch) \| KEY$[ch]` |
| [KEYCOUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/KEYCOUNT.md) | Function | `cnt = KEYCOUNT(ch) \| KEYCOUNT[ch]` |
| [LABEL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LABEL.md) | Statement | `LABEL text$ [, col%, row%, width%]` |
| [LBOUND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LBOUND.md) | Function | `low% = LBOUND(array [, dimension%])` |
| [LIFO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LIFO.md) | Function | `LIFO(op$, [val]) \| LIFO.PUSH(val) \| LIFO.POP()` |
| [LINE INPUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LINE INPUT.md) | Statement | `LINE INPUT [;] ["prompt";] string_var$` |
| [LINPUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LINPUT.md) | Statement | `LINPUT [;] ["prompt";] string_var$` |
| [LISTBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LISTBOX.md) | Statement | `LISTBOX text$ [, col%, row%, width%, height%]` |
| [LO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LO.md) | Function | `LO(val%)` |
| [LOCATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOCATE.md) | Statement | `LOCATE [row] [, [col] [, [cursor] [, [start] [, stop]]]]` |
| [LOCKED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOCKED.md) | Function | `is_locked = LOCKED(ch, id) \| LOCKED[ch, id]` |
| [LOGIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGIN.md) | Statement | `LOGIN username$ [, account$] \| HELLO username$ [, account$]` |
| [LPOS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/LPOS.md) | Variable | `LPOS or LPOS(0)` |
| [LPRINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LPRINT.md) | Statement | `LPRINT [USING format$;] expression_list [; \| ,]` |
| [MARGIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MARGIN.md) | Statement | `MARGIN [#channel,] width` |
| [MASK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MASK.md) | Function | `res = MASK(val, mask) \| MASK[mask](val)` |
| [MAT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAT.md) | Statement | `MAT var = expr` |
| [MAT INPUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAT INPUT.md) | Statement | `MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])]` |
| [MAT PRINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAT PRINT.md) | Statement | `MAT PRINT [#file_num,] array_name [;\|,]` |
| [MAT READ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAT READ.md) | Statement | `MAT READ [#channel,] array_name [(num_rows [, num_cols])]` |
| [MAT WRITE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAT WRITE.md) | Statement | `MAT WRITE [#channel,] array_name [;\|,]` |
| [MAT_IDN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAT_IDN.md) | Function | `MAT_IDN(rows [, cols])` |
| [MAT_INV](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAT_INV.md) | Function | `MAT_INV(matrix)` |
| [MAT_RND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAT_RND.md) | Function | `MAT_RND(rows, cols)` |
| [MAT_TRN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAT_TRN.md) | Function | `MAT_TRN(matrix)` |
| [MAXNUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/MAXNUM.md) | Variable | `MAXNUM or MAXNUM()` |
| [MEAN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MEAN.md) | Function | `MEAN(val1, val2 [, ...]) or MEAN(arr) or MEAN{...} or MEAN[arr]` |
| [MED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MED.md) | Function | `MED(val1, val2, ...)` |
| [MENU](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MENU.md) | Statement | `MENU [menu_title$,] item_caption$` |
| [MESG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MESG.md) | Function | `MESG / MESG$()` |
| [MJD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/MJD.md) | Variable | `MJD or MJD()` |
| [MKDSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MKDSTR.md) | Function | `MKDSTR(dbl_val#) \| MKD$(dbl_val#)` |
| [MKISTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MKISTR.md) | Function | `MKISTR(int_val%) \| MKI$(int_val%)` |
| [MKSSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MKSSTR.md) | Function | `MKSSTR(sng_val!) \| MKS$(sng_val!)` |
| [MODDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MODDIR$.md) | Function | `modules$ = MODDIR$ \| count% = MODDIR(0)` |
| [MOUSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MOUSE.md) | Statement | `MOUSE ON \| OFF \| SHOW \| HIDE \| INPUT` |
| [MSGBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MSGBOX.md) | Function | `result% = MSGBOX(prompt$ [, buttons% [, title$]])` |
| [MSGRECV$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MSGRECV$.md) | Function | `MSGRECV$(source$ [, timeout_ms])` |
| [MSGSEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MSGSEND.md) | Statement | `MSGSEND target$, payload$` |
| [MUTEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MUTEX.md) | Function | `m = MUTEX(name$) \| MUTEX_LOCK(m) \| MUTEX_UNLOCK(m)` |
| [MUX VAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MUX VAR.md) | Statement | `MUX channel, state` |
| [NOECHO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NOECHO.md) | Statement | `NO ECHO \| NOECHO` |
| [NOEXTEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NOEXTEND.md) | Statement | `NO EXTEND \| NOEXTEND` |
| [NOMARGIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NOMARGIN.md) | Statement | `NOMARGIN [#channel]` |
| [NOPAGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NOPAGE.md) | Statement | `NOPAGE [#channel]` |
| [NOT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NOT.md) | Function | `NOT(val) or NOT val` |
| [NUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NUM.md) | Function | `NUM(expr)` |
| [OPTIONBUTTON](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OPTIONBUTTON.md) | Statement | `OPTIONBUTTON caption$ [, selected% [, col%, row%, width%]]` |
| [OR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/OR.md) | Function | `OR(val1, val2 [, ...]) or val1 OR val2` |
| [OUT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OUT.md) | Statement | `OUT port, data` |
| [PARAM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PARAM.md) | Statement | `PARAM var1 [: type] [, var2 [: type] ...]` |
| [PARSE_DYNARRAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PARSE_DYNARRAY.md) | Function | `PARSE_DYNARRAY(dyn_str$) \| GROUP_MAP(dyn_str$)` |
| [PAUSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PAUSE.md) | Statement | `PAUSE [seconds \| prompt$]` |
| [PEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PEN.md) | Statement | `PEN ON \| OFF \| STOP` |
| [PI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PI.md) | Variable | `PI or PI()` |
| [PIPE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PIPE.md) | Statement | `PIPE #src_ch TO #dst_ch [BUFFER size] \| STREAMPIPE #src, #dst` |
| [POP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/POP.md) | Statement | `POP [FIFO\|LIFO\|LILO\|FILO] [, channel$] var` |
| [POS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/POS.md) | Variable | `POS or POS(0)` |
| [PR#](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PR#.md) | Statement | `PR# slot_number` |
| [PRINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PRINT.md) | Statement | `PRINT [#n,] [AT x, y \| AT(x, y) \| AT[x, y] \| AT{map}] [@z \| @(z) \| @[z] \| @{map}] [exprlist] [;\|,]` |
| [PRINT AT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PRINT AT.md) | Statement | `PRINT AT(x, y) [expr] \| PRINT @pos [expr] \| PRINT AT{x: col, y: row} [expr]` |
| [PRIORITY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PRIORITY.md) | Statement | `PRIORITY priority_level%` |
| [PTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PTR.md) | Function | `p = PTR(addr) \| PTR[addr]` |
| [PUBLISH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PUBLISH.md) | Statement | `PUBLISH topic$, payload$` |
| [PUBSUB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PUBSUB.md) | Statement | `PUBSUB topic$, payload$ \| PUBSUB.SUB topic$ \| PUBSUB.UNSUB topic$` |
| [PUSH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PUSH.md) | Statement | `PUSH [FIFO\|LIFO\|LILO\|FILO] [, channel$] expr` |
| [PYTHON](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PYTHON.md) | Statement | `PYTHON code$` |
| [PYTHON$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PYTHON$.md) | Function | `PYTHON$(expr$)` |
| [RAISE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RAISE.md) | Statement | `RAISE [SIGNAL] topic$, payload$` |
| [RAISESIGNAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RAISESIGNAL.md) | Statement | `RAISESIGNAL topic$, payload$` |
| [RANDOMIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RANDOMIZE.md) | Statement | `RANDOMIZE [seed% \| TIMER \| GET[#] ch \| INPUT[#] ch \| READ [count] \| arr[()] \| seed$]` |
| [READBIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/READBIT.md) | Function | `READBIT(val, bit)` |
| [READU](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/READU.md) | Function | `status = READU(ch, id, var$) \| rec$ = READU$(ch, id)` |
| [RECEIVE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RECEIVE$.md) | Function | `RECEIVE$(source$ [, timeout_ms])` |
| [REFORMAT ENGINE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REFORMAT ENGINE.md) | Statement | `REFORMAT [line_start[-line_end]]` |
| [RELEASE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RELEASE.md) | Function | `status = RELEASE(ch, id) \| RELEASE(ch) \| RELEASE()` |
| [REMOVE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REMOVE$.md) | Function | `REMOVE$(target$, pattern$ [, count [, mode]]) \| REMOVE(arr, val)` |
| [REPLACE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REPLACE$.md) | Function | `REPLACE$(target$, search$, replace$ [, count [, mode]]) \| REPLACE(arr, pos, val) \| REPLACE$(dyn$, attr, val$)` |
| [RESETBIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RESETBIT.md) | Function | `RESETBIT(val, bit)` |
| [RETRIEVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RETRIEVE.md) | Statement | `RETRIEVE #ch, record` |
| [ROL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ROL.md) | Function | `ROL(val, count [, width])` |
| [ROR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ROR.md) | Function | `ROR(val, count [, width])` |
| [SCREEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SCREEN.md) | Function | `SCREEN(row%, col% [, flag%])` |
| [SEEKEQ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEEKEQ.md) | Statement | `SEEKEQ #ch, key` |
| [SEEKGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEEKGE.md) | Statement | `SEEKGE #ch, key` |
| [SEEKGT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEEKGT.md) | Statement | `SEEKGT #ch, key` |
| [SEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEND.md) | Statement | `SEND target$, payload$` |
| [SETBIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SETBIT.md) | Function | `SETBIT(val, bit)` |
| [SETINDEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SETINDEX.md) | Statement | `SETINDEX #ch, idx$` |
| [SET_BITFIELD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SET_BITFIELD.md) | Function | `res = SET_BITFIELD(val, start_bit, bit_count, new_val)` |
| [SHL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SHL.md) | Function | `SHL(val, count) or val SHL count` |
| [SHR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SHR.md) | Function | `SHR(val, count) or val SHR count` |
| [SPEED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SPEED.md) | Statement | `SPEED [=] rate \| SPEED% = val% \| SPEED& = rate&` |
| [STARDATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/STARDATE.md) | Variable | `STARDATE or STARDATE()` |
| [STARDATE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/STARDATE$.md) | Variable | `STARDATE$ or STARDATE$()` |
| [SUB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SUB.md) | Statement | `SUB name [(param1, param2...)] [STATIC]` |
| [SUBSCRIBE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SUBSCRIBE.md) | Statement | `SUBSCRIBE topic$` |
| [SUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SUM.md) | Function | `SUM(val1, val2 [, ...]) or SUM(arr) or SUM{...} or SUM[arr]` |
| [TEXTBOX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TEXTBOX.md) | Statement | `TEXTBOX text$ [, col%, row%, width%, height%]` |
| [TI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TI.md) | Variable | `TI or TI()` |
| [TI$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TI$.md) | Variable | `TI$ or TI$()` |
| [TICKS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TICKS.md) | Variable | `TICKS or TICKS()` |
| [TICKS_MS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TICKS_MS.md) | Variable | `TICKS_MS or TICKS_MS()` |
| [TICKS_US](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TICKS_US.md) | Variable | `TICKS_US or TICKS_US()` |
| [TIME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TIME.md) | Variable | `TIME or TIME()` |
| [TIME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TIME$.md) | Variable | `TIME$ or TIME$()` |
| [TIMER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TIMER.md) | Variable | `TIMER or TIMER()` |
| [TIMER$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TIMER$.md) | Variable | `TIMER$ or TIMER$()` |
| [TIMERCONTROL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TIMERCONTROL.md) | Statement | `TIMERCONTROL interval_ms%, enabled%` |
| [TODAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TODAY.md) | Variable | `TODAY` |
| [TODAY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TODAY$.md) | Variable | `TODAY$` |
| [TOGGLEBIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TOGGLEBIT.md) | Function | `TOGGLEBIT(val, bit)` |
| [TRUE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TRUE.md) | Variable | `TRUE or TRUE()` |
| [TTY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TTY.md) | Statement | `TTY [tty_number%]` |
| [UBOUND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UBOUND.md) | Function | `high% = UBOUND(array [, dimension%])` |
| [UDX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UDX.md) | Function | `UDX(op$, [arg1, arg2]) \| XCHG(op$, ...)` |
| [UNIXTIME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/UNIXTIME.md) | Variable | `UNIXTIME or UNIXTIME()` |
| [UNLOAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNLOAD.md) | Statement | `UNLOAD module_name$` |
| [UNPUBLISH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNPUBLISH.md) | Statement | `UNPUBLISH topic$ [, payload$] \| UNPUBLISH ALL` |
| [UNSUBSCRIBE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UNSUBSCRIBE.md) | Statement | `UNSUBSCRIBE topic$` |
| [UPDATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UPDATE.md) | Statement | `UPDATE #ch [, record]` |
| [UPTIME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/UPTIME.md) | Variable | `UPTIME or UPTIME()` |
| [UPTIME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/UPTIME$.md) | Variable | `UPTIME$ or UPTIME$()` |
| [UTC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/UTC.md) | Variable | `UTC or UTC()` |
| [UTC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/UTC$.md) | Variable | `UTC$ or UTC$()` |
| [VSCROLLBAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VSCROLLBAR.md) | Statement | `VSCROLLBAR min%, max%, val%, col%, row%, height%` |
| [WEEK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/WEEK.md) | Function | `WEEK or WEEK()` |
| [WEEK$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/WEEK$.md) | Function | `WEEK$ or WEEK$()` |
| [WHO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WHO.md) | Statement | `WHO` |
| [WIDTH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WIDTH.md) | Statement | `WIDTH [columns] [, rows] \| WIDTH [#file_num,] columns` |
| [WRITE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WRITE.md) | Statement | `WRITE [#file_num,] [exprlist]` |
| [WRITEU](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/WRITEU.md) | Function | `status = WRITEU(ch, id, data$)` |
| [XIO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/XIO.md) | Statement | `XIO cmd, [#]channel, aux1, aux2, "filespec$"` |
| [XOR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/XOR.md) | Function | `XOR(val1, val2 [, ...]) or val1 XOR val2` |
| [ZONE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ZONE.md) | Statement | `ZONE [#channel,] width` |

## Graphics, Drawing & BGI (28 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [BGI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BGI.md) | Statement | `SET GRAPHICS width, height [, bpp]` |
| [BORDER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BORDER.md) | Statement | `BORDER color_index` |
| [BRIGHT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BRIGHT.md) | Statement | `BRIGHT state%` |
| [CIRCLE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CIRCLE.md) | Statement | `CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]` |
| [CLS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CLS.md) | Statement | `CLS [[fg] [, bg]]` |
| [COLOR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/COLOR.md) | Statement | `COLOR [foreground] [, [background] [, border]]` |
| [DRAW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DRAW.md) | Statement | `DRAW command_str$` |
| [DRAWTO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DRAWTO.md) | Statement | `DRAWTO x, y [, color]` |
| [FLASH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FLASH.md) | Statement | `FLASH state%` |
| [GFX.COMPAT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GFX.COMPAT.md) | Statement | `SET MODE mode_num` |
| [GRAPHICS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GRAPHICS.md) | Statement | `GRAPHICS mode_index` |
| [HOME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/HOME.md) | Statement | `HOME [[fg] [, bg]]` |
| [INK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INK.md) | Statement | `INK color_index` |
| [INVERSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INVERSE.md) | Statement | `INVERSE [state%]` |
| [LINE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LINE.md) | Statement | `LINE [[x1, y1]]-(x2, y2) [, [color] [, [B\|BF] [, style]]]` |
| [OVER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OVER.md) | Statement | `OVER state%` |
| [PAINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PAINT.md) | Statement | `PAINT (x, y) [, fill_color [, border_color]]` |
| [PALETTE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PALETTE.md) | Statement | `PALETTE [attribute, color]` |
| [PAPER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PAPER.md) | Statement | `PAPER color_index` |
| [PCOPY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PCOPY.md) | Statement | `PCOPY source_page, dest_page` |
| [PLOT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PLOT.md) | Statement | `PLOT [POINTS\|LINES\|AREA]: x, y [; x2, y2 ...]` |
| [PMAP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PMAP.md) | Statement | `PMAP(coord, map_type)` |
| [POINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/POINT.md) | Function | `color% = POINT(x%, y%) \| coord% = POINT(mode%)` |
| [PRESET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PRESET.md) | Statement | `PRESET (x, y) [, color]` |
| [PSET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PSET.md) | Statement | `PSET (x, y) [, color]` |
| [REVERSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REVERSE.md) | Statement | `REVERSE [state%]` |
| [VIEW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VIEW.md) | Statement | `VIEW [[SCREEN] (x1, y1)-(x2, y2) [, fill_color [, border_color]]]` |
| [WINDOW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WINDOW.md) | Statement | `WINDOW [[SCREEN] (x1, y1)-(x2, y2)]` |

## Hardware, GPIO & IoT (83 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ALLOC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ALLOC.md) | Statement | `ALLOC(size_bytes) \| ALLOC var, size` |
| [AREAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/AREAD.md) | Function | `AREAD(pin)` |
| [ATTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ATTR.md) | Variable | `ATTR or ATTR(row, col)` |
| [BANK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BANK.md) | Statement | `BANK bank_number%` |
| [BAUD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BAUD.md) | Variable | `BAUD or BAUD(channel)` |
| [BDOS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BDOS.md) | Variable | `BDOS` |
| [BT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BT.md) | Statement | `BT.START name$ \| BT.CONNECT mac_or_name$` |
| [CLOCKS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CLOCKS.md) | Function | `CLOCKS \| CLOCKS("model") \| CLOCKS[channel] \| CLOCKS{config}` |
| [CLOCKS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CLOCKS$.md) | Function | `CLOCKS$` |
| [CONSOL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CONSOL.md) | Variable | `CONSOL` |
| [CPUSPEED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CPUSPEED.md) | Function | `CPUSPEED \| CPUSPEED("model") \| CPUSPEED[channel] \| CPUSPEED{config}` |
| [CPUSPEED$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CPUSPEED$.md) | Function | `CPUSPEED$ \| CPUSPEED$("model")` |
| [DAC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DAC.md) | Statement | `DAC pin, value \| DAC.WRITE pin, value` |
| [DHT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DHT.md) | Statement | `DHT.READ pin, temp_var, hum_var` |
| [DREAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DREAD.md) | Function | `DREAD(pin)` |
| [DS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DS.md) | Variable | `DS` |
| [DS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DS$.md) | Variable | `DS$` |
| [DWRITE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DWRITE.md) | Statement | `DWRITE pin, value` |
| [ESPNOW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ESPNOW.md) | Statement | `ESPNOW.INIT [channel] \| ESPNOW.ADD.PEER mac$ [, channel] \| ESPNOW.SEND mac$, data$ \| ESPNOW.RECV var$` |
| [FRAMES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/FRAMES.md) | Variable | `FRAMES` |
| [GEMINI.BROWSE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GEMINI.BROWSE.md) | Statement | `GEMINI.BROWSE [url$]` |
| [HALL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HALL.md) | Function | `HALL() \| HALL.READ()` |
| [HIMEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HIMEM.md) | Variable | `HIMEM or MAXRAM` |
| [HTTP.GET$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HTTP.GET$.md) | Function | `HTTP.GET$(url$)` |
| [I2C](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/I2C.md) | Statement | `I2C.WRITE addr, reg, val \| I2C.READ addr, reg, var` |
| [IOT.RPC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IOT.RPC$.md) | Function | `IOT.RPC$(target$, func_call$)` |
| [JOB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/JOB.md) | Variable | `JOB or JOB$` |
| [LOMEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/LOMEM.md) | Variable | `LOMEM` |
| [MEMMAP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MEMMAP.md) | Function | `MEMMAP(region_idx%)` |
| [MQTT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MQTT.md) | Statement | `MQTT.PUBLISH topic$, payload$ \| MQTT.SUBSCRIBE topic$` |
| [NEOPIXEL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NEOPIXEL.md) | Statement | `NEOPIXEL pin, index, r, g, b \| NEOPIXEL.SHOW pin \| NEOPIXEL.CLEAR pin` |
| [NET.CONFIG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NET.CONFIG.md) | Statement | `NET.CONFIG iface$, ip$, netmask$, gateway$, dns$ \| NET ON\|OFF\|STOP` |
| [NET.UNPACK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NET.UNPACK.md) | Statement | `NET.UNPACK packet$, dest_var` |
| [NFC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NFC.md) | Statement | `NFC.INIT [addr] \| NFC.SCAN uid_var$ \| NFC.READ block, data_var$ \| NFC.WRITE block, data$ \| NFC.EMULATE uid$` |
| [PACKET.LEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.LEN.md) | Function | `PACKET.LEN()` |
| [PACKET.MAC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.MAC$.md) | Function | `PACKET.MAC$()` |
| [PACKET.PAYLOAD$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.PAYLOAD$.md) | Function | `PACKET.PAYLOAD$()` |
| [PACKET.PORT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.PORT.md) | Function | `PACKET.PORT()` |
| [PACKET.RSSI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.RSSI.md) | Function | `PACKET.RSSI()` |
| [PACKET.SRC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.SRC$.md) | Function | `PACKET.SRC$()` |
| [PACKET.TYPE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACKET.TYPE$.md) | Function | `PACKET.TYPE$()` |
| [PADDLE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PADDLE.md) | Variable | `PADDLE or PADDLE(n)` |
| [PAGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PAGE.md) | Variable | `PAGE` |
| [PEEKB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PEEKB.md) | Function | `PEEKB(address)` |
| [PEER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PEER.md) | Statement | `PEER.INIT [channel] \| PEER.ADD peer_id$ [, mac$] \| PEER.SEND target$, data$ \| PEER ON\|OFF\|STOP` |
| [PINMODE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PINMODE.md) | Statement | `PINMODE pin, mode` |
| [POINTER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/POINTER.md) | Statement | `POINTER ptr_name AS type` |
| [POKEB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/POKEB.md) | Statement | `POKEB address, byte_val` |
| [PORT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PORT.md) | Statement | `PORT.FIRE port_num \| PORT(p) ON\|OFF\|STOP` |
| [PTRIG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PTRIG.md) | Variable | `PTRIG or PTRIG(n)` |
| [PWM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PWM.md) | Statement | `PWM pin, freq, duty` |
| [RAMBANKS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RAMBANKS.md) | Statement | `RAMBANKS count%` |
| [RBYTE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RBYTE.md) | Statement | `RBYTE [@dev [, sec] :] var1 [, var2 ...] \| RBYTE[dev, sec] var1 ...` |
| [REMOTE.EVAL$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REMOTE.EVAL$.md) | Function | `REMOTE.EVAL$(target$, expr$)` |
| [REMOTE.EXEC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REMOTE.EXEC.md) | Statement | `REMOTE.EXEC target$, cmd$` |
| [SEG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SEG.md) | Statement | `SEG = segment_address% \| DEF SEG = segment_address%` |
| [SERVO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SERVO.md) | Statement | `SERVO pin, angle` |
| [SNIFF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SNIFF.md) | Statement | `SNIFF [ch] [, filter$] \| SNIFF ON\|OFF\|STOP` |
| [SOCK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SOCK.md) | Statement | `SOCK.BIND h, port \| SOCK.LISTEN h \| SOCK.SEND h, data$ \| SOCK.CLOSE h \| SOCK.SETSOCKOPT h, opt$, v` |
| [SOCK.ACCEPT%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SOCK.ACCEPT%.md) | Function | `SOCK.ACCEPT%(listen_h%)` |
| [SOCK.OPEN%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SOCK.OPEN%.md) | Function | `SOCK.OPEN%(proto$)` |
| [SOCK.POLL%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SOCK.POLL%.md) | Function | `SOCK.POLL%(handle% [, mask% [, timeout_ms%]])` |
| [SOCK.RECV$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SOCK.RECV$.md) | Function | `SOCK.RECV$(handle% [, max_len% [, timeout_ms%]])` |
| [SOCK.STATUS%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SOCK.STATUS%.md) | Function | `SOCK.STATUS%(handle%)` |
| [SPEED%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SPEED%.md) | Variable | `SPEED%` |
| [SPEED&](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SPEED&.md) | Variable | `SPEED&` |
| [SPI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SPI.md) | Statement | `SPI.TRANSFER cs_pin, data$` |
| [ST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ST.md) | Variable | `ST` |
| [STATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/STATUS.md) | Variable | `STATUS or ST` |
| [STICK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/STICK.md) | Variable | `STICK or STICK(n)` |
| [STRIG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/STRIG.md) | Variable | `STRIG or STRIG(n)` |
| [SWAP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SWAP$.md) | Variable | `SWAP$` |
| [TNFS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TNFS.md) | Statement | `TNFS.MOUNT host$ [, path$ [, port]] \| TNFS.UNMOUNT` |
| [TNFS.DIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TNFS.DIR$.md) | Function | `TNFS.DIR$([path$ [, pattern$]])` |
| [TOUCH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TOUCH.md) | Function | `TOUCH(pin) \| TOUCH.READ(pin)` |
| [UPNP.FORWARD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UPNP.FORWARD.md) | Statement | `UPNP.FORWARD ext_port [, int_port [, proto$ [, desc$]]]` |
| [UPNP.UNFORWARD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/UPNP.UNFORWARD.md) | Statement | `UPNP.UNFORWARD ext_port [, proto$]` |
| [USER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/USER.md) | Variable | `USER` |
| [VBL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/VBL.md) | Variable | `VBL` |
| [VCOUNT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/VCOUNT.md) | Variable | `VCOUNT` |
| [WBYTE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WBYTE.md) | Statement | `WBYTE [@dev [, sec] :] byte1 [, byte2 ...] \| WBYTE[dev, sec] byte1 ...` |
| [WEBREPL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WEBREPL.md) | Statement | `WEBREPL.START [port] \| WEBREPL.STOP` |
| [WIFI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WIFI.md) | Statement | `WIFI.CONNECT ssid$, pass$ \| WIFI.DISCONNECT \| WIFI.AP ssid$, pass$ [, ch, max] \| WIFI.SCAN \| WIFI.SNIFF ON\|OFF [, ch]` |

## Math, Trigonometry & Numbers (88 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ABS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ABS.md) | Function | `ABS(x)` |
| [ACOS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ACOS.md) | Function | `ACOS(x)` |
| [ANGLE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ANGLE.md) | Function | `ANGLE(x, y)` |
| [ARG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ARG.md) | Function | `ARG(x, y)` |
| [ASIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ASIN.md) | Function | `ASIN(x)` |
| [ATAN2](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ATAN2.md) | Function | `ATAN2(y, x)` |
| [ATN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ATN.md) | Function | `ATN(x)` |
| [CABS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CABS.md) | Function | `CABS(z)` |
| [CARG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CARG.md) | Function | `CARG(z)` |
| [CBOOL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CBOOL.md) | Function | `CBOOL(expr)` |
| [CCUR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CCUR.md) | Function | `CCUR(expr)` |
| [CDBL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CDBL.md) | Function | `CDBL(expr)` |
| [CEIL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CEIL.md) | Function | `CEIL(x)` |
| [CEXP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CEXP.md) | Function | `CEXP(z)` |
| [CINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CINT.md) | Function | `CINT(expr)` |
| [CLAMP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CLAMP.md) | Function | `CLAMP(val, min_val, max_val)` |
| [CLOG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CLOG.md) | Function | `CLOG(z)` |
| [COMP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COMP.md) | Function | `COMP(a, b)` |
| [COMPLEX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COMPLEX.md) | Function | `COMPLEX(real, imag) \| REAL(z) \| IMAG(z) \| CONJG(z) \| ARG(z) \| CABS(z) \| CSIN(z) \| CCOS(z) \| CEXP(z) \| CLOG(z) \| CSQR(z)` |
| [CONJ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CONJ.md) | Function | `CONJ(z)` |
| [COS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COS.md) | Function | `COS(angle)` |
| [COSH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COSH.md) | Function | `COSH(x) \| HCS(x)` |
| [COT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COT.md) | Function | `COT(x)` |
| [CPOW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CPOW.md) | Function | `CPOW(base, exponent)` |
| [CROSS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CROSS.md) | Function | `CROSS(u, v)` |
| [CSC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CSC.md) | Function | `CSC(x)` |
| [CSNG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CSNG.md) | Function | `CSNG(expr)` |
| [CSQR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CSQR.md) | Function | `CSQR(z)` |
| [CSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CSTR.md) | Function | `CSTR(expr)` |
| [CVDMBF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVDMBF.md) | Function | `x# = CVDMBF(s$)` |
| [CVSMBF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVSMBF.md) | Function | `x! = CVSMBF(s$)` |
| [DEGREE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEGREE.md) | Statement | `DEGREE` |
| [DEGREES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DEGREES.md) | Function | `DEGREES(x)` |
| [DET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DET.md) | Function | `DET(A)` |
| [DOT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DOT.md) | Function | `DOT(u, v)` |
| [DP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DP.md) | Function | `DP(num)` |
| [EXP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EXP.md) | Function | `EXP(x)` |
| [FIX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FIX.md) | Function | `FIX(x)` |
| [FLOOR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FLOOR.md) | Function | `FLOOR(x)` |
| [FPT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FPT.md) | Function | `FPT(num) / FP(num)` |
| [FV](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FV.md) | Function | `x = FV(rate, nper, pmt [, pv [, type]])` |
| [GRAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GRAD.md) | Function | `GRAD(x)` |
| [HYPOT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HYPOT.md) | Function | `HYPOT(val1, val2 [, ...]) or val1 HYPOT val2` |
| [IMAG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IMAG.md) | Function | `IMAG(z)` |
| [INT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INT.md) | Function | `INT(x)` |
| [IPMT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IPMT.md) | Function | `x = IPMT(rate, per, nper, pv [, fv [, type]])` |
| [IPT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IPT.md) | Function | `IPT(num) / IP(num)` |
| [IRR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/IRR.md) | Function | `x = IRR(val1, val2, ...)` |
| [LERP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LERP.md) | Function | `LERP(a, b, t)` |
| [LGT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LGT.md) | Function | `LGT(x)` |
| [LOG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOG.md) | Function | `LOG(x)` |
| [LOG10](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOG10.md) | Function | `LOG10(x) \| LGT(x)` |
| [LOG2](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LOG2.md) | Function | `LOG2(x)` |
| [MAG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAG.md) | Function | `MAG(x, y) \| MAG(complex_z) \| MAG(x)` |
| [MAX](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAX.md) | Function | `MAX(val1, val2 [, ...]) or val1 MAX val2` |
| [MIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MIN.md) | Function | `MIN(val1, val2 [, ...]) or val1 MIN val2` |
| [MKDMBF$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MKDMBF$.md) | Function | `s$ = MKDMBF$(x#)` |
| [MKSMBF$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MKSMBF$.md) | Function | `s$ = MKSMBF$(x!)` |
| [MOD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MOD.md) | Function | `MOD(val1, val2) or val1 MOD val2` |
| [NEG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NEG.md) | Function | `NEG(x) or NEG x` |
| [NPER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NPER.md) | Function | `x = NPER(rate, pmt, pv [, fv [, type]])` |
| [NPV](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NPV.md) | Function | `x = NPV(rate, val1, val2, ...)` |
| [PDIF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PDIF.md) | Function | `PDIF(a, b)` |
| [PMT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PMT.md) | Function | `x = PMT(rate, nper, pv [, fv [, type]])` |
| [PN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PN.md) | Function | `PN(expr$) \| PN(op$, ...)` |
| [POL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/POL.md) | Function | `POL(x, y [, idx]) \| POL[x, y]` |
| [PPMT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PPMT.md) | Function | `x = PPMT(rate, per, nper, pv [, fv [, type]])` |
| [PV](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PV.md) | Function | `x = PV(rate, nper, pmt [, fv [, type]])` |
| [RADIAN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RADIAN.md) | Statement | `RADIAN` |
| [RADIANS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RADIANS.md) | Function | `RADIANS(x)` |
| [RATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RATE.md) | Function | `x = RATE(nper, pmt, pv [, fv [, type [, guess]]])` |
| [REAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REAL.md) | Function | `REAL(z)` |
| [REC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REC.md) | Function | `REC(r, theta [, idx]) \| REC[r, theta]` |
| [REMAINDER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REMAINDER.md) | Function | `REMAINDER(x, y)` |
| [RND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RND.md) | Function | `RND[(x)]` |
| [ROUND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ROUND.md) | Function | `ROUND(x [, decimals])` |
| [RPN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RPN.md) | Function | `RPN(expr$) \| STACK(expr$) \| RPN(op$, ...)` |
| [SCALE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SCALE.md) | Statement | `SCALE factor` |
| [SEC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SEC.md) | Function | `SEC(x)` |
| [SGN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SGN.md) | Function | `SGN(x)` |
| [SIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SIN.md) | Function | `SIN(angle)` |
| [SINH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SINH.md) | Function | `SINH(x) \| HSN(x)` |
| [SQR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SQR.md) | Function | `SQR(x)` |
| [STACK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/STACK.md) | Statement | `STACK POP var1 [, var2, ...] \| STACK EXPORT var1 [, var2] \| STACK DUMP \| STACK CLEAR \| STACK SWAP` |
| [TAN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TAN.md) | Function | `TAN(angle)` |
| [TANH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TANH.md) | Function | `TANH(x) \| HTN(x)` |
| [TRUNCATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TRUNCATE.md) | Function | `TRUNCATE(x [, n])` |
| [XP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/XP.md) | Function | `XP(num)` |

## Metaprogramming & Introspection (40 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ALIAS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ALIAS.md) | Statement | `ALIAS name = expansion \| ALIAS LIST \| ALIAS CLEAR \| ALIAS OPER op = expansion` |
| [AUTO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/AUTO.md) | Statement | `AUTO [start_line] [,step]` |
| [CATALOG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CATALOG.md) | Statement | `CATALOG [category_name \| keyword1 [, keyword2...]]` |
| [CATEGORY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CATEGORY.md) | Statement | `CATEGORY [keyword \| category_name]` |
| [CATEGORY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CATEGORY$.md) | Function | `CATEGORY$(keyword$)` |
| [CHAIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CHAIN.md) | Statement | `CHAIN [MERGE] filename_expr [, [line_number] [, ALL] [, MERGE]]` |
| [CHECK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CHECK.md) | Statement | `CHECK [file$] [SUMMARY\|DETAILED\|JSON\|CONFLICTS\|SMART]` |
| [COMPILE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/COMPILE.md) | Statement | `COMPILE [ALL \| SUB \| JIT]` |
| [CONT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CONT.md) | Statement | `CONT` |
| [DEBUG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEBUG.md) | Statement | `DEBUG [ON\|OFF\|DUMP\|STACK\|MEMORY\|VARS] \| TRACE \| TRON \| TROFF \| BREAK \| CONT \| BACKTRACE \| INFO \| DUMP` |
| [EDIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EDIT.md) | Statement | `EDIT [filename$ \| line_number]` |
| [HELP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/HELP.md) | Statement | `HELP [keyword \| command \| block_target]` |
| [HOSTNAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/HOSTNAME.md) | Statement | `HOSTNAME` |
| [ISARRAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ISARRAY.md) | Statement | `result = ISARRAY(expression)` |
| [ISNUMERIC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ISNUMERIC.md) | Statement | `result = ISNUMERIC(expression)` |
| [JIT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/JIT.md) | Statement | `JIT [ON \| OFF \| AUTO \| BYTECODE \| NATIVE \| FAST \| SAFE]` |
| [KEYWORD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/KEYWORD.md) | Statement | `KEYWORD target SET prop = val \| KEYWORD target GET prop \| KEYWORD target LIST \| KEYWORD CLEAR` |
| [LIST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LIST.md) | Statement | `LIST [start_line] [- [end_line]] [, filename_expr]` |
| [LLIST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LLIST.md) | Statement | `LLIST [start_line] [- [end_line]]` |
| [LOAD](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOAD.md) | Statement | `LOAD filename_expr [, R]` |
| [MERGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MERGE.md) | Statement | `MERGE filename_expr` |
| [MODULE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MODULE.md) | Statement | `MODULE "ModuleName" [EXPORTS sym1, sym2, ...]` |
| [NEW](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/NEW.md) | Statement | `NEW` |
| [OVERRIDE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OVERRIDE.md) | Statement | `OVERRIDE target WITH GOSUB line_num \| OVERRIDE target WITH sub_name \| OVERRIDE CLEAR` |
| [PROGRAM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PROGRAM$.md) | Function | `PROGRAM$(line_number) \| PROGRAM$("COUNT")` |
| [REFORMAT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REFORMAT.md) | Statement | `REFORMAT [spaces_per_indent] [target] [CHECK\|STRICT\|SPLIT\|UPPER\|LOWER\|PRESERVE\|SPACES\|UNPACK]` |
| [RENAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RENAME.md) | Statement | `RENAME \| RENAME old_var TO new_var \| RENAME CONFLICTS [PREFIX "v_"] \| RENAME SMART [CASE "mixed"\|"snake"] \| RENAME UNDO` |
| [RENUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RENUM.md) | Statement | `RENUM ["file.bas" [,start [,step]] [TO "out.bas"]] \| [new_start] [,[old_start] [,step]]` |
| [REVERT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REVERT.md) | Statement | `REVERT [tool_tag$] \| REVERT [ tool_tag$ ] \| REVERT { tool: "..." }` |
| [RUN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RUN.md) | Statement | `RUN [line_number \| filename [, R]]` |
| [SAVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SAVE.md) | Statement | `SAVE filename_expr [, A \| P]` |
| [SCOPE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SCOPE.md) | Statement | `SCOPE [BEGIN \| END \| DISABLE kw \| ENABLE kw \| HOOK ... \| MODULE name \| PRIVATE sym]` |
| [SCRATCH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SCRATCH.md) | Statement | `SCRATCH` |
| [SIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SIZE.md) | Function | `SIZE(var_or_type)` |
| [TEST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TEST.md) | Statement | `TEST "suite_name" [SUBCASE "case_name"]` |
| [TRACE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TRACE.md) | Statement | `TRACE {ON \| OFF \| line_num}` |
| [TROFF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TROFF.md) | Statement | `TROFF` |
| [TRON](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TRON.md) | Statement | `TRON` |
| [USERNAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/USERNAME.md) | Statement | `USERNAME` |
| [VARS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VARS.md) | Statement | `VARS [prefix$]` |

## Networking & Protocols (25 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [GEMINI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GEMINI.md) | Statement | `GEMINI.SERVE [port] [, root_dir$] \| GEMINI.GET$(url$)` |
| [GEMINI.GET$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GEMINI.GET$.md) | Function | `GEMINI.GET$(url$)` |
| [GEMINI.META$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GEMINI.META$.md) | Function | `GEMINI.META$()` |
| [GEMINI.STATUS%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GEMINI.STATUS%.md) | Function | `GEMINI.STATUS%()` |
| [GOPHER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GOPHER.md) | Statement | `GOPHER.SERVE [port] [, root_dir$] \| GOPHER.GET$(url$)` |
| [GOPHER.GET$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GOPHER.GET$.md) | Function | `GOPHER.GET$(url$)` |
| [IP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/IP$.md) | Variable | `IP$ or IP$()` |
| [NBYTESWAITING](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NBYTESWAITING.md) | Variable | `NBYTESWAITING` |
| [NCONNECTED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NCONNECTED.md) | Variable | `NCONNECTED` |
| [NEOF](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NEOF.md) | Variable | `NEOF` |
| [NERROR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NERROR.md) | Variable | `NERROR` |
| [NET.GATEWAY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NET.GATEWAY$.md) | Variable | `NET.GATEWAY$` |
| [NET.PACK$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NET.PACK$.md) | Function | `NET.PACK$(var)` |
| [NHTTPSTATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NHTTPSTATUS.md) | Variable | `NHTTPSTATUS` |
| [NSTATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/NSTATUS.md) | Variable | `NSTATUS` |
| [UPNP.EXTERNALIP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UPNP.EXTERNALIP$.md) | Function | `UPNP.EXTERNALIP$()` |
| [UPNP.STATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UPNP.STATUS.md) | Function | `UPNP.STATUS(port[, proto$])` |
| [UPNP.STATUS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UPNP.STATUS$.md) | Function | `UPNP.STATUS$(port[, proto$])` |
| [WIFI.RSSI](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WIFI.RSSI.md) | Variable | `WIFI.RSSI` |
| [WIFI.SSID$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WIFI.SSID$.md) | Variable | `WIFI.SSID$` |
| [WIFI.STATUS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WIFI.STATUS$.md) | Variable | `WIFI.STATUS$` |
| [_BATTERY%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/_BATTERY%.md) | Variable | `_BATTERY%` |
| [_CPU_LOAD%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/_CPU_LOAD%.md) | Variable | `_CPU_LOAD%` |
| [_FREE_STACK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/_FREE_STACK.md) | Variable | `_FREE_STACK` |
| [_TEMPERATURE%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/_TEMPERATURE%.md) | Variable | `_TEMPERATURE%` |

## Sound & Multimedia (6 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [BEEP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/BEEP.md) | Statement | `BEEP [count [, delay]]` |
| [PICTURE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PICTURE.md) | Statement | `PICTURE name [(parameter_list)] ... END PICTURE` |
| [PLAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PLAY.md) | Statement | `PLAY command_string` |
| [SOUND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SOUND.md) | Statement | `SOUND frequency, duration` |
| [VIEWPORT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VIEWPORT.md) | Statement | `VIEWPORT xmin, xmax, ymin, ymax` |
| [VOICE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VOICE.md) | Statement | `VOICE channel, waveform, attack, decay, sustain, release` |

## String Manipulation & Text (54 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ASC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ASC.md) | Function | `ASC(string_expr$)` |
| [ASCII](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ASCII.md) | Function | `ASCII(str_expr) \| NUM(char_expr)` |
| [ATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ATH$.md) | Function | `ATH$(hex_str$)` |
| [BIN$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/BIN$.md) | Function | `BIN$(x)` |
| [CHR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CHR$.md) | Function | `CHR$(code)` |
| [COLLATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/COLLATE.md) | Statement | `COLLATE STANDARD \| COLLATE NATIVE \| COLLATE table$` |
| [COMSTR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/COMSTR$.md) | Function | `COMSTR$(str$)` |
| [CVT$%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/CVT$%.md) | Function | `CVT$%(str_val) \| CVT%$(int_val) \| CVT$F(str_val) \| CVTF$(flt_val) \| CVT$$(str_val, flags) \| SWAP%(int_val)` |
| [DECOMSTR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DECOMSTR$.md) | Function | `DECOMSTR$(comp_str$)` |
| [DIF$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DIF$.md) | Function | `DIF$(str_a, str_b)` |
| [EDIT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EDIT$.md) | Function | `EDIT$(str$, action_mask%)` |
| [ERT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ERT$.md) | Function | `ERT$(error_code)` |
| [FORMAT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FORMAT$.md) | Function | `FORMAT$(val, mask$)` |
| [HEX$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HEX$.md) | Function | `HEX$(x) \| HEX(hex_str$)` |
| [HTA$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HTA$.md) | Function | `HTA$(str$ \| num)` |
| [ICASE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ICASE$.md) | Function | `ICASE$(str$)` |
| [INSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INSTR.md) | Function | `INSTR([start,] str$, match$ [, mode])` |
| [LCASE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LCASE$.md) | Function | `LCASE$(str$)` |
| [LEFT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LEFT$.md) | Function | `LEFT$(str$, n)` |
| [LEN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LEN.md) | Function | `LEN(str$)` |
| [LTRIM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/LTRIM$.md) | Function | `LTRIM$(str$)` |
| [MCASE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MCASE$.md) | Function | `MCASE$(str$)` |
| [MID$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MID$.md) | Function | `MID$(str$, start [, length [, step]])` |
| [MID$ STMT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MID$ STMT.md) | Statement | `MID$(string_var, start [, length]) = replacement$` |
| [NUM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/NUM$.md) | Function | `NUM$(numeric_val) \| NUM1$(numeric_val) \| VAL%(str_val)` |
| [OCT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/OCT$.md) | Function | `OCT$(x)` |
| [OVERLAY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/OVERLAY$.md) | Function | `OVERLAY$(target$, source$, pos)` |
| [PACK$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PACK$.md) | Function | `PACK$(fmt$, val1 [, val2, ...])` |
| [PLACE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PLACE$.md) | Function | `PLACE$(str_val, place_flag, decimal_digits)` |
| [PROD$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PROD$.md) | Function | `PROD$(str_a, str_b [, precision])` |
| [QUO$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/QUO$.md) | Function | `QUO$(str_a, str_b [, precision])` |
| [RAD$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RAD$.md) | Function | `RAD$(numeric_val)` |
| [REVERSE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/REVERSE$.md) | Function | `REVERSE$(str$)` |
| [RIGHT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RIGHT$.md) | Function | `RIGHT$(str$, n)` |
| [RINSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RINSTR.md) | Function | `RINSTR([start,] str$, match$ [, mode])` |
| [RTRIM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/RTRIM$.md) | Function | `RTRIM$(str$)` |
| [SEG$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SEG$.md) | Function | `SEG$(str_expr, start_pos, end_pos)` |
| [SHUFFLE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SHUFFLE$.md) | Function | `SHUFFLE$(str$ [, seed])` |
| [SPACE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SPACE$.md) | Function | `SPACE$(n)` |
| [SPC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SPC.md) | Function | `SPC(n%)` |
| [STR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/STR$.md) | Function | `STR$(x)` |
| [STRING$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/STRING$.md) | Function | `STRING$(n, char_spec)` |
| [SUM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SUM$.md) | Function | `SUM$(str_a, str_b)` |
| [TAB](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TAB.md) | Function | `TAB(column%)` |
| [TCASE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TCASE$.md) | Function | `TCASE$(str$)` |
| [TRANSLATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TRANSLATE.md) | Statement | `TRANSLATE var$ USING table$ \| TRANSLATE var$, from$, to$ \| TRANSLATE[from$, to$] var$` |
| [TRANSLATE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TRANSLATE$.md) | Function | `TRANSLATE$(src$, from$, to$) \| TRANSLATE$(src$, table$) \| TRANSLATE$[from$, to$](src$)` |
| [TRIM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TRIM$.md) | Function | `TRIM$(str$)` |
| [UCASE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UCASE$.md) | Function | `UCASE$(str$)` |
| [UNPACK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UNPACK.md) | Function | `UNPACK(fmt$, bin_str$)` |
| [UPS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/UPS$.md) | Function | `UPS$(str_expr) \| UPS(str_expr)` |
| [VAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/VAL.md) | Function | `VAL(str$)` |
| [VERIFY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/VERIFY.md) | Function | `VERIFY(target$, charset$ [, start%])` |
| [XLATE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/XLATE$.md) | Function | `XLATE$(src_str, table_str)` |

## System & Environment (130 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [ARCH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ARCH$.md) | Variable | `ARCH$ or SYS.ARCH$` |
| [AVAILMEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/AVAILMEM.md) | Variable | `AVAILMEM or SYS.MEMORY.AVAIL` |
| [BASEDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BASEDIR$.md) | Variable | `BASEDIR$` |
| [BASENAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BASENAME$.md) | Variable | `BASENAME$ or BASNAME$` |
| [BIOCHECKSUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BIOCHECKSUM.md) | Variable | `BIOCHECKSUM` |
| [BIOSIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BIOSIZE.md) | Variable | `BIOSIZE` |
| [BIOSTATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/BIOSTATUS.md) | Variable | `BIOSTATUS` |
| [COMMAND$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMMAND$.md) | Variable | `COMMAND$ or COMMAND$(n)` |
| [COMPILER$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMPILER$.md) | Variable | `COMPILER$ or SYS.COMPILER$` |
| [COMPUTERNAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMPUTERNAME.md) | Variable | `COMPUTERNAME or COMPUTERNAME$` |
| [COMPUTERNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMPUTERNAME$.md) | Variable | `COMPUTERNAME$ or COMPUTERNAME$()` |
| [COMSPEC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMSPEC.md) | Variable | `COMSPEC or COMSPEC$` |
| [COMSPEC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/COMSPEC$.md) | Variable | `COMSPEC$ or COMSPEC$()` |
| [CORES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/CORES.md) | Variable | `CORES or SYS.CORES` |
| [DATADIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DATADIR$.md) | Variable | `DATADIR$` |
| [DATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DATE.md) | Function | `DATE or DATE()` |
| [DATE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DATE$.md) | Function | `DATE$ \| DATE$(day_num)` |
| [DATESERIAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DATESERIAL.md) | Function | `DATESERIAL(year%, month%, day%)` |
| [DATEVALUE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DATEVALUE.md) | Function | `DATEVALUE(date_str$)` |
| [DAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DAY.md) | Function | `DAY(serial#)` |
| [DAY$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/DAY$.md) | Function | `DAY$ [(date_expr)]` |
| [DEEPSLEEP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEEPSLEEP.md) | Statement | `DEEPSLEEP [ms] \| LIGHTSLEEP [ms]` |
| [DOCSDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DOCSDIR$.md) | Variable | `DOCSDIR$` |
| [DOSVER$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DOSVER$.md) | Variable | `DOSVER$ or DOSVER` |
| [DRIVE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/DRIVE$.md) | Variable | `DRIVE$ or SYS.DRIVE$` |
| [EDITION$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EDITION$.md) | Variable | `EDITION$ or SYS.EDITION$` |
| [ENVIRON$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/ENVIRON$.md) | Function | `ENVIRON$(var_name$)` |
| [EPOCHDATE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EPOCHDATE.md) | Function | `EPOCHDATE([serial#])` |
| [ERDEV$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ERDEV$.md) | Variable | `ERDEV$ or ERDEV$()` |
| [ERL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ERL.md) | Variable | `ERL or ERL()` |
| [ERR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ERR.md) | Variable | `ERR or ERR()` |
| [ERRORLEVEL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ERRORLEVEL.md) | Variable | `ERRORLEVEL` |
| [EXEC$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/EXEC$.md) | Function | `EXEC$(binary$ [, arg1$ [, arg2$...]])` |
| [EXEDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EXEDIR$.md) | Variable | `EXEDIR$ or APPDIR$` |
| [EXENAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EXENAME$.md) | Variable | `EXENAME$` |
| [EXEPATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EXEPATH$.md) | Variable | `EXEPATH$` |
| [EXT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EXT$.md) | Variable | `EXT$ or EXTNAME$` |
| [EXTERR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/EXTERR.md) | Variable | `EXTERR` |
| [FRE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/FRE.md) | Function | `FRE([pool_code])` |
| [FREEFILE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/FREEFILE.md) | Variable | `FREEFILE or FREEFILE()` |
| [FREQ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/FREQ.md) | Statement | `FREQ mhz` |
| [GOODBYE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GOODBYE.md) | Statement | `GOODBYE` |
| [GUID](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GUID.md) | Function | `GUID` |
| [GUID$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/GUID$.md) | Function | `GUID$` |
| [HELPDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HELPDIR$.md) | Variable | `HELPDIR$` |
| [HOME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOME$.md) | Variable | `HOME$` |
| [HOMEDRIVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOMEDRIVE.md) | Variable | `HOMEDRIVE or HOMEDRIVE$` |
| [HOMEDRIVE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOMEDRIVE$.md) | Variable | `HOMEDRIVE$ or HOMEDRIVE$()` |
| [HOMEPATH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOMEPATH.md) | Variable | `HOMEPATH or HOMEPATH$` |
| [HOMEPATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOMEPATH$.md) | Variable | `HOMEPATH$ or HOMEPATH$()` |
| [HOSTNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/HOSTNAME$.md) | Variable | `HOSTNAME$ or SYS.HOSTNAME$` |
| [HOUR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HOUR.md) | Function | `HOUR[(serial#)] / HOURS / HOUR$` |
| [HOURS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/HOURS.md) | Function | `HOURS [(time_expr)]` |
| [INP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/INP.md) | Function | `INP(port)` |
| [ISADMIN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/ISADMIN.md) | Variable | `ISADMIN or SYS.ISADMIN or ISROOT` |
| [KERNEL$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/KERNEL$.md) | Variable | `KERNEL$ or SYS.KERNEL$` |
| [LANG$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/LANG$.md) | Variable | `LANG$` |
| [LOGDEBUG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGDEBUG.md) | Statement | `LOGDEBUG message$ [, tag$]` |
| [LOGERROR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGERROR.md) | Statement | `LOGERROR message$ [, tag$]` |
| [LOGGER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGGER.md) | Statement | `LOGGER message$ [, level]` |
| [LOGINFO](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGINFO.md) | Statement | `LOGINFO message$ [, tag$]` |
| [LOGNAME](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/LOGNAME.md) | Variable | `LOGNAME or LOGNAME$` |
| [LOGNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/LOGNAME$.md) | Variable | `LOGNAME$ or LOGNAME$()` |
| [LOGTRACE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGTRACE.md) | Statement | `LOGTRACE message$ [, tag$]` |
| [LOGWARN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LOGWARN.md) | Statement | `LOGWARN message$ [, tag$]` |
| [MACHINE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/MACHINE$.md) | Variable | `MACHINE$ or SYS.MACHINE$` |
| [MAGTAPE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MAGTAPE.md) | Function | `MAGTAPE(function_code, channel_num, arg)` |
| [MEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/MEM.md) | Variable | `MEM or FRE(0)` |
| [MEM.FREE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MEM.FREE.md) | Function | `MEM.FREE() \| MEM.ALLOC()` |
| [MEMMAP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/MEMMAP$.md) | Variable | `MEMMAP$ or MEMMAP$()` |
| [MINUTE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MINUTE.md) | Function | `MINUTE[(serial#)] / MINUTES / MINUTE$` |
| [MINUTES](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MINUTES.md) | Function | `MINUTES [(time_expr)]` |
| [MONTH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MONTH.md) | Function | `MONTH(serial#)` |
| [MONTH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/MONTH$.md) | Function | `MONTH$ [(date_expr)]` |
| [OS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/OS$.md) | Variable | `OS$ or SYS.OS$` |
| [PATH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PATH.md) | Variable | `PATH or PATH$` |
| [PATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PATH$.md) | Variable | `PATH$ or PATH$()` |
| [PEEK](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/PEEK.md) | Function | `PEEK(addr)` |
| [PID](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PID.md) | Variable | `PID or SYS.PID` |
| [PLATFORM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PLATFORM$.md) | Variable | `PLATFORM$ or SYS.PLATFORM$` |
| [PROFILE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PROFILE$.md) | Variable | `PROFILE$ or SYS.PROFILE$` |
| [PROGNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PROGNAME$.md) | Variable | `PROGNAME$` |
| [PROMPT$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PROMPT$.md) | Variable | `PROMPT$` |
| [PTRSIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/PTRSIZE.md) | Variable | `PTRSIZE or SYS.PTRSIZE` |
| [REBOOT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REBOOT.md) | Statement | `REBOOT \| RESET` |
| [SCRIPTDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SCRIPTDIR$.md) | Variable | `SCRIPTDIR$ or DIRPATH$` |
| [SCRIPTNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SCRIPTNAME$.md) | Variable | `SCRIPTNAME$ or FILENAME$` |
| [SCRIPTPATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SCRIPTPATH$.md) | Variable | `SCRIPTPATH$` |
| [SECOND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SECOND.md) | Function | `SECOND[(serial#)] / SECONDS / SECOND$` |
| [SECONDS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SECONDS.md) | Function | `SECONDS [(time_expr)]` |
| [SECURITY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SECURITY.md) | Statement | `SECURITY [level_number \| LEVEL level_number]` |
| [SELFTEST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SELFTEST.md) | Statement | `SELFTEST` |
| [SET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SET.md) | Statement | `SET option$ = value` |
| [SETMEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SETMEM.md) | Function | `SETMEM(bytes&)` |
| [SHELL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SHELL.md) | Statement | `SHELL [command_string$]` |
| [SHELL$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SHELL$.md) | Variable | `SHELL$` |
| [SIOAVAIL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SIOAVAIL.md) | Variable | `SIOAVAIL` |
| [SIOSTATUS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SIOSTATUS.md) | Variable | `SIOSTATUS` |
| [SPEC%](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SPEC%.md) | Function | `SPEC%(code, arg1 [, arg2])` |
| [SSEG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SSEG.md) | Function | `SSEG(var)` |
| [SYS](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/SYS.md) | Function | `SYS(code) \| SYS(cmd_str) \| SYS()` |
| [SYS.ARCH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SYS.ARCH$.md) | Variable | `SYS.ARCH$` |
| [SYS.MEMORY.AVAIL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SYS.MEMORY.AVAIL.md) | Variable | `SYS.MEMORY.AVAIL` |
| [SYS.MEMORY.TOTAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SYS.MEMORY.TOTAL.md) | Variable | `SYS.MEMORY.TOTAL` |
| [SYS.OS$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/SYS.OS$.md) | Variable | `SYS.OS$` |
| [SYSTEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SYSTEM.md) | Statement | `SYSTEM \| BYE \| SHELL [command_string$]` |
| [TEMP$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TEMP$.md) | Variable | `TEMP$` |
| [TEMPDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TEMPDIR$.md) | Variable | `TEMPDIR$ or SYS.TEMPDIR$` |
| [TERM$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TERM$.md) | Variable | `TERM$` |
| [TIM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TIM.md) | Function | `TIM(n)` |
| [TIMESERIAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TIMESERIAL.md) | Function | `TIMESERIAL(hour%, minute%, second%)` |
| [TIMEVALUE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TIMEVALUE.md) | Function | `TIMEVALUE(time_str$)` |
| [TIMEZONE$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TIMEZONE$.md) | Function | `TIMEZONE$` |
| [TIME_PART](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TIME_PART.md) | Function | `TIME_PART(interval$ [, date_serial#]) / TIMEPART(...)` |
| [TOTALMEM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TOTALMEM.md) | Variable | `TOTALMEM or SYS.MEMORY.TOTAL` |
| [TXN](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/TXN.md) | Variable | `TXN or TXNSTATUS` |
| [TZ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TZ.md) | Function | `TZ` |
| [TZ$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/TZ$.md) | Function | `TZ$` |
| [USERNAME$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/USERNAME$.md) | Variable | `USERNAME$ or SYS.USERNAME$` |
| [USERPATH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/USERPATH.md) | Variable | `USERPATH or USERPATH$` |
| [USERPATH$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/USERPATH$.md) | Variable | `USERPATH$ or USERPATH$()` |
| [VER](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/VER.md) | Variable | `VER` |
| [VERSION](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VERSION.md) | Statement | `VERSION "x.y.z" or VERSION$ / VER$` |
| [VERSION$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/VERSION$.md) | Variable | `VERSION$ or SYS.VERSION$` |
| [WALL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WALL.md) | Statement | `WALL message$` |
| [WEEKDAY](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/WEEKDAY.md) | Function | `WEEKDAY(serial#)` |
| [WINVER$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WINVER$.md) | Variable | `WINVER$ or WINVER` |
| [WORDSIZE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WORDSIZE.md) | Variable | `WORDSIZE or SYS.WORDSIZE` |
| [WORKDIR$](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/variables/WORKDIR$.md) | Variable | `WORKDIR$ or CURDIR$` |
| [YEAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/YEAR.md) | Function | `YEAR(serial#)` |

## Variables, Memory & Data Types (40 items)

| Item | Type | Formal Syntax |
| :--- | :--- | :--- |
| [CHANGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CHANGE.md) | Statement | `CHANGE string_expr TO num_array \| CHANGE num_array TO string_var` |
| [CLEAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CLEAR.md) | Statement | `CLEAR [string_space%] \| CLEAR [, [mem_limit%] [, stack_size%]]` |
| [CLR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CLR.md) | Statement | `CLR [var1, var2, ...]` |
| [COMMON](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/COMMON.md) | Statement | `COMMON [SHARED] var1 [()] [, var2 [()]...]` |
| [CONST](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/CONST.md) | Statement | `CONST constantname = expression [, constantname = expression...]` |
| [DATA](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DATA.md) | Statement | `DATA constant1 [, constant2...]` |
| [DECR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DECR.md) | Statement | `DECR variable [, step]` |
| [DEF SEG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEF SEG.md) | Statement | `DEF SEG [= address]` |
| [DEFDBL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEFDBL.md) | Statement | `DEFDBL letter_range [, letter_range...]` |
| [DEFINT](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEFINT.md) | Statement | `DEFINT letter_range [, letter_range...]` |
| [DEFLNG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEFLNG.md) | Statement | `DEFLNG letter_range [, letter_range...]` |
| [DEFSNG](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEFSNG.md) | Statement | `DEFSNG letter_range [, letter_range...]` |
| [DEFSTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DEFSTR.md) | Statement | `DEFSTR letter_range [, letter_range...]` |
| [DIM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/DIM.md) | Statement | `DIM [#channel,] [SHARED] [DYNAMIC \| STATIC] array_name(subscripts...) [*len] [AS type [*len]] [, ...]` |
| [ENUM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ENUM.md) | Statement | `ENUM enum_name` |
| [ERASE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/ERASE.md) | Statement | `ERASE array_name1 [, array_name2...]` |
| [EXCHANGE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/EXCHANGE.md) | Statement | `EXCHANGE var1, var2` |
| [GLOBAL](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/GLOBAL.md) | Statement | `GLOBAL variable [AS type] [, variable...]` |
| [INCR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/INCR.md) | Statement | `INCR variable [, step]` |
| [LET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LET.md) | Statement | `[LET] variable = expression` |
| [LSET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/LSET.md) | Statement | `LSET string_var = string_expression` |
| [MAP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAP.md) | Statement | `MAP (map_name) var1 [= len] [, var2 [= len] ...] \| MAP (map_name) ... MAPEND` |
| [MAPEND](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/MAPEND.md) | Statement | `MAPEND` |
| [OPTION](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/OPTION.md) | Statement | `OPTION BASE {0 \| 1} \| OPTION EXPLICIT` |
| [POKE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/POKE.md) | Statement | `POKE offset, byte_val \| OUT port, val` |
| [PUBLIC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/PUBLIC.md) | Statement | `PUBLIC [SUB\|FUNCTION] name [, name...]` |
| [READ](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/READ.md) | Statement | `READ var1 [, var2...]` |
| [REDIM](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REDIM.md) | Statement | `REDIM [PRESERVE] array_name(subscripts...)` |
| [REMOVE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/REMOVE.md) | Statement | `REMOVE var_or_array, value \| REMOVE$ str_var_or_array, target_str$` |
| [RESTORE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RESTORE.md) | Statement | `RESTORE [line_number \| label] \| RESTORE #file_num` |
| [RSET](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/RSET.md) | Statement | `RSET string_var = string_expression` |
| [SHARE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SHARE.md) | Statement | `SHARE variable [, variable...]` |
| [SHARED](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SHARED.md) | Statement | `SHARED variable [, variable...]` |
| [SHUFFLE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SHUFFLE.md) | Statement | `SHUFFLE arr[()] [, seed] \| SHUFFLE (arr [, seed]) \| SHUFFLE str$ [, seed]` |
| [STATIC](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/STATIC.md) | Statement | `STATIC var1 [AS type] [, var2 ...]` |
| [SWAP](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/SWAP.md) | Statement | `SWAP variable1, variable2` |
| [TYPE](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/TYPE.md) | Statement | `TYPE type_name` |
| [VAR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/VAR.md) | Statement | `VAR [DIM] varname [AS type] [= expr]` |
| [VARPTR](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/functions/VARPTR.md) | Function | `VARPTR(var) \| VARPTR$(var)` |
| [WITH](file:///C:/Users/rtdos/GitHub/basic-plus-plus/docs/keywords/statements/WITH.md) | Statement | `WITH object` |

