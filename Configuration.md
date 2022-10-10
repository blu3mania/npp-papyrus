# Configuration
Most of this plugin's behavior can be modified in this plugin's *Settings* dialog. However, if it's needed
to change the settings file *Papyrus.ini* directly, it can be found in *"plugins\config"* directory under
Notepad++'s data folder, which by default is *"%APPDATA%\Notepad++"*. Also, this plugin's menu shortcuts
can be remapped from Notepad++'s *Shortcut Mapper* dialog under *Settings* menu.

The following are settings that can be changed in this plugin's *Settings* dialog.

## Lexer tab

### Folding behavior
By default, the lexer will fold separately when it encounters *Else* and *ElseIf*, i.e. each *If/Else/ElseIf*
block will be folded by itself. If this behavior is not desired, disable this option so that when folding an
*If*, the whole block from *If* to *EndIf* will be folded.

### Class names caching
When a script file references other scripts (referred to as *class* hereafter), default lexer behavior is
to check the file name every time a word is not recognized as other types, even if the same word has been
previously checked and determined as a class or not a class. This usually should be fast enough, but in a
script file that repeatedly references the same classes many times, it may be desired to cache checked
class names to reduce number of I/O operations. This can be achieved by enabling this option.

However, there is a caveat, that when a new script file is generated in one of the import directories,
the new class name will not be dynamically recognized if the name has already been checked and treated as
"not a class name". When this happens, simply disable this option to force the lexer to re-check for class
names. Afterwards this option can be enabled again.

### Class names as links
Since Papyrus script often references other script files, it would be convenient to be able to open those
files without going through open file dialog. By enabling this feature, every class name is shown as a link.
When hovering over, the style will change to the configured style. By default, the link is shown as
underlined, with blue foreground and white background (kind of a web standard).

To activate a link, by default it requires double click while holding *Ctrl* key to activate, so although
when hovered over the text shows as a link, single click can still move cursor to the word without triggering
the activation, and double click will just select/highlight the word without triggering activation as well.

The key modifier can be configured with any combination of *Shift/Ctrl/Alt*. And if really needed, the behavior
of requiring double click can be configured in *Papyrus.ini*. **But be warned**, single click behavior usually
causes large amount of text gets selected in the newly opened file window so it will be more of a trouble
than convenience.

### Papyrus Script Lexer styles
These styles can be configured from Notepad++'s *Style Configurator* dialog under *Settings* menu. A convenient
link is provided.


## Keyword Matcher tab
For many language features in Papyrus, there is a begin/end pair, such as *Function/EndFunction* and
*Property/EndProperty*. Also for flow control, there are always pairs, like *While/EndWhile* and *If/EndIf*.
Keywords matching enables highlighting these paired keywords when one is selected (just when cursor is over
the word, not necessarily have to be double clicked).

The configuration allows enabling keywords matching for each distinctive language features. For *If/EndIf*,
it also further provides an option to highlight *Else/ElseIf* as well.

Some "begin" keywords may have multiple "end" keywords, e.g. Property can be matched with either EndProperty,
Auto, or AutoReadOnly. This is properly handled. For flow control, nested flow controls are correctly handled,
so highlighting always happens at the same level. The highlighting style can be configured and by default it
is a light purple filled round box which is similar to Notepad++'s XML tags matching.

In the rare case when a keyword cannot be matched, like when there is a typo, or it's an unfinished function
definition, unmatched style will be used to highlight. It is configurable as well and by default it is a
red rectangle.

Usually the default indicator ID (17) won't conflict with other plugins. However, there is no guarantee that
will never happen, so this plugin allows you to choose a different indicator if there is a conflict. The
valid numbers are between 9 and 20. Keep in mind other plugins may use indicator IDs as well, for example,
DSpellCheck uses 19. Make sure you **do not** change this ID if everything works fine.


