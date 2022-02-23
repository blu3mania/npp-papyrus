# Installation
## Install the plugin
This plugin can be installed from Notepad++'s *Plugins Admin* if you are using Notepad++ version 8+, in which
case the configuration file *"Papyrus.xml"* will be handled automatically. In the rare case when permission
is required on Notepad++'s configuration folder, which by default is *"%APPDATA%\Notepad++"*, this plugin will
prompt to run the copy command with elevated privilege. When this happens, please accept the UAC prompt. If
somehow that also fails, this plugin will show a notice and you can manually copy the config file. **Note**,
the data folder is ***not*** under Notepad++'s installation folder if you used default directory. Please see
[this post](https://community.notepad-plus-plus.org/topic/16996/new-plugins-home-round-2) for more details.

For manual installation, extract the release package to *"plugins\Papyrus"* directory under Notepad++'s
installation folder, which by default is *"%PROGRAMFILES%\Notepad++"* if you installed 64-bit version, or
*"%PROGRAMFILES(x86)%\Notepad++"* if you installed 32-bit version. If it is the first time you install this
plugin, you will need to manually create *"Papyrus"* folder under *"plugins"* directory. After that, just
launch Notepad++ and the plugin should be able to automatically copy *"Papyrus.xml"* file to
*"plugins\config"* directory under Notepad++'s data folder, similar to the handling when it's installed by
*Plugins Admin*.

**WARNING:**
- Do not install versions prior to v0.3.0 if you are using Notepad++ v8.3+.
- Do not install v0.3.0+ if you are using a Notepad++ version prior to v8.3.


## Install extra stuff
If you want, there are a few xml files under *"extras"* directory that you can also install to provide other
features that are not part of the lexer/compiler:

- **autoCompletion\Papyrus Script.xml**

  You can either use this plugin's *"Install auto completion support"* feature under *Advanced* menu, or
  manually copy this file to *"autoCompletion"* directory under Notepad++'s installation folder.

  It provides auto-completion support for functions defined in base game, *SKSE*, and even *SkyUI*. Though,
  if you are currently using a Notepad++ version that is older than v8, please note that there was
  [a bug](https://github.com/notepad-plus-plus/notepad-plus-plus/issues/3997) that made it case sensitive when
  using the default *"Function and word completion"* option for Notepad++'s auto-completion feature. You
  should update to the latest Notepad++ release, but if you have to stay with the old version, the suggestion
  is to either configure auto-completion to *"Function completion"* only (*note*, you will lose auto-completion
  on words), or always use uppercase for the first letter when typing a game defined function name.

  *Note*, the content of this auto-completion definition file is copied from
  [Creation Kit web](https://www.creationkit.com/index.php?title=Papyrus_Autocomplete).

- **functionList\Papyrus Script.xml**

  You can use this plugin's *"Install function list support"* feature under *Advanced* menu, which
  automatically copies this file and also sets correct langID in *overrideMap.xml*.

  If you want to do it manually, it is a bit tricky. First, copy this file to *functionList* directory under
  Notepad++'s data folder (again, it's not Notepad++'s installation folder, and by default it's
  *"%APPDATA%\Notepad++"*). Then, you need to update overrideMap.xml under that folder. The key is to find the
  langID assigned to Papyrus lexer, which can be obtained by using the plugin's *"Show langID"* feature under
  *Advanced* menu.

  A sample *overrideMap.xml* is provided that works with Notepad++ v7.9.x when Papyrus plugin is the only lexer
  plugin, or it happens to be the *first one* to Notepad++ (*note*, it ***may not*** be based on plugins'
  alphabetic order).

  If you ever install another lexer plugin which results in the change of langID assigned to this plugin's
  lexer, you can simply use *"Install function list support"* menu again to get *overrideMap.xml* updated, or
  manually update it by following the instructions above.

  Once installed, when viewing a Papyrus script file you can use *View -> Function List* menu to show all
  defined functions in the script.

  *Note*, the content of this function list definition file is copied from
  [Creation Kit web](https://www.creationkit.com/index.php?title=Notepad%2B%2B_Setup#Papyrus_Function_List_for_Notepad.2B.2B).

- **userDefineLangs\Papyrus.udl.xml**

  This is just a user-defined language for Papyrus script. If you don't want to install a DLL, and don't need
  the plugin's advanced features & compilation support, you can just copy this file to *userDefineLangs*
  directory under Notepad++'s data folder.

  It has mostly the same syntax highlighting as this plugin's lexer, but lacks support for advanced features
  such as properties recognition/folding, user defined class names and functions recognition, keywords
  matching, class name links, and configurable folding behavior on *Else/ElseIf*.

  It's strongly **not recommended** to install it along with the plugin. You may get unexpected behavior when
  you open a *.psc* file, and you don't gain anything from using this UDL over the included lexer in this
  plugin anyway.
