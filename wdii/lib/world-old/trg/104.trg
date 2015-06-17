#10537
Portal to Hell~
1 c 4
enter~
if (%arg% == pentagram) || (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 10401
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
