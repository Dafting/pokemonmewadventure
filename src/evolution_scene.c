#include "global.h"
#include "task.h"
#include "evolution_graphics.h"
#include "palette.h"
#include "main.h"
#include "text.h"
#include "text_window.h"
#include "pokemon.h"
#include "string_util.h"
#include "battle.h"
#include "unknown_task.h"
#include "data2.h"
#include "decompress.h"
#include "m4a.h"
#include "trade.h"
#include "menu.h"
#include "pokedex.h"
#include "species.h"
#include "sound.h"
#include "songs.h"
#include "rom4.h"
#include "battle_message.h"
#include "pokemon_summary_screen.h"

struct EvoInfo
{
    u8 preEvoSpriteID;
    u8 postEvoSpriteID;
    u8 evoTaskID;
    u8 field_3;
};

#define sEvoInfo (*(struct EvoInfo *)(ewram + 0x14800))

void EvolutionRenameMon(struct Pokemon *mon, u16 oldSpecies, u16 newSpecies);
void sub_8024CEC(void);
void sub_8023A80(void);
void sub_802BC6C(void);
void nullsub_6(void);

extern struct Window gUnknown_03004210;
extern u16 gUnknown_030042A4;
extern u16 gUnknown_030042A0;
extern u16 gUnknown_030042C0;
extern u16 gUnknown_030041B4;
extern u16 gUnknown_03004288;
extern u16 gUnknown_03004280;
extern u16 gUnknown_030041B0;
extern u16 gUnknown_030041B8;
extern u8 gBattleTerrain;
extern u8 gReservedSpritePaletteCount;
extern u16 word_2024E82;

extern void * const gUnknown_081FAF4C[];
extern const u8* const gBattleStringsTable[];

extern struct SpriteTemplate gUnknown_02024E8C;
extern u8 gUnk_2009000[]; // won't match if I 'ewram' it
extern bool8 gAffineAnimsDisabled;
extern MainCallback gUnknown_03005E94;

extern u8 gBattleCommunication[];
#define sEvoCursorPos        gBattleCommunication[1] // when learning a new move
#define sEvoGraphicsTaskID   gBattleCommunication[2]

// this file's functions
void EvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, bool8 canStopEvo, u8 partyID);
void Task_EvolutionScene(u8 taskID);
void Task_TradeEvolutionScene(u8 taskID);
void VBlankCB_EvolutionScene(void);
void VBlankCB_TradeEvolutionScene(void);
void HBlankCB_EvolutionScene(void);
void CB2_EvolutionSceneUpdate(void);
void CB2_TradeEvolutionSceneUpdate(void);

void CB2_BeginEvolutionScene(void)
{
    UpdatePaletteFade();
    RunTasks();
}

#define tState          data[0]
#define tMonPtrHI       data[1]
#define tMonPtrLO       data[2]
#define tPreEvoSpecies  data[3]
#define tPostEvoSpecies data[4]
#define tCanStop        data[5] // in first fast data[5] only checks that
#define tBits           data[5] // in the second task, it works as a bitfield
#define tData6          data[6]
#define tData8          data[8]
#define tData9          data[9]
#define tData10         data[10]
#define tEvoWasStopped  data[11]
#define tPartyID        data[12]

#define TASK_BIT_CAN_STOP       0x1
#define TASK_BIT_x80            0x80

void Task_BeginEvolutionScene(u8 taskID)
{
    struct Pokemon* mon = NULL;
    switch (gTasks[taskID].tState)
    {
    case 0:
        BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
        gTasks[taskID].tState++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            u16 speciesToEvolve;
            bool8 canStopEvo;
            u8 partyID;

            mon = (struct Pokemon*)(gTasks[taskID].tMonPtrHI | (gTasks[taskID].tMonPtrLO << 0x10));
            speciesToEvolve = gTasks[taskID].tPostEvoSpecies;
            canStopEvo = gTasks[taskID].tCanStop;
            partyID = gTasks[taskID].tPartyID;

            DestroyTask(taskID);
            EvolutionScene(mon, speciesToEvolve, canStopEvo, partyID);
        }
        break;
    }
}

void BeginEvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, bool8 canStopEvo, u8 partyID)
{
    u8 taskID = CreateTask(Task_BeginEvolutionScene, 0);
    gTasks[taskID].tState = 0;
    gTasks[taskID].tMonPtrHI = (u32)(mon);
    gTasks[taskID].tMonPtrLO = (u32)(mon) >> 0x10;
    gTasks[taskID].tPostEvoSpecies = speciesToEvolve;
    gTasks[taskID].tCanStop = canStopEvo;
    gTasks[taskID].tPartyID = partyID;
    SetMainCallback2(CB2_BeginEvolutionScene);
}

void EvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, bool8 canStopEvo, u8 partyID)
{
    u8 name[20];
    u16 currSpecies;
    u32 TiD, PiD;
    const struct CompressedSpritePalette** pokePal;
    u8 ID;
    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);
    CpuFill32(0, (void*)(VRAM), VRAM_SIZE);

    REG_MOSAIC = 0;
    REG_WIN0H = 0;
    REG_WIN0V = 0;
    REG_WIN1H = 0;
    REG_WIN1V = 0;
    REG_WININ = 0;
    REG_WINOUT = 0;

    SetUpWindowConfig(&gWindowConfig_81E6C58);
    ResetPaletteFade();

    gUnknown_030042A4 = 0;
    gUnknown_030042A0 = 0;
    gUnknown_030042C0 = 0;
    gUnknown_030041B4 = 0;
    gUnknown_03004288 = 0;
    gUnknown_03004280 = 0;
    gUnknown_030041B0 = 256;
    gUnknown_030041B8 = 0;

    InitWindowFromConfig(&gUnknown_03004210, &gWindowConfig_81E6C58);
    gBattleTerrain = 9;

    sub_800D6D4();
    sub_800DAB8();
    ResetSpriteData();
    remove_some_task();
    ResetTasks();
    FreeAllSpritePalettes();

    gReservedSpritePaletteCount = 4;

    GetMonData(mon, MON_DATA_NICKNAME, name);
    StringCopy10(gStringVar1, name);
    StringCopy(gStringVar2, gSpeciesNames[speciesToEvolve]);

    // preEvo sprite
    currSpecies = GetMonData(mon, MON_DATA_SPECIES);
    TiD = GetMonData(mon, MON_DATA_OT_ID);
    PiD = GetMonData(mon, MON_DATA_PERSONALITY);
    DecompressPicFromTable_2(&gMonFrontPicTable[currSpecies],
                             gMonFrontPicCoords[currSpecies].coords,
                             gMonFrontPicCoords[currSpecies].y_offset,
                             (void*)(0x2000000),
                             gUnknown_081FAF4C[1], currSpecies);
    pokePal = (void*) sub_80409C8(currSpecies, TiD, PiD);
    LoadCompressedPalette(*pokePal, 0x110, 0x20);

    GetMonSpriteTemplate_803C56C(currSpecies, 1);
    gUnknown_02024E8C.affineAnims = gDummySpriteAffineAnimTable;
    sEvoInfo.preEvoSpriteID = ID = CreateSprite(&gUnknown_02024E8C, 120, 64, 30);

    gSprites[ID].callback = nullsub_37;
    gSprites[ID].oam.paletteNum = 1;
    gSprites[ID].invisible = 1;

    // postEvo sprite
    DecompressPicFromTable_2(&gMonFrontPicTable[speciesToEvolve],
                             gMonFrontPicCoords[speciesToEvolve].coords,
                             gMonFrontPicCoords[speciesToEvolve].y_offset,
                             (void*)(0x2000000),
                             gUnknown_081FAF4C[3], speciesToEvolve);
    pokePal = (void*) sub_80409C8(speciesToEvolve, TiD, PiD);
    LoadCompressedPalette(*pokePal, 0x120, 0x20);

    GetMonSpriteTemplate_803C56C(speciesToEvolve, 3);
    gUnknown_02024E8C.affineAnims = gDummySpriteAffineAnimTable;
    sEvoInfo.postEvoSpriteID = ID = CreateSprite(&gUnknown_02024E8C, 120, 64, 30);
    gSprites[ID].callback = nullsub_37;
    gSprites[ID].oam.paletteNum = 2;
    gSprites[ID].invisible = 1;

    LoadEvoSparkleSpriteAndPal();

    sEvoInfo.evoTaskID = ID = CreateTask(Task_EvolutionScene, 0);
    gTasks[ID].tState = 0;
    gTasks[ID].tPreEvoSpecies = currSpecies;
    gTasks[ID].tPostEvoSpecies = speciesToEvolve;
    gTasks[ID].tMonPtrHI = (u32)(mon);
    gTasks[ID].tMonPtrLO = (u32)(mon) >> 0x10;
    gTasks[ID].tCanStop = canStopEvo;
    gTasks[ID].tData6 = 1;
    gTasks[ID].tEvoWasStopped = FALSE;
    gTasks[ID].tPartyID = partyID;

    memcpy(gUnk_2009000, &gPlttBufferUnfaded[0x20], 0x60);

    REG_DISPCNT = DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP;
    SetHBlankCallback(HBlankCB_EvolutionScene);
    SetVBlankCallback(VBlankCB_EvolutionScene);
    m4aMPlayAllStop();
    SetMainCallback2(CB2_EvolutionSceneUpdate);
}

