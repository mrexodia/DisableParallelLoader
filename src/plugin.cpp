#include <phnt_windows.h>
#include <phnt.h>

#include "plugin.h"

static duint pluginEnabled = 0;

PLUG_EXPORT void CBCREATEPROCESS(CBTYPE, PLUG_CB_CREATEPROCESS* info)
{
	if (pluginEnabled)
	{
		// Reference: https://blogs.blackberry.com/en/2017/10/windows-10-parallel-loading-breakdown
		auto peb = DbgGetPebAddress(DbgGetProcessId());
		if (peb)
		{
			auto processParameters = Script::Memory::ReadPtr(peb + offsetof(_PEB, ProcessParameters));
			if (processParameters)
			{
				auto loaderThreadsPtr = processParameters + offsetof(_RTL_USER_PROCESS_PARAMETERS, LoaderThreads);
				dprintf("PEB->ProcessParameters->LoaderThreads: %u -> 1\n", Script::Memory::ReadDword(loaderThreadsPtr));
				Script::Memory::WriteDword(loaderThreadsPtr, 1);
			}
		}
	}
}

enum
{
	MenuDisableParallelLoader,
};

PLUG_EXPORT void CBMENUENTRY(CBTYPE, PLUG_CB_MENUENTRY* info)
{
	switch (info->hEntry)
	{
	case MenuDisableParallelLoader:
	{
		pluginEnabled = pluginEnabled == 0 ? 1 : 0;
		BridgeSettingSetUint("DisableParallelLoader", "Enabled", pluginEnabled);
		_plugin_menuentrysetchecked(pluginHandle, MenuDisableParallelLoader, pluginEnabled);
	}
	break;
	}
}

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
	return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
}

//Do GUI/Menu related things here.
void pluginSetup()
{
	_plugin_menuaddentry(hMenu, MenuDisableParallelLoader, "Set loader threads to 1");
	BridgeSettingGetUint("DisableParallelLoader", "Enabled", &pluginEnabled);
	if (pluginEnabled != 0)
		pluginEnabled = 1;
	_plugin_menuentrysetchecked(pluginHandle, MenuDisableParallelLoader, pluginEnabled);
}
