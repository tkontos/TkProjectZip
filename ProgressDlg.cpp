// ProgressDlg.cpp : implementation of the CProgressDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "ProgressDlg.h"

LRESULT CProgressDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   CenterWindow(GetParent());
   m_hWndProgCtrl = GetDlgItem(IDC_PROGRESS1);
   ::SendMessage(m_hWndProgCtrl, PBM_SETRANGE32, (WPARAM)0, (LPARAM)m_solHelper.GetFileList().size());

   m_taskGroup.run([this] { 
      bool bOk = this->m_solHelper.ZipAllFiles(this->m_sFileName, this);
      this->PostMessage( WM_COMMAND, bOk ? IDOK : IDCANCEL); 
   });

   return TRUE;
}

LRESULT CProgressDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   m_taskGroup.cancel();
   m_taskGroup.wait();
   EndDialog(wID);
   return 0;
}
