# Requirements #

To compile the program, you will need

  * [GTK+](http://www.gtk.org/)
  * [Libglade2](http://www.jamesh.id.au/software/libglade/)

Most Linux distributions have these as development packages. In Ubuntu, for example, you have to install `libgtk2.0-dev` and `libglade2-dev`:

```
  sudo apt-get install libgtk2.0-dev libglade2-dev
```

# Compilation #

To compile, enter the source directory and type

```
  make
```

This will output the program executable, `wbfs_gtk`.

# Installation #

You don't need to install the program to use it.

But if you want, simply copy the executable to some directory in your path. For example:

```
  sudo cp wbfs_gtk /usr/local/bin
```

(assuming `/usr/local/bin` is in your path).