' MINE
'
' Mine is the classic game where a field of hidden mines is presented, and the
' user tries to find the mines based on mine counts in adjacent squares.
'
' Derived from the Pascal version.

' helper defines

false% = 0
true% = -1

' colors

black%   = 0
white%   = 1
red%     = 2
green%   = 3
blue%    = 4
cyan%    = 5
yellow%  = 6
magenta% = 7

' events

etchar%    = 0  ' ANSI character returned
etup%      = 1  ' cursor up one line
etdown%    = 2  ' down one line
etleft%    = 3  ' left one character
etright%   = 4  ' right one character
ettab%     = 19 ' tab
etenter%   = 20 ' enter line
etmouba%   = 47 ' mouse button assertion
etterm%    = 60 ' terminate program

' program defines

maxxs%   = 8 ' size of grid
maxys%   = 8
maxmine% = 10 ' number of mines to place

dim mine%(maxxs%, maxys%) ' mine exists map
dim vis%(maxxs%, maxys%)  ' square is uncovered map
dim flag%(maxxs%, maxys%) ' square is flagged map

' Contruct table to calculate adjacent squares

dim xoff%(8) ' x offset table
dim yoff%(8) ' y offset table

xoff%(1) =  0: yoff%(1) = -1 ' up
xoff%(2) = +1: yoff%(2) = -1 ' upper right
xoff%(3) = +1: yoff%(3) =  0 ' right
xoff%(4) = +1: yoff%(4) = +1 ' lower right
xoff%(5) =  0: yoff%(5) = +1 ' down
xoff%(6) = -1: yoff%(6) = +1 ' lower left
xoff%(7) = -1: yoff%(7) =  0 ' left
xoff%(8) = -1: yoff%(8) = -1 ' upper left

' declares, not required but nice

dim x%, y%    ' user move coordinates
dim done%     ' game over
dim centerx%  ' center of screen position x
dim centery%  ' center of screen position y
dim cursorx%  ' cursor location x
dim cursory%  ' cursor location y
dim badguess% ' bad guess display flag
dim mousex%   ' mouse position x
dim mousey%   ' mouse position y

maxx% = 80
maxy% = 25

' Shared variables declaration
shared false%, true%, black%, white%, red%, green%, blue%, cyan%, yellow%, magenta%
shared etchar%, etup%, etdown%, etleft%, etright%, ettab%, etenter%, etterm%, etmouba%
shared maxxs%, maxys%, maxmine%, done%, centerx%, centery%, cursorx%, cursory%, badguess%, mousex%, mousey%
shared maxx%, maxy%

' nextevent global variables
dim nextevent_etype%, nextevent_char$, nextevent_moupx%, nextevent_moupy%, nextevent_amoubn%
shared nextevent_etype%, nextevent_char$, nextevent_moupx%, nextevent_moupy%, nextevent_amoubn%

sub get_event
  nextevent_etype% = 0
  nextevent_char$ = ""
  nextevent_moupx% = 0
  nextevent_moupy% = 0
  nextevent_amoubn% = 0
  
  do
    k$ = inkey$()
    if k$ <> "" then
      nextevent_char$ = k$
      if len(k$) = 2 and asc(left$(k$, 1)) = 0 then
        select case asc(right$(k$, 1))
          case 72: nextevent_etype% = etup%
          case 80: nextevent_etype% = etdown%
          case 75: nextevent_etype% = etleft%
          case 77: nextevent_etype% = etright%
        end select
      else
        select case asc(k$)
          case 13: nextevent_etype% = etenter%
          case 9:  nextevent_etype% = ettab%
          case 27: nextevent_etype% = etterm%
          case else: nextevent_etype% = etchar%
        end select
      end if
      exit sub
    end if
    
    mb = pen(3)
    if mb <> 0 then
      nextevent_etype% = etmouba%
      nextevent_amoubn% = mb
      nextevent_moupx% = int(pen(4) / 8) + 1
      nextevent_moupy% = int(pen(5) / 16) + 1
      exit sub
    end if
    
    sleep 0.01
  loop
end sub

' Find adjacent mines
function adjacent%(x%, y%)
  dim mines%
  dim xn%, yn%
  dim i%
  
  mines% = 0
  for i% = 1 to 8
     xn% = x%+xoff%(i%)
     yn% = y%+yoff%(i%)
     if xn% >= 1 and xn% <= maxxs% and yn% >= 1 and yn% <= maxys% and mine%(xn%, yn%) then mines% = mines%+1
  next i%
  adjacent% = mines%
end function

' Set adjacent squares visible
sub visadj(x%, y%)
  dim xn%, yn%
  dim i%
   
  for i% = 1 to 8
     xn% = x%+xoff%(i%)
     yn% = y%+yoff%(i%)
     if xn% >= 1 and xn% <= maxxs% and yn% >= 1 and yn% <= maxys% and not vis%(xn%, yn%) then vis%(xn%, yn%) = true%: if adjacent%(xn%, yn%) = 0 then visadj(xn%, yn%)
  next i%
end sub

' Display board
sub display
  dim x%
  dim y%
  dim cnt%
   
  bcolor(yellow%)
  for y% = 1 to maxys%
     for x% = 1 to maxxs%
        cursor(centerx%+x%-1, centery%+y%-1)
        if vis%(x%, y%) and mine%(x%, y%) then print "*";
        if vis%(x%, y%) and not mine%(x%, y%) then cnt% = adjacent%(x%, y%): if cnt% = 0 then print "."; else print chr$(cnt%+asc("0"));
        if not vis%(x%, y%) and flag%(x%, y%) and badguess% then print "X";
        if not vis%(x%, y%) and flag%(x%, y%) and not badguess% then print "M";
        if not vis%(x%, y%) and not flag%(x%, y%) then print "=";
     next x%
  next y%
  print