void CB2_EvolutionSceneLoadGraphics(void)
{
    u8 ID;
    const struct CompressedSpritePalette** pokePal;
    u16 postEvoSpecies;
    u32 TiD, PiD;
    struct Pokemon* Mon = &gPlayerParty[gTasks[sEvoInfo.evoTaskID].tPartyID];

    postEvoSpecies = gTasks[sEvoInfo.evoTaskID].tPostEvoSpecies;
    TiD = GetMonData(Mon, MON_DATA_OT_ID);
    PiD = GetMonData(Mon, MON_DATA_PERSONALITY);

    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);
    CpuFill32(0, (void*)(VRAM), VRAM_SIZE);

    REG_MOSAIC = 0;
    REG_WIN0H = 0;
    REG_WIN0V = 0;
    REG_WIN1H = 0;
    REG_WIN1V = 0;
    REG_WININ = 0;
    REG_WINOUT = 0;
    SetUpWindowConfig(&gWindowConfig_81E6C58);
    ResetPaletteFade();
    gUnknown_030042A4 = 0;
    gUnknown_030042A0 = 0;
    gUnknown_030042C0 = 0;
    gUnknown_030041B4 = 0;
    gUnknown_03004288 = 0;
    gUnknown_03004280 = 0;
    gUnknown_030041B0 = 256;
    gUnknown_030041B8 = 0;

    InitWindowFromConfig(&gUnknown_03004210, &gWindowConfig_81E6C58);
    gBattleTerrain = 9;

    sub_800D6D4();
    sub_800DAB8();
    ResetSpriteData();
    FreeAllSpritePalettes();
    gReservedSpritePaletteCount = 4;

    DecompressPicFromTable_2(&gMonFrontPicTable[postEvoSpecies],
                             gMonFrontPicCoords[postEvoSpecies].coords,
                             gMonFrontPicCoords[postEvoSpecies].y_offset,
                             (void*)(0x2000000),
                             gUnknown_081FAF4C[3], postEvoSpecies);
    pokePal = (void*) sub_80409C8(postEvoSpecies, TiD, PiD);
    LoadCompressedPalette(*pokePal, 0x120, 0x20);

    GetMonSpriteTemplate_803C56C(postEvoSpecies, 3);
    gUnknown_02024E8C.affineAnims = gDummySpriteAffineAnimTable;
    sEvoInfo.postEvoSpriteID = ID = CreateSprite(&gUnknown_02024E8C, 120, 64, 30);

    gSprites[ID].callback = nullsub_37;
    gSprites[ID].oam.paletteNum = 2;

    REG_DISPCNT = DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP;
    SetHBlankCallback(HBlankCB_EvolutionScene);
    SetVBlankCallback(VBlankCB_EvolutionScene);
    SetMainCallback2(CB2_EvolutionSceneUpdate);
    BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
}

