#1
memory test trigger~
0 o 199
~
* assign this to a mob, force the mob to mremember you, then enter the
* room the mob is in while visible (not via goto)
say I remember you, %actor.name%!
~
#2
mob greet test~
0 g 100
100~
say Hello, %actor.name%, how are things to the %direction%?
~
#3
obj get test~
1 g 100
~
oecho You hear, 'Please put me down, %actor.name%'
~
#4
room test~
2 g 100
~
wait 10
wsend %actor% %actor.level% is a slow level no?
~
#5
car/cdr test~
0 d 100
test~
say speech: %speech%
say car: %speech.car%
say cdr: %speech.cdr%
~
#6
subfield test~
0 c 100
test~
* test to make sure %actor.skill(skillname)% works
%actor.skill(hide)% = 20
say your hide ability is %actor.skill(hide)% percent.
~
#7
object otransform test~
1 jl 7
test~
* test of object transformation (and remove trigger)
* test is designed for objects 3020 and 3021
* assign the trigger then wear/remove the item
* repeatedly.
oecho Beginning object transform.
if %self.vnum% == 3020
  otransform 3021
else
  otransform 3020
end
oecho Transform complete.
~
#8
makeuid and remote testing~
2 c 100
test~
* makeuid test ---- assuming your MOBOBJ_ID_BASE is 200000,
* this will display the names of the first 10 mobs loaded on your MUD,
* if they are still around.
eval counter 0
while (%counter% < 10)
  makeuid mob 200000+%counter%
  wecho #%counter%      %mob.id%   %mob.name%
  eval counter %counter% + 1
done
*
*
* this will also serve as a test of getting a remote mob's globals.
* we know that puff, when initially loaded, is id 200000. We'll use remote
* to give her a global, then %mob.globalname% to read it.
makeuid mob 200000
eval globalname 12345
remote globalname %mob.id%
wecho %mob.name%'s "globalname" value is %mob.globalname%
~
#9
mtransform test~
0 g 100
~
* mtransform test
* as a greet trigger, entering the room will cause
* the mob this is attached to, to toggle between mob 1 and 99.
mecho Beginning transform.
if %self.vnum%==1
  mtransform 99
else
  mtransform 1
end
mecho Transform complete.
~
#10
mud school 1~
1 g 100
~
   wecho The Wardome asks, 'I will kill you, %actor.name% !!!'
