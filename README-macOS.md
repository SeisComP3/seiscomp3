# Seiscomp3 for Mac OS X installation notes
## Pre-Requirements (Xcode, GFortran, homebrew)
Compiled on Mac OS X El Capitan 10.12.5 with Xcode 8.3 and gfortran 6.3

### Install Xcode from command-line
Open up Terminal.app and type:
 `xcode-select --install`

### Install GFortran
https://gcc.gnu.org/wiki/GFortranBinaries

### Install Homebrew
`/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
Webpage here:
	http://brew.sh

All the packages will be installed in /usr/local/Cellar

	
Check if your system is correctly setup with:
`brew doctor`
	
## Install seiscomp3 dependencies

```
brew install cmake
brew install boost
brew install mysql
brew install flex
brew install openssl
brew install md5sha1sum
```

Install qt4 with Homebrew
Note: Qt4 is no longer officialy supported by Homebrew - by default it installs qt5

`brew install cartr/qt4/qt`


### Install MySQL 

Copy default MYSQL configuration file to /usr/local/etc/
`sudo cp $(brew --prefix mysql)/support-files/my-default.cnf /etc/my.cnf`

## Build seiscomp3 for OS X
```
mkdir build-seiscomp3-osx
cd build-seiscomp3-osx   
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/seiscomp3 ../seiscomp3-src-osx
make
make install
```

Note: If you have installed python from Homebrew and want to use this python installation rather than the system's default, then use cmake like this (e.g with python 2.7):

`cmake -DCMAKE_INSTALL_PREFIX=${HOME}/seiscomp3 -DSC_GLOBAL_GUI=ON -DPYTHON_EXECUTABLE:FILEPATH=/usr/local/bin/python -DPYTHON_LIBRARY=/usr/local/Cellar/python/2.7.13/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib ../seiscomp3-src-osx`

If everything compiled fine, the files will be installed in ${HOME}/seiscomp3
 

## Increase max open files for seedlink on System Startup

To avoid getting seedlink errors when starting seiscomp with "files open exceed max files ..." we need to increase the max open files, we need to set it at startup.

```
Place this plist file in /Library/LaunchDaemons/
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

Set permission:
`sudo chown root:wheel /Library/LaunchDaemons/limit.seiscomp3-maxfiles.plist`
Launch it:
`sudo launchctl load -w /Library/LaunchDaemons/limit.seiscomp3-maxfiles.plist`

    
