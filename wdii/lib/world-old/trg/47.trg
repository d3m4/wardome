#4700
Juargan greet~
0 g 25
~
wait 2s
say Ola aventureiro, voce quer ouvir uma historia?
~
#4701
new trigger~
0 d 100
sim~
wait 1s
say Bem, a lenda conta que a alma de um antigo rei ainda vive pelas minhas cavernas...
wait 1s
say O nome do rei de certa forma ja' foi esquecido, ele era Grimdale, o grande... Morreu em uma grande batalha contra umber hulks que invadiram nossa antiga vila subterranea.
wait 3s
say Conta-se que em seu sarcofago ha um poderoso equipamento, que se for entregue para a pessoa certa, Grimdale tera a sua alma libertada, e podera finalmente descansar em paz.
wait 2s
say Resta a voce acreditar no que esta certo e o que esta errado... Boa estadia em meu reino.
~
#4702
new trigger~
0 g 50
~
if %actor.race% == DWARF
wait 1s
say Ola companheiro!!!
wait 2s
say Voce e' bem vindo aqui! Voce ja' ouviu as historias de Juargan?
return;
if %actor.race% == ELF
wait 1s
spit %actor.name%
wait 1s
say SAIA CRIATURA! Elfos nao sao bem vindos aqui!
return;
if %actor.race% == ORC
wait 1s
shout ORCCCCCCCCCC!!!!!!!!!!!! GUARDAS!!!
kill %actor.name%
return;
if %actor.race% == OGRE
wait 1s
say Ogros... Por Talos... AHHHHHHHHHHHHHHHH!!!
flee
return;
else
wait 1s
say O que voce quer???
wait 2a
say Por favor, saia daqui, essa e' uma vila de anoes... Por isso que nao gosto de Juargan.. ele fica chamando pessoas de outras racas.. Blergh...
return;
~
$~
