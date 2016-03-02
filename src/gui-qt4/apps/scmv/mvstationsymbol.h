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


#ifndef __MVSTATIONSYMBOL_H___
#define __MVSTATIONSYMBOL_H___

#include <seiscomp3/gui/datamodel/stationsymbol.h>


class MvStationSymbol : public Seiscomp::Gui::StationSymbol {
	DECLARE_RTTI;

	public:
		MvStationSymbol(Seiscomp::Gui::Map::Decorator* decorator = NULL);
		MvStationSymbol(double latitude,
		                double longitude,
		                Seiscomp::Gui::Map::Decorator* decorator = NULL);

	public:
		void setIdDrawingColor(const QColor& color);
		void setIdDrawingEnabled(bool val);
		bool isIdDrawingEnabled() const;
		void setDrawFullID(bool);

		void setCharacter(const QChar& c);
		void setCharacterDrawingColor(const QColor& color);
		void setCharacterDrawingEnabled(bool val);
		bool isCharacterDrawingEnabled() const;

		const std::string& networkCode() const;
		void setNetworkCode(const std::string& networkCode);

		const std::string& stationCode() const;
		void setStationCode(const std::string& stationCode);

		const std::string& locationCode() const;
		void setLocationCode(const std::string& locationCode);

		const std::string& channelCode() const;
		void setChannleCode(const std::string& channelCode);

	protected:
		virtual void customDraw(const Seiscomp::Gui::Map::Canvas *canvas, QPainter& painter);

	private:
		void init();

		void drawID(QPainter& painter);
		void drawCharacter(QPainter& painter);

	private:
		std::string _networkCode;
		std::string _stationCode;
		std::string _locationCode;
		std::string _channelCode;

		QColor      _idDrawingColor;
		bool        _isDrawingIdEnabled;
		bool        _drawFullId;

		QChar       _char;
		QColor      _characterDrawingColor;
		bool        _isCharacterDrawingEnabled;
};

#endif