void CB2_TradeEvolutionSceneLoadGraphics(void)
{
    struct Pokemon* Mon = &gPlayerParty[gTasks[sEvoInfo.evoTaskID].tPartyID];
    u16 postEvoSpecies = gTasks[sEvoInfo.evoTaskID].tPostEvoSpecies;

    switch (gMain.state)
    {
    case 0:
        REG_DISPCNT = 0;
        SetHBlankCallback(NULL);
        SetVBlankCallback(NULL);
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 4;
        gUnknown_030042A4 = 0;
        gUnknown_030042A0 = 0;
        gUnknown_030042C0 = 0;
        gUnknown_030041B4 = 0;
        gUnknown_03004288 = 0;
        gUnknown_03004280 = 0;
        gUnknown_030041B0 = 256;
        gUnknown_030041B8 = 0;
        gMain.state++;
        break;
    case 1:
        SetUpWindowConfig(&gWindowConfig_81E6F84);
        InitWindowFromConfig(&gUnknown_03004828->field_4, &gWindowConfig_81E6F84);
        gMain.state++;
        break;
    case 2:
        LoadTextWindowGraphics(&gUnknown_03004828->field_4);
        gUnknown_03004828->field_34 = SetTextWindowBaseTileNum(2);
        LoadTextWindowGraphics(&gUnknown_03004828->field_4);
        MenuZeroFillScreen();
        ResetPaletteFade();
        gMain.state++;
        SetHBlankCallback(HBlankCB_EvolutionScene);
        SetVBlankCallback(VBlankCB_TradeEvolutionScene);
        break;
    case 3:
        sub_804E22C();
        gMain.state++;
        break;
    case 4:
        {
            const struct CompressedSpritePalette** pokePal;
            u32 TiD = GetMonData(Mon, MON_DATA_OT_ID);
            u32 PiD = GetMonData(Mon, MON_DATA_PERSONALITY);
            DecompressPicFromTable_2(&gMonFrontPicTable[postEvoSpecies],
                                     gMonFrontPicCoords[postEvoSpecies].coords,
                                     gMonFrontPicCoords[postEvoSpecies].y_offset,
                                     (void*)(0x2000000),
                                     gUnknown_081FAF4C[3], postEvoSpecies);
            pokePal = (void*) sub_80409C8(postEvoSpecies, TiD, PiD);
            LoadCompressedPalette(*pokePal, 0x120, 0x20);
            gMain.state++;
        }
        break;
    case 5:
        {
            u8 ID;

            GetMonSpriteTemplate_803C56C(postEvoSpecies, 3);
            gUnknown_02024E8C.affineAnims = gDummySpriteAffineAnimTable;
            sEvoInfo.postEvoSpriteID = ID = CreateSprite(&gUnknown_02024E8C, 120, 64, 30);

            gSprites[ID].callback = nullsub_37;
            gSprites[ID].oam.paletteNum = 2;
            gMain.state++;
        }
        break;
    case 6:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        SetMainCallback2(CB2_TradeEvolutionSceneUpdate);
        REG_DISPCNT = DISPCNT_OBJ_ON | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_OBJ_1D_MAP;
        break;
    }
}

