/*===========================================================================================================================
    Name:       dataless.C

    Purpose:    synchronization of GDI and SeisComp

    Language:   C++, ANSI standard.

    Author:     Peter de Boer, KNMI

    Revision:	2007-11-26	0.1	initial version

===========================================================================================================================*/
#include <fstream>
#include "dataless.h"

#include <seiscomp3/core/exceptions.h>

#define SEISCOMP_COMPONENT sync_dlsv
#include <seiscomp3/logging/log.h>

using namespace std;

/*******************************************************************************
* Function:     SynchronizeDataless                                            *
* Parameters:   inventory to merge, string name of the file to process         *
*               and output filename                                            *
* Returns:      nothing                                                        *
* Description:  this function calls CreateCommand, SendCommand and ProcessFiles*
********************************************************************************/
bool Dataless::SynchronizeDataless(Seiscomp::DataModel::Inventory *inv,
                                   const std::string &dataless)
{
	SEISCOMP_INFO("START PROCESSING DATALESS");

	invent = new Inventory(_dcid, _net_description, _net_type, _net_start, _net_end,
	                       _temporary, _restricted, _shared, inv);
	invent->vic = new VolumeIndexControl();
	invent->adc = new AbbreviationDictionaryControl();
	invent->sc = new StationControl();
	return ParseDataless(dataless);
}

/*******************************************************************************
* Function:     ParseDataless                                                  *
* Parameters:   inventory to merge and name of the file to process             *
* Returns:      nothing                                                        *
* Description:  reads the file and processes every 4096 bytes of metadata      *
********************************************************************************/
bool Dataless::ParseDataless(const string &file)
{
	char buf[LRECL];
	int pos1, pos2;
	ifstream dataless(file.c_str());
	
	try
	{
		if(dataless.is_open())
		{
			while(dataless.read(buf, LRECL))
			{
				pos1 = pos2 = 0;
				string record(buf);
				string volume = SplitString(record, LINE_SEPARATOR, pos1, pos2);
				if(volume.size()!=7)
				{
					pos1 = pos2 = 0;
					volume = SplitString(record, '*', pos1, pos2);
				}
				pos2++;
				if(volume[volume.size()-1] == 'V')
				{
					invent->vic->ParseVolumeRecord(record.substr(pos2, (LRECL-pos2)));
				}
				else if(volume[volume.size()-1] == 'A')
				{ 
					invent->adc->ParseVolumeRecord(record.substr(pos2, (LRECL-pos2)));
				}
				else if(volume[volume.size()-1] == 'S')
				{
					invent->sc->ParseVolumeRecord(record.substr(pos2, (LRECL-pos2)));
				}
			}

			invent->sc->Flush();

			invent->SynchronizeInventory();
			invent->vic->EmptyVectors();
			invent->adc->EmptyVectors();
			invent->sc->EmptyVectors();

			if ( invent->fixedErrors() > 0 ) {
				std::cerr << "********************************************************************************" << std::endl;
				std::cerr << "* WARNING!                                                                     *" << std::endl;
				std::cerr << "*------------------------------------------------------------------------------*" << std::endl;
				std::cerr << "* Errors found in input dataless SEED which were fixed by the conversion. This *" << std::endl;
				std::cerr << "* may lead to subsequent errors or undefined behaviour. Check and correct the  *" << std::endl;
				std::cerr << "* errors in dataless SEED and do the conversion again.                         *" << std::endl;
				std::cerr << "********************************************************************************" << std::endl;
			}

			return true;
		}
		else
		{
			SEISCOMP_ERROR("Cannot open %s", file.c_str());
		}
	}
	catch(BadConversion &o)
	{
		SEISCOMP_ERROR("Bad conversion: %s", o.what());
	}
	catch(out_of_range &o)
	{
		SEISCOMP_ERROR("Out of range: %s", o.what());
	}
	catch(length_error &o)
	{
		SEISCOMP_ERROR("Length error: %s", o.what());
	}
	catch(Seiscomp::Core::ValueException &o)
	{
		SEISCOMP_ERROR("Value error: %s", o.what());
	}
	catch(Seiscomp::Core::GeneralException &o)
	{
		SEISCOMP_ERROR("Error: %s", o.what());
	}
	catch(...)
	{
		SEISCOMP_ERROR("An unknown error has occurred");
	}

	return false;
}
