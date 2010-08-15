// ui.c

#include "rsrc-client.h"
#include "client.h"


// locally used definitions
#define TIMER_REFRESHMOTD 1


// text internationalizations
typedef struct text_s
{
   char mainwindow_title[64];
   char login[32];
   char password[32];
   char button_play[32];
   char button_setup[32];
   char setupwindow_title[64];
   char keyboard_shortcuts[64];
   char shortcut_inventory[64];
   char shortcut_charactersheet[64];
   char shortcut_chatterchannels[64];
   char shortcut_groupsheet[64];
   char shortcut_spellbook[64];
   char shortcut_macros[64];
   char shortcut_exchange[64];
   char shortcut_options[64];
   char shortcut_screenshot[64];
   char shortcut_bigchatwindow[64];
   char shortcut_areamap[64];
   char shortcut_attackmode[64];
   char saved_accounts[64];
   char button_add[32];
   char button_delete[32];
   char display_options[64];
   char display_hinttext[256];
   char screen_resolution[32];
   char resolution_large[32];
   char resolution_medium[32];
   char resolution_small[32];
   char fullscreen[32];
   char keep_proportions[32];
   char wait_vsync[64];
   char cc_colors[64];
   char button_pickupcolor[32];
   char colors_total[64];
   char button_ok[32];
   char button_cancel[32];
} text_t;


// global variables used in this module
static unsigned long current_colorcode;
static int color_index;
text_t texts;


// functions used in this module
static void PopulateTexts_MainDlg (HWND hWnd);
static void PopulateTexts_ConfigDlg (HWND hWnd);
static void CenterWindow (HWND hWnd);
static account_t *Account_FindByUsername (const char *username);
static void PopulateAccountsComboBox (HWND hWnd, int loginctl_id, int passwdctl_id, const char *wanted_login);
static void Language_Load (void);


int WINAPI MainDialogProc (HWND hWnd, unsigned int message, unsigned int wParam, long lParam)
{
   // mesage handler for launcher's main window

   unsigned short wParam_hiword;
   unsigned short wParam_loword;
   account_t *account;

   // filter out the commonly used message values
   wParam_hiword = HIWORD (wParam);
   wParam_loword = LOWORD (wParam);

   // have we just fired up this window ?
   if (message == WM_INITDIALOG)
   {
      // ask for the message of the day
      _beginthread (Loop_MOTD, 0, NULL);

      // refresh the MOTD area every 250 milliseconds
      SetTimer (hWnd, TIMER_REFRESHMOTD, 250, NULL);

      // load the language file and populate the window texts
      Language_Load ();
      PopulateTexts_MainDlg (hWnd);

      // center the window and set the window title
      CenterWindow (hWnd);

      // set dialog icons (small one for title bar & big one for task manager)
      SendMessage (hWnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon (hAppInstance, (char *) ICON_MAIN));
      SendMessage (hWnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon (hAppInstance, (char *) ICON_MAIN));

      // populate and tidy login/password fields
      PopulateAccountsComboBox (hWnd, COMBOBOX_LOGIN, TEXTINPUT_PASSWORD, last_login);

      // convert the server address static control to a hyperlink
      ConvertStaticToHyperlink (hWnd, PICTURE_MAIN);

      // convert the server text control to a hyperlink and set its text
      SetDlgItemText (hWnd, TEXT_URL, config.server_name);
      ConvertStaticToHyperlink (hWnd, TEXT_URL);

      return (true);
   }

   // else is the window shutting down ?
   else if (message == WM_DESTROY)
   {
      KillTimer (hWnd, TIMER_REFRESHMOTD); // destroy the timer we used to refresh the MOTD
      return (true);
   }

   // else did we click the close button on the title bar ?
   else if (message == WM_CLOSE)
   {
      EndDialog (hWnd, false); // close the proxy window
      return (true);
   }

   // else did we alter a control INSIDE the window ?
   else if (message == WM_COMMAND)
   {
      // was it the server URL ?
      if ((wParam_loword == PICTURE_MAIN) || (wParam_loword == TEXT_URL))
      {
         ShellExecute (NULL, "open", config.server_url, NULL, NULL, SW_MAXIMIZE);
         return (true);
      }

      // else is it the login combo box ?
      else if (wParam_loword == COMBOBOX_LOGIN)
      {
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, sizeof (last_login)); // get combo box text
         account = Account_FindByUsername (last_login); // find appropriate login
         if (account != NULL)
            SetDlgItemText (hWnd, TEXTINPUT_PASSWORD, account->password); // if found, set the appropriate password
      }

      // else was it the CONFIG button ?
      if (wParam_loword == BUTTON_CONFIG)
      {
         // fire up the config dialog box and let it run until we press either button
         DialogBox (hAppInstance, (char *) DIALOG_CONFIG, hWnd, ConfigDialogProc);

         // populate the window texts again (in case user switched language)
         PopulateTexts_MainDlg (hWnd);

         // set default text in text fields, in case user changed that
         SetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login);
         SetDlgItemText (hWnd, TEXTINPUT_PASSWORD, password);
         return (true);
      }

      // else was it the PLAY button ?
      else if (wParam_loword == BUTTON_PLAY)
      {
         // disable the window controls
         EnableWindow (GetDlgItem (hWnd, COMBOBOX_LOGIN), false);
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_PASSWORD), false);
         EnableWindow (GetDlgItem (hWnd, BUTTON_PLAY), false);
         EnableWindow (GetDlgItem (hWnd, BUTTON_CONFIG), false);

         // collect login and password
         memset (last_login, 0, sizeof (last_login));
         memset (password, 0, sizeof (password));
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, 256);
         GetDlgItemText (hWnd, TEXTINPUT_PASSWORD, password, 256);

         EndDialog (hWnd, true); // end dialog now
         return (true);
      }
   }

   // else is it a timer event AND is it our timer AND has the MOTD changed ?
   else if ((message == WM_TIMER) && (wParam == TIMER_REFRESHMOTD) && messagetext_modified)
   {
      SetDlgItemText (hWnd, TEXTINPUT_MOTD, messagetext); // refresh the MOTD display
      messagetext_modified = false; // do it only once
      return (true); // we handled this message
   }

   return (false);
}


