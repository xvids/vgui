#include "main.h"

ofstream logfile;
char* BaseDir = nullptr;

cl_clientfunc_t *g_pClient = nullptr;
cl_clientfunc_t g_Client;

cl_enginefunc_t *g_pEngine = nullptr;
cl_enginefunc_t g_Engine;

IVGuiModuleLoader* g_pIVGuiModuleLoader = nullptr;
IRunGameEngine* g_pIRunGameEngine = nullptr;
IGameUI* g_pGameUI = nullptr;
IGameConsole* g_pConsole = nullptr;

vgui::IPanel* g_pIPanel = nullptr;
vgui::ISurface* g_pISurface = nullptr;
vgui::IEngineVGui* g_pIEngineVGui = nullptr;

void add_log( const char *fmt , ... )
{
	if ( !fmt )	return;
	va_list va_alist;
	char logbuf[256] = { 0 };
	va_start( va_alist , fmt );
	vsnprintf( logbuf + lstrlenA( logbuf ) , sizeof( logbuf ) - lstrlenA( logbuf ) , fmt , va_alist );
	va_end( va_alist );
	logfile << logbuf << endl;
}

CreateInterfaceFn CaptureFactory( char* FactoryModule )
{
	CreateInterfaceFn Interface = 0;

	while ( !Interface )
	{
		HMODULE hFactoryModule = GetModuleHandleA( FactoryModule );

		if ( hFactoryModule )
		{
			Interface = (CreateInterfaceFn)( GetProcAddress( hFactoryModule , CREATEINTERFACE_PROCNAME ) );
		}

		Sleep( 100 );
	}

	return Interface;
}

PVOID CaptureInterface( CreateInterfaceFn Interface , char* InterfaceName )
{
	PVOID dwPointer = nullptr;

	while ( !dwPointer )
	{
		dwPointer = (PVOID)( Interface( InterfaceName , 0 ) );

		Sleep( 100 );
	}

	return dwPointer;
}

vgui::IPanel* RootPanel;
vgui::IPanel* ClientPanel;
vgui::IPanel* GameUIPanel;

VHookTable PanelHook;

void WINAPI PaintTraversePanel( vgui::IPanel* vguiPanel , bool forceRepaint , bool allowForce )
{
	PanelHook.UnHook();
	g_pIPanel->PaintTraverse( vguiPanel , forceRepaint , allowForce );

	const char* PanelClassName = g_pIPanel->GetClassName( vguiPanel );

	g_pISurface->DrawSetColor( 0 , 255 , 0 , 255 );
	g_pISurface->DrawLine( 10 , 10 , 60 , 10 );
	g_pISurface->DrawFilledRect( 10 , 20 , 60 , 70 );
	g_pISurface->DrawOutlinedRect( 10 , 80 , 60 , 130 );

	add_log( "PanelClassName %s" , PanelClassName );
	
	PanelHook.ReHook();
}

DWORD WINAPI CheatEntry( LPVOID lpThreadParameter )
{	
	CreateInterfaceFn GameUI_Factory = CaptureFactory( "gameui.dll" );
	CreateInterfaceFn VGUI_Factory = CaptureFactory( "vgui2.dll" );
	CreateInterfaceFn HW_Factory = CaptureFactory( "hw.dll" );

	if ( GameUI_Factory && VGUI_Factory && HW_Factory )
	{
		add_log( "GameUI_Factory: [%X]" , GameUI_Factory );

		g_pIVGuiModuleLoader = (IVGuiModuleLoader*)( CaptureInterface( GameUI_Factory , VGUIMODULELOADER_INTERFACE_VERSION ) );
		g_pIRunGameEngine = (IRunGameEngine*)( CaptureInterface( GameUI_Factory , RUNGAMEENGINE_INTERFACE_VERSION ) );
		g_pGameUI = (IGameUI*)( CaptureInterface( GameUI_Factory , GAMEUI_INTERFACE_VERSION ) );
		g_pConsole = (IGameConsole*)( CaptureInterface( GameUI_Factory , GAMECONSOLE_INTERFACE_VERSION ) );

		g_pIPanel = ( vgui::IPanel* )( CaptureInterface( VGUI_Factory , VGUI_PANEL_INTERFACE_VERSION ) );

		g_pISurface = ( vgui::ISurface* )( CaptureInterface( HW_Factory , VGUI_SURFACE_INTERFACE_VERSION ) );
		g_pIEngineVGui = ( vgui::IEngineVGui* )( CaptureInterface( HW_Factory , VENGINE_VGUI_VERSION ) );
	
		if ( !g_pConsole->IsConsoleVisible() )
			g_pConsole->Activate();

		g_pConsole->Clear();

		g_pConsole->DPrintf( "g_pIVGuiModuleLoader: %X\n" , g_pIVGuiModuleLoader );
		g_pConsole->DPrintf( "g_pIRunGameEngine: %X\n" , g_pIRunGameEngine );
		g_pConsole->DPrintf( "g_pGameUI: %X\n" , g_pGameUI );
		g_pConsole->DPrintf( "g_pConsole: %X\n", g_pConsole );
		g_pConsole->DPrintf( "g_pIPanel: %X\n" , g_pIPanel );
		g_pConsole->DPrintf( "g_pISurface: %X\n" , g_pISurface );
		g_pConsole->DPrintf( "g_pIEngineVGui: %X\n" , g_pIEngineVGui );

		RootPanel = g_pIEngineVGui->GetPanel( vgui::PANEL_ROOT );
		ClientPanel = g_pIEngineVGui->GetPanel( vgui::PANEL_CLIENTDLL );
		GameUIPanel = g_pIEngineVGui->GetPanel( vgui::PANEL_GAMEUIDLL );

		int wide , tall;

		g_pIPanel->GetSize( RootPanel , wide , tall );
		g_pConsole->DPrintf( "RootPanel -> wide: %i | tall: %i\n" , wide , tall );
		g_pIPanel->GetSize( ClientPanel , wide , tall );
		g_pConsole->DPrintf( "ClientPanel -> wide: %i | tall: %i\n" , wide , tall );
		g_pIPanel->GetSize( GameUIPanel , wide , tall );
		g_pConsole->DPrintf( "GameUIPanel -> wide: %i | tall: %i\n" , wide , tall );

		if( PanelHook.HookTable( (DWORD)g_pIPanel ) )
		{
			PanelHook.HookIndex( 41 , PaintTraversePanel );
		}
	}

	return 0;
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL , DWORD fdwReason , LPVOID lpReserved )
{
	char LogFile[MAX_PATH] = { 0 };

	if ( fdwReason == DLL_PROCESS_ATTACH )
	{
		DisableThreadLibraryCalls( hinstDLL );

		BaseDir = new char[MAX_PATH];

		GetModuleFileNameA( hinstDLL , BaseDir , MAX_PATH );

		char* pos = BaseDir + strlen( BaseDir );
		while ( pos >= BaseDir && *pos != '\\' ) --pos; pos[1] = 0;

		lstrcpyA( LogFile , BaseDir );
		lstrcatA( LogFile , "\\debug.txt" );

		remove( LogFile );
		logfile.open( LogFile , ios::app );

		CreateThread( 0 , 0 , CheatEntry , 0 , 0 , 0 );
	}

	return TRUE;
}