void TradeEvolutionScene(struct Pokemon* mon, u16 speciesToEvolve, u8 preEvoSpriteID, u8 partyID)
{
    u8 name[20];
    u16 currSpecies;
    u32 TiD, PiD;
    const struct CompressedSpritePalette** pokePal;
    u8 ID;

    GetMonData(mon, MON_DATA_NICKNAME, name);
    StringCopy10(gStringVar1, name);
    StringCopy(gStringVar2, gSpeciesNames[speciesToEvolve]);

    gAffineAnimsDisabled = TRUE;

    // preEvo sprite
    currSpecies = GetMonData(mon, MON_DATA_SPECIES);
    PiD = GetMonData(mon, MON_DATA_PERSONALITY);
    TiD = GetMonData(mon, MON_DATA_OT_ID);
    sEvoInfo.preEvoSpriteID = preEvoSpriteID;
    DecompressPicFromTable_2(&gMonFrontPicTable[speciesToEvolve],
                             gMonFrontPicCoords[speciesToEvolve].coords,
                             gMonFrontPicCoords[speciesToEvolve].y_offset,
                             (void*)(0x2000000),
                             gUnknown_081FAF4C[1], speciesToEvolve);
    pokePal = (void*) sub_80409C8(speciesToEvolve, TiD, PiD);
    LoadCompressedPalette(*pokePal, 0x120, 0x20);

    GetMonSpriteTemplate_803C56C(speciesToEvolve, 1);
    gUnknown_02024E8C.affineAnims = gDummySpriteAffineAnimTable;
    sEvoInfo.postEvoSpriteID = ID = CreateSprite(&gUnknown_02024E8C, 120, 64, 30);

    gSprites[ID].callback = nullsub_37;
    gSprites[ID].oam.paletteNum = 2;
    gSprites[ID].invisible = 1;

    LoadEvoSparkleSpriteAndPal();

    sEvoInfo.evoTaskID = ID = CreateTask(Task_TradeEvolutionScene, 0);
    gTasks[ID].tState = 0;
    gTasks[ID].tPreEvoSpecies = currSpecies;
    gTasks[ID].tPostEvoSpecies = speciesToEvolve;
    gTasks[ID].tMonPtrHI = (u32)(mon);
    gTasks[ID].tMonPtrLO = (u32)(mon) >> 0x10;
    gTasks[ID].tData6 = 1;
    gTasks[ID].tEvoWasStopped = FALSE;
    gTasks[ID].tPartyID = partyID;

    SetMainCallback2(CB2_TradeEvolutionSceneUpdate);
}

void CB2_EvolutionSceneUpdate(void)
{
    AnimateSprites();
    BuildOamBuffer();
    sub_800374C(&gUnknown_03004210);
    UpdatePaletteFade();
    RunTasks();
}

void CB2_TradeEvolutionSceneUpdate(void)
{
    AnimateSprites();
    BuildOamBuffer();
    sub_80035AC(&gUnknown_03004828->field_4);
    UpdatePaletteFade();
    RunTasks();
}

extern const u8 gUnknown_083F868C[];

