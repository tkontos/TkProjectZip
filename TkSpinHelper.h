#pragma once
#include "atlwin.h"

class CTkSpinHelper :
   public CMessageMap
{
public:
   CTkSpinHelper(HWND hWndParent = 0)
      : m_hWndParent( hWndParent)
   {
   }
   ~CTkSpinHelper(void);

   void  InitWindow( HWND hWndParent)
   {
      m_hWndParent = hWndParent;
   }

   void  InitCtrlID( int idc);
   void  InitCtrlID( int idc, int iMin, int iMax, int iIncr = 1)
   {
      return InitCtrlID(idc, double(iMin), double(iMax), double(iIncr), 0);
   }
   void  InitCtrlID( int idc, double dMin, double dMax, double dIncr, int iDecPlaces);
   void  ShowSpinner( int idc, bool bShow);
   void  EnableSpinner( int idc, bool bEnable);

   BEGIN_MSG_MAP(CTkSpinHelper)
      NOTIFY_CODE_HANDLER( UDN_DELTAPOS, OnDeltaposSpin)
   END_MSG_MAP()

   LRESULT  OnDeltaposSpin ( int wID, NMHDR* pNMHDR, BOOL &bHandled);

protected:
   virtual void   OnSpinHandler( int iCtrlId, double dVal, int iDelta);

private:
   struct CtrlData
   {
      bool     bUserHandled;
      double   dMin, dMax;
      double   dIncr;
      int      iDecPlaces;
      HWND     hWndEdit, hWndSpin;
   };
   typedef std::map<int, CtrlData>  CtrlDataMap;

   HWND        m_hWndParent;
   CtrlDataMap m_map;

   void     InitCtrlImpl   ( int idc, CtrlData &cd);
   int      GetDataCtrlID  ( int iSpinId);
   void     SetCtrlData    ( const CtrlData &cd, int iCtrlId, double dVal, int iDelta);
};

template <class T>
class CTkSpinHelperT : public CTkSpinHelper
{
public:
   typedef void (T::*FUNC)(int iCtrlId, double dVal, int iDelta);

   CTkSpinHelperT( T *pt, FUNC pf, HWND hWndParent = NULL)
      : CTkSpinHelper(hWndParent)
   {
      m_pt = pt;
      m_pf = pf;
   }

protected:
   virtual void   OnSpinHandler( int iCtrlId, double dVal, int iDelta)
   {
      (m_pt->*m_pf)(iCtrlId, dVal, iDelta);
   }

private:
   T *m_pt;
   FUNC m_pf;
};

