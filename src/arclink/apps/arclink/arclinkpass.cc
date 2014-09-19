#include <iostream>
#include "encrypterror.h"
#include "encryptpasswordhandle.h"
#include <termios.h>

// This are from the getopt (unistd)
extern int optind, opterr;
extern char *optarg;

const char *VERSION = "Version 0.1";

namespace {
	int getch() {
		int ch;
		struct termios t_old, t_new;
		
		tcgetattr(STDIN_FILENO, &t_old);
		t_new = t_old;
		t_new.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &t_new);
		
		ch = getchar();
		
		tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
		return ch;
	}

	std::string getpass(const char *prompt, bool show_asterisk)
	{
		const char BACKSPACE=127;
		const char RETURN=10;
		
		std::string password;
		password.clear();
		unsigned char ch=0;
		
		std::cerr << prompt << std::ends;
		
		while((ch=getch())!=RETURN)
		{
			if(ch==BACKSPACE)
			{
				if(password.length()!=0)
				{
					if(show_asterisk)
						std::cerr <<"\b \b";
					password.resize(password.length()-1);
				}
			}
			else
			{
				password+=ch;
				if(show_asterisk)
					std::cerr <<'*';
			}
		}
		std::cerr <<std::endl;
		return password;
	}
}

void showHelp() {
	std::cout << "Arclinkpass is a password file utility for ArcLink server (" << VERSION << ")" << std::endl;
	std::cout << std::endl << " This tool can export (dump) a password table from the input file, convert from";
	std::cout << std::endl << " one password to a new one, or extract one specified password givin a user";
	std::cout << std::endl << " id from the file."  << std::endl;
	std::cout << std::endl << " It always read from a file and write to the stdout.";
	std::cout << std::endl << " Password will be asked during exection." << std::endl;

	std::cout << std::endl << "Valid Options:" << std::endl;
	std::cout << "  -h\t\t: " << "Show this help message" << std::endl;
	std::cout << "  -f <filename>\t: " << "Supply the input password filename" << std::endl;
	std::cout << "  -d\t\t: " << "Dump mode" << std::endl;
	std::cout << "  -c\t\t: " << "Convert mode" << std::endl;
	std::cout << "  -s <User Id> \t: " << "Search mode " << std::endl;
}

int main(int argc, char **argv){
	std::string filename;
	std::string person; 

	bool dumpMode    = false;
	bool convertMode = false;
	bool searchMode  = false;
	int op = 0; 

	// turn of error from getopt
	opterr = 0;

	// initialization
	filename.clear();
	person.clear();

	while(int c = getopt(argc, argv, "hdcs:f:")) {
		if ( c == EOF ) break;
		if (c == 'd') {
			dumpMode = true;
			op ++;
			continue;
		} else if (c == 'c') {
			convertMode = true;
			op ++;
			continue;
		} else if (c == 's') {
			searchMode = true;
			person.append(optarg);
			op ++;
			continue;
		} else if (c == 'f') {
			filename.append(optarg);
			continue;
		} else if (c == 'h') {
			showHelp();
			return 0;
		}

		std::cerr << "Invalid option " << argv[optind -1]	 << std::endl;
		return 1;
	}

	// Check options are consistent
	if ( op == 0 ) {
		std::cerr << "No action request, nothing to do." << std::endl << std::endl;
		showHelp();
		return 1;
	}

	if ( op > 1 ) {
		std::cerr << "Can only do one operation at a time." << std::endl;
		return 1;
	}

	if ( filename.length() == 0 ) {
		std::cerr << "No filename provided." << std::endl;
		return 1;
	}

	// Setup needed passwords
	std::string oldPassword;
	std::string newPassword;

	oldPassword = getpass("Please provide the current password: ", true);

	if (convertMode) {
		newPassword = getpass("Please provide the password for the new file: ", true);

		// Check that password is at least one char
		if (newPassword.length() < 1) {
			std::cerr << "Passwords supplied is too small." << std::endl;
			return 1;
		}

		// Check that the user can retype the password
		std::string newnewpassword = getpass("Please re-type the password: ", true);
		if (newPassword != newnewpassword) {
			std::cerr << "Passwords supplied does not match." << std::endl;
			return 1;
		}
	}

	// Get the handler
	SSLWrapper::EncryptPasswordHandle handler = SSLWrapper::EncryptPasswordHandle(filename, oldPassword);

	try {
		// Search
		if (searchMode) {
			std::cout << "Password for user " << person << " is '" << handler.findPassword(person) << "'" << std::endl;

		// Dump
		} else if (dumpMode) {
			std::vector< std::pair<std::string, std::string> > ids =  handler.loadIds();
			for(uint i = 0; i < ids.size(); i++)
				std::cout << ids[i].first << ":" << ids[i].second << std::endl;

		// Convert
		} else if (convertMode) {
			std::vector< std::pair<std::string, std::string> > ids =  handler.loadIds();
			SSLWrapper::EncryptPasswordHandle newHandler = SSLWrapper::EncryptPasswordHandle("/dev/null", newPassword);
			for(uint i = 0; i < ids.size(); i++)
				std::cout << ids[i].first << ":" << newHandler.encrypt(ids[i].second) << std::endl;
		}
	} catch (SSLWrapper::EncryptError e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
