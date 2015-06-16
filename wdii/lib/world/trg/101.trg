#10107
Bilbo Help~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
sniff %actor.name%
say I lost my book somewhere... I need find!
end
~
#10108
Bilbo~
0 ajn 100
~
if (%object.vnum% != 10107)
wait 1s
say I don't want that!
return 0
else
wait 1s
say thanks!
mload obj 10113
mload obj 10114
give mithril %actor.name%
give maegnas %actor.name%
wait 1s
mteleport bilbo 0
drop book
mteleport bilbo 10108
end
~
#10109
Purge Book~
2 h 100
~
if (%object.vnum% == 10107)
wteleport %object% 1
return 0
end
~
#10113
Maegnas Damage~
1 j 100
~
if (%actor.race% == orc)
oset 10113 value1 40
oset 10113 value2 20
else
return 0
oset 10113 value1 20
oset 10113 value2 20
endif
~
#10115
Healer~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
shake %actor.name%
say Hello traveller! Here you can receive a bit of elvish medicin.
say Enjoy it!
end
~
#10141
Moria~
2 ad 100
Mellon~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wait 1s
wsend %actor.name% You says the magical words, and the big rock beneath you opens.
wait 2s
wteleport %actor.name% 10142
wecho %actor.name% says the magical words, and the big rock beneath him opens.
wecho Then something pushes %actor.name% towards the entrance.
wsend %actor.name% Then something pushes you towards the entrance, and you wake up in the entrance of some sort of cave.
wait 1s
wforce %actor.name% look
end
~
#10150
Narsil Hilt~
1 jl 100
~
osend %actor.name% The Fragment (Hilt) of Narsil makes serious wounds in you!
odamage %actor.name% 100
~
#10151
Narsil Middle~
1 jl 100
~
osend %actor.name% The Fragment (Middle) of Narsil makes serious wounds in you!
odamage %actor.name% 300
~
#10152
Narsil Tip~
1 jl 100
~
osend %actor.name% The Fragment (Tip) of Narsil makes serious wounds in you!
odamage %actor.name% 200
~
#10153
Reforged Narsil~
0 jn 100
~
if (%actor.canbeseen%) && (%object.vnum% == 10150) && (%object.vnum% == 10151) (%object.vnum% == 10151)
wait 1s
mpurge narsil
mpurge narsil
mpurge narsil
wait 1s
mload obj 29930
Give Sword %actor.name%
else
wait 1s
say I don't want that!
return 0
end
~
$~
