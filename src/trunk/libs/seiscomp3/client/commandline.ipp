/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

template <typename T>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description, T* storage,
                            bool storageAsDefault) {
	options_description* o = findGroup(group);
	if ( o ) {
		if ( storageAsDefault && storage )
			(options_description_easy_init(o))(option, boost::program_options::value<T>(storage)->default_value(*storage), description);
		else
			(options_description_easy_init(o))(option, boost::program_options::value<T>(storage), description);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description,
                            std::vector<T>* storage) {
	options_description* o = findGroup(group);
	if ( o )
		// NOTE: Because of a bug in boost 1.33.1 the option multitoken() eats up
		//       all arguments until the end of the commandline. So multitoken works
		//       only for switches at the end of the commandline. To make it work
		//       multitoken is disabled. So one has to give the option multiple times.
		//       e.g.: 'test -i foo -i bar -j' instead of 'test -i foo bar -j'
		//(options_description_easy_init(o))(option, boost::program_options::value<std::vector<T> >(&storage)->multitoken(), description);
		(options_description_easy_init(o))(option, boost::program_options::value<std::vector<T> >(storage), description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, typename DT>
void CommandLine::addOption(const char* group, const char* option,
                            const char* description, T* storage,
                            const DT& defaultValue) {
	options_description* o = findGroup(group);
	if ( o )
		(options_description_easy_init(o))(option, boost::program_options::value<T>(storage)->default_value(defaultValue), description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void CommandLine::addCustomOption(const char* group,
                                  const char* option,
                                  const char* description,
                                  T* customValidator) {
	options_description* o = findGroup(group);
	if ( o )
		(options_description_easy_init(o))(option, customValidator, description);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
inline T CommandLine::option(const std::string& option) const {
	try {
		return boost::any_cast<T>(_variableMap[option].value());
	}
	catch ( ... ) {
		throw Core::TypeException("Invalid type for cast");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int LEN>
inline T CommandLine::option(const char (&option)[LEN]) const {
	return this->option<T>(std::string(option));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