void CreateShedinja(u16 preEvoSpecies, struct Pokemon* mon)
{
    u32 data = 0;
    if (gEvolutionTable[preEvoSpecies].evolutions[0].method == EVO_LEVEL_NINJASK && gPlayerPartyCount < 6)
    {
        s32 i;
        struct Pokemon* Shedinja = &gPlayerParty[gPlayerPartyCount];
        const struct EvolutionData* EvoTable;
        const struct EvolutionData* Evos;

        CopyMon(Shedinja, mon, sizeof(struct Pokemon));
        SetMonData(Shedinja, MON_DATA_SPECIES, (void*)(&gEvolutionTable[preEvoSpecies].evolutions[1].targetSpecies));
        SetMonData(Shedinja, MON_DATA_NICKNAME, (void*)(gSpeciesNames[gEvolutionTable[preEvoSpecies].evolutions[1].targetSpecies]));
        SetMonData(Shedinja, MON_DATA_HELD_ITEM, (void*)(&data));
        SetMonData(Shedinja, MON_DATA_MARKINGS, (void*)(&data));
        SetMonData(Shedinja, MON_DATA_10, (void*)(&data));
        for (i = MON_DATA_COOL_RIBBON; i < MON_DATA_COOL_RIBBON + 5; i++)
            SetMonData(Shedinja, i, (void*)(&data));
        for (i = MON_DATA_CHAMPION_RIBBON; i <= MON_DATA_FATEFUL_ENCOUNTER; i++)
            SetMonData(Shedinja, i, (void*)(&data));
        SetMonData(Shedinja, MON_DATA_STATUS, (void*)(&data));
        data = 0xFF;
        SetMonData(Shedinja, MON_DATA_MAIL, (void*)(&data));

        CalculateMonStats(Shedinja);
        CalculatePlayerPartyCount();

        // can't match it otherwise, ehh
        EvoTable = gEvolutionTable;
        Evos = EvoTable + preEvoSpecies;
        GetNationalPokedexFlag(SpeciesToNationalPokedexNum(Evos->evolutions[1].targetSpecies), 2);
        GetNationalPokedexFlag(SpeciesToNationalPokedexNum(Evos->evolutions[1].targetSpecies), 3);

        if (GetMonData(Shedinja, MON_DATA_SPECIES) == SPECIES_SHEDINJA
            && GetMonData(Shedinja, MON_DATA_LANGUAGE) == LANGUAGE_JAPANESE
            && GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NINJASK)
                SetMonData(Shedinja, MON_DATA_NICKNAME, gUnknown_083F868C);
    }
}

extern const u8 gUnknown_08400C4A[];
extern const u8 gUnknown_08400C60[];
extern const u8 gUnknown_08400C8D[];

