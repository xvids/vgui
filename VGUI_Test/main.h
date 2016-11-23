#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#include "hook.h"

#include "ValveSDK/sdk.h"

#include "ValveSDK/VGUI/VGUI_Frame.h"
#include "ValveSDK/VGUI/VGUI_Panel.h"

#pragma comment(lib,"VGUI")

/*
gameui.dll

VGuiModuleLoader003
RunGameEngine006
MusicManager001
GameUI007
GameConsole003
*/

#include "GameUI/IVGuiModuleLoader.h"
#include "GameUI/IRunGameEngine.h"
#include "GameUI/IGameUI.h"
#include "GameUI/IGameConsole.h"

/*
vgui2.DLL

VGUI_Panel007
KeyValues003
VGUI_ivgui006
VGUI_System009
VGUI_Surface026
VGUI_Scheme009
VGUI_Localize002
VGUI_Localize003
VGUI_InputInternal001
VGUI_Input004
*/

#include "VGUI2/IHTML.h"
#include "VGUI2/IPanel.h"
#include "VGUI2/ISurface.h"

/*
hw.DLL

EngineSurface007
VENGINE_GAMEUIFUNCS_VERSION001
VENGINE_HLDS_API_VERSION002
VENGINE_LAUNCHER_API_VERSION002
GameServerData001
VGUI_Surface026
VEngineVGui001
BaseUI001
*/

#include "VGUI2/IEngineVGui.h"

void add_log( const char *fmt , ... );

extern char* BaseDir;

extern cl_clientfunc_t *g_pClient;
extern cl_clientfunc_t g_Client;

extern cl_enginefunc_t *g_pEngine;
extern cl_enginefunc_t g_Engine;