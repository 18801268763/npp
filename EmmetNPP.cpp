
/*
 ZenCoding Python Plugin
 This code needs tidying up - it should be split out into classes
 - there's waaaay too many global variables (i.e. more than none :)
*/

#include "stdafx.h"
#include "AboutDlg.h"
#include "PluginInterface.h"
#include "FuncItemManager.h"

#define CHECK_INITIALISED()  if (!g_initialised){ initialise(); }

struct SCNotification
{
	NMHDR nmhdr;
};


/* Info for Notepad++ */
CONST TCHAR PLUGIN_NAME[]	= _T("Emmet");

FuncItem	*funcItem = NULL;

/* Global data */
NppData				nppData;
HANDLE				g_hModule			= NULL;
bool                g_pythonFailure     = false;
bool                g_initialised       = false;
bool				g_expandIsTab		= false;
bool				g_autoSelectProfile = true;
bool				g_watchSave			= false;
TCHAR				g_settingsFile[MAX_PATH];
TCHAR				g_iniPath[MAX_PATH];

int g_fiAutoProfile  = -1;
int g_fiProfileXhtml = -1;
int g_fiProfileHtml  = -1;
int g_fiProfileXml   = -1;
int g_fiProfilePlain = -1;
int g_currentProfileIndex = -1;
tstring g_currentProfile;

/* Dialogs */
AboutDialog		aboutDlg;


FuncItemManager *g_funcItemManager = NULL;


// Runs a string in python
void runScript(const TCHAR *script);
void runString(const TCHAR *str, int messageType = PYSCR_EXECSTATEMENT);

// Checks if the key is tab on it's own
bool keyIsTab(const ShortcutKey& key);

// Sets the profile (ticks menu item, and informs Python)
void setProfile(const TCHAR *profileName, int cmdIndex);


// Settings
void loadSettings();
void saveSettings();

void initialise()
{
	TCHAR configPath[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(configPath));
	
	//tstring my_zen_settings(configPath);
	//my_zen_settings.append(_T("\\ZenCodingPython\\zencoding\\my_zen_settings.py"));

	//	
	//if (!::PathFileExists(my_zen_settings.c_str()))
	//{
	//	std::ofstream mySettingsFile(my_zen_settings.c_str());

	//	mySettingsFile << "my_zen_settings = {\n"
	//					  "     'html': {\n"
	//					  "      	'abbreviations': {\n"
	//					  "			'jq': '<script type=\"text/javascript\" src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.4.4/jquery.min.js\"></script>',\n"
	//					  "			'demo': '<div id   =\"demo\"></div>'\n"
	//					  "		}\n"
	//					  "   }\n"
	//					  "}\n";
	//	mySettingsFile.close();

	//}

	tstring emmet_module(_T("from Npp import notepad, console\n"));
	emmet_module.append(_T("import os.path\n"));
	emmet_module.append(_T("sys.path.insert(0, os.path.join(notepad.getPluginConfigDir(), 'EmmetNPP'))\n"));
	emmet_module.append(_T("sys.path.insert(0, os.path.join(notepad.getNppDir(), 'plugins', 'EmmetNPP'))\n"));
	emmet_module.append(_T("console.write('Starting Emmet\\n')\n"));
	emmet_module.append(_T("console.write('%s\\n' % sys.path)\n"));
	emmet_module.append(_T("import npp_emmet\n"));

	/*tstring emmet_module(_T("sys.path.append(r'"));
	emmet_module.append(configPath);
	emmet_module.append(_T("\\EmmetNPP')\nimport npp_emmet\n"));*/

	runString(emmet_module.c_str());

	g_initialised = true;

	// Set the current profile if it's not xhtml (the default)
	/*if (g_fiProfileHtml == g_currentProfileIndex)
	{
		setProfile(_T("html"), g_fiProfileHtml);
	}
	else if (g_fiProfileXml == g_currentProfileIndex)
	{
		setProfile(_T("xml"), g_fiProfileXml);
	}
	else if (g_fiProfilePlain == g_currentProfileIndex)
	{
		setProfile(_T("plain"), g_fiProfilePlain);
	}*/

	

}

void doExpandAbbreviation()
{
	CHECK_INITIALISED();
	if (g_expandIsTab)
	{
		runString(_T("npp_emmet.expand_abbreviation(True)"));
	}
	else
	{
		runString(_T("npp_emmet.expand_abbreviation(False)"));
	}
}

	

