# Installation
## Install the plugin
Extract the release package to "plugins\Papyrus" directory under Notepad++'s installation folder, which by
default is "%PROGRAMFILES%\Notepad++" if you installed 64-bit version. If this is the first time you install
this plugin, you will need to create "Papyrus" folder manually.

After that, just launch Notepad++ and the plugin should be able to automatically copy Papyrus.xml file to
"plugins\config" directory under Notepad++'s data folder, which by default is "%APPDATA%\Notepad++". If it
fails to do so it will show a notice and you can manually copy the config file. **Note**, the data folder
is ***not*** under Notepad++'s installation folder if you used default directory.
Please see [this post](https://community.notepad-plus-plus.org/topic/16996/new-plugins-home-round-2) for
more details.

## Install extra stuff
If you want, there are a few xml files under extra directory that you can also install to provide other
features that are not part of teh lexer/compiler:

- autoCompletion\Papyrus Script.xml

  You can either use this plugin's "Install auto completion support" feature under Advanced menu, or manually
  copy this file to "autoCompletion" directory under Notepad++'s installation folder.
  
  It provides auto-completion support for functions defined in base game, SKSE, and even SkyUI. Though, there
  is currently [a bug in Notepad++](https://github.com/notepad-plus-plus/notepad-plus-plus/issues/3997) that
  makes it case sensitive when using the default "Function and word completion" option for Notepad++'s
  auto-completion feature.

  The suggestion is to either change it to "Function completion" only (you lose auto-completion on words), or
  always use uppercase for the first letter when typing a game defined function name.

  *Note*, the content of this auto-completion definition file is copied from
  [Creation Kit web](https://www.creationkit.com/index.php?title=Papyrus_Autocomplete).

- functionList\Papyrus Script.xml

  You can use this plugin's "Install function list support" feature under Advanced menu, which automatically
  copies this file and also sets correct langID in overrideMap.xml.
  
  If you want to do it manually, it is a bit tricky. First, copy this file to "functionList" directory under
  Notepad++'s data folder. Then, you need to update overrideMap.xml under that folder. The key is to find the
  langID assigned to Papyrus lexer, which can be obtained by using the plugin's "Show langID" feature under
  Advanced menu.
  
  A sample overrideMap.xml is provided that works with Notepad++ v7.9.3 when Papyrus plugin is the only lexer
  plugin, or it happens to be the "first one" to Notepad++ (*note*, it **may not** be based on plugins'
  alphabetic order).

  If you ever install another lexer plugin which results in the change of langID assigned to this plugin's
  lexer, you can simply use "Add function list support" again to get overrideMap.xml updated, or manually
  update it according to the instructions above.

  Once installed, when viewing a Papyrus script file you can use "View" -> "Function List" menu to show all
  defined functions in the script.

  *Note*, the content of this function list definition file is copied from
  [Creation Kit web](https://www.creationkit.com/index.php?title=Notepad%2B%2B_Setup#Papyrus_Function_List_for_Notepad.2B.2B).

- userDefineLangs\Papyrus.udl.xml

  This is just a user-defined language for Papyrus script. If you don't want to install a DLL, and don't need
  the plugin's advanced syntax highlighting & compilation support, you can just copy this file to
  "userDefineLangs" directory under Notepad++'s data folder.

  It has mostly the same syntax highlighting as this plugin's lexer, but lacks support for advanced features
  such as properties recognition/folding, user defined class names and functions recognition, and configurable
  folding behavior on else/elseif.

  It's strongly not recommended to install it along with the plugin. You may get unexpected behavior when you
  open a .psc file, and you don't gain anything over the lexer anyway.
