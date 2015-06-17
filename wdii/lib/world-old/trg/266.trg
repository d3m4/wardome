#26600
Ajudante~
0 d 100
"quero ajuda"~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WSobre o que voce gostaria de aprender?&n
wait 1s
tell %actor.name% &WScore? Wimpy? Spells? Skills? Kill? Chat? Affect? Buy? Breath? Sell? List? Holy Points? Meta? Direcoes? Pk? Roleplay Points? Recall? Tell?&n
wait 1s
tell %actor.name% &WEscolha uma Opcao acima e digite: '&GSay Opcao&n'
end
~
#26601
Ajudante - Score~
0 d 100
score~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WScore eh o comando que contem todas as informacoes sobre o seu personagem, como atributos, dinheiro, hp, mana, move, entre outras coisas.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Score&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26602
Ajudante - Wimpy~
0 d 100
wimpy~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WWimpy eh o seu salva-vidas, utilizado como 'wimpy <numero>' ele fara voce fugir da luta quando sua hp atingir um nivel abaixo do numero colocado com o comando.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Wimpy&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26603
Ajudante - Spells~
0 d 100
spells~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WSpells sao magias, que quando utilizadas gastam sua mana, as magias variam desde Fireball, que joga uma imensa bola de fogo na sala ateh Heal, que cura uma grande quantidade de dano.&n
tell %actor.name% &WPara ver as magias que voce conhece, digite 'spells'. Para ver as que voce ainda ira aprender, digite "slist".&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Spells&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26604
Ajudante - Skills~
0 d 100
skills~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WSkills sao habilidades que seu personagem possui, desde o famoso Bash, que derruba seu oponente, ateh Steal, a skill dos Thieves para roubar. Para ver as skills que voce conhece,&n
wait 1s
tell %actor.name% &Wdigite 'skills', para ver as que voce ainda ira aprender, digite "slist".&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Skills&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26605
Ajudante - Kill~
0 d 100
kill~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WKill eh o comando utilizado para atacar um mob ou player, pode ser abreviado para 'k <nome do mob ou player>'.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Kill&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26606
Ajudante - Chat~
0 d 100
chat~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WChat eh o canal utilizado para conversas, ateh o nivel 10, voce nao podera utiliza-lo, para poder falar com todos, igual no chat, voce deve utilizar o newbie chat, com o seguinte comando "newbie <mensagem>" (sem as aspas)&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Chat&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26607
Ajudante - Affect~
0 d 100
affect~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO comando affect mostra para voce as skills e spells que estao lhe afetando neste momento, alem de mostrar o nome da skill/spell, ele mostra tambem como ela te afeta e por quanto tempo.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Affect&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26608
Ajudante - Buy~
0 d 100
buy~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WBuy eh o comando utilizado para comprar itens nos Shops. O comando basico eh 'buy <item>' mas pode ser utilizado de varias formas distintas, como por exemplo, voce deseja comprar 5 waybreads no Baker, 
wait 1s
tell %actor.name% &Wvoce deve digitar 'buy 5 waybread', ou voce deseja comprar uma espada mas existem 7 espadas diferentes no shop, voce pode utilizar o numero do item para comprar a espada 'buy #7'.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Buy&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26609
Ajudante - Sell~
0 d 100
sell~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO comando sell eh utilizado para vender itens que voce possua, utilizado no seguinte formato 'sell <item>'. Soh pode ser utilizado em Shops.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Sell&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26610
Ajudante - List~
0 d 100
list~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO comando list, serve para listar os items que estao a venda nos shops espalhados pelo Wardome.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp List&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26611
Ajudante - Holy Points~
0 d 100
"holy points"~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WHoly Points sao pontos que voce ganha a cada nivel atingido, voce ganha 7 Holy Ponits por nivel que passar, que podem ser gastos utilizando o comando meta, para comprar HP, Mana, Move e Atributos.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Holypoints&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26612
saindo da zone~
2 d 100
"estou pronto"~
if %actor.level% == 1
wsend %actor.name% The Spirit of Wardome tells you, ' Voce precisa ser level 2 para sair desta zona.'
return 0
else
wsend %actor.name% The spirit of Wardome tells you, 'Boa sorte e muitas aventuras em WarDome.'
wait 1s
wsend %actor.name% Voce esta na sala preparatoria. Em breve sera teleportado para o seu templo.
wteleport %actor.name% 500
wecho &R%actor.name%&W desaparece numa nuvem de fumaca!&n
return 1
end
~
#26613
Ajudante - Meta~
0 d 100
meta~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO comando Meta eh utilizado nos Meta Shops, para comprar HP, Mana, Move e Atributos. Os Meta Shops estao localizados em Wardome City, quando voce estiver em um, digite 'meta' para ver uma lista do que aquele shop oferece.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Meta&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26614
Ajudante - Direcoes~
0 d 100
direcoes~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO Wardome Mud conta com dezenas zonas dos mais variados temas, indo de Lord of the Rings ateh Notre Dame, para explorar estas areas, existem as direcoes, que sao comandos para se movimentar pelo mundo de Wardome,&n
wait 1s
tell %actor.name% &Was direcoes sao basicamente as 4 principais direcoes da rosa-dos-ventos, North, South, East, West, mais as direcoes Up e Down, para andar, pode se usar simplesmente a inicial de cada uma, N, S, E, W, U, D.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Directions&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26615
Ajudante - Pk~
0 d 100
pk~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WO Wardome, eh um Mud PK, oque isto significa? Significa que aqui, pk signifaca Player Killing, ou seja player pode matar player, mas existem restricoes, soh eh possivel matar alguem que esteja dentro do seu range,&n
wait 1s
tell %actor.name% &Wou seja 9 niveis a mais ou a menos, eles sao identificados por um * no who. Players Remort, nao podem atacar players R0. Voce serah questionado se deseja ser ou nao pk, ao atingir o nivel 10.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Pk&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26616
Ajudante - Roleplay Points~
0 d 100
"roleplay points"~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WRoleplay sao pontos que voce ganha reportando bugs, participando de quest e etc, e que podem ser gastos a down do Hazaard, para comprar Equipamentos Especiais.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Roleplay&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26617
Ajudante - Breath~
0 d 100
breath~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WBreath, eh o seu folego, ele diminui quando voce luta ou entra em algum lugar muito fechado, tome cuidado, pois se ele chegar a zero, voce morrera sem ar.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Breath&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26618
Ajudante - Recall~
0 d 100
recall~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WRecall eh uma Magia que faz voce voltar ao seu templo. Ateh o nivel 10 vc poderah desfrutar dessa magia livremente (sem nenhum custo), basta digitar &Rrecall&W.&n
wait 1s
tell %actor.name% &WA partir do nivel 11 o recall passara a custar uma certa quantia de Exp, assim vc devera comprar um "&GScroll of Recall&W" q pode ser encontrado no Magic Shop.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Recall&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26619
Ajudante - Tell~
0 d 100
tell~
if (%actor.vnum% == -1) && (%actor.canbeseen%)
wait 1s
tell %actor.name% &WTell eh uma maneira de voce conversar exclusivamente com uma unica pessoa que vc escolher, basta digitar o simples comando: '&gtell <nome do player> <mensagem>&W'&n
wait 1s
tell %actor.name% &WApos receber algum tell, basta digitar '&greply <mensagem>&W' para responder.&n
wait 2s
tell %actor.name% &WSe ainda tem alguma duvida sobre este assunto, digite '&RHelp Tell&W' para maiores explicacoes!&n
tell %actor.name% &WSe voce tiver mais alguma duvida, digite: '&GSay Opcao&n'
end
~
#26624
saindo sala (ing)~
2 d 100
"I am done"~
if %actor.level% == 1
wsend %actor.name% The spirit of WarDome tells you, 'You need advance to the second level before enter the WarDome.'
return 0
else
wsend %actor.name% The spirit of WarDome tells you, 'Have a nice time playing WarDome MUD!'
wait 1s
wsend %actor.name% You are in the preparation room. You will be teleported to your temple soon.
wteleport %actor.name% 500
wecho &R%actor.name% &Wdisappears in a puff of smoke!&n
return 1
end
~
#26630
worm ataca~
0 ak 100
~
say MMMmmmMMMmmmMMM.
kill %actor.name%
~
#26697
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
#26698
Merlin~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
shake %actor.name%
say Say "&cSend me back&g" if you want back to &yAshlandar&g.&n
end
~
#26699
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
$~