void doWrapWithAbbreviation()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('wrap_with_abbreviation')"));
}


void doNextEditPoint()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('next_edit_point')"));
}

void doPreviousEditPoint()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('prev_edit_point')"));
}

void doSelectNextItem()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('select_next_item')"));
}

void doSelectPreviousItem()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('select_previous_item')"));
}

void doMatchPairInward()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('match_pair_inward')"));
}

void doMatchPairOutward()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('match_pair_outward')"));
}

void doGoToMatchingPair()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('go_to_matching_pair')"));
}

void doMergeLines()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('merge_lines')"));
}

void doToggleComment()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('toggle_comment')"));
}

void doSplitJoinTag()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('split_join_tag')"));
}

void doRemoveTag()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('remove_tag')"));
}


void doUpdateImageSize()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('update_image_size')"));
}

//void doAddEntryAbbreviation()
//{
//	CHECK_INITIALISED();
//	runString(_T("npp_zen_coding.add_entry('abbreviations')"));
//}
//
//void doAddEntrySnippet()
//{
//	CHECK_INITIALISED();
//	runString(_T("npp_zen_coding.add_entry('snippets')"));
//}

void doEvaluateMathExpression()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('evaluate_math_expression')"));
}

void doReflectCssValue()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('reflect_css_value')"));
}

void doInsertLinebreak()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('insert_formatted_line_break_only')"));
}

void doDataURL()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.run_action('encode_decode_data_url')"));
}

void doAutocomplete()
{
	CHECK_INITIALISED();
	runString(_T("npp_emmet.show_autocomplete()"));
}

//void doAddAbbreviation()
//{
//	CHECK_INITIALISED();
//	runString(_T("npp_zen_coding.add_entry('abbreviations')"));
//}
//
//void doAddSnippet()
//{
//	CHECK_INITIALISED();
//	runString(_T("npp_zen_coding.add_entry('snippets')"));
//}

void doAbout()
{
	aboutDlg.doDialog();
}



void setProfile(const TCHAR *profileName)
{
	int profileIndex = -1;
	if (_tcscmp(profileName, _T("xhtml")))
	{
		profileIndex = g_fiProfileXhtml;
	}
	else if (_tcscmp(profileName, _T("xml")))
	{
		profileIndex = g_fiProfileXml;
	}
	else if (_tcscmp(profileName, _T("html")))
	{
		profileIndex = g_fiProfileHtml;
	}
	else if (_tcscmp(profileName, _T("plain")))
	{
		profileIndex = g_fiProfilePlain;
	}

	if (-1 != profileIndex)
	{
		setProfile(profileName, profileIndex);
	}
}

void setProfile(const TCHAR *profileName, int cmdIndex)
{
	
	if (-1 != g_currentProfileIndex)
	{
		::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[g_currentProfileIndex]._cmdID, FALSE);
	}
	
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[cmdIndex]._cmdID, TRUE);
	
	/*if (g_initialised)
	{
		TCHAR cmd[150];
		_tcscpy_s(cmd, 150, _T("npp_zen_coding.set_profile('"));
		_tcscat_s(cmd, 150, profileName);
		_tcscat_s(cmd, 150, _T("')"));

		runString(cmd);
	}*/
	g_currentProfile = profileName;
	g_currentProfileIndex = cmdIndex;
}

void doProfileAutoSelect()
{
	g_autoSelectProfile = !g_autoSelectProfile;
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[g_fiAutoProfile]._cmdID, g_autoSelectProfile ? TRUE : FALSE);
	saveSettings();
}

void doProfileXhtml()
{
	setProfile(_T("xhtml"), g_fiProfileXhtml);
}

void doProfileHtml()
{
	setProfile(_T("html"), g_fiProfileHtml);
}

void doProfileXml()
{
	setProfile(_T("xml"), g_fiProfileXml);
}

void doProfilePlain()
{
	setProfile(_T("plain"), g_fiProfilePlain);
}




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	g_hModule = hModule;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
	aboutDlg.init(static_cast<HINSTANCE>(g_hModule), nppData);
	
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(g_iniPath));
	_tcscat_s(g_iniPath, MAX_PATH, _T("\\EmmetNPP.ini"));
}


extern "C" __declspec(dllexport) CONST TCHAR * getName()
{
	return PLUGIN_NAME;
}


extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	



	if (g_funcItemManager != NULL)
	{
		delete g_funcItemManager;
	}
	g_funcItemManager = new FuncItemManager();


	g_funcItemManager->addFunction(_T("Expand abbreviation"), doExpandAbbreviation, VK_RETURN, MODIFIER_CTRL | MODIFIER_ALT, false);
	g_funcItemManager->addFunction(_T("Wrap with abbreviation"), doWrapWithAbbreviation, VK_RETURN, MODIFIER_CTRL | MODIFIER_ALT | MODIFIER_SHIFT, false);
	g_funcItemManager->addSplitter(); // ----------------------
	g_funcItemManager->addFunction(_T("Next edit point"), doNextEditPoint, VK_RIGHT, MODIFIER_CTRL | MODIFIER_ALT, false);
	g_funcItemManager->addFunction(_T("Previous edit point"), doPreviousEditPoint, VK_LEFT, MODIFIER_CTRL | MODIFIER_ALT, false);
	g_funcItemManager->addFunction(_T("Select next item"), doSelectNextItem);
	g_funcItemManager->addFunction(_T("Select previous item"), doSelectPreviousItem);
	g_funcItemManager->addSplitter(); // ----------------------
	g_funcItemManager->addFunction(_T("Match pair inward"), doMatchPairInward);
	g_funcItemManager->addFunction(_T("Match pair outward"), doMatchPairOutward);
	g_funcItemManager->addFunction(_T("Go to matching pair"), doGoToMatchingPair);
	g_funcItemManager->addSplitter(); // ----------------------
	g_funcItemManager->addFunction(_T("Split / join tag"), doSplitJoinTag);
	g_funcItemManager->addFunction(_T("Remove tag"), doRemoveTag);
	g_funcItemManager->addSplitter(); // ----------------------
	g_funcItemManager->addFunction(_T("Insert formatted linebreak"), doInsertLinebreak);
	g_funcItemManager->addFunction(_T("Toggle comment"), doToggleComment);
	g_funcItemManager->addFunction(_T("Update image size"), doUpdateImageSize);
	g_funcItemManager->addFunction(_T("Encode/decode image to data:URL"), doDataURL);
	g_funcItemManager->addFunction(_T("Reflect CSS value"), doReflectCssValue);
	g_funcItemManager->addFunction(_T("Evalute math expression"), doEvaluateMathExpression);
	//g_funcItemManager->addSplitter(); // ----------------------
	//g_funcItemManager->addFunction(_T("Add abbreviation"), doAddAbbreviation);
	//g_funcItemManager->addFunction(_T("Add snippet"), doAddSnippet);
	//g_funcItemManager->addSplitter(); // ----------------------
	//g_funcItemManager->addFunction(_T("Autocomplete"), doAutocomplete);
	g_funcItemManager->addSplitter(); // ----------------------
	/*g_fiAutoProfile  = g_funcItemManager->addFunction(_T("Auto select profile"), doProfileAutoSelect, NULL, true);
	g_fiProfileXhtml = g_funcItemManager->addFunction(_T("Profile: xhtml"), doProfileXhtml, NULL, true);
	g_fiProfileHtml  = g_funcItemManager->addFunction(_T("Profile: html"), doProfileHtml);
	g_fiProfileXml   = g_funcItemManager->addFunction(_T("Profile: xml"), doProfileXml);
	g_fiProfilePlain = g_funcItemManager->addFunction(_T("Profile: plain"), doProfilePlain);
	g_funcItemManager->addSplitter();*/ // ----------------------
	g_funcItemManager->addFunction(_T("About"), doAbout);
	
	funcItem = g_funcItemManager->getFuncItems(nbF);
	return funcItem;
}




extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{

	switch(notifyCode->nmhdr.code)
	{
		case NPPN_READY:
			{
				ShortcutKey key;
				::SendMessage(nppData._nppHandle, NPPM_GETSHORTCUTBYCMDID, funcItem[0]._cmdID, reinterpret_cast<LPARAM>(&key));
				g_expandIsTab = keyIsTab(key);	
				loadSettings();
				// Lazy initialisation, so don't call initialise() yet
				//initialise();
			}
			break;

		case NPPN_SHORTCUTREMAPPED:
			if (funcItem[0]._cmdID == notifyCode->nmhdr.idFrom)
			{
				g_expandIsTab = keyIsTab(*reinterpret_cast<ShortcutKey*>(notifyCode->nmhdr.hwndFrom));
			}
			break;

		case NPPN_BUFFERACTIVATED:
		case NPPN_LANGCHANGED:
			if (g_autoSelectProfile)
			{
				int lang = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERLANGTYPE, notifyCode->nmhdr.idFrom, 0);
				switch(lang)
				{
					case L_XML:
						setProfile(_T("xml"), g_fiProfileXml);
						break;

					case L_TXT:
						setProfile(_T("plain"), g_fiProfilePlain);
						break;

					case L_HTML:
					default:
						setProfile(_T("xhtml"), g_fiProfileXhtml);
						break;
				}
			}
			break;
		case NPPN_FILESAVED:
			if (g_watchSave)
			{
				TCHAR filename[MAX_PATH];
				::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, notifyCode->nmhdr.idFrom, reinterpret_cast<LPARAM>(filename));
				if (0 == _tcsicmp(filename, g_settingsFile))
				{
					if (g_initialised)
					{
						runString(_T("npp_emmet.reload()"));
						::MessageBox(nppData._nppHandle, _T("Emmet settings automatically refreshed"), _T("Emmet for Notepad++"), MB_ICONINFORMATION);
					}
					else
					{
						::MessageBox(nppData._nppHandle, _T("New Emmet settings have been applied"), _T("Emmet for Notepad++"), MB_ICONINFORMATION);
					}
				}
			}
			break;
	}

	
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT message, WPARAM wParam , LPARAM lParam)
{
	return TRUE;
}


#ifdef _UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
#endif


void runScript(TCHAR *str)
{
	runString(str, PYSCR_EXECSCRIPT);
}




void runString(const TCHAR *script, int messageType /* = PYSCR_EXECSTATEMENT */)
{
	if (g_pythonFailure)
		return;

	PythonScript_Exec pse;
	pse.structVersion = 1;

	pse.completedEvent = NULL;
	pse.deliverySuccess = FALSE;
	pse.flags = 0;
	pse.script = script;
	
	

	TCHAR pluginName[] = _T("PythonScript.dll");
	CommunicationInfo commInfo;
	commInfo.internalMsg = messageType;
	commInfo.srcModuleName = PLUGIN_NAME;
	
	commInfo.info = reinterpret_cast<void*>(&pse);
	// SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, reinterpret_cast<WPARAM>(pluginName), reinterpret_cast<LPARAM>(&commInfo));

	BOOL delivery = SendMessage(nppData._nppHandle, NPPM_MSGTOPLUGIN, reinterpret_cast<WPARAM>(pluginName), reinterpret_cast<LPARAM>(&commInfo));
	if (!delivery)
	{
		MessageBox(NULL, _T("Python Script Plugin not found.  Please install the Python Script plugin from Plugin Manager"), _T("Emmet"), 0);
		g_pythonFailure = true;
	}	
	else if (!pse.deliverySuccess)
	{
		MessageBox(NULL, _T("Python Script Plugin did not accept the script"), _T("Emmet"), 0);
		g_pythonFailure = true;
	}
}

bool keyIsTab(const ShortcutKey& key)
{
	if (key._key == VK_TAB && (key._isAlt == false && key._isCtrl == false && key._isShift == false))
	{
		return true;
	}
	else
	{
		return false;
	}

}


void saveSettings()
{
	::WritePrivateProfileString(_T("Settings"), _T("AutoProfile"), g_autoSelectProfile ? _T("1") : _T("0"), g_iniPath);
	::WritePrivateProfileString(_T("Settings"), _T("CurrentProfile"), g_currentProfile.c_str(), g_iniPath);
}

void loadSettings()
{
	int result = ::GetPrivateProfileInt(_T("Settings"), _T("AutoProfile"), 1, g_iniPath);
	g_autoSelectProfile = (result == 1);
	if (!g_autoSelectProfile)
	{
		TCHAR tmp[20];
		::GetPrivateProfileString(_T("Settings"), _T("CurrentProfile"), _T("xhtml"), tmp, 20, g_iniPath);
		g_currentProfile = tmp;
		setProfile(g_currentProfile.c_str());
	}
	::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[g_fiAutoProfile]._cmdID, g_autoSelectProfile ? TRUE : FALSE);

}
