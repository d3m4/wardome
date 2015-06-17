#20250
Move Emyn Muil~
2 g 100
~
if (%actor.vnum% == -1) (%actor.canbeseen%)
wsend %actor.name% You feel even more tired.
wechoaround %actor.name% %actor.name% feel even more tired.
wdamage %actor.name% 50
end
~
$~
