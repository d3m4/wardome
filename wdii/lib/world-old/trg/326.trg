#32601
testando~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
jump %actor.name%
say fala viado dos infernos
end
~
#32602
teste level~
0 gk 100
~
say oi
~
#32603
gandalf~
0 agjn 100
~
if (%object.vnum% != 5217)
wait 1s
say Esse nao me parece o item que eu pedi.
return 0
else
wait 1s
say Muito obrigado amigo, agora va ate a ladra da biblioteca perdida e lhe entregue esse item que eu lhe dei.
mload obj 537
wait 1s
give spell %actor.name%
wait 1s
mteleport gandalf 581
end
~
#32604
sora~
0 ajn 100
~
if (%object.vnum% != 537)
wait 1s
say Esse nao me parece o item que esperava de Gandalf.
return 0
else
wait 1s
say Muito obrigado grande %actor.name%, agora lhe presentearei com um valioso premio.
mload obj 3999
wait 1s
give vale %actor.name%
wait 1s
mteleport sora 0
junk spell
mteleport sora 3919
end
~
$~
