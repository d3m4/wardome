#503
Ashlandar to Merlin~
2 d 100
"Send me to Merlin"~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wecho the Questmaster Ashlandar says, 'Ok &w%actor.name%&g, I will send you to &yMerlin &gnow.'
wait 1s
wecho &R%actor.name% &Wdisappears&w.&n
wteleport %actor.name% 17374
wait 1s
wforce %actor.name% Up
return 0
end
~
#510
human temple~
2 g 100
~
if %actor.race != human% then
say Voce nao e bem vindo em nosso templo.
oforce %actor.name% s
end
~
$~