int WINAPI ConfigDialogProc (HWND hWnd, unsigned int message, unsigned int wParam, long lParam)
{
   // mesage handler for launcher's main window

   unsigned short wParam_hiword;
   unsigned short wParam_loword;
   static COLORREF custom_colors[16];
   CHOOSECOLOR cc;
   char temp_string[256];
   account_t *account;
   int index;

   // filter out the commonly used message values
   wParam_hiword = HIWORD (wParam);
   wParam_loword = LOWORD (wParam);

   // have we just fired up this window ?
   if (message == WM_INITDIALOG)
   {
      // center the window
      CenterWindow (hWnd);

      // set dialog icons (small one for title bar & big one for task manager)
      SendMessage (hWnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon (hAppInstance, (char *) ICON_MAIN));
      SendMessage (hWnd, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon (hAppInstance, (char *) ICON_MAIN));

      // populate the language combo box
      ComboBox_ResetContent (GetDlgItem (hWnd, COMBOBOX_LANGUAGE));
      ComboBox_AddString (GetDlgItem (hWnd, COMBOBOX_LANGUAGE), "English");
      ComboBox_AddString (GetDlgItem (hWnd, COMBOBOX_LANGUAGE), "Français");

      // populate the window texts
      PopulateTexts_ConfigDlg (hWnd);

      // populate and tidy login/password fields
      PopulateAccountsComboBox (hWnd, COMBOBOX_LOGIN, TEXTINPUT_PASSWORD, texts.login);

      // set window controls to their values
      if (config.options_language == LANGUAGE_FRENCH)
         ComboBox_SelectString (GetDlgItem (hWnd, COMBOBOX_LANGUAGE), -1, "Français");
      else
         ComboBox_SelectString (GetDlgItem (hWnd, COMBOBOX_LANGUAGE), -1, "English");

      CheckDlgButton (hWnd, CHECKBOX_FULLSCREEN, config.options_fullscreen);
      CheckDlgButton (hWnd, CHECKBOX_KEEPRATIO, config.options_keepaspectratio);
      CheckDlgButton (hWnd, CHECKBOX_WAITFORVSYNC, config.options_waitforvsync);

      // set window controls to their values (macros)
      CheckDlgButton (hWnd, CHECKBOX_MACRO_INVENTORY, (config.shortcutkeys_inventory != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_inventory);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_INVENTORY, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_CHARACTER, (config.shortcutkeys_character != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_character);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_CHARACTER, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_CHATWINDOW, (config.shortcutkeys_chatwindow != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_chatwindow);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_CHATWINDOW, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_GROUP, (config.shortcutkeys_group != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_group);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_GROUP, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_SPELLBOOK, (config.shortcutkeys_spellbook != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_spellbook);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_SPELLBOOK, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_MACROS, (config.shortcutkeys_macros != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_macros);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_MACROS, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_EXCHANGE, (config.shortcutkeys_exchange != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_exchange);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_EXCHANGE, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_OPTIONS, (config.shortcutkeys_options != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_options);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_OPTIONS, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_SCREENSHOT, (config.shortcutkeys_screenshot != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_screenshot);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_SCREENSHOT, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_BIGCHATWINDOW, (config.shortcutkeys_bigchatwindow != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_bigchatwindow);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_BIGCHATWINDOW, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_AREAMAP, (config.shortcutkeys_areamap != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_areamap);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_AREAMAP, temp_string);
      CheckDlgButton (hWnd, CHECKBOX_MACRO_ATTACKMODE, (config.shortcutkeys_attackmode != 0x00));
      SAFE_snprintf (temp_string, sizeof (temp_string), "%c", config.shortcutkeys_attackmode);
      SetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_ATTACKMODE, temp_string);

      // enable/disable the relevant controls
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_INVENTORY), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_INVENTORY));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_CHARACTER), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_CHARACTER));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_CHATWINDOW), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_CHATWINDOW));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_GROUP), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_GROUP));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_SPELLBOOK), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_SPELLBOOK));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_MACROS), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_MACROS));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_EXCHANGE), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_EXCHANGE));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_OPTIONS), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_OPTIONS));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_SCREENSHOT), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_SCREENSHOT));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_BIGCHATWINDOW), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_BIGCHATWINDOW));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_AREAMAP), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_AREAMAP));
      EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_ATTACKMODE), IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_ATTACKMODE));
      EnableWindow (GetDlgItem (hWnd, CHECKBOX_KEEPRATIO), config.options_fullscreen);

      if (config.channelcolor_count > 0)
         current_colorcode = config.channelcolors[0]; // get the first custom color

      return (true);
   }

   // else are we redrawing the CC sample text ?
   else if ((message == WM_CTLCOLORSTATIC) || (message == WM_CTLCOLOREDIT))
   {
      if ((lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLETITLE))
               || (lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLENAME)))
      {
         SetBkMode ((HDC) wParam, TRANSPARENT); // set transparency mode
         SetTextColor ((HDC) wParam, RGB (255, 255, 255)); // set text color to the one we want
         return ((LRESULT) GetStockObject (DKGRAY_BRUSH)); // return dark gray brush as background
      }
      else if ((lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLE))
               || (lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLEBRACKET))
               || (lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLEBRACKET2))
               || (lParam == (LPARAM) GetDlgItem (hWnd, TEXT_CCSAMPLECOLON)))
      {
         SetBkMode ((HDC) wParam, TRANSPARENT); // set transparency mode
         SetTextColor ((HDC) wParam, current_colorcode); // set text color to the one we want
         return ((LRESULT) GetStockObject (DKGRAY_BRUSH)); // return dark gray brush as background
      }
   }

   // else did we click the close button on the title bar ?
   else if (message == WM_CLOSE)
   {
      EndDialog (hWnd, false); // close the window
      return (true);
   }

   // else did we alter a control INSIDE the window ?
   else if (message == WM_COMMAND)
   {
      // else was it the OK button ?
      if (wParam_loword == BUTTON_OK)
      {
         // grab the new config before closing the window
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, sizeof (last_login));
         GetDlgItemText (hWnd, TEXTINPUT_PASSWORD, password, sizeof (password));
         config.options_fullscreen = IsDlgButtonChecked (hWnd, CHECKBOX_FULLSCREEN);
         config.options_keepaspectratio = IsDlgButtonChecked (hWnd, CHECKBOX_KEEPRATIO);
         config.options_waitforvsync = IsDlgButtonChecked (hWnd, CHECKBOX_WAITFORVSYNC);

         GetDlgItemText (hWnd, COMBOBOX_SCREENRESOLUTION, temp_string, sizeof (temp_string));
         if (strcmp (temp_string, texts.resolution_small) == 0)
            config.options_screenresolution = SCREENRESOLUTION_640X480;
         else if (strcmp (temp_string, texts.resolution_medium) == 0)
            config.options_screenresolution = SCREENRESOLUTION_800X600;
         else
            config.options_screenresolution = SCREENRESOLUTION_1024X768;

         // grab the new config before closing the window (macros)
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_INVENTORY, temp_string, 2);
         config.shortcutkeys_inventory = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_INVENTORY) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_CHARACTER, temp_string, 2);
         config.shortcutkeys_character = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_CHARACTER) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_CHATWINDOW, temp_string, 2);
         config.shortcutkeys_chatwindow = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_CHATWINDOW) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_GROUP, temp_string, 2);
         config.shortcutkeys_group = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_GROUP) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_SPELLBOOK, temp_string, 2);
         config.shortcutkeys_spellbook = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_SPELLBOOK) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_MACROS, temp_string, 2);
         config.shortcutkeys_macros = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_MACROS) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_EXCHANGE, temp_string, 2);
         config.shortcutkeys_exchange = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_EXCHANGE) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_OPTIONS, temp_string, 2);
         config.shortcutkeys_options = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_OPTIONS) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_SCREENSHOT, temp_string, 2);
         config.shortcutkeys_screenshot = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_SCREENSHOT) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_BIGCHATWINDOW, temp_string, 2);
         config.shortcutkeys_bigchatwindow = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_BIGCHATWINDOW) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_AREAMAP, temp_string, 2);
         config.shortcutkeys_areamap = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_AREAMAP) ? temp_string[0] : 0x00);
         GetDlgItemText (hWnd, TEXTINPUT_SHORTCUTKEYS_ATTACKMODE, temp_string, 2);
         config.shortcutkeys_attackmode = (IsDlgButtonChecked (hWnd, CHECKBOX_MACRO_ATTACKMODE) ? temp_string[0] : 0x00);

         EndDialog (hWnd, false); // close the window
         return (true);
      }

      // else was it the CANCEL button ?
      else if (wParam_loword == BUTTON_CANCEL)
      {
         EndDialog (hWnd, false); // close the window
         return (true);
      }

      // else is it the login combo box ?
      else if (wParam_loword == COMBOBOX_LOGIN)
      {
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, sizeof (last_login)); // get combo box text
         account = Account_FindByUsername (last_login); // find appropriate login
         if (account != NULL)
            SetDlgItemText (hWnd, TEXTINPUT_PASSWORD, account->password); // if found, set the appropriate password
      }

      // else is it the language combo box ?
      else if ((wParam_hiword == CBN_SELCHANGE) && (wParam_loword == COMBOBOX_LANGUAGE))
      {
         GetDlgItemText (hWnd, COMBOBOX_LANGUAGE, temp_string, sizeof (temp_string)); // get combo box text
         if (stricmp (temp_string, "Français") == 0)
            config.options_language = LANGUAGE_FRENCH; // french language
         else
            config.options_language = LANGUAGE_ENGLISH; // french language
     
         Language_Load (); // load the right language
         PopulateTexts_ConfigDlg (hWnd); // populate the window texts again
      }

      // else is it the ADDACCOUNT button ?
      else if (wParam_loword == BUTTON_ADDACCOUNT)
      {
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, sizeof (last_login)); // get combo box text
         account = Account_FindByUsername (last_login); // find appropriate login
         if (account != NULL)
         {
            // login found, ask for update
            if (MessageBox_printf (MB_YESNO | MB_ICONQUESTION, "Confirmation", "Mettre à jour les informations du compte %s ?", account->username) != IDYES)
               return (false); // if the user cancels, forget it

            // update the appropriate password and populate the listbox again
            GetDlgItemText (hWnd, TEXTINPUT_PASSWORD, account->password, sizeof (account->password));
            PopulateAccountsComboBox (hWnd, COMBOBOX_LOGIN, TEXTINPUT_PASSWORD, account->username);
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Mise à jour effective", "Les informations du compte %s ont été modifiées.", account->username);
         }
         else
         {
            // login NOT found, ask for update
            if (MessageBox_printf (MB_YESNO | MB_ICONQUESTION, "Confirmation", "Ajouter les informations pour le compte %s ?", last_login) != IDYES)
               return (false); // if the user cancels, forget it

            // we have a new login/password pair, reallocate space to hold it
            config.accounts = (account_t *) SAFE_realloc (config.accounts, config.account_count, config.account_count + 1, sizeof (account_t), false);

            // copy login and password in place
            GetDlgItemText (hWnd, COMBOBOX_LOGIN, config.accounts[config.account_count].username, sizeof (config.accounts[config.account_count].username));
            GetDlgItemText (hWnd, TEXTINPUT_PASSWORD, config.accounts[config.account_count].password, sizeof (config.accounts[config.account_count].password));

            config.account_count++; // remember we know now one login/password pair more

            PopulateAccountsComboBox (hWnd, COMBOBOX_LOGIN, TEXTINPUT_PASSWORD, last_login);
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Mise à jour effective", "Les informations du compte %s ont été ajoutées.", last_login);
         }
      }

      // else is it the DELETEACCOUNT button ?
      else if (wParam_loword == BUTTON_DELETEACCOUNT)
      {
         GetDlgItemText (hWnd, COMBOBOX_LOGIN, last_login, sizeof (last_login)); // get combo box text
         account = Account_FindByUsername (last_login); // find appropriate login
         if (account != NULL)
         {
            // login found, ask for update
            if (MessageBox_printf (MB_YESNO | MB_ICONQUESTION, "Confirmation", "Supprimer les informations du compte %s ?", account->username) != IDYES)
               return (false); // if the user cancels, forget it

            // cycle through all accounts and find the one we want
            for (index = 0; index < config.account_count; index++)
               if (stricmp (config.accounts[index].username, last_login) == 0)
                  break; // stop when we find it

            // shift the rest of the array one position down
            memmove (&config.accounts[index], &config.accounts[index + 1], (config.account_count - index - 1) * sizeof (account_t));

            // we have a new login/password pair, reallocate space to hold it
            config.accounts = (account_t *) SAFE_realloc (config.accounts, config.account_count, config.account_count - 1, sizeof (account_t), false);
            config.account_count--; // remember we know now one login/password pair less

            // populate the listbox again
            PopulateAccountsComboBox (hWnd, COMBOBOX_LOGIN, TEXTINPUT_PASSWORD, "Votre login");
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Mise à jour effective", "Les informations du compte %s ont été supprimées.", last_login);
         }
         else
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Information", "Aucun compte nommé %s n'a été enregistré.", last_login);
      }

      // else is it the "new CC color" button ?
      else if (wParam_loword == BUTTON_PICKUPCOLOR)
      {
         // prepare a color pick dialog box
         memset (&cc, 0, sizeof (cc));
         cc.lStructSize = sizeof (cc);
         cc.hwndOwner = hWnd;
         cc.lpCustColors = (unsigned long *) custom_colors;
         cc.rgbResult = current_colorcode;
         cc.Flags = CC_FULLOPEN | CC_RGBINIT;

         // fire it up
         if (ChooseColor (&cc))
         {
            current_colorcode = cc.rgbResult; // save away returned color
            InvalidateRect (hWnd, NULL, 0); // and redraw window
         }
      }

      // else are we circling around the custom colors ?
      else if (wParam_loword == BUTTON_CCCOLORSLEFT)
      {
         // only do this if there are actually colors to switch to
         if (config.channelcolor_count > 1)
         {
            color_index--; // circle down through the custom colors array
            if (color_index < 0)
               color_index = config.channelcolor_count - 1;
            current_colorcode = config.channelcolors[color_index]; // update current custom color
            InvalidateRect (hWnd, NULL, 0); // and redraw window
         }
      }
      else if (wParam_loword == BUTTON_CCCOLORSRIGHT)
      {
         // only do this if there are actually colors to switch to
         if (config.channelcolor_count > 1)
         {
            color_index++; // circle up through the custom colors array
            if (color_index > config.channelcolor_count - 1)
               color_index = 0;
            current_colorcode = config.channelcolors[color_index]; // update current custom color
            InvalidateRect (hWnd, NULL, 0); // and redraw window
         }
      }

      // else do we want to add the current color to the list of custom color codes ?
      else if (wParam_loword == BUTTON_CCCOLORSADD)
      {
         // resize custom colors array to hold a color code more
         config.channelcolors = (unsigned long *) SAFE_realloc (config.channelcolors, config.channelcolor_count, config.channelcolor_count + 1, sizeof (unsigned long), false);
         config.channelcolors[config.channelcolor_count] = current_colorcode; // add new color
         config.channelcolor_count++; // remember we know now one custom color more

         // update the amount of colors display
         SAFE_snprintf (temp_string, sizeof (temp_string), "%d couleurs dans la liste", config.channelcolor_count);
         SetDlgItemText (hWnd, TEXT_CCCOLORSAMOUNT, temp_string);

         // print a nice message
         MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Ajout d'une couleur personnalisée", "Cette couleur a été ajoutée à la liste des couleurs personnalisées.");
         InvalidateRect (hWnd, NULL, 0); // and redraw window
      }

      // else do we want to delete the current color from the list of custom color codes ?
      else if (wParam_loword == BUTTON_CCCOLORSDELETE)
      {
         // find the color we want in the custom colors array
         for (index = 0; index < config.channelcolor_count; index++)
            if (config.channelcolors[index] == current_colorcode)
               break; // break as soon as we found it

         // found it ?
         if (index < config.channelcolor_count)
         {
            current_colorcode = config.channelcolors[(index + 1 < config.channelcolor_count ? index + 1 : 0)];
            for (; index < config.channelcolor_count - 1; index++)
               config.channelcolors[index] = config.channelcolors[index + 1]; // shuffle down the array

            // resize custom colors array to hold one color code less
            config.channelcolors = (unsigned long *) SAFE_realloc (config.channelcolors, config.channelcolor_count, config.channelcolor_count - 1, sizeof (unsigned long), false);
            config.channelcolor_count--; // remember we know now one custom color more

            // update the amount of colors display
            SAFE_snprintf (temp_string, sizeof (temp_string), texts.colors_total, config.channelcolor_count);
            SetDlgItemText (hWnd, TEXT_CCCOLORSAMOUNT, temp_string);

            /// print a nice message
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Suppression d'une couleur personnalisée", "Cette couleur a été supprimée de la liste des couleurs personnalisées.");
            InvalidateRect (hWnd, NULL, 0); // and redraw window
         }
         else
            MessageBox_printf (MB_OK | MB_ICONEXCLAMATION, "Suppression d'une couleur personnalisée", "Cette couleur n'existe pas dans la liste actuelle des couleurs personnalisées.");
      }

      // else is it one of the macro check boxes ?
      else if (wParam_loword == CHECKBOX_MACRO_INVENTORY)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_INVENTORY), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_CHARACTER)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_CHARACTER), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_CHATWINDOW)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_CHATWINDOW), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_GROUP)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_GROUP), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_SPELLBOOK)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_SPELLBOOK), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_MACROS)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_MACROS), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_EXCHANGE)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_EXCHANGE), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_OPTIONS)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_OPTIONS), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_SCREENSHOT)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_SCREENSHOT), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_BIGCHATWINDOW)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_BIGCHATWINDOW), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_AREAMAP)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_AREAMAP), IsDlgButtonChecked (hWnd, wParam_loword));
      else if (wParam_loword == CHECKBOX_MACRO_ATTACKMODE)
         EnableWindow (GetDlgItem (hWnd, TEXTINPUT_SHORTCUTKEYS_ATTACKMODE), IsDlgButtonChecked (hWnd, wParam_loword));

      // else is it the fullscreen checkbox ?
      else if (wParam_loword == CHECKBOX_FULLSCREEN)
         EnableWindow (GetDlgItem (hWnd, CHECKBOX_KEEPRATIO), IsDlgButtonChecked (hWnd, wParam_loword));
   }

   return (false); // return false since we didn't process the message
}


