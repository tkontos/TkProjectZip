// TkProjectZip.cpp : main source file for TkProjectZip.exe
//

#include "stdafx.h"

#include "resource.h"

#include "aboutdlg.h"
#include "SolutionHelper.h"
#include "ProjZipSettingsDlg.h"
#include "ProgressDlg.h"

CAppModule _Module;

int Usage()
{
   const TCHAR szHelp[] = 
      _T("Usage: TkProjectZip [solutionFileName]\n");

   AtlMessageBox(NULL, szHelp, _T("Command Line Arg Error"));
   return -1;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
   const TCHAR *pszFileName = 0;
   for(int i=1; i<__argc; ++i)
   {
      const TCHAR *p = __targv[i];
      if (!(p[0] == '-' || p[0] == '/'))
      {
         pszFileName = p;
         continue;
      }

      switch(tolower(p[1]))
      {
      case 'i':
         break;

      default:
         return Usage();
      }
   }

   CProjZipSettings prjSettings;
   CString sFileName(pszFileName ? pszFileName : _T(""));
   if (sFileName.GetLength() == 0 || !prjSettings.m_bOnlyShowOnShiftKey || IsShiftKeyPressed())
   {
      CProjZipSettingsDlg dlg(prjSettings, sFileName);
      if (dlg.DoModal() != IDOK)
         return -1;
   }

   CWaitCursor wc;

   CSolutionHelper solHelper(prjSettings);
   if (!solHelper.LoadSolution(sFileName))
   {
      wc.Restore();
      AtlMessageBox(NULL, _T("Unable to load solution"), MB_ICONSTOP);
      return -2;
   }

   bfs::path slnPath((const TCHAR *)sFileName);
   if (prjSettings.m_bAddDateToZipFilename)
   {
      const TCHAR *p = _tcsrchr(sFileName, '.');
      std::tstring sfn = p ? std::tstring(sFileName, p) : std::tstring(sFileName);

      COleDateTime now = COleDateTime::GetCurrentTime();
      TCHAR sz[100];
      _stprintf_s(sz, _T("%s.%04d_%02d%02d.zip"), sfn.c_str(), now.GetYear(), now.GetMonth(), now.GetDay());

      slnPath = sz;
   }
   else
   {
      slnPath.replace_extension(_T("tkpz.zip"));
   }

   wc.Restore();
   CProgressDlg progDlg( solHelper, slnPath.wstring().c_str());
   bool bOk = progDlg.DoModal() == IDOK;
   // bool bOk = solHelper.ZipAllFiles(slnPath.string().c_str());
   if (bOk)
   {
      if (prjSettings.m_bOpenZipDirectory)
      {
         std::wstring sln = slnPath.wstring();
         std::replace(sln.begin(), sln.end(), '/', '\\');
         std::wstring cmdArg = std::wstring(_T("/select,")) + sln;
         ShellExecute(NULL, _T("Open"), _T("Explorer"), cmdArg.c_str(), NULL, SW_SHOWDEFAULT);
      }
      else
      {
         AtlMessageBox(NULL, _T("Solution zipped successfully"), _T("ProjectZip"), MB_OK);
      }
   }
   else
   {
      AtlMessageBox(NULL, _T("Unable to zip solution"), MB_ICONSTOP | MB_OK);
   }
   return 0;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
   HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
// HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
   ATLASSERT(SUCCEEDED(hRes));

   // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
   ::DefWindowProc(NULL, 0, 0, 0L);

   AtlInitCommonControls(ICC_BAR_CLASSES);   // add flags to support other controls

   hRes = _Module.Init(NULL, hInstance);
   ATLASSERT(SUCCEEDED(hRes));

   int nRet = Run(lpstrCmdLine, nCmdShow);

   _Module.Term();
   ::CoUninitialize();

   return nRet;
}
