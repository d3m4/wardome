#30500
Portal 1~
1 c 4
enter~
if (%arg% == portal)
oecho Uma luz ofuscante interrompe a escuridao.
oechoaround %actor.name% %actor.name% entra no portal e desaparece.
osend %actor.name% Voce entra no portal e uma luz o cega por alguns instantes
oteleport %actor.name% 30503
wait 2s
osend %actor.name% Sua visao retorna, voce se sente estranho.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#30506
new trigger~
0 g 100
~
      if (%arg% == buraco)
      oecho A blinding light fills the room.
      oechoaround %actor.name% %actor.name% enters in the painting.
      osend %actor.name% You have crossed the painting.
      oteleport %actor.name% 30537
      wait 1s
      oforce %actor.name% look
      return 1
      else
      return 0
      end
~
$~