~
#11
entering room~
0 g 100
~
say i think i'm the best
~
#12
teste~
0 e 100
~
shutdown
~
#13
Master Learn~
0 agi 100
~
say Hello %actor.name% are you ready to Learn to play Mud?
wait 4s
Say Follow me %actor.name%
wait 7s
say Good. Here we will learn some Basic commands.
wait 2s
n
wait 6s
say Type Inv to see your Inventory.
wait 6s
say In the Inventory do you have some Equips.Type Wear All.
wait 8s
say Good, now you are not more skinned.! hehehehe
wait 7s
say Now, do you need a Weapon right? So type Wield Sword for this.
wait 9s
say Do you think is prepared for the fight?
wait 7s
say But I don't think, for that we will continue.
wait 10s
n
say Cool. Now you need to eat and to Drink.
wait 7s
say Type INV again. Do you see a bread and a bottle right?
wait 10s
say Then.Type Eat bread and later Drink bottle.
wait 10s
say Spectacular. I am beginning to be proud!
wait 10s
say The Time is short, we will continue.
wait 7s
n
wait 12s
say The magic is something fabulous. Would you like a lot to learn, not?
wait 7s
say So Look for this.
wait 7s
recite  identify
wait 6s 
say It's not Fantastic? To do that  types  RECITE <name of Scroll>
wait 7s
say To use a spell it's simple. Type Cast " name of Spell "
wait 8s
say The " are Fundamental
wait 7s
say Then adventurous nobleman. It's time of me  to leave. Go to North and enter in the true Battle.
say That the force is with you.
gossip That the force is with you %actor.name%
wait 5s
group all
ungroup
wait 3s
s
s
s
~
#14
Specials Commands~
0 g 100
~
wait 2s
say Hello %actor.name% here you learn somes Specials Commands.
wait 7s
Say The first command is Score. To use  types  SCORE
wait 7s
say The second is Whois. This command show somes player's Stats. For this type WHOIS <NAME OF PLAYER>
wait 7s
say The third is META POINTS, that is the most important because with him you add Hp, Mv, and Mn or Attributes as Str, Dex, Wiz etc..
wait 7s
say to use Go to Cryognist Center or Pit Bull's Gym and type META, in your score he is called as Holy Points.
wait 7s
Say Now a command that soon will be created. Challend, with him you can challenge somebody same being PK.
wait 7s
Say So it's this.. Go and unmask this New World Called &RW&rA&RR&rD&RO&rM&RE &rM&RU&rD
~
#16
illusion finish~
0 n 100
~
wait 50s
mecho The illusion slowly fades out of existence.
mpurge me
~
#17
mana sword finish~
1 n 100
~
wait 10s
oecho The mana sword dissolves in your hands.
opurge me
~
#18
Quest Shop~
0 g 100
~
se of A newbie guardian
~
#19
GateWay to Hell~
1 cg 4
enter~
if (%arg% == pentagram)
oecho A Dark light fills the room.
oechoaround %actor.name% %actor.name% enters in the pentagram and disappears.
osend %actor.name% You enter in the pentagram and the Darklights blind you for some instants.
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
#20
bem vindo~
0 h 100
~
if ispc ($n)
say Welcome to Attlantis!
~
#21
rosnar~
0 e 100
~
if ispc ($n)
$n looks 
tell $n Grrrrrrr
~
#22
welcome~
0 h 100
~
if ispc($n)
say KILL ME, PLEASE!!!!
~
#25
hazaard~
0 d 100
hazaard~
wait 2s
say How can I help you, %actor.name%?
~
#30
Wardome Helper~
0 g 100
~
say Here %actor.name% you learn all for your need.
~
#31
Wardome Helper 2~
0 g 100
~
mecho The Wardome Helper chats, 'Be Welcome %actor.name% in this new World called Wardome.
wait 4s
say In this school you will learn how to survive in this world.
wait 5s
say type say help to a brief help.
wait 4s
say Digite say ajuda para receber algumas dicas.
~
#32
responde~
0 d 100
help~
The MUD use basically english commands. Type help to view the main commands.
say The Mud use basically english commands. Type help to view the main commands.
mload obj 602
give guide %actor.name%
~
#33
ajuda~
0 d 100
ajuda~
say O Mud usa basicamente comandos em ingles. Digite help para ver os principais comandos disponiveis.
~
#34
fonte~
1 b 100
~
oecho You hear a whisper, 'Throw a coin in me and make a request. The Gods will bless you.'
~
#35
fonte2~
1 c 100
give~
say My trigger commandlist is not complete!
~
#36
Ashlandar~
0 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
smile %actor.name%
say Type "&GLIST&n" to see the current &Wquests&n.
say Say "&cSend me to Merlin&n" If you looking for the &RQuest&YBook&n i could send you to &yMerlin&n.
end
~
#37
sala600~
2 g 100
~
wait 2s
wecho The Wardome talks to you, 'You are at the Entrance of Wardome School, go to east and lets rock!'
wecho The Wardome talks to you, 'Go to the east - type E for abreviation.'
~
#38
room601~
2 g 100
~
wait 2s
wecho The Wardome talks to you, 'Hello %actor.name% are you ready to Learn how to play MUD?'
wecho The Wardome talks to you, 'So... Let's go to the (n)north. Only type N'
~
#39
room604~
2 g 100
~
wait 2s
wecho The Wardome talks to you, 'Good. Here we will learn some Basic commands.'
wecho The Wardome talks to you, 'Let's take a look to the (n)north.'
~
#40
room605~
2 g 100
~
wait 5s
wecho The Wardome talks to you, 'Type INV to see your inventory.'
wait 5s
wecho The Wardome talks to you, 'In the Inventory you have some equips. So Type wear all to wear them.'
wait 8s
wecho The Wardome talks to you, 'Good, now you are not more skinned!'
wait 7s
wecho The Wardome talks to you, 'Now, do you need a Weapon right? So type wield sword for this.'
wait 9s
wecho The Wardome talks to you, 'Do you think is prepared for the fight?'
wait 7s
wecho The Wardome talks to you, 'But I don't think, for that we will continue.'
wait 3s
wecho The Wardome talks to you, 'So type (n) to go to the north and continue with this.'
~
#41
room606~
2 g 100
~
wait 5s
wecho The Wardome talks to you, 'Cool. Now you need how to eat and Drink.'
wait 7s
wecho The Wardome talks to you, 'Type INV again. Do you see a bread and a bottle right?'
wait 10s
wecho The Wardome talks to you, 'Then, type Eat bread and after Drink bottle.'
wait 10s
wecho The Wardome talks to you, 'Spectacular. I'm beginning to be proud of you!'
wait 10s
wecho The Wardome talks to you, 'The Time is short, we will continue.'
wait 5s
wecho The Wardome talks to you, 'Go to the (n)north to continue learning.'
~
#42
room607~
2 g 100
~
wait 5s
wecho The Wardome talks to you, 'The magic is something fabulous. Would you like a lot to learn, not?'
wait 7s
wecho The Wardome talks to you, 'So Look for this.'
wait 6s 
wecho The Wardome talks to you, 'It's not Fantastic? To do that  types  RECITE <name of Scroll>'
wait 7s
wecho The Wardome talks to you, 'To use a spell it's simple. Type Cast "name of Spell"'
wait 8s
wecho The Wardome talks to you, 'The " are Fundamental'
wait 7s
wecho The Wardome talks to you, 'Then adventurous nobleman. It's time of me to leave. Go to North and enter in the true Battle.'
wecho The Wardome talks to you, 'Good Luck dude!'
~
#44
mana sword set timer~
1 n 100
~
otimer 5
~
#45
mana sword~
1 f 100
~
eval nome %self.name%
eval umcdr %nome.cdr%
eval doiscdr %umcdr.cdr%
eval dono %doiscdr.trim%
oecho The mana sword of %dono% dissolves and vanishes.
opurge %self%
~
#50
Hazaard~
0 g 100
~
if (%actor.vnum% == -1) && (%actor.canbeseen%) && (%actor.level% < 6)
shake %actor.name%
say The &CMETA GUILDS&n stay at &c2sen&n or &c2s4e&n.
say You can change your &CHOLY POINTS&n in any &GMETA GUILD&n.
say &WTo go to the &RNEWBIE ZONE&W, follow &cs3ese&W.
end
if (%actor.vnum% == -1) && (%actor.canbeseen%) && (%actor.level% > 5)
shake %actor.name%
say Type &clist&n and change with me your &GQUEST POINTS&n.
end
~
#51
wardome guard~
0 g 100
~
if (%actor.vnum% == -1) && (%actor.canbeseen%) && (%actor.level% == 1)
protect %actor.name%
mecho The Guard exclaims, 'We will protect the city and it's citizens untill the end!'
end
~
#52
temples~
2 g 100
~
wecho The Wardome remembers, 'Don't forget to type look map adventurer.'
~
#53
astral~
1 c 4
enter~
if (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 7700
wait 2s 
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#55
info~
0 e 100
appears in the middle of the room.~
mecho Hello, New adventure, to go to the Wardome School &MTYPE INFO.&n
~
#56
sobre skills -> hazaard~
0 d 100
tell me about skills~
wait 2
say I can help you win some new skills.
wait 8
say You will need &CQUEST POINTS&n and some &GHOLY POINTS&n to "buy"...
wait 8
say Good Luck!!!
~
#60
healer~
0 cd 100
heal~
if %speech.cdr% == 
msend %actor% My list:
msend %actor% armor          10000
msend %actor% bless          50000
end
if %speech.cdr% == bless
say bless you!
mat %actor.name% look
end
~
#61
healer 2~
0 c 100
heal~
return 1
say Passoooooouuuuu!!!
say %arg%
look %actor.name%
kiss %actor.name%
cast 'armor' %actor.name%
%self.skill(hide)% = 100
~
#62
Gateguard~
0 g 100
~
if (%direction% == east)
wait 1s
close gate
lock gate
wait 1s
say Admittance to city is now 10 coins for your group.
end
~
#63
gateguard1~
0 m 10
~
wait 1s
unlock gate
open gate
wait 20s
close gate
lock gate
~
#64
gateguard2~
0 m 1
~
wait 1s
say This is not enough!
give %amount% coins %actor.name%
~
#65
gateguard 3~
0 g 100
~
if (%direction% == west)
wait 1s
close gate
lock gate
wait 1s
emote to attention as you approach.
wait 1s
say Admittance to city is now 10 coins for your group.
end
~
#66
gateguard 5~
0 m 10
~
wait 1s
unlock gate
open gate
wait 20s
close gate
lock gate
~
#67
gateguard 6~
0 m 1
~
wait 1s
say This is not enough!
give %amount% coins %actor.name%
~
#68
gateguard fecha~
0 g 100
~
if (%direction% == east)
close gate
lock gate
end
~
#69
gateguard7~
0 g 100
~
If (%direction% == west)
unlock gate
open gate
wait 10s
close gate
lock gate
end
~
#70
gateguard8~
0 g 100
~
If (%direction% == east)
unlock gate
open gate
wait 10s
close gate
lock gate
end
~
#71
Portal Wishes~
1 c 4
enter~
if (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 14383
wait 2s
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#72
Portal Desert~
1 c 4
enter~
if (%arg% == 2.portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 14300
wait 2s
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#73
Portal Oracle~
1 c 4
enter~
if (%arg% == 3.portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the portal and disappears.
osend %actor.name% You enter in the portal and the lights blind you for some instants.
oteleport %actor.name% 2231
wait 2s
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#74
lady~
0 d 0
veste~
wear all
~
#75
special~
1 n 100
~
opurge %self%
~
#76
purgar tempus remove~
1 l 100
~
oecho The Bloody sword of tempus lost its power.
opurge %self%
~
#77
sword of tempus~
1 f 100
~
oecho The bloody sword of Tempus lost its power.
opurge %self%
~
#78
set timer tempus~
1 f 100
~
oecho The Bloody sword of tempus lost its power.
   opurge %self%
~
#79
setar timer~
1 g 100
~
otimer 10
~
#80
stargate~
1 c 7
enter~
if (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the stargate and disappears.
osend %actor.name% You enter in the stargate and the lights blind you for some instants.
oteleport %actor.name% 14300
wait 2s
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#81
stargate2~
1 c 4
enter~
if (%arg% == gate) || (%arg% == stargate) || (%arg% == portal)
oecho A blinding light fills the room.
oechoaround %actor.name% %actor.name% enters in the stargate and disappears.
osend %actor.name% You enter in the stargate and the lights blind you for some instants.
oteleport %actor.name% 5348
wait 2s 
osend %actor.name% Your vision returns. You feel strange.
wait 2s
oforce %actor.name% look
return 1
else
return 0
end
~
#82
look stargate~
1 c 4
look~
if (%arg% == stargate) || (%arg% == gate) || (%arg% == portal)
osend %actor.name% You see a image of a desert through the portal.
return 1
else
return 0
end
~
#85
portal rats lair~
2 c 100
enter~
if %arg% == portal
wechoaround %actor.name% %actor.name% enters in the portal and disappears.
wsend %actor.name% You enter in the portal. You feel strange.
wteleport %actor.name% 3839
wait 2s
wforce %actor.name% look
return 1
else
return 0
end
~
#86
portal rats lair 2~
2 c 100
enter~
if %arg% == portal
wechoaround %actor.name% %actor.name% enters in the portal and disappears.
wsend %actor.name% You enter in the portal. You feel strange.
wteleport %actor.name% 3838
wait 2s
wforce %actor.name% look
return 1
else
return 0
end
~
#90
Head hunter~
0 g 100
~
if (%actor.level% <= 50)
say Hello friend, how about a journey ?
wait 1s
say I'm a crazy to kill somebody...
wait 2s
say I can do this for just 3000000 coins.
end
~
#91
Head hunter 2~
0 m 3000000
~
if (%actor.level% <= 50) 
shake %actor.name%
wait 1s
mjunk 3000000 coins
wait 1s
say So, we have a deal... 
wait 3s
fol %actor.name%
wait 1s
say Just group me. (type group all)
end
~
#92
Head hunter 3~
0 m 1
~
wait 1s
laugh %actor% 
wait 1s
say With this coins i can just help you to kill a fido...
wait 1s
give %amount% coins %actor%
~
#94
Head hunter 4~
0 k 100
~
assist %actor.name%
assist %actor%
~
#96
teste barco~
1 f 100
~
oteleport %self.name% 1204
~
#97
falar nome~
0 k 100
~
mload mob 13011
oecho &RDerrepente arkanus se transforma em dois!!!!
mtransform 12201
wait 5
wecho TESTANTO, NANDO GAY!
echo coloquei essa linha
~
#98
schyte~
1 c 2
hold wield wear~
if (%actor.name% != Zhantar)
wait 10s
oforce %actor.name% rem all
oforce %actor.name% drop all
odamage %actor.name% 1000
oecho the Schyte of Death brightly burns at %acotr.name%'s hand.
end
~
#99
eq vnum 99~
1 j 100
the Scythe of Death~
if (%actor.name% != Fenix)
oecho the Scythe of Death glows as %actor.name% wears it.
oecho The muscles in %actor.name%'s arm swell as he wiel the Schyte of Death.
odamage %actor.name% 10000
end
~
$~
