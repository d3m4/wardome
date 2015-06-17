#22200
Welmar say~
0 g 100
~
say Hello citizen! What do you want here? Do you want to make an arena?
~
#22201
King Arthur geet~
0 g 100
~
say I want the holy grail.. where is it?
~
#22202
Blessed ingot~
0 ajn 100
~
if (%object.vnum% != 17307)
   wait 1s
   say I don't want this!
   return 0
   else
   wait 1s
   say So you have find it! You deserve a gift... Wait a second.
   mload obj 17351
   wait 1s
   give ingot %actor.name%
   wait 1s
   mteleport arthur 0
   drop grail
   mteleport arthur 17370
   shake %actor.name%
   say thanks for everything!
   end
~
#22203
Painting~
1 c 4
enter~
   if (%arg% == painting)
   oecho A blinding light fills the room.
   oechoaround %actor.name% %actor.name% enters in the painting.
   osend %actor.name% You have crossed the painting.
   oteleport %actor.name% 6632
   wait 1s
   oforce %actor.name% look
   return 1
   else
   return 0
   end
~
#22204
Fireplace~
1 c 4
enter~
if (%arg% == fireplace)
oechoaround %actor.name% %actor.name% enters in the fireplace.
osend %actor.name% You have entered the fireplace.
oteleport %actor.name% 6631
wait 1s
oforce %actor.name% look
return 1
else
return 0
end
~
#22205
Ghost attack~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
say BuuuUUUUUuuuuuUUUUUUU!!!!
kill %actor.name%
~
#22206
Enter hole~
2 c 100
enter~
      if (%arg% == buraco)
      wecho A blinding light fills the room.
      wechoaround %actor.name% %actor.name% enters in the painting.
      wsend %actor.name% You have crossed the painting.
      wteleport %actor.name% 30537
      wait 1s
      wforce %actor.name% look
      return 1
      else
      return 0
      end
~
#22207
Damage room~
2 g 100
~
wdamage %actor.name% 100
~
#22208
Tentativa de exp~
2 g 100
~
if ( %char.exp% < 0 )
%send% %actor.name% Voce nao tem experiencia suficiente para entrar aqui.
return 0
end
~
#22209
Charism greet~
0 g 100
~
wait 1s
say Do you want to pass from me? Nah.. I will never let you...
wait 2s
say BUT.... If you trully needs it... hm...
wait 2s
say No.. i cannot let you pass...
wait 1s
blink %actor.name%
~
#22210
Load Avatar Charism~
2 g 100
~
wload mob 9500
wsend %actor.name% When you enter the room a brightly light fills it, than you see The great avatar of charism.
wecho A brightly light fills the room, and the powerfull Avatar of Charism appears after %actor.name%.
~
#22211
Bribe Avatar charism~
0 m 1
~
wait 1
      if (%amount% < 400) 
        say Did you really think I was that cheap, %actor.name%.
        snarl 
      else
        context %actor.id%
        set has_bribed_guard 1
        global has_bribed_guard
        whisper %actor.name% Enter when you're ready. I'll lock the door behind you.
~
#22240
Yentle races~
0 g 100
~
 if (%actor.race% == DWARF) (%actor.vnum% == -1) 
   wait 1s
   say Ola companheiro!!!
   wait 2s
   say Voce e' bem vindo aqui! Voce ja' ouviu as historias de Juargan?
   return 0
else
   wait 1s
   say Saia daqui! Intrusos nao sao bem vindos! So' aceitamos anoes!
   wait 2s
   spit %actor.name%
   return 0
   end
