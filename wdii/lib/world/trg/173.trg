#17313
Merlin~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
shake %actor.name%
say Say "&cSend me back&g" if you want back to &yAshlandar&g.&n
end
~
#17375
Merlin to Ashlandar~
2 d 100
"Send me back"~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wecho Merlin says, 'Ok &w%actor.name%&g, I will send you back.'
wait 1s
wsend %actor.name% Merlin stares at you and utters the words, 'Word of Recall'.
wteleport %actor.name% 503
wecho Merlin stares at %actor.name% and utters the words, 'Word of Recall'.
wecho &R%actor.name% &Wdisappears&w.&n
wait 1s
wforce %actor.name% Look
return 0
end
~
$~