static int WINAPI CCSampleWndProc (HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam)
{
   // CC sample text window procedure

   // control redraw
   if ((message == WM_CTLCOLORSTATIC) || (message == WM_CTLCOLOREDIT))
   {
      SetBkMode ((HDC) wParam, OPAQUE); // set opaque mode
      return ((LRESULT) GetStockObject (BLACK_BRUSH)); // return the black brush
   }

   // call the default window procedure to keep things going
   return (DefWindowProc (hWnd, message, wParam, lParam));
}


static void PopulateTexts_MainDlg (HWND hWnd)
{
   // helper function that populates the texts in the main window

   // populate the window texts
   SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM) texts.mainwindow_title);
   SetDlgItemText (hWnd, TEXT_LOGIN, texts.login);
   SetDlgItemText (hWnd, TEXT_PASSWORD, texts.password);
   SetDlgItemText (hWnd, BUTTON_PLAY, texts.button_play);
   SetDlgItemText (hWnd, BUTTON_CONFIG, texts.button_setup);

   return; // finished
}


static void PopulateTexts_ConfigDlg (HWND hWnd)
{
   // helper function that populates the texts in the setup window

   char temp_string[256];

   // populate the window texts
   SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM) texts.setupwindow_title);
   SetDlgItemText (hWnd, TEXT_KEYBOARDSHORTCUTS, texts.keyboard_shortcuts);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_INVENTORY, texts.shortcut_inventory);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_CHARACTERSHEET, texts.shortcut_charactersheet);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_CHATTERCHANNELS, texts.shortcut_chatterchannels);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_GROUPSHEET, texts.shortcut_groupsheet);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_SPELLBOOK, texts.shortcut_spellbook);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_MACROS, texts.shortcut_macros);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_EXCHANGE, texts.shortcut_exchange);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_OPTIONS, texts.shortcut_options);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_SCREENSHOT, texts.shortcut_screenshot);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_BIGCHATWINDOW, texts.shortcut_bigchatwindow);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_AREAMAP, texts.shortcut_areamap);
   SetDlgItemText (hWnd, CHECKBOX_MACRO_ATTACKMODE, texts.shortcut_attackmode);
   SetDlgItemText (hWnd, TEXT_SAVEDACCOUNTS, texts.saved_accounts);
   SetDlgItemText (hWnd, TEXT_LOGIN, texts.login);
   SetDlgItemText (hWnd, TEXT_PASSWORD, texts.password);
   SetDlgItemText (hWnd, BUTTON_ADDACCOUNT, texts.button_add);
   SetDlgItemText (hWnd, BUTTON_DELETEACCOUNT, texts.button_delete);
   SetDlgItemText (hWnd, TEXT_DISPLAYOPTIONS, texts.display_options);
   SetDlgItemText (hWnd, TEXT_DISPLAYOPTIONSTIP, texts.display_hinttext);
   SetDlgItemText (hWnd, TEXT_DISPLAYOPTIONS, texts.display_options);
   SetDlgItemText (hWnd, TEXT_SCREENRESOLUTION, texts.screen_resolution);
   ComboBox_ResetContent (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION));
   ComboBox_AddString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), texts.resolution_large);
   ComboBox_AddString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), texts.resolution_medium);
   ComboBox_AddString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), texts.resolution_small);
   SetDlgItemText (hWnd, CHECKBOX_FULLSCREEN, texts.fullscreen);
   SetDlgItemText (hWnd, CHECKBOX_KEEPRATIO, texts.keep_proportions);
   SetDlgItemText (hWnd, CHECKBOX_WAITFORVSYNC, texts.wait_vsync);
   SetDlgItemText (hWnd, TEXT_CCCOLORS, texts.cc_colors);
   SetDlgItemText (hWnd, BUTTON_PICKUPCOLOR, texts.button_pickupcolor);
   SetDlgItemText (hWnd, BUTTON_CCCOLORSADD, texts.button_add);
   SetDlgItemText (hWnd, BUTTON_CCCOLORSDELETE, texts.button_delete);
   SetDlgItemText (hWnd, TEXT_CCCOLORSAMOUNT, texts.colors_total);
   SetDlgItemText (hWnd, BUTTON_OK, texts.button_ok);
   SetDlgItemText (hWnd, BUTTON_CANCEL, texts.button_cancel);

   // update the screen resolutions display
   if (config.options_screenresolution == SCREENRESOLUTION_640X480)
      ComboBox_SelectString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), -1, texts.resolution_small);
   else if (config.options_screenresolution == SCREENRESOLUTION_800X600)
      ComboBox_SelectString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), -1, texts.resolution_medium);
   else
      ComboBox_SelectString (GetDlgItem (hWnd, COMBOBOX_SCREENRESOLUTION), -1, texts.resolution_large);

   // update the amount of colors display
   SAFE_snprintf (temp_string, sizeof (temp_string), texts.colors_total, config.channelcolor_count);
   SetDlgItemText (hWnd, TEXT_CCCOLORSAMOUNT, temp_string);

   return; // finished
}