~
#22241
Juargan legend~
0 g 50
~
wait 2s
say Ola aventureiro.. o que fazes pelo meu reino?
wait 2s
say desejas ouvir uma velha lenda de minha terra?
~
#22242
Juargan Legend II~
0 d 1
sim~
wait 1s
say Se assim desejas...
wait 1s
stand
wait 2s
say A muito tempo atras, o nosso reino possuia um poderoso rei, poucos ainda lembram de seu nome...
wait 4s
say Grimdale, o Bravo anao anciao... Com sua alta estatura, 1,38m e seus fortes musculos espantava qualquer criatura que assombrasse o tao prospero Reino de Grimdale...
wait 6s
say Depois de anos e anos prosperando, o reino foi invadido por Drows, e o Rei anciao matou milhares de elfos-drows, mas algo de estranho aconteceu...
wait 5s
say A lute demorou dias, e foi vencida pelos anoes de Grimdale, mas esse ficou apatico, e cada dia perdia um pouco de sua forca, ate' que depois de 63 noites, o grande rei faleceu... Seu corpo estava fragil e seco, apenas pele e osso... Era terrivel!
wait 7s
say Foi feito um grande sarcofago, e seu corpo foi enterrado em uma GIGANTESCA tumba... E conta-se que a sua alma anda parambulando pelos corredores de meu reino ate' os dias de hoje.
wait 5s
sit
wait 1s
say Bem.. Essa e' a historia de Grimdale, resta a voce acreditar no que desejar... So' sei que se sua alma foi realmente aprisionada, algo de extraordinario pode acontecer se ela for liberada.
wait 4s
say Voce quer ouvir essa historia novamente?
~
#22243
Legend Juargan III~
0 d 0
nao~
wait 1s
say Hm.. tudo bem... boa estadia em meu reino.
~
#22244
Yentle fala para SIM~
0 d 1
sim~
if (%actor.race% == DWARF)
wait 1s
say Nao le de ouvidos! Ele anda meio louco..
wait 2s
say Um dia desses estava passando pelo o seu quarto a noite, e ouvi ele ficar resmungando: "Quarta matrona... Quarta matrona..."
wait 4s
say E alem disso ele pediu para mim procurar um cajado que seria usado para ressucitar Grimdale... Que era pra mim levar pra a rainha ana!
wait 4s
say Se ela soubesse da loucura de Juargan provavelmente colocaria outro regente em nosso reino!
wait 4s
say Hm.. Mas pensando bem nao seria nada mal... Eu sou o unico capaz de tomar conta desse lugar.. Ha-ha-ha!
wait 4s
say Opa... acho que nao deveria ter falado isso... Esqueca por favor...
wait 3s
say Apenas para comentar novamente... Nao de ouvidos a Juargan...
wait 2s
say Bom passeio por aqui companheiro!
return 0
else
wait 1s
say SIM O QUE? Saia daqui logo!
wait 1s
fart
wait 1s
say Oops...
return 0
end
~
#22245
Yentle Say NO~
0 d 1
nao~
if %actor.race% == DWARF
wait 1s
say Entao nem perca seu tempo! Ele esta insano!
return 0
else
wait 1s
say O que? Tais maluco? Saia daqui louco!
return 0
end
~
#22246
Queen ana~
0 d 1
oi ola ajuda~
wait 1s
say O que desejas? Alguma noticia de Juargan?
~
#22247
queen juargan louco~
0 d 1
louco insano~
wait 1s
say Hahahaha... Juargan louco? Tenho certeza que nao.. Foi Yentle que falou para voce nao?
wait 2s
laugh
wait 1s
say Yentle esta querendo colocar coisas na minha cabeca so' para eu nomea-lo como Novo rei.. E o que Juargan estava falando tem muito sentido sim! Quarta matrona e' devido a Drow-matron da quarta casa...
wait 5s
say E sobre o cajado... E' que Yentle tem medo de procura-lo... Se voce encontrar um cajado perto de Grimdale por favor.. tente traze-lo ate mim. Preciso fazer uma pocao da verdade para ser entregue para a drow... Te esplico melhor se conseguir trazer...
wait 6s
say Estou contando com a sua ajuda.
spit
kiss %actor.name%
~
#22248
Pocao para Queen~
0 j 100
~
   if (%object.vnum% != 4708)
   wait 1s
   say O que e' isso?
   return 0
   else
   wait 1s
   say Voce encontrou! Ate' que enfim! Espere um segundo..
wait 2s
hold staff
wait 2s
rem staff
wait 1s
junk staff
say Consegui!
   mload obj 6530
   wait 2s
give pocao %actor.name%
wait 2s
   say De isso para a drow, e pergunte-a o que aconteceu no ataque ao reino de Grimdale!
   mteleport queen 0
   drop head
   mteleport queen 6554
wait 2s
say Boa sorte...
   end
~
#22249
Queen say drow~
0 d 0
drow pocao~
wait 1s
say Voce nao sabe? Entao acho que dei o equipamento para a pessoal errada... Pensei que Yentle tinha mandado voce... Esqueca por favor...
~
#22250
Matron greet~
0 g 100
~
wait 1s
say O que voce faz aqui! Saia antes que eu te mate!
~
#22252
Mudanca de Matron~
0 j 100
~
if (%object.vnum% != 6530)
   return 0
   else
   wait 1s
   say O que e' iss(ssss)o?
   wait 2s
   say Eu acho que nao poss(sssss)o tomar... mas... hm.. bem....
   wait 1s
   wecho A Mae-Drow olha bem para a poc(ssss)ao...
   wait 1s
   say O que ela faz(ssss)?
   wait 2s
   mecho A Mae-drow toma a pocao.
   mjunk pocao
   wait 2s
   mecho Um brilho azulado cobre a Mae-drow.
   wait 2s
   say O que acontec(sssss)eu comigo?
   mjunk pocao
   mload mob 5121
   mpurge self
   end
