Route111_WinstrateFamilysHouse_MapScripts:: @ 815BF24
	.byte 0

Route111_WinstrateFamilysHouse_EventScript_15BF25:: @ 815BF25
	lock
	faceplayer
	setvar 0x8008, 2
	msgbox Route111_WinstrateFamilysHouse_Text_191FCE, 4
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BF3A:: @ 815BF3A
	lock
	faceplayer
	setvar 0x8008, 3
	checkflag 277
	goto_if_eq Route111_WinstrateFamilysHouse_EventScript_15BF72
	msgbox Route111_WinstrateFamilysHouse_Text_19208A, 4
	giveitem ITEM_MACHO_BRACE
	compare RESULT, 0
	goto_if_eq Route111_WinstrateFamilysHouse_EventScript_1A029B
	setflag 277
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BF72:: @ 815BF72
	msgbox Route111_WinstrateFamilysHouse_Text_19214D, 4
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BF80:: @ 815BF80
	lock
	faceplayer
	setvar 0x8008, 1
	msgbox Route111_WinstrateFamilysHouse_Text_192190, 4
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BF95:: @ 815BF95
	lock
	faceplayer
	setvar 0x8008, 4
	checkflag 4
	goto_if_eq Route111_WinstrateFamilysHouse_EventScript_15BFB6
	msgbox Route111_WinstrateFamilysHouse_Text_192219, 4
	setflag 4
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BFB6:: @ 815BFB6
	msgbox Route111_WinstrateFamilysHouse_Text_192334, 4
	goto Route111_WinstrateFamilysHouse_EventScript_15BFC4
	end

Route111_WinstrateFamilysHouse_EventScript_15BFC4:: @ 815BFC4
	closemessage
	applymovement 0x8008, Route111_WinstrateFamilysHouse_Movement_1A083D
	waitmovement 0
	release
	end
