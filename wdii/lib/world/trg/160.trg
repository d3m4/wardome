#16001
azeroth dagger~
1 j 100
~
oecho The dagger says, 'Do not let anyone live!'
~
#16002
dagger zeroth~
1 l 100
~
oecho The dagger says, '%actor.name%, are you tired of killing already?'
~
#16003
sith saber~
1 jl 100
~
oecho The red Light Saber says, 'SITH FOREVER!!!'
~
#16004
Disciple Destroyer~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wecho Merlin says, 'Ok %actor.name%, I will send you back.'
wait 1s
wsend %actor.name% Merlin stares at you and utters the words, 'Word of Recall'.
wteleport %actor.name% 503
wecho Merlin stares at %actor.name% and utters the words, 'Word of Recall'.
wecho %actor.name% disappears.
wait 1s
wforce %actor.name% Look
return 0
end
~
#16005
Disciple Destroyer to City~
2 d 100
"Send me to City"~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wecho the Disciple of Destroyer says, 'Ok %actor.name%, I will send you to City now.'
wait 1s
wecho %actor.name% disappears.
wteleport %actor.name% 503
wait 1s
wforce %actor.name% Up
return 0
end
~
$~