## Error Annotator tab
When Papyrus compiler reports compilation errors, original plugin can show the list of errors in a window,
where user can click on a row to jump to the error line of that file. In addition to this behavior, this
plugin is also capable of showing directly in the editor window where errors are reported.

### Annotation
Show annotations under error lines. They are inside a round box. By default, the font is italic with red
foreground and bright grey background. When there are multiple errors on the same line, they are grouped
in the same annotation box.

### Indication
Show indications where errors happen. By default, the indicator is a red squiggly line under the word or
operator where error is, similar to what a spell checker does. There are many styles to choose from.

Similar to Keyword Matcher's indicator ID, Error Annotator's indicator ID can be changed as well. The
default ID (18) should not cause conflict usually, but if it does, follow the same instructions provided
in [Keyword Matcher tab](#keyword-matcher-tab) section. *Note*, after changing it, existing indications
may be rendered incorrectly if edits have been made. You can recompile the script to make them show
correctly again, or simply fix all bugs reported against your script. 😀


## Compiler tab

### Compiler mode
This plugin automatically detects supported games (*Skyrim*, *Skyrim SE/AE*, *Fallout 4*), and selects Auto
mode by default. In Auto mode, if a script is loaded from a folder under a detected game, that game's
settings will be used to compile the script. If the script is from a location outside of any game's install
path, the default game configured for Auto mode will be used to compile the script, and in this case, the
output of compilation will be placed under configured default output directory, instead of that game's
configured output folder.

You can also choose a specific game to be used for all script files, but be warned, doing so means that
even if you are editing a script from another game, designated game's compiler settings will be used and
likely that would cause unexpected result. If unsure, just use Auto mode.

Obviously if no game is detected, the plugin will not be able to choose a compiler. If somehow a game is
installed but registry key is not present (hmmm...), you can still manually enable the game and configure
all settings in that game's tab. Whenever a game is enabled, the game's configuration tab will be added.

If for whatever reason you don't want the plugin to use a detected game, you can disable it manually. If
that game is currently selected as Auto mode's default game, the next game in the list will be automatically
chosen instead. Whenever a game is disabled, the game's configuration tab will be removed.

For each game, a convenient button *Configure* is provided to go directly to that game's configuration
tab.

### Allow compiling files not recognized as Papyrus script
This setting allows the compiler to compile a script file that is not selected as *Papyrus Script* in
language menu. It is only useful if you want to use a user-defined language instead of using the lexer
provided by this plugin, or for some reason you don't want to use syntax highlighting at all (😕).


## Games tabs
Each enabled game will have its own configuration tab. Most configurations are self-explanatory, and you
usually should just leave the default values untouched. For *Import directories* and *Output directory*,
make sure that you **do not add trailing backslash**. There are a few checkboxes:

### Anonymize generated .PEX
This setting allows you to anonymize the generated *.pex* file. In case you are not aware, when you use
PapyrusCompiler to compile any script your user account and machine name are stored inside the generated
*.pex* file. In FO4's case, the whole path to the source *.psc* file is also stored there. Very sneaky,
*Bethesda*!

Since this is a big **privacy concern**, this plugin provides a way to anonymize those info by replacing
all of them with dashes. You are recommended to leave it checked, unless you want to showcase your fancy
machine name to the whole modding world. 😀

### Optimize flag
This setting instructs Papyrus compiler to optimize output (*"-op"*). Not sure how much that helps but it's
usually not a bad thing. However, there are cases that compilation would pass but the compiler then chokes
on the assembly file (*.pas*). If that happens, temporarily disable this flag and compile the script.

### Release flag
This setting only applies to *Fallout 4*. It instructs Papyrus compiler to use release mode (*"-r"*), which
removes all debugOnly function calls and optimizes the output, supposedly reducing the output size.

### Final flag
This setting only applies to *Fallout 4*. It instructs Papyrus compiler to use final mode (*"-final"*), which
removes all betaOnly function calls and optimizes the output, supposedly reducing the output size.
