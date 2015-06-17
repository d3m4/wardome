#28600
potion dano~
1 c 2
q~
if (%arg% == s) || (%arg% == so) || (%arg% == sou) || (%arg% == soul)
if (%actor.room% != 10)
osend %actor.name% Your body can't control the power!
odamage %actor.name% 10000
end
return 1
else
return 0
end
~
#28652
Dragon Barrel~
1 cn 2
drink~
if (%arg% == barrel)
oechoaround %actor.name% The liquid of the Dragon's Barrel flows mysteriously and it is full.
opurge %self%
oload obj 28652
end
~
#28653
Dragon Barrel Load~
1 n 100
~
oechoaround %actor.name% The liquid of the Dragon's Barrel flows mysteriously and it is full.
oload obj 28652
~
$~