void Task_EvolutionScene(u8 taskID)
{
    struct Pokemon* mon = (struct Pokemon*)(gTasks[taskID].tMonPtrHI | (gTasks[taskID].tMonPtrLO << 0x10));

    // check if B Button was held, so the evolution gets stopped
    if (gMain.newAndRepeatedKeys == B_BUTTON && gTasks[taskID].tState != 8 && gTasks[taskID].tBits & TASK_BIT_CAN_STOP)
    {
        gTasks[taskID].tState = 16;
        if (gTasks[sEvoGraphicsTaskID].isActive)
            gTasks[sEvoGraphicsTaskID].EvoGraphicsTaskEvoStop = TRUE;
    }
    switch (gTasks[taskID].tState)
    {
    case 0:
        BeginNormalPaletteFade(-1, 0, 0x10, 0, 0);
        gSprites[sEvoInfo.preEvoSpriteID].invisible = 0;
        gTasks[taskID].tState++;
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            StringExpandPlaceholders(gStringVar4, gUnknown_08400C4A);
            sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
            gTasks[taskID].tState++;
        }
        break;
    case 2:
        if (gUnknown_03004210.state == 0)
        {
            PlayCry1(gTasks[taskID].tPreEvoSpecies, 0);
            gTasks[taskID].tState++;
        }
        break;
    case 3:
        if (IsCryFinished)
        {
            PlaySE(BGM_ME_SHINKA);
            gTasks[taskID].tState++;
        }
        break;
    case 4:
        if (!IsSEPlaying)
        {
            PlayNewMapMusic(BGM_SHINKA);
            gTasks[taskID].tState++;
            BeginNormalPaletteFade(0x1C, 4, 0, 0x10, 0);
        }
        break;
    case 5:
        if (!gPaletteFade.active)
        {
            sEvoGraphicsTaskID = LaunchTask_PreEvoSparklesSet1(17);
            gTasks[taskID].tState++;
        }
        break;
    case 6:
        if (!gTasks[sEvoGraphicsTaskID].isActive)
        {
            gTasks[taskID].tState++;
            sEvoInfo.field_3 = 1;
            sEvoGraphicsTaskID = LaunchTask_PreEvoSparklesSet2();
        }
        break;
    case 7:
        if (!gTasks[sEvoGraphicsTaskID].isActive)
        {
            sEvoGraphicsTaskID = sub_8149E7C(sEvoInfo.preEvoSpriteID, sEvoInfo.postEvoSpriteID);
            gTasks[taskID].tState++;
        }
        break;
    case 8:
        if (--sEvoInfo.field_3 == 0)
        {
            sEvoInfo.field_3 = 3;
            if (!gTasks[sEvoGraphicsTaskID].isActive)
                gTasks[taskID].tState++;
        }
        break;
    case 9:
        sEvoGraphicsTaskID = LaunchTask_PostEvoSparklesSet1();
        gTasks[taskID].tState++;
        break;
    case 10:
        if (!gTasks[sEvoGraphicsTaskID].isActive)
        {
            sEvoGraphicsTaskID = LaunchTask_PostEvoSparklesSet2AndFlash(gTasks[taskID].tPostEvoSpecies);
            gTasks[taskID].tState++;
        }
        break;
    case 11:
        if (!gTasks[sEvoGraphicsTaskID].isActive)
        {
            PlaySE(SE_EXP);
            gTasks[taskID].tState++;
        }
        break;
    case 12:
        if (!IsSEPlaying)
        {
            m4aMPlayAllStop();
            PlayCry1(gTasks[taskID].tPostEvoSpecies, 0);
            memcpy(&gPlttBufferUnfaded[0x20], gUnk_2009000, 0x60);
            BeginNormalPaletteFade(0x1C, 0, 0, 0x10, 0);
            gTasks[taskID].tState++;
        }
        break;
    case 13:
        if (IsCryFinished && !gPaletteFade.active)
        {
            StringExpandPlaceholders(gStringVar4, gUnknown_08400C60);
            sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
            PlayBGM(BGM_FANFA5);
            gTasks[taskID].tState++;
            SetMonData(mon, MON_DATA_SPECIES, (void*)(&gTasks[taskID].tPostEvoSpecies));
            CalculateMonStats(mon);
            EvolutionRenameMon(mon, gTasks[taskID].tPreEvoSpecies, gTasks[taskID].tPostEvoSpecies);
            GetNationalPokedexFlag(SpeciesToNationalPokedexNum(gTasks[taskID].tPostEvoSpecies), 2);
            GetNationalPokedexFlag(SpeciesToNationalPokedexNum(gTasks[taskID].tPostEvoSpecies), 3);
            IncrementGameStat(14);
        }
        break;
    case 14:
        if (gUnknown_03004210.state == 0)
        {
            u16 var = sub_803B7C8(mon, gTasks[taskID].tData6);
            if (var != 0 && !gTasks[taskID].tEvoWasStopped)
            {
                u8 text[11];

                gTasks[taskID].tBits |= TASK_BIT_x80;
                gTasks[taskID].tData6 = 0;
                gTasks[taskID].tData8 = 0;
                GetMonData(mon, MON_DATA_NICKNAME, text);
                StringCopy10(gBattleTextBuff1, text);
                if (var == 0xFFFF)
                    gTasks[taskID].tState = 21;
                else if (var == 0xFFFE)
                    break;
                else
                    gTasks[taskID].tState = 19;
            }
            else
            {
                BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
                gTasks[taskID].tState++;
            }
        }
        break;
    case 15: // task has finished, return
        if (!gPaletteFade.active)
        {
            if (!(gTasks[taskID].tBits & TASK_BIT_x80))
                sub_8053E90();
            if (!gTasks[taskID].tEvoWasStopped)
                CreateShedinja(gTasks[taskID].tPreEvoSpecies, mon);
            DestroyTask(taskID);
            SetMainCallback2(gUnknown_03005E94);
        }
        break;
    case 16: // evolution has been canceled
        if (!gTasks[sEvoGraphicsTaskID].isActive)
        {
            m4aMPlayAllStop();
            BeginNormalPaletteFade(0x6001C, 0, 0x10, 0, 0x7FFF);
            gTasks[taskID].tState++;
        }
        break;
    case 17: // play cry of the pokemon trying to evolve again, evolution has been stopped
        if (!gPaletteFade.active)
        {
            PlayCry1(gTasks[taskID].tPreEvoSpecies, 0);
            gTasks[taskID].tState++;
        }
        break;
    case 18: // after the cry, print the string 'WHOA IT DID NOT EVOLVE!!!'
        if (IsCryFinished)
        {
            StringExpandPlaceholders(gStringVar4, gUnknown_08400C8D);
            sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
            gTasks[taskID].tEvoWasStopped = TRUE;
            gTasks[taskID].tState = 14;
        }
        break;
    case 19: // yay, your poke evolved so it would be nice to print a msg that says it
        if (gUnknown_03004210.state == 0 && !IsSEPlaying())
        {
            sub_8024CEC();
            PlayFanfare(BGM_FANFA1);
            StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[3]);
            sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
            gTasks[taskID].tData6 = 0x40;
            gTasks[taskID].tState++;
        }
        break;
    case 20:
        if (gUnknown_03004210.state == 0 && !IsSEPlaying() && --gTasks[taskID].tData6 == 0)
            gTasks[taskID].tState = 14;
        break;
    case 21: // try to learn a new move
        switch (gTasks[taskID].tData8)
        {
        case 0:
            if (gUnknown_03004210.state == 0 && !IsSEPlaying())
            {
                sub_8024CEC();
                StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[4]);
                sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
                gTasks[taskID].tData8++;
            }
            break;
        case 1:
            if (gUnknown_03004210.state == 0 && !IsSEPlaying())
            {
                StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[5]);
                sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
                gTasks[taskID].tData8++;
            }
            break;
        case 2:
            if (gUnknown_03004210.state != 0)
                break;
            if (!IsSEPlaying())
            {
                StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[6]);
                sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
                gTasks[taskID].tData9 = 5;
                gTasks[taskID].tData10 = 9;
                gTasks[taskID].tData8++;
            }
        case 3:
            if (gUnknown_03004210.state == 0 && !IsSEPlaying())
            {
                sub_8023A80();
                gTasks[taskID].tData8++;
                sEvoCursorPos = 0;
                sub_802BC6C();
            }
            break;
        case 4:
            if (gMain.newKeys & DPAD_UP && sEvoCursorPos != 0)
            {
                PlaySE(SE_SELECT);
                nullsub_6();
                sEvoCursorPos = 0;
                sub_802BC6C();
            }
            if (gMain.newKeys & DPAD_DOWN && sEvoCursorPos == 0)
            {
                PlaySE(SE_SELECT);
                nullsub_6();
                sEvoCursorPos = 1;
                sub_802BC6C();
            }
            if (gMain.newKeys & A_BUTTON)
            {
                StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[292]);
                sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
                PlaySE(SE_SELECT);
                if (sEvoCursorPos != 0)
                    gTasks[taskID].tData8 = gTasks[taskID].tData10;
                else
                {
                    gTasks[taskID].tData8 = gTasks[taskID].tData9;
                    if (gTasks[taskID].tData8 == 5)
                        BeginNormalPaletteFade(-1, 0, 0, 0x10, 0);
                }
            }
            if (gMain.newKeys & B_BUTTON)
            {
                StrCpyDecodeToDisplayedStringBattle(gBattleStringsTable[292]);
                sub_8002EB0(&gUnknown_03004210, gStringVar4, 144, 2, 15);
                PlaySE(SE_SELECT);
                gTasks[taskID].tData8 = gTasks[taskID].tData10;
            }
            break;
        case 5:
            if (!gPaletteFade.active)
            {
                sub_809D9F0(gPlayerParty, gTasks[taskID].tPartyID,
                            gPlayerPartyCount - 1, CB2_EvolutionSceneLoadGraphics,
                            word_2024E82);
                gTasks[taskID].tData8++;
            }
            break;
        case 6:
            if (!gPaletteFade.active && gMain.callback2 == CB2_EvolutionSceneUpdate)
            {
                if (sub_809FA30() == 4)
                    gTasks[taskID].tData8 = 9;
                else
                {

                }
            }
            break;
        }
        break;
    }
}
