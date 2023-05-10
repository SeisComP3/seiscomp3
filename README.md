> **Note**  
> This project is archived and will not longer be supported. The successor of SeisComP3 is **SeisComP** which
> is being continued at https://github.com/seiscomp. The current homepage can be found at https://www.seiscomp.de.

# About

Project homepage: http://www.seiscomp3.org

This software has been developed by the [GEOFON Program](http://geofon.gfz-potsdam.de) at [Helmholtz Centre Potsdam, GFZ German Research Centre for Geosciences](http://www.gfz-potsdam.de) and [gempa GmbH](http://www.gempa.de).

SeisComP3 is distributed under the [SeisComP Public License](COPYING)

> **Note**

> - The purpose of this repository is to test upcoming features and to
>   integrate community source code and patches
> - For production systems only use the official releases from http://www.seiscomp3.org or the corresponding tags in this repository.
> - Commercial modules obtained from [gempa GmbH](http://www.gempa.de) are only
>   available for official releases. Binary compatibility of intermediate
>   SeisComP3 versions is not guaranteed.


# Compiling

The easiest way to compile SeisComP3 is to use the provided Makefile.cvs which
creates the build directory inside the source tree.

```
$ make -f Makefile.cvs
$ cd build
$ make
$ make install
```

By default all files are installed under $HOME/seiscomp3. This location can be
changed with cmake or with its frontend ccmake.

Basically the build directory can live anywhere. The following steps create
a build directory, configure the build and start it:

```
$ mkdir sc3-build
$ cd sc3-build
$ ccmake /path/to/sc3-src
# Configure with ccmake
$ make
$ make install
```

## Step-by-step instructions

1. Checkout SeisComP3 source code from Github

   ```
   sysop@host:~$ git clone https://github.com/SeisComP3/seiscomp3.git sc3-src
   sysop@host:~$ cd sc3-src
   sysop@host:~/sc3-src$
   ```

2. Change into the desired branch (if not master) or checkout tag
   ```
   sysop@host:~/sc3-src$ git checkout release/jakarta/2017.124.02
   ```

3. Configure the build

   SeisComP3 is using cmake as build environment. For users that are not experienced
   with cmake it is recommended to use `ccmake`, an ncurses frontend which is launched
   by the default `Makefile.cvs`.
   
   ```
   sysop@host:~/sc3-src$ make -f Makefile.cvs
   ```
   
   This will bring up the cmake frontend. Press `c` to configure the build initially.
   If cmake is being used, the variables can be passed as command line options:

   ```
   sysop@host:~/sc3-src/build$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/dir ..
   ```

   With ccmake some components can be activated and deactivated such as database
   backends you want to compile support for. The default just enables MySQL. Once done
   with options, press `c` again to apply the changes. If everything runs without errors,
   press `g` to generate the Makefiles. `ccmake` will quit if the Makefiles have been
   generated:
   
   ```
   *** To build the sources change into the 'build' directory and enter make[ install] ***
   sysop@host:~/sc3-src$ cd build
   sysop@host:~/sc3-src/build$ make
   ```
   
   If `make` finished without errors, install SeisComP3 with
   
   ```
   sysop@host:~/sc3-src/build$ make install
   ```
   
   All files are then installed under `~/seiscomp3` or under the directory you have
   specified with ```CMAKE_INSTALL_PREFIX```.
  

# Dependencies

To compile the sources the following development packages are required (Redhat/CentOS package names):

- flex
- libxml2-devel
- boost-devel
- openssl-devel
- ncurses-devel
- mysql-devel
- postgresql-devel (optional)
- python-devel
- m2crypto-devel
- qt4-devel