~
#22253
Matron I say juargan~
0 d 0
Juargan Grimdale~
wait 1s
say O que voce esta falando? Voce esta' louco?
wait 2s
mecho A Mae-Drow parece nervosa.
wait 1s
say Saia daqui logo! Nao vou falar nada! Nem que eu morra!
~
#22254
Matron II say Juargan~
0 d 0
Juargan~
wait 1s
say Quem? Juargan? Aquele bastardo? Temos que fazer um novo ataque a vila dele...
~
#22255
Matron II say Grimdale~
0 d 0
Grimdale~
wait 1s
say Ate' hoje nao me esquec(sssss)o daquele dia... Minha maldic(ssssss)ao func(sssss)ionou!
wait 2s
say E a sua alma ficou apris(zzzzz)ionado ate' hoj(jjjj)e!
wait 2s
say Mas para curar e' complicado... E nao irei falar!
wait 2s
say Por favor, saia daqui.. E.. por que estou falando isso tudo??
~
#22256
Matron II say cura~
0 d 1
cura curar medo liberacao alma~
   wait 1s
   say Ahhh.. Eu nao poss(ssssss)o falar iss(sssss)o!
   wait 2s
   mecho A Mae-Drow muda seu tom de voz e diz...
   wait 1s
   say A unica pessoa capaz de liberar o espirito de Grimdale e' um druida que mora dentro de uma arvore... E' so' falar que voce quer a cura para a maldicao do 'Aprisionamento de Alma'.
   wait 5s
   say E leve junto um pouco de po' de aranha que pode ser encontrado perto daqui... E' o ingrediente principal para a pocao que ele vai ter que fazer.
   wait 3s
   mecho O efeito da pocao comeca a terminar.
   wait 1s
   say O que eu estava falando???
   mload mob 5120
   mpurge self
~
#22257
Matron I say Grimdale~
0 d 100
grimdale~
wait 1s
say Grimdale? Nao conheco ninguem com esse nome.. Nunca ouvi falar.
wait 1s
mecho A mae-drow engole em seco.
~
#22258
Druid Po de aranha~
0 j 100
~
      if (%object.vnum% != 5160)
      return 0
      else
      wait 1s
      Say Bem.. Apenas pelo produto eu ja' sei o que voce quer que eu faca... Espere alguns segundos...
      wait 2s
      mecho O druida tira um grande caldeirao de um compartimento estranho.
      wait 3s
      mecho O druida joga o po de aranha no caldeirao junto com alguns outros itens comuns.. Como agua, pedacos de folha, etc...
      wait 2s
      mecho O druida fecha os olhos e fala algumas palavras estranhas.
      wait 3s
mecho Uma luz forte sai de dentro do caldeirao, e um cheiro forte e desagradavel comeca a te irritar.
      wait 3s
      mecho Finalmente o druida tira de dentro do caldeirao um pouco da pocao.
      wait 2s
      say Aqui esta... Espero que voce saiba como utiliza-la.
mload obj 6160
      give pocao %actor.name%
      mjunk aranha
      end
~
#22259
Druid say aprisionamento~
0 d 1
aprisionamento alma~
wait 1s
say Hm... Por que voce quer saber disso? Bem.. nao tem problema...
wait 2s
say Consiga para mim Po de aranha, entao farei uma pocao para voce...
wait 3s
say Com essa pocao, va' ate algum coelho da floresta, e despeje sobre ele (pour).
wait 3s
say Depois disso, pegue a criatura que vai estar paralizada, e sacrifique-a na frente da alma aprisionada.
wait 4s
say Pronto.. apenas isso que voce tem que fazer.
~
#22260
Rabbit pour~
0 c 100
pour~
if %arg% == rabbit
wait 1s
mecho Voce despeja a pocao em cima do coelho.
wait 2s
mecho Ele se contrai durante segundos, e fica paralizado
mload obj 5161
mpurge self
return 1
~
#22261
Sacrifice Rabbit~
2 c 2
sacrifice~
  if %arg% == coelho
wforce %actor.name% junk coelho
wait 1s
wecho Voce sacrifica o coelho na sala.
wait 1s
wecho Uma luz comeca a brilhar de Grimdale.
wait 2s
wecho Por alguns segundos voce nao ve nada de tao claro que a sala esta.
wait 2s
wecho Quando tudo fica um pouco mais normal, voce nota que todos os espiritos dessa sala desapareceram, e apenas a alma de Grimdale ainda esta' em pe'.
wpurge Grimdale
wpurge spirit
wpurge spirit
wpurge spirit
wload mob 4730
wait 3s
wecho A alma de Grimdale fala: Muito obrigado por me ajudar... Serei grato para sempre...
wait 3s
wecho A alma de Grimdale fala: Fique com a essencia de minha alma dentro desse amuleto, e adeus...
wait 4s
wecho Um clarao cobre a sala, e Grimdale desaparece, enquanto um amuleto surpreendentemente aparece no chao.
wpurge grimdale
wload obj 4732
return 1
else
wecho What do you want to sacrifice?
return 1
end
~
#22262
Alma de Grimdale~
0 k 100
~
say Voce nunca conseguira me deter.. sou um espirito! Pare com isso!
mdamage grimdale -10000
~
$~
