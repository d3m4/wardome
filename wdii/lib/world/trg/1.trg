#100
portal~
1 c 4
enter~
if (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 7700
wait 2s 
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#101
recall ~
1 c 2
testerecall~
if (%arg% == my) || (%arg% == me)
osend %actor.name% A white winged descends from the sky to take you home.
oechoaround %actor.name% %actor.name% blows the whistle and a white, winged horse descends from the sky.
osend %actor.name% Pegasus has brought you to Emma's home.
oteleport %actor.name% 500
wait 2s
osend %actor.name% descends from the sky on a winged horse.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#102
purge scythe~
1 f 100
~
oecho scythe of death disappears.
   opurge %self%
~
#103
set timer scythe~
1 g 100
~
otimer 5
~
#104
purge scythe remove~
1 l 100
~
oecho scythe of death disappears.
opurge %self%
~
#110
spiked collar~
1 jl 100
~
osend %actor.name% a Spiked Collar makes serious wounds in you!
odamage %actor.name% 400
~
$~
