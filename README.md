# xCHM

UNIX CHM viewer.

## Prerequisites

In order to be able to compile the code you need to have wxWidgets and
CHMLIB installed.

Get the GTK+ version (also known as wxGTK). xCHM currently compiles
with all flavours of wxWidgets, but only works well and looks truly
appealing with wxGTK, so I recommend against bothering to use it with
anything else.

## Installing

Type `./configure --help` if you're interested in parameters you can
pass to configure for compile switches. Most people will be perfectly
happy doing a:

```
./configure && make
```

Once everything is built, type

```
make install
```

and the executable should be somewhere in your `$PATH` (hopefully :)).
Now you can just type

```
xchm
```

and start your session.

## Build with

* [wxWidgets](http://www.wxwidgets.org) - GUI
* [CHMLIB](http://www.jedrea.com/chmlib/) - CHM access logic

## Generating documentation

If you'd like to generate developer documentation go to the root
of the distribution and type
```
doxygen
```
This of course implies that you have doxygen installed. The documentation
will be generated in the doc directory in HTML and LaTex format. The doc
directory will be created in the process of creating the documentation so
it's ok that it's not present by default in the tarball.

## Authors

* **Razvan Cojocaru** [rzvncj](https://github.com/rzvncj)

## Happy reading!
