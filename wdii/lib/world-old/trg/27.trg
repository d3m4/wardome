#2700
primeira sala~
2 d 100
sim~
wait 2
wecho &WThe spirit of WarDome says, 'Voce falou &Rsim&W para todos presentes na sala.'&n
wait 3
wecho &WThe spirit of WarDome says, 'Para ir para norte, digite &Rnorth&W ou &Rn&W e entao voce seguira para a proxima sala.'&n
~
#2701
segunda sala~
2 d 100
score?~
wait 2
wecho &WThe spirit of WarDome says, 'O comando &Rscore &Wmostra todas informacoes disponiveis sobre seu personagem, como dano, peso carregado, titulo , nivel e pontos de experiencia. Verifique-o constantemente para ficar por dentro de seu personagem.'&n
wait 3
wecho &WThe spirit of WarDome says, 'Detalhe importante: voce nao possui somente o comando 'say' para comunicacao, existem 3 outros: 'chat', 'gossip' e 'tell'.'&n
wait 4
wecho &WThe spirit of WarDome says, 'O &Rtell&W e' o unico com formato diferente, use-o nesse formato: 'tell <jogador> <texto>', isso enviara' <texto> ao <jogador>.'&n
~
#2711
boat~
1 c 4
jump~
if (%arg% == ship) || (%arg% == boat)
oecho A blind fog to all in the boat.
oechoaround %actor.name% %actor.name% jumps on boat and disapears in the fog.
osend %actor.name% You jumps on the boat and slowly you see the dock looking smaller in the dark river.
oteleport %actor.name% 2787
wait 2s
osend %actor.name% You vision come back after the fog dissipates.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#2712
Elf - Lord of the Rings~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
shake %actor.name%
say &WWelcome to &YMiddle Earth&w, %actor.name%&n
say &wBuy the &yPathway&w of the place you want to be and give it to me... I'll send you there!&n
end
~
#2713
Teleport City~
0 j 100
~
if (%object.vnum% == 2700)
say Okay &W%actor.name%&g, I'll send you to The &YShire&g.&n
wait 1s
mteleport %actor.name% 2714
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2707)
say Okay &W%actor.name%&g, I'll send you to The &WG&wrey &CH&cavens&g.&n
wait 1s
mteleport %actor.name% 30084
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2702)
say Okay &W%actor.name%&g, I'll send you to The &cBree&g.&n
wait 1s
mteleport %actor.name% 2744
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2703)
say Okay &W%actor.name%&g, I'll send you to The &GV&galfenda&g.&n
wait 1s
mteleport %actor.name% 10100
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2704)
say Okay &W%actor.name%&g, I'll send you to The &YM&yoria&g.&n
wait 1s
mteleport %actor.name% 10141
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2708)
say Okay &W%actor.name%&g, I'll send you to The &YDi&ymr&Will &yDa&Yle&g.&n
wait 1s
mteleport %actor.name% 20200
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2705)
say Okay &W%actor.name%&g, I'll send you to The &GLo&gth&Wlo&gri&Gen&g.&n
wait 1s
mteleport %actor.name% 29857
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
elseif (%object.vnum% == 2709)
say Okay &W%actor.name%&g, I'll send you to The &BA&bn&Wdu&bi&Bn&n&g.&n
wait 1s
mteleport %actor.name% 29976
mecho %actor.name% disappears in a puff of smoke.
wait 1s
mpurge %object.name%
else
return 0
say Nice try, I don't want that!
drop %object.name%
endif
~
#2715
portal - Lord of The Ring~
1 c 4
enter~
if (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 2700
wait 2s 
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
$~
