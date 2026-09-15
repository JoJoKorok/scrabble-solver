# Installation and troubleshooting

Scrabble Solver produces separate packages for 64-bit Windows and Debian. The
Windows package includes GTK and its runtime files; the Debian package uses GTK
provided by the operating system.

| Platform | Package | Supported setup |
|---|---|---|
| Windows x86-64 | Portable ZIP | Extract and run in place |
| Debian 13 amd64 | `.deb` package | Install with APT |
| Debian 12 or another Linux distribution | Source build | Follow the build instructions in the README |

Packages attached to a tagged GitHub release are intended for normal use.
Packages downloaded from a GitHub Actions run are temporary development builds
from that specific commit. An Actions artifact may be a ZIP containing the
actual Windows ZIP or Debian package, so extract the downloaded artifact once
before following the platform instructions below.

## Windows portable package

1. Download `scrabble-solver-<version>-windows-x86_64.zip`.
2. Extract the complete archive to a folder you can keep, such as a folder
   under `Documents` or `Applications`.
3. Open the extracted folder, open `bin`, and run `scrabble_solver.exe`.

Keep the entire extracted directory together. Moving only the executable will
separate it from the bundled GTK libraries, dictionary, stylesheet, and other
runtime data it needs.

The portable package is not currently code-signed, so Windows may display a
reputation warning for a newly downloaded build. Confirm that the file came
from this project's GitHub repository before deciding whether to run it.

To update, close the application and replace the old extracted directory with
the directory from the newer archive. To uninstall, delete the extracted
directory. The saved dictionary preference is stored separately at
`%LOCALAPPDATA%\scrabble-solver\settings.ini`.

## Debian package

The prebuilt Debian package targets Debian 13 on 64-bit Intel or AMD systems.
From the directory containing the downloaded package, install it with APT so
required GTK libraries are resolved automatically:

```sh
sudo apt install ./scrabble-solver_<version>-1_amd64.deb
```

Launch **Scrabble Solver** from the desktop application menu or run:

```sh
scrabble_solver
```

Install a newer package with the same `apt install ./...` command. Remove the
application with:

```sh
sudo apt remove scrabble-solver
```

Removing the package does not delete the saved dictionary preference under
`${XDG_CONFIG_HOME:-~/.config}/scrabble-solver/settings.ini`.

For Debian 12, another Linux distribution, or a development checkout, use the
[source-build instructions](../README.md#build-on-debian).

## Dictionary setup

The application starts with its bundled ENABLE 2K word-game dictionary, so no
separate dictionary download is required. This list intentionally differs from
current commercial and tournament dictionaries.

To use a compatible word list:

1. Select **Choose file** in the Dictionary section.
2. Choose a plain-text file containing one word per line.
3. Confirm that the Dictionary section shows the selected filename and a
   nonzero word count.

Words must contain 1–15 ASCII letters. Case is ignored; unsupported lines and
duplicates are skipped. Select **Use bundled** to discard the saved custom path
and return to ENABLE 2K.

## Common problems

### The Windows download does not contain an executable

If the download came from GitHub Actions, first extract the outer artifact ZIP.
Then extract the `scrabble-solver-<version>-windows-x86_64.zip` inside it. The
executable is under the resulting `bin` directory.

### Windows reports a missing DLL or application data

Extract the full portable archive again and leave its directory structure
unchanged. Do not copy `scrabble_solver.exe` out of `bin` or mix files from two
different builds.

### Debian reports unmet dependencies

Use `apt install ./package-name.deb`, including the leading `./`, rather than
installing with `dpkg -i`. APT can obtain the GTK dependencies declared by the
package. If APT cannot find them, confirm that the system is Debian 13 amd64 and
that its configured package repositories are enabled.

### A word is rejected or missing from the results

Check the dictionary name displayed in the application. ENABLE 2K is not an
official NASPA or Collins list, so its accepted words can differ. A custom list
must follow the format described above.

Also confirm that **Your Rack** contains only your current tiles, using `?` or
`*` for a blank. Keep every visible play synchronized on the board. Use
**Record opponent move** for the other player's word; opponent tiles are
validated from the board and dictionary without using your rack.

### Suggestions do not match the physical board

Use **Undo** to remove the most recently recorded turn, then enter it again at
the correct first square and direction. If several earlier turns are wrong,
select **New game** and reconstruct the board in order. Board state is kept for
the current application session and is not restored after the app closes.

### A saved custom dictionary no longer loads

The file may have moved, been renamed, or become unavailable. Select another
file or choose **Use bundled**. If the saved path cannot be cleared in the
interface, close the app and remove its `settings.ini` file at the platform path
listed above; the bundled dictionary will be used on the next launch.

## Reporting a problem

When opening a [bug report](https://github.com/JoJoKorok/scrabble-solver/issues/new/choose),
include the operating system, package filename or commit, displayed dictionary,
rack, relevant board position, and the exact error message. A screenshot is
helpful for board or layout problems.
