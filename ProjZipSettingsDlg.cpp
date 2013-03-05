// VersionInfoDlg.cpp : implementation of the CVersionInfoDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProjZipSettingsDlg.h"

const TCHAR CProjZipSettings::m_szRegKey[] = _T("Software\\4N Systems\\ProjectZip");
const TCHAR szRecentFiles[] = _T("RecentFiles");

LRESULT CProjZipSettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CenterWindow();
   m_spinHelper.InitWindow(*this);
   m_spinHelper.InitCtrlID(IDC_DAYS,1,90,1);

   m_cbFile.SubclassWindow(GetDlgItem(IDC_COMBO1));
   m_cbFile.ReadFromRegistry(CProjZipSettings::m_szRegKey, szRecentFiles);
   for(int i=m_cbFile.GetCount(); --i >= 0; )
   {
      CString str;
      m_cbFile.GetLBText(i, str);
      if (!FileExists(str))
         m_cbFile.RemoveFromList(str);
   }

   if (!m_slnFileName.IsEmpty())
      m_cbFile.AddToList(m_slnFileName);
   DoDataExchange( DDX_LOAD);
   RadioClicked();

   CModuleVersion ver;
   if (ver.GetFileVersionInfo(NULL))
   {
      TCHAR sz[100];
      _stprintf_s(sz, _T("TkProjectZip Version %d.%02d.%04d.%04d"),
         HIWORD(ver.dwFileVersionMS), LOWORD(ver.dwFileVersionMS),
         HIWORD(ver.dwFileVersionLS), LOWORD(ver.dwFileVersionLS));
      SetWindowText(sz);
   }

   return TRUE;
}

LRESULT CProjZipSettingsDlg::OnFileBrowse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   CFileDialog dlg( TRUE, NULL, NULL,
      OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
      _T("Visual studion solution files (*.sln)\0*.sln\0All Files (*.*)\0*.*\0"));

   if (m_pzs.m_sInitialDirectory.GetLength() > 0)
      dlg.m_ofn.lpstrInitialDir = m_pzs.m_sInitialDirectory;

   if (dlg.DoModal() != IDOK)
      return 0;

   bfs::wpath p(dlg.m_szFileName);
   m_pzs.m_sInitialDirectory = p.parent_path().string().c_str();

   m_cbFile.AddToList(dlg.m_szFileName);
   m_cbFile.SetCurSel(0);

   return 0;
}

LRESULT CProjZipSettingsDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   if (wID == IDOK)
   {
      DoDataExchange(DDX_SAVE);
      m_cbFile.WriteToRegistry(CProjZipSettings::m_szRegKey, szRecentFiles);
      m_pzs.Save();
   }
   EndDialog(wID);
   return 0;
}

void CProjZipSettingsDlg::RadioClicked()
{
   DoDataExchange(DDX_SAVE, IDC_RADIO1);
   GetDlgItem(IDC_DAYS).EnableWindow( m_pzs.m_bRecentFiles);
   GetDlgItem(IDC_CHECK3).EnableWindow( !m_pzs.m_bRecentFiles);
   if (m_pzs.m_bRecentFiles)
      SendDlgItemMessage(IDC_CHECK3, BM_SETCHECK, 0);
}

