#29854
Gollum Says~
0 h 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wait 1s
say My ring....
wait 2s
say My Precious...
wait 3s
say Give me!.... I need my Precious One....
wait 2s
cry
end
~
#29855
Gollum II~
0 ajn 100
~
if (%object.vnum% != 29825)
wait 1s
say I don't want that!
return 0
else
wait 1s
mteleport Gollum 0
drop cursed
mteleport Gollum 29854
wait 1s
mload obj 29812
give old %actor.name%
wait 1s
say You are a dumbass, &RDIE!&n
say Give me back the ring
wait 1s
kill %actor.name%
end
~
$~
