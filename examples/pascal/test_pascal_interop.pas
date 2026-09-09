program test_pascal_interop;

{$IFDEF FPC}
  {$MODE OBJFPC}{$H+}
{$ENDIF}

uses
  SysUtils, basicpp;

var
  Engine: TBppEngine;
  Score: Double;
  Player: string;
  ResultVal: Double;
begin
  WriteLn('====================================================================');
  WriteLn('   BASIC++ FreePascal / Delphi Interoperability Example');
  WriteLn('====================================================================');

  Engine := TBppEngine.Create(32);
  try
    WriteLn('Initialized BASIC++ v', Engine.Version);

    // 1. Variable Injection
    Engine.SetNumber('SCORE', 15000.0);
    Engine.SetString('PLAYER$', 'PascalDev');

    // 2. Statement Execution
    Engine.Exec('BONUS = 2500 : TOTAL = SCORE + BONUS');

    // 3. Variable Retrieval
    Score := Engine.GetNumber('TOTAL');
    Player := Engine.GetString('PLAYER$');
    WriteLn('Player: ', Player, ' | Total Score: ', Score:0:2);

    // 4. Expression Evaluation
    ResultVal := Engine.EvalNumber('SQR(100) + 5 * 10');
    WriteLn('Expression SQR(100) + 5 * 10 = ', ResultVal:0:2);

    WriteLn('====================================================================');
    WriteLn('   PASCAL INTEROP COMPLETED SUCCESSFULLY');
    WriteLn('====================================================================');
  finally
    Engine.Free;
  end;
end.
