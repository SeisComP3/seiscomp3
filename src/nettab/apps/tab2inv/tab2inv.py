#!/usr/bin/env python

import sys
from optparse import OptionParser
from nettab.tab import Tab
from seiscomp3 import IO

def main():
	# Creating the parser
	parser = OptionParser(usage="Tab to Inventory (sc3) converter", version="1.0", add_help_option=True)

	parser.add_option("-i", "--ip", type="string",
					  help="Prefix to be added to each instrument generated.", dest="instrumentPrefix", default=None)

	parser.add_option("-f", "--filterf", type="string",
					  help="Indicates a folder containing the filters coefficients files", dest="ffolder", default=None)

	parser.add_option("-x", "--xmlf", type="string",
					  help="Indicates a folder containing the XML inventory files (needed for station group support)", dest="xfolder", default=None)

	parser.add_option("-D", "--database", type="string",
					  help="Database URL for inventory (needed for station group support)", dest="database", default=None)

	parser.add_option("", "--force", action="store_true",
					help="Don't stop on error of individual files", dest="force", default=False)
	
	parser.add_option("-g", "--generate", action="store_true",
					help="Generate XML file at the end", dest="generate", default=False)

	parser.add_option("-c", "--check", action="store_true",
					help="Check the loaded files", dest="check", default=False)

	parser.add_option("-d", "--default", type="string",
					help="Indicates the default file", dest="defaultFile", default=None)

	parser.add_option("-o", "--output", type="string",
					help="Indicates the output file", dest="outFile", default="-")

	# Parsing & Error check
	(options, args) = parser.parse_args()
	error = False
	
	if len(args) < 1:
		print("No input file(s) to digest", file=sys.stderr)
		error = True
	
	if error:
		print("Use -h for help on usage", file=sys.stderr)
		return 1

	# Execution
	try:
		inv = None
		t=Tab(options.instrumentPrefix, options.defaultFile, options.ffolder, options.xfolder, options.database)
		for f in args:
			try:
				t.digest(f)
			except Exception as e:
				print("Error digesting %s:\n %s" % (f, e), file=sys.stderr)
				if not options.force:
					raise e

		if options.check:
			t.check()
			return
		
		if options.generate:
			inv = t.sc3Obj()
			if inv:
				ar = IO.XMLArchive()
				print("Generating file: %s" % options.outFile, file=sys.stderr)
				ar.create(options.outFile)
				ar.setFormattedOutput(True)
				ar.setCompression(False)
				ar.writeObject(inv)
				ar.close()
	except Exception as e:
		print("Error: " + str(e), file=sys.stderr)
		return 1
	finally: 
		print("Ending.", file=sys.stderr)

	return 0

if __name__ == "__main__":
	ret = main()
	sys.exit(ret)
