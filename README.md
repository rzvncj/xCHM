# xCHM

UNIX CHM viewer.

## Screenshots

![Python documentation](./art/screenshot.png)

## Prerequisites

In order to be able to compile the code you need to have wxWidgets and
CHMLIB installed.

Get the GTK+ version (also known as wxGTK). xCHM currently compiles
with all flavours of wxWidgets, but only works well and looks truly
appealing with wxGTK, so I recommend against bothering to use it with
anything else.

## Installing

(If you've just cloned the code - as opposed to using a release tarball -
you need to run `./bootstrap` first.)

Type `./configure --help` if you're interested in parameters you can
pass to configure for compile switches. Most people will be perfectly
happy doing a:

```
./configure && make
```

Once everything is built, type:

```
make install
```

and the executable should be somewhere in your `$PATH` (hopefully :)).
Now you can just type

```
xchm
```

and start your session.

## Built with

* [wxWidgets](http://www.wxwidgets.org) - GUI
* [CHMLIB](http://www.jedrea.com/chmlib/) - CHM access logic

## Generating documentation

If you'd like to generate developer documentation go to the root
of the distribution and type:

```
doxygen
```

This of course implies that you have doxygen installed. The documentation
will be generated in the doc directory in HTML and LaTex format.

## Binaries

I am no longer providing Mac and Windows binaries, just the source code.
Some Linux distributions already provide binaries ([Debian](https://packages.debian.org/search?searchon=names&keywords=xchm),
[Arch](https://archlinux.org/packages/extra/x86_64/xchm/),
[Gentoo](https://packages.gentoo.org/packages/app-text/xchm),
[Ubuntu](https://packages.ubuntu.com/search?keywords=xchm), etc.)
For Mac, please see [MacPorts](https://www.macports.org/ports.php?by=name&substr=xchm).
Windows users can (and should) just use the standard viewer (hh.exe).

## Authors

* **Răzvan Cojocaru** [rzvncj](https://github.com/rzvncj)

## Happy reading!