static void CenterWindow (HWND hWnd)
{
   // this function centers the window on the desktop.

   RECT rRect, rParentRect;
   HWND hParentWnd;
   int width;
   int height;
   int parent_width;
   int parent_height;
   int x;
   int y;

   // get the current rectangle of the current window
   GetWindowRect (hWnd, &rRect);
   width = rRect.right - rRect.left;
   height = rRect.bottom - rRect.top;

   hParentWnd = GetDesktopWindow (); // if parent window not found, get handle to desktop

   // get the rectangle of the parent window
   GetWindowRect (hParentWnd, &rParentRect);
   parent_width = rParentRect.right - rParentRect.left;
   parent_height = rParentRect.bottom - rParentRect.top;

   // now compute the new X and Y positions so as to have the window centered in its parent
   x = rParentRect.left + parent_width / 2 - width / 2;
   y = rParentRect.top + parent_height / 2 - height / 2;

   // now ask to change the position of the window
   SetWindowPos (hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

   return; // finished
}


static account_t *Account_FindByUsername (const char *username)
{
   // helper function to retrieve a pointer to a login struct given a certain username

   int index;

   // cycle through all login/password pairs...
   for (index = 0; index < config.account_count; index++)
      if (stricmp (config.accounts[index].username, username) == 0)
         return (&config.accounts[index]); // return the one we find

   return (NULL); // else return a NULL pointer
}


static void PopulateAccountsComboBox (HWND hWnd, int loginctl_id, int passwdctl_id, const char *wanted_login)
{
   // helper function to populate the login combo box and set the password field accordingly

   account_t *current_account;
   int index;

   // reset the logins listbox first
   ComboBox_ResetContent (GetDlgItem (hWnd, loginctl_id));

   // are there logins predefined ?
   if (config.account_count > 0)
   {
      // find the last used login
      current_account = Account_FindByUsername (wanted_login);
      if (current_account == NULL)
         current_account = &config.accounts[0]; // failsafe on the first one if we can't find it

      // populate the listbox
      for (index = 0; index < config.account_count; index++)
         ComboBox_AddString (GetDlgItem (hWnd, loginctl_id), config.accounts[index].username);

      // and select the most recently used one
      ComboBox_SelectString (GetDlgItem (hWnd, loginctl_id), -1, current_account->username);

      // set the matching password
      SetDlgItemText (hWnd, passwdctl_id, current_account->password);
   }
   else
   {
      // set default text in text fields
      SetDlgItemText (hWnd, loginctl_id, "Votre login");
      SetDlgItemText (hWnd, passwdctl_id, "Mot de passe");
   }

   return; // finished
}


static void Language_Load (void)
{
   // this function loads and parses the right language file and fills in the texts structure

   char line_buffer[256];
   char string_data[256];
   int string_id;
   FILE *fp;

   // depending on the language we're supposed to read, open the right file
   if (config.options_language == LANGUAGE_FRENCH)
      SAFE_snprintf (string_data, sizeof (string_data), "%s\\frenchgui.elng", app_path);
   else
      SAFE_snprintf (string_data, sizeof (string_data), "%s\\englishgui.elng", app_path);

   // open the right file
   if (fopen_s (&fp, string_data, "rb") != 0)
   {
      MessageBox_printf (MB_OK, "Error", "No suitable language file could be found. Please reinstall the game.");
      return; // bomb out on error
   }

   // read line per line
   while (fgets (line_buffer, sizeof (line_buffer), fp) != NULL)
   {
      if (sscanf_s (line_buffer, "[%d] \"%[^\"]\"", &string_id, string_data, sizeof (string_data)) != 2)
         continue; // discard invalid lines

      // pick up those we are interested in
      if (string_id == 226)
         SAFE_strncpy (texts.mainwindow_title, string_data, sizeof (texts.mainwindow_title));
      else if (string_id == 227)
         SAFE_strncpy (texts.login, string_data, sizeof (texts.login));
      else if (string_id == 228)
         SAFE_strncpy (texts.password, string_data, sizeof (texts.password));
      else if (string_id == 229)
         SAFE_strncpy (texts.button_play, string_data, sizeof (texts.button_play));
      else if (string_id == 230)
         SAFE_strncpy (texts.button_setup, string_data, sizeof (texts.button_setup));
      else if (string_id == 231)
         SAFE_strncpy (texts.setupwindow_title, string_data, sizeof (texts.setupwindow_title));
      else if (string_id == 232)
         SAFE_strncpy (texts.keyboard_shortcuts, string_data, sizeof (texts.keyboard_shortcuts));
      else if (string_id == 233)
         SAFE_strncpy (texts.shortcut_inventory, string_data, sizeof (texts.shortcut_inventory));
      else if (string_id == 234)
         SAFE_strncpy (texts.shortcut_charactersheet, string_data, sizeof (texts.shortcut_charactersheet));
      else if (string_id == 235)
         SAFE_strncpy (texts.shortcut_chatterchannels, string_data, sizeof (texts.shortcut_chatterchannels));
      else if (string_id == 236)
         SAFE_strncpy (texts.shortcut_groupsheet, string_data, sizeof (texts.shortcut_groupsheet));
      else if (string_id == 237)
         SAFE_strncpy (texts.shortcut_spellbook, string_data, sizeof (texts.shortcut_spellbook));
      else if (string_id == 238)
         SAFE_strncpy (texts.shortcut_macros, string_data, sizeof (texts.shortcut_macros));
      else if (string_id == 239)
         SAFE_strncpy (texts.shortcut_exchange, string_data, sizeof (texts.shortcut_exchange));
      else if (string_id == 240)
         SAFE_strncpy (texts.shortcut_options, string_data, sizeof (texts.shortcut_options));
      else if (string_id == 241)
         SAFE_strncpy (texts.shortcut_screenshot, string_data, sizeof (texts.shortcut_screenshot));
      else if (string_id == 242)
         SAFE_strncpy (texts.shortcut_bigchatwindow, string_data, sizeof (texts.shortcut_bigchatwindow));
      else if (string_id == 243)
         SAFE_strncpy (texts.shortcut_areamap, string_data, sizeof (texts.shortcut_areamap));
      else if (string_id == 244)
         SAFE_strncpy (texts.shortcut_attackmode, string_data, sizeof (texts.shortcut_attackmode));
      else if (string_id == 245)
         SAFE_strncpy (texts.saved_accounts, string_data, sizeof (texts.saved_accounts));
      else if (string_id == 246)
         SAFE_strncpy (texts.button_add, string_data, sizeof (texts.button_add));
      else if (string_id == 247)
         SAFE_strncpy (texts.button_delete, string_data, sizeof (texts.button_delete));
      else if (string_id == 248)
         SAFE_strncpy (texts.display_options, string_data, sizeof (texts.display_options));
      else if (string_id == 249)
         SAFE_strncpy (texts.display_hinttext, string_data, sizeof (texts.display_hinttext));
      else if (string_id == 250)
         SAFE_strncpy (texts.screen_resolution, string_data, sizeof (texts.screen_resolution));
      else if (string_id == 251)
         SAFE_strncpy (texts.resolution_large, string_data, sizeof (texts.resolution_large));
      else if (string_id == 252)
         SAFE_strncpy (texts.resolution_medium, string_data, sizeof (texts.resolution_medium));
      else if (string_id == 253)
         SAFE_strncpy (texts.resolution_small, string_data, sizeof (texts.resolution_small));
      else if (string_id == 254)
         SAFE_strncpy (texts.fullscreen, string_data, sizeof (texts.fullscreen));
      else if (string_id == 255)
         SAFE_strncpy (texts.keep_proportions, string_data, sizeof (texts.keep_proportions));
      else if (string_id == 256)
         SAFE_strncpy (texts.wait_vsync, string_data, sizeof (texts.wait_vsync));
      else if (string_id == 257)
         SAFE_strncpy (texts.cc_colors, string_data, sizeof (texts.cc_colors));
      else if (string_id == 258)
         SAFE_strncpy (texts.button_pickupcolor, string_data, sizeof (texts.button_pickupcolor));
      else if (string_id == 259)
         SAFE_strncpy (texts.colors_total, string_data, sizeof (texts.colors_total));
      else if (string_id == 260)
         SAFE_strncpy (texts.button_ok, string_data, sizeof (texts.button_ok));
      else if (string_id == 261)
         SAFE_strncpy (texts.button_cancel, string_data, sizeof (texts.button_cancel));
   }

   fclose (fp); // finished, close the file
   return; // and return
}
