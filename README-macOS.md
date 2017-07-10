# Seiscomp3 for macOS compilation notes

Succesfully compiled and tested on Mac OS X 10.11 & macOS Sierra 10.12.5 with Xcode 8.3 and gfortran 6.3

If everything compiled fine, seiscomp3 will be installed in your Home Directory: ${HOME}/seiscomp3

## Pre-Requirements

- Apple Xcode 8.x or later (clang command-line installation works or download Xcode from the App Store)
- GNU GFortran
- homebrew package manager for macOS

### Install Apple's Xcode 

You can choose to install the developer tools via command-line only (saves space),
or download the whole Xcode IDE (8GB or more) from Apple's App Store.

The command-line works pretty well, so open up `Terminal.app` from Applications > Utilities and type:
 `xcode-select --install`

### Install GFortran

Install GNU Fortran by downloading the latest installer for macOS here:
https://gcc.gnu.org/wiki/GFortranBinaries

### Install Homebrew
Homebrew is the missing package manager for Mac: [Homebrew Webpage here] (http://brew.sh)

In Terminal.app copy/paste this command:

`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`

All the dependencies packages will be installed in /usr/local/Cellar/
	
Check if your system is correctly setup with:
`brew doctor`

Note: If you have Macports package manager installed it's better to not to mix up with Homebrew.
or you could rename Macports default directory /opt/local to /opt/local.OFF

### Install seiscomp3 dependencies

With Homebrew installed now, we just need to install seiscomp3 dependencies packages
```
brew install cmake
brew install boost
brew install mysql
brew install flex
brew install openssl
brew install md5sha1sum
```

### Install qt4 with Homebrew

`brew install cartr/qt4/qt`

Note: Qt 4.x is no longer officialy supported by Homebrew - by default Qt 5.x is installed.
[See homebrew-qt4 page] (https://github.com/cartr/homebrew-qt4)


### Configure MySQL at startup

Copy default MYSQL configuration file to /usr/local/etc/
`sudo cp $(brew --prefix mysql)/support-files/my-default.cnf /etc/my.cnf`

For better performance with a MySQL database, adjust the following parameters in /etc/my.cnf

```
innodb_buffer_pool_size = 64M
innodb_flush_log_at_trx_commit = 2
```

To have launchd start mysql now and restart at login:
`brew services start mysql`

Recommended: MySQL Workbench application
MySQL Workbench from Oracle is a free GUI to administer MySQL databases.
[Install MySQL Workbench from Oracle's website] (https://dev.mysql.com/downloads/workbench/)

### Build seiscomp3 for macOS

Now we are ready to compile seiscomp3 for macOS with GUI manually:

```
cd seiscomp3-jakarta-macos-src
mkdir build-seiscomp3-osx
cd build-seiscomp3-osx   
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/seiscomp3 -DSC_GLOBAL_GUI=ON ../seiscomp3-jakarta-macos-src
make -j 2
make install
```

Note: If you have installed Python versions with Homebrew and want to use this Python installation, rather than the System's default, then use cmake like this (e.g with python 2.7):

`cmake -DCMAKE_INSTALL_PREFIX=${HOME}/seiscomp3 -DSC_GLOBAL_GUI=ON -DPYTHON_EXECUTABLE:FILEPATH=/usr/local/bin/python -DPYTHON_LIBRARY=/usr/local/Cellar/python/2.7.13/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib ../seiscomp3-jakarta-macos-src`

If everything compiled fine, the files will be installed in ${HOME}/seiscomp3
 

### Increase max open files for seedlink on System Startup

To avoid getting seedlink errors when starting seiscomp3 with "files open exceed max files ...",
increase the max open files on system's startup.

Create a plist file named: `limit.seiscomp3-maxfiles.plist` with this content:

```
<?xml version="1.0" encoding="UTF-8"?>  
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"  
       "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">  
<dict>
<key>Label</key>
<string>limit.seiscomp3-maxfiles</string>
<key>ProgramArguments</key>
<array>
     <string>launchctl</string>
     <string>limit</string>
     <string>maxfiles</string>
     <string>524288</string>
     <string>524288</string>
</array>
   <key>RunAtLoad</key>
   <true/>
   <key>ServiceIPC</key>
   <false/>
 </dict>
</plist>  
```

Place this plist file in /Library/LaunchDaemons/

Then set root:wheel permission with command:
`sudo chown root:wheel /Library/LaunchDaemons/limit.seiscomp3-maxfiles.plist`

Launch it with command:
`sudo launchctl load -w /Library/LaunchDaemons/limit.seiscomp3-maxfiles.plist`

### Check current seiscomp3 configuration
http://www.seiscomp3.org/doc/jakarta/current/apps/global.html

