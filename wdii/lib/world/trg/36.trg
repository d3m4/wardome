#3600
Ola~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
say Someone have to kill him... That *****! Hey you! What are you doing here! Are you a spy?
end
~
#3601
Ola~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
say I want the Black Kings head! Bring it to me!
end
~
#3602
Entrega da cabeca negra~
0 ajn 100
~
if (%object.vnum% != 3621)
wait 1s
say I don't want it!
return 0
else
wait 1s
say You did it! I must give you something...
mload obj 3622
wait 1s
give king %actor.name%
wait 1s
mteleport king 0
drop head
mteleport king 3661
end
~
#3603
Entrega da cabeca branca~
0 ajn 100
~
if (%object.vnum% != 3620)
wait 1s
say I don't want this s***!
return 0
else
wait 1s
say Ha-ha! You killed him! Thanks!
mload obj 3623
wait 1s
give king %actor.name%
wait 1s
mteleport king 0
drop head
mteleport king 3605
end
~
$~