end sub

' Initialize board
sub clrbrd
  dim x%
  dim y%
  dim n%
   
  for x% = 1 to maxxs%
     for y% = 1 to maxys%
        mine%(x%, y%) = false
        vis%(x%, y%) = false
        flag%(x%, y%) = false
     next y%
  next x%
  for n% = 1 to maxmine
     do
        x% = int(rnd(0)*maxxs%) + 1
        y% = int(rnd(0)*maxys%) + 1
     loop until not mine%(x%, y%)
     mine%(x%, y%) = true
  next n%
end sub

' Clear line
sub clrlin(y%, clr%)
  dim i%
  cursor(1, y%)
  bcolor(clr%)
  for i% = 1 to maxx%
     print " ";
  next i%
end sub

' Print centered string
sub prtmid(y%, s$)
  cursor(int(maxx% / 2) - int(len(s$) / 2), y%)
  print s$;
end sub

' Draw character box
sub tbox(sx%, sy%, ex%, ey%, c$, bclr%, fclr%)
  dim x%, y%
  
  bcolor(bclr%)
  fcolor(fclr%)
  cursor(sx%, sy%)
  for x% = sx% to ex%: print c$;: next x%
  cursor(sx%, ey%)
  for x% = sx% to ex%: print c$;: next x%
  for y% = sy%+1 to ey%-1
     cursor(sx%, y%)
     print c$;
  next y%
  for y% = sy%+1 to ey%-1
     cursor(ex%, y%)
     print c$;
  next y%
end sub

' Check coordinates on board
function onboard%(x%, y%)
  dim inx%
  dim iny%
  
  inx% = x% >= centerx% and x% <= centerx%+maxxs%-1
  iny% = y% >= centery% and y% <= centery%+maxys%-1
  onboard% = inx% and iny%
end function

' Check replay
sub replay
  bcolor(cyan%)
  prtmid(maxy%, "PLAY AGAIN (Y/N) ?")
  do
     do
        get_event
     loop until nextevent_etype% = etchar% or nextevent_etype% = etterm%
     if nextevent_etype% = etterm% then done% = true%
  loop until lcase(nextevent_char$) = "y" or lcase(nextevent_char$) = "n" or done%
  
  if lcase(nextevent_char$) = "n" or done% then done% = true% else clrlin(maxy%-2, cyan%): clrlin(maxy%, cyan%): clrbrd: cursorx% = centerx%: cursory% = centery%: badguess% = false%
end sub

' Process square "hit"
sub hit(x%, y%)
  dim xi%, yi%
  dim viscnt%
  
  vis%(x%, y%) = true%
  if not mine%(x%, y%) then goto 500
  
  for yi% = 1 to maxys%
     for xi% = 1 to maxxs%
        if mine%(xi%, yi%) then vis%(xi%, yi%) = true%
     next xi%
  next yi%
  badguess% = true%
  display
  bcolor(red%)
  prtmid(maxy%-2, "*** YOU HIT A MINE ! ***")
  replay
  goto 600
  
500 REM
  if adjacent%(x%, y%) = 0 then visadj(x%, y%)
  viscnt% = 0
  for yi% = 1 to maxys%
     for xi% = 1 to maxxs%
        if vis%(xi%, yi%) then viscnt% = viscnt%+1
     next xi%
  next yi%
  if maxxs%*maxys%-viscnt% = maxmine% then display: bcolor(red%): prtmid(maxy%-2, "*** YOU WIN ! ***"): replay
  
600 REM
  display
end sub

' Main process

auto(false%)
bcolor(cyan%)
clear
bcolor(magenta%)
prtmid(1, "******* Mine game 1.0 ********")
centerx% = int(maxx% / 2) - int(maxxs% / 2)
centery% = int(maxy% / 2) - int(maxys% / 2)
tbox(centerx%-1, centery%-1, centerx%+maxxs%, centery%+maxys%, " ", blue%, black%)
bcolor(white%)
clrbrd
display
done% = false%
cursorx% = centerx%
cursory% = centery%
badguess% = false%

do
   cursor(cursorx%, cursory%)
   x% = cursorx%-centerx%+1
   y% = cursory%-centery%+1
   get_event
   select case nextevent_etype%
      case ettab%
         flag%(x%, y%) = not flag%(x%, y%)
         display
      case etenter%
         hit(x%, y%)
      case etup%
         if cursory% > centery% then cursory% = cursory%-1
      case etleft%
         if cursorx% > centerx% then cursorx% = cursorx%-1
      case etdown%
         if cursory% < centery%+maxys%-1 then cursory% = cursory%+1
      case etright%
         if cursorx% < centerx%+maxxs%-1 then cursorx% = cursorx%+1
      case etmouba%
         mousex% = nextevent_moupx%
         mousey% = nextevent_moupy%
         if nextevent_amoubn% = 1 and onboard%(mousex%, mousey%) then cursorx% = mousex%: cursory% = mousey%: x% = cursorx%-centerx%+1: y% = cursory%-centery%+1: hit(x%, y%)
         if nextevent_amoubn% = 2 and onboard%(mousex%, mousey%) then cursorx% = mousex%: cursory% = mousey%: x% = cursorx%-centerx%+1: y% = cursory%-centery%+1: flag%(x%, y%) = not flag%(x%, y%): display
   end select
loop until done% or nextevent_etype% = etterm%

auto(true%)
bcolor(white%)
fcolor(black%)
clear
end