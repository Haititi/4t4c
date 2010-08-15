// hyperlinks.c

#include <windows.h>


#define PROP_ORIGINAL_PROC "_Hyperlink_Original_Proc_"
#define PROP_STATIC_HYPERLINK "_Hyperlink_From_Static_"


// standard hand cursor definition
#ifndef IDC_HAND
#define IDC_HAND 147
#endif


// hyperlink font
static HFONT hyperlink_font;


// hyperlinks.c function prototypes
void ConvertStaticToHyperlink (HWND hwndParent, unsigned int wndcontrol_id);
static int WINAPI _HyperlinkParentProc (HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);
static int WINAPI _HyperlinkProc (HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam);


void ConvertStaticToHyperlink (HWND hwndParent, unsigned int wndcontrol_id)
{
   // subclass the parent so we can color the controls as we desire

   HWND hwndCtl;
   WNDPROC pfnOrigProc;
   LOGFONT lf;

   hwndCtl = GetDlgItem (hwndParent, wndcontrol_id);

   if (hwndParent != NULL)
   {
      pfnOrigProc = (WNDPROC) GetWindowLong (hwndParent, GWL_WNDPROC);
      if (pfnOrigProc != _HyperlinkParentProc)
      {
         SetProp (hwndParent, PROP_ORIGINAL_PROC, (HANDLE) pfnOrigProc);
         SetWindowLongPtr (hwndParent, GWL_WNDPROC, (long) (WNDPROC) _HyperlinkParentProc);
      }
   }

   // make sure the control will send notifications
   SetWindowLongPtr (hwndCtl, GWL_STYLE, GetWindowLong (hwndCtl, GWL_STYLE) | SS_NOTIFY);

   // subclass the existing control
   SetProp (hwndCtl, PROP_ORIGINAL_PROC, (HANDLE) GetWindowLong (hwndCtl, GWL_WNDPROC));
   SetWindowLongPtr (hwndCtl, GWL_WNDPROC, (long) (WNDPROC) _HyperlinkProc);

   // create an updated font by adding an underline
   hyperlink_font = (HFONT) SendMessage (hwndCtl, WM_GETFONT, 0, 0);
   GetObject (hyperlink_font, sizeof (lf), &lf);
   lf.lfUnderline = 1;
   hyperlink_font = CreateFontIndirect (&lf);
   SendMessage (hwndCtl, WM_SETFONT, (WPARAM) hyperlink_font, 0);

   // set a flag on the control so we know what color it should be
   SetProp (hwndCtl, PROP_STATIC_HYPERLINK, (HANDLE) 1);
   return;
}


static int WINAPI _HyperlinkParentProc (HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
   HDC hdc;
   HWND hwndCtl;
   WNDPROC pfnOrigProc;
   LRESULT lr;

   pfnOrigProc = (WNDPROC) GetProp (hwnd, PROP_ORIGINAL_PROC);

   if (message == WM_CTLCOLORSTATIC)
   {
      hdc = (HDC) wParam;
      hwndCtl = (HWND) lParam;

      if (GetProp (hwndCtl, PROP_STATIC_HYPERLINK) != NULL)
      {
         lr = CallWindowProc (pfnOrigProc, hwnd, message, wParam, lParam);
         SetTextColor (hdc, RGB (0, 0, 192));
         return (lr);
      }
   }
   else if (message == WM_DESTROY)
   {
      SetWindowLongPtr (hwnd, GWL_WNDPROC, (long) pfnOrigProc);
      RemoveProp (hwnd, PROP_ORIGINAL_PROC);
   }

   return (CallWindowProc (pfnOrigProc, hwnd, message, wParam, lParam));
}


static int WINAPI _HyperlinkProc (HWND hwnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
   HCURSOR hCursor;
   WNDPROC pfnOrigProc;

   pfnOrigProc = (WNDPROC) GetProp (hwnd, PROP_ORIGINAL_PROC);

   if (message == WM_SETCURSOR)
   {
      // since IDC_HAND is not available on all operating systems,
      // we will load the arrow cursor if IDC_HAND is not present.
      hCursor = LoadCursor (NULL, MAKEINTRESOURCE (IDC_HAND));
      if (hCursor == NULL)
         hCursor = LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW));

      SetCursor (hCursor);
      return (TRUE);
   }
   else if (message == WM_DESTROY)
   {
      SetWindowLongPtr (hwnd, GWL_WNDPROC, (long) pfnOrigProc);
      RemoveProp (hwnd, PROP_ORIGINAL_PROC);

      DeleteObject (hyperlink_font);

      RemoveProp (hwnd, PROP_STATIC_HYPERLINK);
   }

   return (CallWindowProc (pfnOrigProc, hwnd, message, wParam, lParam));
}
