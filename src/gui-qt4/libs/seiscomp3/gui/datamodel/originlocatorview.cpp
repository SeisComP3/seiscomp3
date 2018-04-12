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


#define SEISCOMP_COMPONENT Gui::OriginLocatorView

#include "originlocatorview.h"
#include <seiscomp3/gui/core/diagramwidget.h>
#include <seiscomp3/gui/core/locator.h>
#include <seiscomp3/gui/core/gradient.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/gui/core/tensorrenderer.h>
#include <seiscomp3/gui/datamodel/pickerview.h>
#include <seiscomp3/gui/datamodel/importpicks.h>
#include <seiscomp3/gui/datamodel/locatorsettings.h>
#include <seiscomp3/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/databasearchive.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/seismology/locsat.h>
#include <seiscomp3/math/conversions.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/utils/misc.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/processing/magnitudeprocessor.h>

#include "ui_mergeorigins.h"
#include "ui_renamephases.h"
#include "ui_originlocatorview_commit.h"
#include "ui_originlocatorview_comment.h"

#include <algorithm>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;


namespace Seiscomp {
namespace Gui {

namespace {

const int UsedRole = Qt::UserRole + 1;
const int HoverRole = Qt::UserRole + 2;

MAKEENUM(
	ArrivalListColumns,
	EVALUES(
		USED,
		STATUS,
		PHASE,
		WEIGHT,
		METHOD,
		POLARITY,
		TAKEOFF,
		NETWORK,
		STATION,
		CHANNEL,
		RESIDUAL,
		DISTANCE,
		AZIMUTH,
		TIME,
		SLOWNESS,
		BACKAZIMUTH,
		UNCERTAINTY,
		CREATED,
		LATENCY
	),
	ENAMES(
		"Used",
		"Status",
		"Phase",
		"Weight",
		"Method",
		"Polarity",
		"Takeoff",
		"Net",
		"Sta",
		"Loc/Cha",
		"Res",
		"Dis",
		"Az",
		"Time",
		"Baz",
		"Slo",
		"+/-",
		"Created",
		"Latency"
	)
);


MAKEENUM(
	PlotTabs,
	EVALUES(
		PT_DISTANCE,
		PT_AZIMUTH,
		PT_TRAVELTIME,
		PT_MOVEOUT,
		PT_POLAR,
		PT_FM
	),
	ENAMES(
		"Distance",
		"Azimuth",
		"TravelTime",
		"MoveOut",
		"Polar",
		"FirstMotion"
	)
);


MAKEENUM(
	PlotCols,
	EVALUES(
		PC_DISTANCE,
		PC_RESIDUAL,
		PC_TRAVELTIME,
		PC_AZIMUTH,
		PC_REDUCEDTRAVELTIME,
		PC_POLARITY,
		PC_FMAZI,
		PC_FMDIST
	),
	ENAMES(
		"Distance",
		"Residual",
		"TravelTime",
		"Azimuth",
		"MoveOut",
		"Polarity"
		"FMAzimuth",
		"FMDistance"
	)
);



QVariant colAligns[ArrivalListColumns::Quantity] = {
	QVariant(),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter)
};


bool colVisibility[ArrivalListColumns::Quantity] = {
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false
};

inline int getMask(const QModelIndex &index) {
	int mask = 0;
	if ( index.sibling(index.row(), BACKAZIMUTH).data().isValid() )
		mask |= Seismology::LocatorInterface::F_BACKAZIMUTH;
	if ( index.sibling(index.row(), SLOWNESS).data().isValid() )
		mask |= Seismology::LocatorInterface::F_SLOWNESS;
	if ( index.sibling(index.row(), TIME).data().isValid() )
		mask |= Seismology::LocatorInterface::F_TIME;

	return mask;
}


void getRects(QList<QRect> &rects, const QStyleOptionViewItem &option,
              int labelWidth, int statusWidth, int spacing) {
	QStyle *style = qApp->style();
	int checkBoxWidth = style->subElementRect(QStyle::SE_CheckBoxIndicator,
	                                          &option).width();

	QRect statusRect = option.rect;
	statusRect.setWidth(statusWidth);

	QRect checkboxRect = statusRect;
	checkboxRect.translate(statusRect.width() + spacing, 0);
	checkboxRect.setWidth(checkBoxWidth);

	QRect rect = checkboxRect;
	rect.translate(checkboxRect.width() + spacing, 0);
	rect.setWidth(labelWidth);

	for ( int i = 0; i < 3; ++i ) {
		rects.push_back(rect);
		rect.translate(rect.width() + spacing, 0);
	}

	rects.append(statusRect);
	rects.append(checkboxRect);
}


class ArrivalsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		ArrivalsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( (left.column() == RESIDUAL && right.column() == RESIDUAL) ||
			     (left.column() == DISTANCE && right.column() == DISTANCE) ||
			     (left.column() == AZIMUTH && right.column() == AZIMUTH) ||
			     (left.column() == LATENCY && right.column() == LATENCY) ||
			     (left.column() == TAKEOFF && right.column() == TAKEOFF) ||
			     (left.column() == WEIGHT && right.column() == WEIGHT) )
				return sourceModel()->data(left, Qt::UserRole).toDouble() <
				       sourceModel()->data(right, Qt::UserRole).toDouble();
			else
				return QSortFilterProxyModel::lessThan(left, right);
		}
};


void setBold(QWidget *w, bool b = true) {
	QFont f = w->font();
	f.setBold(b);
	w->setFont(f);
}

void setItalic(QWidget *w, bool i = true) {
	QFont f = w->font();
	f.setItalic(i);
	w->setFont(f);
}



class RenamePhases : public QDialog {
	public:
		RenamePhases(QWidget *parent = 0, Qt::WindowFlags f = 0)
		: QDialog(parent, f) {
			ui.setupUi(this);
		}

	public:
		Ui::RenamePhases ui;
};


class CommitOptions : public QDialog {
	public:
		CommitOptions(QWidget *parent = 0, Qt::WindowFlags f = 0)
		: QDialog(parent, f) {
			ui.setupUi(this);

			// Fill event types
			ui.comboEventTypes->addItem("- unset -");
			for ( int i = (int)EventType::First; i < (int)EventType::Quantity; ++i ) {
				if ( (EventType::Type)i == NOT_EXISTING )
					ui.comboEventTypes->insertItem(1, EventType::NameDispatcher::name(i));
				else
					ui.comboEventTypes->addItem(EventType::NameDispatcher::name(i));
			}

			EventType defaultType = EARTHQUAKE;
			ui.comboEventTypes->setCurrentIndex(ui.comboEventTypes->findText(defaultType.toString()));

			ui.comboEventTypeCertainty->addItem("- unset -");
			for ( int i = (int)EventTypeCertainty::First; i < (int)EventTypeCertainty::Quantity; ++i )
				ui.comboEventTypeCertainty->addItem(EventTypeCertainty::NameDispatcher::name(i));
			ui.comboEventTypeCertainty->setCurrentIndex(0);

			ui.comboOriginStates->addItem("- unset -");
			for ( int i = (int)EvaluationStatus::First; i < (int)EvaluationStatus::Quantity; ++i )
				ui.comboOriginStates->addItem(EvaluationStatus::NameDispatcher::name(i));
		}

	public:
		Ui::OriginCommitOptions ui;
};


class CommentEdit : public QDialog {
	public:
		CommentEdit(QWidget *parent = 0, Qt::WindowFlags f = 0)
		: QDialog(parent, f) {
			ui.setupUi(this);
		}

	public:
		Ui::OriginCommentOptions ui;
};


class NodalPlaneDialog : public QDialog {
	public:
		NodalPlaneDialog(QWidget *parent = 0) : QDialog(parent) {
			resize(184, 144);

			setWindowTitle("Nodal plane");

			QVBoxLayout *vboxLayout = new QVBoxLayout(this);
			QGridLayout *gridLayout = new QGridLayout();
			QLabel *label = new QLabel("Strike", this);
			QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(4), static_cast<QSizePolicy::Policy>(5));
			sizePolicy.setHorizontalStretch(0);
			sizePolicy.setVerticalStretch(0);
			sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
			label->setSizePolicy(sizePolicy);

			gridLayout->addWidget(label, 0, 0, 1, 1);

			QLabel *label_2 = new QLabel("Dip", this);

			gridLayout->addWidget(label_2, 1, 0, 1, 1);

			QLabel *label_3 = new QLabel("Rake", this);

			gridLayout->addWidget(label_3, 2, 0, 1, 1);

			sbStrike = new QSpinBox(this);
			sbStrike->setSuffix(" deg");
			sbStrike->setObjectName(QString::fromUtf8("sbStrike"));
			sbStrike->setAlignment(Qt::AlignRight);
			sbStrike->setMaximum(360);
			sbStrike->setMinimum(-360);

			gridLayout->addWidget(sbStrike, 0, 1, 1, 1);

			sbDip = new QSpinBox(this);
			sbDip->setSuffix(" deg");
			sbDip->setObjectName(QString::fromUtf8("sbDip"));
			sbDip->setAlignment(Qt::AlignRight);
			sbDip->setMaximum(360);
			sbDip->setMinimum(-360);

			gridLayout->addWidget(sbDip, 1, 1, 1, 1);

			sbRake = new QSpinBox(this);
			sbRake->setSuffix(" deg");
			sbRake->setObjectName(QString::fromUtf8("sbRake"));
			sbRake->setAlignment(Qt::AlignRight);
			sbRake->setMaximum(360);
			sbRake->setMinimum(-360);

			gridLayout->addWidget(sbRake, 2, 1, 1, 1);

			vboxLayout->addLayout(gridLayout);

			QSpacerItem *spacerItem = new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

			vboxLayout->addItem(spacerItem);

			QHBoxLayout *hboxLayout = new QHBoxLayout();
			QSpacerItem *spacerItem1 = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

			hboxLayout->addItem(spacerItem1);

			QPushButton *okButton = new QPushButton(this);
			okButton->setText(QApplication::translate("", "OK", 0, QApplication::UnicodeUTF8));

			hboxLayout->addWidget(okButton);

			QPushButton *cancelButton = new QPushButton(this);
			cancelButton->setText(QApplication::translate("", "Cancel", 0, QApplication::UnicodeUTF8));

			hboxLayout->addWidget(cancelButton);

			vboxLayout->addLayout(hboxLayout);

			QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
			QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		}

	public:
		QSpinBox *sbStrike;
		QSpinBox *sbDip;
		QSpinBox *sbRake;
};


QPointF equalarea(double azi, double dip) {
	dip = 90-dip;
	if ( dip > 90 ) {
		dip = 180 - dip;
		azi -= 180;
	}

	double z = sqrt(2)*sin(0.5*deg2rad(dip));
	return QPointF(z, azi);
}


class PlotWidget : public OriginLocatorPlot {
	public:
		enum ShapeType {
			ST_CIRCLE,
			ST_TRIANGLE,
			ST_TRIANGLE2,
			ST_RECT,
			ST_CROSS
		};

		enum PolarityType {
			POL_POSITIVE,
			POL_NEGATIVE,
			POL_UNDECIDABLE,
			POL_UNSET,
			POL_QUANTITY
		};

		struct Shape {
			Shape() : shown(false) {}

			Shape(ShapeType t, int s)
			: type(t), fillUsed(false), shown(true) {
				setSize(s);
			}

			Shape(ShapeType t, int s, const QBrush &f)
			: type(t), fillUsed(true), fill(f), shown(true) {
				setSize(s);
			}

			Shape(ShapeType t, int s, const QBrush &f, const QPen &st)
			: type(t), fillUsed(true), fill(f), stroke(st), shown(true) {
				setSize(s);
			}

			void setSize(int s) {
				size = s;

				if ( type == ST_TRIANGLE ) {
					poly = QPolygonF() << QPointF(-1,0.85)
									   << QPointF(1,0.85)
									   << QPointF(0,-0.85);
				}
				else if ( type == ST_TRIANGLE2 ) {
					poly = QPolygonF() << QPointF(1,-0.85)
									   << QPointF(-1,-0.85)
									   << QPointF(0,0.85);
				}
				else
					poly.clear();

				if ( !poly.isEmpty() ) {
					qreal hsize = size*0.5;
					for ( int i = 0; i < poly.size(); ++i )
						poly[i] *= hsize;
				}
			}


			void init(const string &param) {
				vector<string> toks;
				try {
					toks = SCApp->configGetStrings(param);
				}
				catch ( ... ) {
					return;
				}

				shown = true;
				poly.clear();

				if ( toks.size() > 0 ) {
					if ( toks[0] == "circle" )
						type = ST_CIRCLE;
					else if ( toks[0] == "triangle" )
						type = ST_TRIANGLE;
					else if ( toks[0] == "triangle2" )
						type = ST_TRIANGLE2;
					else if ( toks[0] == "rectangle" )
						type = ST_RECT;
					else if ( toks[0] == "cross" )
						type = ST_CROSS;
					else if ( toks[0] == "none" ) {
						shown = false;
						return;
					}
					else {
						SEISCOMP_WARNING("%s: wrong shape shape type: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
				}

				if ( toks.size() > 1 ) {
					if ( !Core::fromString(size, toks[1]) || size < 0 ) {
						SEISCOMP_WARNING("%s: invalid size: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
				}

				setSize(size);

				if ( toks.size() > 2 ) {
					QColor color;
					if ( toks[2] == "none" ) {
						fill = Qt::NoBrush;
						fillUsed = true;
					}
					else if ( !fromString(color, toks[2]) ) {
						SEISCOMP_WARNING("%s: wrong color definition: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
					else {
						fill = color;
						fillUsed = true;
					}
				}

				if ( toks.size() > 3 ) {
					QColor color;
					if ( toks[3] == "none" ) {
						stroke = Qt::NoPen;
					}
					else if ( !fromString(color, toks[3]) ) {
						SEISCOMP_WARNING("%s: wrong color definition: drawing disabled",
						                 param.c_str());
						shown = false;
						return;
					}
					else
						stroke = color;
				}
			}


			void draw(QPainter &p) const {
				if ( !shown ) return;
				p.setPen(stroke);
				drawWithoutStroke(p);
			}


			void drawWithoutStroke(QPainter &p) const {
				switch ( type ) {
					case ST_CIRCLE:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawEllipse(-size/2,-size/2,size,size);
						break;
					case ST_CROSS:
						if ( fillUsed )
							p.setPen(fill.color());
						p.drawLine(-size/2,-size/2,size/2,size/2);
						p.drawLine(-size/2,size/2,size/2,-size/2);
						break;
					case ST_RECT:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawRect(-size/2,-size/2,size,size);
						break;
					case ST_TRIANGLE:
					case ST_TRIANGLE2:
						if ( fillUsed )
							p.setBrush(fill);
						p.drawPolygon(poly);
						break;
				};
			}


			ShapeType type;
			int       size;
			bool      fillUsed;
			QBrush    fill;
			QPen      stroke;
			bool      shown;
			QPolygonF poly;
		};


		enum StationNameMode {
			SNM_OFF,
			SNM_OUTWARDS,
			SNM_INWARDS
		};


	public:
		PlotWidget(QWidget *parent = 0, ArrivalModel *model = 0)
		: OriginLocatorPlot(parent), _model(model), _commitButton(this) {
			//_renderer.setTColor(QColor(224,224,224));
			//_renderer.setShadingEnabled(true);
			_dragStarted = false;
			_customDraw = false;
			_drawStationNames = SNM_OFF;
			_dirty = false;
			_preferredTensorDirty = false;
			_preferredTensorValid = false;
			_showPreferredFM = true;

			_npLabel = new QLabel(this);
			//_npLabel->setCursor(Qt::PointingHandCursor);
			_npLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
			_npLabel->setVisible(_customDraw);

			_commitButton.setVisible(_customDraw);
			_commitButton.setText("C");
			_commitButton.setToolTip("Confirm and send the current solution.");
			_commitButton.setMenu(new QMenu(this));
			QAction *commitWithMTAction = _commitButton.menu()->addAction("With MT sololution...");

			connect(_npLabel, SIGNAL(linkActivated(const QString &)),
			        this, SLOT(linkClicked()));
			connect(&_commitButton, SIGNAL(clicked(bool)),
			        this, SLOT(commitButtonClicked(bool)));
			connect(commitWithMTAction, SIGNAL(triggered(bool)),
			        this, SLOT(commitWithMTTriggered(bool)));

			set(90,90,0);

			_renderer.setShadingEnabled(true);
			_renderer.setMaterial(0.8,0.2);

			// Negative
			_shapes[POL_POSITIVE] = Shape(ST_CIRCLE, 10);
			_shapes[POL_POSITIVE].init("olv.fmplot.shape.polarity.positive");
			_shapes[POL_NEGATIVE] = Shape(ST_CIRCLE, 10, Qt::NoBrush);
			_shapes[POL_NEGATIVE].init("olv.fmplot.shape.polarity.negative");
			_shapes[POL_UNDECIDABLE] = Shape(ST_CROSS, 10);
			_shapes[POL_UNDECIDABLE].init("olv.fmplot.shape.polarity.undecidable");
			_shapes[POL_UNSET] = Shape();
			_shapes[POL_UNSET].init("olv.fmplot.shape.polarity.unset");

			_shapeAxis[0] = Shape(ST_TRIANGLE, 14, Qt::red, QPen(Qt::black));
			_shapeAxis[0].init("olv.fmplot.shape.t-axis");
			_shapeAxis[1] = Shape(ST_TRIANGLE, 14, Qt::NoBrush, QPen(Qt::red));
			_shapeAxis[1].init("olv.fmplot.shape.p-axis");
		}


		const Shape &shape(PolarityType type) {
			return _shapes[type];
		}


		void resetPreferredFM() {
			_preferredFMBuffer = QImage();
			_preferredTensor = Math::Tensor2Sd();
			_preferredTensorValid = false;
			update();
		}


		void setPreferredFM(const Math::Tensor2Sd &t) {
			_preferredTensor = t;
			_preferredTensorValid = true;
			_preferredTensorDirty = true;
			update();
		}


		void setPreferredFM(double str, double dip, double rake) {
			NODAL_PLANE np;
			np.str = str;
			np.dip = dip;
			np.rake = rake;

			Math::np2tensor(np, _preferredTensor);
			setPreferredFM(_preferredTensor);
		}


		void setCustomDraw(bool f) {
			_dragStarted = false;
			_customDraw = f;
			_dirty = false;
			_preferredTensorDirty = false;
			_npLabel->setVisible(_customDraw);
			_commitButton.setVisible(_customDraw);
		}


		void set(double str, double dip, double rake) {
			_np1.str = str;
			_np1.dip = dip;
			_np1.rake = rake;

			Math::np2tensor(_np1, _tensor);
			set(_tensor);
		}

		void set(const Math::Tensor2Sd &tensor) {
			_tensor = tensor;
			Math::Spectral2Sd spec;
			Math::Vector3d tn, td;

			spec.spect(_tensor);
			spec.sort();

			Math::AXIS t,n,p;
			Math::spectral2axis(spec, t,n,p, 0);
			Math::pa2nd(spec.n1, spec.n3, tn, td);
			Math::nd2dc(tn, td, &_np1, &_np2);

			_tAxis = equalarea(t.str, t.dip);
			_pAxis = equalarea(p.str, p.dip);

			_npLabel->setText(QString("NP1: <a href=\"np1\">%1/%2/%3</a> "
			                          "NP2: <a href=\"np2\">%4/%5/%6</a>")
			                  .arg(_np1.str, 0, 'f', 0)
			                  .arg(_np1.dip, 0, 'f', 0)
			                  .arg(_np1.rake, 0, 'f', 0)
			                  .arg(_np2.str, 0, 'f', 0)
			                  .arg(_np2.dip, 0, 'f', 0)
			                  .arg(_np2.rake, 0, 'f', 0));

			Math::Matrix3f tmp;
			Math::tensor2matrix(_tensor, tmp);

			_renderer.setPColor(palette().color(QPalette::Base));
			_renderer.setTColor(palette().color(QPalette::AlternateBase));
			_renderer.setBorderColor(palette().color(QPalette::WindowText));
			_renderer.render(_buffer, _tensor, tmp);

			update();
		}

		const NODAL_PLANE &np1() const {
			return _np1;
		}

		const NODAL_PLANE &np2() const {
			return _np2;
		}


	protected:
		void updateContextMenu(QMenu &menu) {
			if ( !_customDraw ) return;
			menu.addSeparator();
			QMenu *subShowStations = menu.addMenu("Draw station names");
			QAction *act1 = subShowStations->addAction("Outwards");
			act1->setCheckable(true);
			act1->setChecked(_drawStationNames == SNM_OUTWARDS);
			act1->setData(1001);
			QAction *act2 = subShowStations->addAction("Inwards");
			act2->setCheckable(true);
			act2->setChecked(_drawStationNames == SNM_INWARDS);
			act2->setData(1002);
			QAction *act3 = subShowStations->addAction("Off");
			act3->setCheckable(true);
			act3->setChecked(_drawStationNames == SNM_OFF);
			act3->setData(1000);

			QActionGroup *group = new QActionGroup(&menu);
			group->addAction(act1);
			group->addAction(act2);
			group->addAction(act3);

			QAction *act = menu.addAction("Enable shading");
			act->setCheckable(true);
			act->setChecked(_renderer.isShadingEnabled());

			act = menu.addAction("Show preferred solution (if available)");
			act->setCheckable(true);
			act->setChecked(_showPreferredFM);
		}

		void handleContextMenuAction(QAction *action) {
			OriginLocatorPlot::handleContextMenuAction(action);
			if ( action == NULL ) return;
			if ( action->data().toInt() == 1000 ) {
				_drawStationNames = SNM_OFF;
				update();
			}
			else if ( action->data().toInt() == 1001 ) {
				_drawStationNames = SNM_OUTWARDS;
				update();
			}
			else if ( action->data().toInt() == 1002 ) {
				_drawStationNames = SNM_INWARDS;
				update();
			}
			else if ( action->text() == "Enable shading" ) {
				_renderer.setShadingEnabled(action->isChecked());
				_dirty = true;
				_preferredTensorDirty = true;
				update();
			}
			else if ( action->text() == "Show preferred solution (if available)" ) {
				_showPreferredFM = action->isChecked();
				update();
			}
		}

		void linkClicked() {
			NodalPlaneDialog dlg(this);
			dlg.sbStrike->setValue(_np1.str);
			dlg.sbDip->setValue(_np1.dip);
			dlg.sbRake->setValue(_np1.rake);
			if ( dlg.exec() != QDialog::Accepted ) return;
			set(dlg.sbStrike->value(), dlg.sbDip->value(),dlg.sbRake->value());
		}

		void commitButtonClicked(bool) {
			emit focalMechanismCommitted();
		}

		void commitWithMTTriggered(bool) {
			QMenu *m = _commitButton.menu();
			emit focalMechanismCommitted(true, m->mapToGlobal(QPoint()));
		}

		void mousePressEvent(QMouseEvent *event) {
			if ( !_customDraw ) {
				DiagramWidget::mousePressEvent(event);
				return;
			}

			if ( event->button() != Qt::LeftButton ) return;

			if ( hoveredValue() != -1 ) {
				emit clicked(hoveredValue());
				return;
			}

			if ( event->pos().y() < diagramRect().top() ) return;

			_startDragPos = event->pos();
			_dragStarted = true;
		}


		void mouseReleaseEvent(QMouseEvent *event) {
			DiagramWidget::mouseReleaseEvent(event);
			if ( event->button() != Qt::LeftButton ) return;
			_dragStarted = false;
		}


		void mouseMoveEvent(QMouseEvent *event) {
			if ( !_customDraw || !_dragStarted ) {
				DiagramWidget::mouseMoveEvent(event);
				return;
			}

			QPoint delta = event->pos() - _startDragPos;

			double deltaX = (double)delta.x() / (double)diagramRect().width();
			double deltaY = (double)delta.y() / (double)diagramRect().height();

			Math::Matrix3d m, mx, my;
			mx.loadRotateY(deg2rad(-deltaY * 180));
			my.loadRotateX(deg2rad(-deltaX * 180));
			m.mult(mx, my);

			_tensor.rotate(m);
			set(_tensor);

			_startDragPos = event->pos();

			update();
		}


		void diagramAreaUpdated(const QRect &rect) {
			_dirty = true;
			_preferredTensorDirty = true;
			_npLabel->setGeometry(0,0,width(),diagramRect().top());
			_commitButton.move(width()-_commitButton.width(),_npLabel->height()+4);
		}


		void paintSphericalBackground(QPainter &painter) {
			if ( !_customDraw ) {
				DiagramWidget::paintSphericalBackground(painter);
				return;
			}

			if ( _dirty ) {
				_buffer = QImage(diagramRect().size(), QImage::Format_ARGB32);
				Math::Matrix3f m;
				Math::tensor2matrix(_tensor, m);
				_renderer.setPColor(palette().color(QPalette::Base));
				_renderer.setTColor(palette().color(QPalette::AlternateBase));
				_renderer.setBorderColor(palette().color(QPalette::WindowText));
				_renderer.render(_buffer, _tensor, m);
				_dirty = false;
			}

			if ( _preferredTensorDirty && _preferredTensorValid && _showPreferredFM ) {
				_preferredFMBuffer = QImage(diagramRect().size(), QImage::Format_ARGB32);
				Math::Matrix3f m;
				Math::tensor2matrix(_preferredTensor, m);
				_renderer.setPColor(palette().color(QPalette::Base));
				_renderer.setTColor(palette().color(QPalette::Highlight));
				_renderer.setBorderColor(palette().color(QPalette::WindowText));
				_renderer.render(_preferredFMBuffer, _preferredTensor, m);

				// Blend complete buffer 25%
				uchar *data = _preferredFMBuffer.bits();
				for ( int y = 0; y < _preferredFMBuffer.height(); ++y ) {
					QRgb *rgb = (QRgb*)data;
					for ( int x = 0; x < _preferredFMBuffer.width(); ++x, ++rgb )
						*rgb = (*rgb & 0x00FFFFFF) | (((*rgb >> 24)*1/4) << 24);
					data += _preferredFMBuffer.bytesPerLine();
				}
			}

			painter.drawImage(diagramRect().topLeft(), _buffer);

			if ( _showPreferredFM )
				painter.drawImage(diagramRect().topLeft(), _preferredFMBuffer);

			QRectF tmp(_displayRect);
			_displayRect.setRight(1);

			QPoint p;

			painter.setRenderHint(QPainter::Antialiasing, true);

			// Draw T Axis
			p = (this->*project)(_tAxis);
			painter.translate(p);
			_shapeAxis[0].draw(painter);
			painter.translate(-p);

			// Draw P Axis
			p = (this->*project)(_pAxis);
			painter.translate(p);
			_shapeAxis[1].draw(painter);
			painter.translate(-p);

			_displayRect = tmp;

			painter.setRenderHint(QPainter::Antialiasing, false);
		}


		void drawValue(int id, QPainter& painter, const QPoint& p,
		               SymbolType type, bool valid) const {
			if ( _customDraw ) {
				if ( valid ) {
					painter.setRenderHint(QPainter::Antialiasing);

					int pol = value(id, PC_POLARITY);

					painter.translate(p.x(), p.y());

					_shapes[pol].drawWithoutStroke(painter);

					if ( _drawStationNames != SNM_OFF && _model ) {
						double az = value(id, PC_FMAZI);
						QString sta =
							_model->data(_model->index(id, STATION), Qt::DisplayRole).toString();
						painter.setPen(Qt::black);
						if ( az >= 0 && az <= 180 ) {
							painter.rotate(az-90);
							if ( _drawStationNames == SNM_INWARDS )
								painter.drawText(-(_shapes[pol].size/2+2)-width(),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignRight, sta);
							else
								painter.drawText((_shapes[pol].size/2+2),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignLeft, sta);

							painter.rotate(90-az);
						}
						else {
							painter.rotate(az+90);
							if ( _drawStationNames == SNM_INWARDS )
								painter.drawText((_shapes[pol].size/2+2),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignLeft, sta);
							else
								painter.drawText(-(_shapes[pol].size/2+2)-width(),-_textHeight,width(),_textHeight*2,
								                 Qt::AlignVCenter | Qt::AlignRight, sta);
							painter.rotate(-az-90);
						}
					}

					painter.translate(-p.x(), -p.y());
				}
			}
			else
				DiagramWidget::drawValue(id, painter, p, type, valid);
		}


	private:
		Shape             _shapes[POL_QUANTITY];
		Shape             _shapeAxis[2];
		ArrivalModel     *_model;
		QToolButton       _commitButton;
		QLabel           *_npLabel;
		QImage            _buffer;
		QImage            _preferredFMBuffer;
		QPointF           _tAxis, _pAxis;
		TensorRenderer    _renderer;
		bool              _customDraw;
		StationNameMode   _drawStationNames;
		bool              _dirty;
		Math::Tensor2Sd   _tensor;
		Math::NODAL_PLANE _np1, _np2;

		bool              _showPreferredFM;
		bool              _preferredTensorDirty;
		bool              _preferredTensorValid;
		Math::Tensor2Sd   _preferredTensor;

		QPoint            _startDragPos;
		bool              _dragStarted;
};


std::string wfid2str(const DataModel::WaveformStreamID &id) {
	return id.networkCode() + "." + id.stationCode() + "." +
	       id.locationCode();
}

QString wfid2qstr(const DataModel::WaveformStreamID &id) {
	return QString("%1.%2.%3.%4")
	       .arg(id.networkCode().c_str())
	       .arg(id.stationCode().c_str())
	       .arg(id.locationCode().c_str())
	       .arg(id.channelCode().c_str());
}


typedef std::pair<std::string, std::string> PickPhase;
typedef std::pair<PickPtr, int> PickWithFlags;
typedef std::map<PickPhase, PickWithFlags> PickedPhases;


}


ArrivalDelegate::ArrivalDelegate(QWidget *parent = NULL)
: QStyledItemDelegate(parent)
, _margin(2), _spacing(4), _statusRectWidth(6), _labelWidth(0) {
	_flags[0] = Seismology::LocatorInterface::F_TIME;
	_flags[1] = Seismology::LocatorInterface::F_SLOWNESS;
	_flags[2] = Seismology::LocatorInterface::F_BACKAZIMUTH;

	_labels[0] = "T";
	_labels[1] = "S";
	_labels[2] = "B";

	if ( parent )
		_statusRectWidth = parent->fontMetrics().width('A');
}

bool ArrivalDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) {
	if ( index.column() != USED )
		return QStyledItemDelegate::editorEvent(event, model, option, index);

	if ( event->type() == QEvent::MouseButtonPress ||
	     event->type() == QEvent::MouseButtonDblClick ) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		if ( mouseEvent->buttons() & Qt::LeftButton ) {
			QPoint pos = mouseEvent->pos();

			bool ret = QStyledItemDelegate::editorEvent(event, model, option, index);

			QList<QRect> rects;
			getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

			if ( rects[4].contains(pos) ) {
				int flags = (Qt::CheckState)index.data(UsedRole).toInt(),
				    mask = getMask(index);
				if ( flags == 0 )
					model->setData(index, mask, UsedRole);
				else
					model->setData(index, 0, UsedRole);
			}
			else {
				int flags = index.data(UsedRole).toInt(),
				    mask = getMask(index);
				for ( int i = 0; i < 3; ++i ) {
					bool enabled = mask & _flags[i];
					if ( !enabled ) continue;

					if ( rects[i].contains(pos) ) {
						if ( flags & _flags[i] )
							flags &= ~_flags[i];
						else
							flags |= _flags[i];

						model->setData(index, flags, UsedRole);
					}
				}
			}

			return ret;
		}
	}
	else if ( event->type() == QEvent::MouseMove ) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
		QPoint pos = mouseEvent->pos();

		QList<QRect> rects;
		getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

		int hover = -1,
		    mask = getMask(index);
		for ( int i = 0; i < 3; ++i ) {
			bool enabled = mask & _flags[i];
			if ( !enabled ) continue;

			if ( rects[i].contains(pos) ) {
				hover = i;
				break;
			}
		}

		model->setData(index, hover, HoverRole);

		return false;
	}

	return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool ArrivalDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) {
	if ( index.column() != USED )
		return QStyledItemDelegate::helpEvent(event, view, option, index);

	if ( event->type() == QEvent::ToolTip ) {
		QPoint pos = event->pos();

		QList<QRect> rects;
		getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

		if ( rects[4].contains(pos) ) {
			QToolTip::showText(event->globalPos(),
			                   tr("Toggle if arrival should be used or not."),
			                   view);
			return true;
		}
		else {
			static const char *FlagNames[3] = {"time", "slowness", "backazimuth"};

			int mask = getMask(index);

			for ( int i = 0; i < 3; ++i ) {
				if ( !rects[i].contains(pos) ) continue;

				bool enabled = mask & _flags[i];
				if ( !enabled ) {
					QToolTip::showText(event->globalPos(),
					                   tr("The pick does not have a %1 value and the usage flag is therefore disabled.")
					                   .arg(FlagNames[i]),
					                   view);
					return true;
				}

				QToolTip::showText(event->globalPos(),
				                   tr("Toggle %1 usage.")
				                   .arg(FlagNames[i]),
				                   view);
				return true;
			}
		}
	}

	return QStyledItemDelegate::helpEvent(event, view, option, index);
}

void ArrivalDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const {
	if ( index.column() != USED ) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	painter->save();

	QPen pen = painter->pen();
	if ( option.state & QStyle::State_Selected ) {
		painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
		pen.setColor(option.palette.color(QPalette::HighlightedText));
	}
	else {
		pen.setColor(option.palette.color(QPalette::WindowText));
	}

	QList<QRect> rects;
	getRects(rects, option, _labelWidth, _statusRectWidth, _spacing);

	QRect statusRect = rects[3];//.center().x() - 4, rects[3].center().y() - 4, 9, 9);

	painter->fillRect(statusRect, index.data(Qt::BackgroundRole).value<QColor>());

	QStyleOptionButton boxStyle;
	boxStyle.state = QStyle::State_Enabled;

	int flags = index.data(UsedRole).toInt(),
	    mask = getMask(index);

	flags &= mask;

	if( flags == mask ) {
		boxStyle.state |= QStyle::State_On;
	}
	else if ( flags ) {
		boxStyle.state |= QStyle::State_NoChange;
	}
	else {
		boxStyle.state |= QStyle::State_Off;
	}

	boxStyle.direction = QApplication::layoutDirection();
	boxStyle.rect = rects[4];
	QApplication::style()->drawControl(QStyle::CE_CheckBox, &boxStyle, painter);

	int hoverIndex = index.data(HoverRole).toInt();

	for ( int i = 0; i < 3; ++i ) {
		if ( i == hoverIndex && option.state & QStyle::State_MouseOver ) {
			QFont font = option.font;
			font.setBold(true);
			font.setPointSize(font.pointSize() + 2);
			painter->setFont(font);
		}

		bool enabled = mask & _flags[i];
		bool checked = (flags & _flags[i]) && enabled;
		if ( !enabled )
			painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
		else
			painter->setPen(pen);

		painter->drawText(rects[i], Qt::AlignVCenter | Qt::AlignHCenter,
		                  checked?_labels[i]:"-");
		painter->setFont(option.font);
	}

	painter->restore();
}

QSize ArrivalDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
	if ( index.column() != USED ) {
		return QStyledItemDelegate::sizeHint(option, index);
	}

	QFont font = option.font;
	font.setBold(true);

	_labelWidth = 0;

	font.setPointSize(font.pointSize() + 2);

	QFontMetrics fm(font);
	int labelHeight = 0;
	for ( int i = 0; i < 3; ++i ) {
		QRect rect = fm.boundingRect(_labels[i]);
		_labelWidth = max(_labelWidth, rect.width());
		labelHeight = max(labelHeight, rect.height());
	}

	QStyle *style = qApp->style();
	int checkBoxWidth = style->subElementRect(QStyle::SE_CheckBoxIndicator,
	                                          &option).width();

	int width = 2 * _margin + _statusRectWidth + 3 * _labelWidth +
	            4 * _spacing + checkBoxWidth,
	    height = 2 * _margin + max(labelHeight,
	                               option.decorationSize.height());

	return QSize(width, height);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Implementation of ArrivalModel

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ArrivalModel::ArrivalModel(DataModel::Origin* origin, QObject *parent)
 : QAbstractTableModel(parent) {

	_disabledForeground = Qt::gray;

	for ( int i = 0; i < ArrivalListColumns::Quantity; ++i )
		if ( i == DISTANCE ) {
			if ( SCScheme.unit.distanceInKM )
				_header << QString("%1 (km)").arg(EArrivalListColumnsNames::name(i));
			else
				_header << QString("%1 (deg)").arg(EArrivalListColumnsNames::name(i));
		}
		else if ( i == TIME ) {
			if ( SCScheme.dateTime.useLocalTime )
				_header << QString("%1 (%2)").arg(EArrivalListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
			else
				_header << QString("%1 (UTC)").arg(EArrivalListColumnsNames::name(i));
		}
		else
			_header << EArrivalListColumnsNames::name(i);

	setOrigin(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setOrigin(DataModel::Origin* origin) {
	_pickTimeFormat = "%T.%";
	_pickTimeFormat += Core::toString(SCScheme.precision.pickTime);
	_pickTimeFormat += "f";

	_origin = origin;
	if ( _origin ) {
		_used.fill(Seismology::LocatorInterface::F_NONE, _origin->arrivalCount());
		_backgroundColors.fill(QVariant(), _origin->arrivalCount());
		_enableState.fill(true, _origin->arrivalCount());
		_takeOffs.fill(QVariant(), _origin->arrivalCount());
		_hoverState.fill(-1, _origin->arrivalCount());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setRowColor(int row, const QColor& c) {
	if ( row >= rowCount() ) return;
	_backgroundColors[row] = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ArrivalModel::rowCount(const QModelIndex &) const {
	return _origin?_origin->arrivalCount():0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ArrivalModel::columnCount(const QModelIndex &) const {
	return _header.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant ArrivalModel::data(const QModelIndex &index, int role) const {
	if ( !index.isValid() )
		return QVariant();

	if ( index.row() >= (int)_origin->arrivalCount() )
		return QVariant();

	if ( index.column() == USED ) {
		if ( role == UsedRole ) {
			return (int)_used[index.row()];
		}
		else if ( role == HoverRole ) {
			return _hoverState[index.row()];
		}
	}

	Arrival* a = _origin->arrival(index.row());
	Pick* pick;
	char buf[10];

	if ( role == Qt::DisplayRole ) {
		switch ( index.column() ) {
			case USED:
				/*
				try {
					snprintf(buf, 10, "   %.3f", a->weight());
					return buf;
				}
				catch ( Core::ValueException& ) {}
				break;
				*/
				return QVariant();

			case CREATED:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						return timeToString(pick->creationInfo().creationTime(), "%T.%1f");
					}
					catch ( ValueException& ) {}
				}
				break;

			case STATUS:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						const char *strStat = pick->evaluationMode().toString();

						if ( pick->methodID().empty() )
							return QString("%1").arg(strStat && *strStat?(char)toupper(*strStat):'-');
						else
							return QString("%1<%2>").arg(strStat && *strStat?(char)toupper(*strStat):'-')
							                        .arg((char)toupper(pick->methodID()[0]));
					}
					catch ( ValueException& ) {}
				}
				break;

			case LATENCY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						Core::TimeSpan lcy = pick->creationInfo().creationTime() - pick->time();
						long lcy_secs = lcy.seconds();
						return QString("%1:%2:%3")
						         .arg(lcy_secs/3600, 2, 10, QChar('0'))
						         .arg(lcy_secs/60, 2, 10, QChar('0'))
						         .arg(lcy_secs % 60, 2, 10, QChar('0'));
					}
					catch ( ValueException& ) {}
				}
				break;

			// Phase
			case PHASE:
				return a->phase().code().c_str();
				break;

			case WEIGHT:
				try {
					snprintf(buf, 10, "%.2f", a->weight());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			// Networkcode
			case NETWORK:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return pick->waveformID().networkCode().c_str();
				break;

			// Stationcode
			case STATION:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return pick->waveformID().stationCode().c_str();
				break;

			// Locationcode + Channelcode
			case CHANNEL:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					if ( pick->waveformID().locationCode().empty() )
						return pick->waveformID().channelCode().c_str();
					else
						return (pick->waveformID().locationCode() + '.' + pick->waveformID().channelCode()).c_str();
				}
				break;

			// Picktime
			case TIME:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick )
					return timeToString(pick->time().value(), _pickTimeFormat.c_str());
				break;

			case BACKAZIMUTH:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				try {
					if ( pick )
						return pick->backazimuth().value();
				}
				catch ( ValueException& ) {}
				break;

			case SLOWNESS:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				try {
					if ( pick )
						return pick->horizontalSlowness().value();
				}
				catch ( ValueException& ) {}

				break;

			// Picktime
			case UNCERTAINTY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						if ( pick->time().lowerUncertainty() == pick->time().upperUncertainty() )
							return QString("%1s").arg(pick->time().lowerUncertainty());
						else
							return QString("-%1s/+%2s")
							       .arg(pick->time().lowerUncertainty())
							       .arg(pick->time().upperUncertainty());
					}
					catch ( ... ) {}

					try {
						return QString("%1s").arg(pick->time().uncertainty());
					}
					catch ( ... ) {}
				}
				break;

			// Residual
			case RESIDUAL:
				try {
					snprintf(buf, 10, "%.2f", a->timeResidual());
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			// Distance
			case DISTANCE:
				try {
					double distance = a->distance();
					if ( SCScheme.unit.distanceInKM )
						snprintf(buf, 10, "%.*f", SCScheme.precision.distance, Math::Geo::deg2km(distance));
					else
						snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
					return buf;
				}
				catch ( ValueException& ) {}
				break;

			// Azimuth
			case AZIMUTH:
				try {
					return (int)a->azimuth();
				}
				catch ( ValueException& ) {}

			case METHOD:
				pick = Pick::Find(a->pickID());
				if ( pick )
					return pick->methodID().c_str();
				break;

			case POLARITY:
				pick = Pick::Find(a->pickID());
				if ( pick ) {
					try {
						return pick->polarity().toString();
					}
					catch ( ... ) {}
				}
				break;

			case TAKEOFF:
				try {
					snprintf(buf, 10, "%.1f", a->takeOffAngle());
					return buf;
				}
				catch ( ... ) {
					if ( _takeOffs[index.row()].canConvert(QVariant::Double) ) {
						snprintf(buf, 10, "%.1f", _takeOffs[index.row()].toDouble());
						return buf;
					}
					return _takeOffs[index.row()];
				}
				break;

			default:
				break;
		}
	}
	else if ( role == Qt::UserRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try { return fabs(a->timeResidual()); } catch ( ValueException& ) {}
				break;
			// Distance
			case DISTANCE:
				try { return a->distance(); } catch ( ValueException& ) {}
				break;
			case AZIMUTH:
				try { return a->azimuth(); } catch ( ValueException& ) {}
				break;
			case TAKEOFF:
				try {
					return a->takeOffAngle();
				}
				catch ( ValueException& ) {
					return _takeOffs[index.row()];
				}
				break;
			case WEIGHT:
				try { return a->weight(); } catch ( ValueException& ) {}
				break;
			case LATENCY:
				pick = Pick::Cast(PublicObject::Find(a->pickID()));
				if ( pick ) {
					try {
						Core::TimeSpan lcy = pick->creationInfo().creationTime() - pick->time();
						return (double)lcy;
					}
					catch ( ValueException& ) {}
				}
				break;
			default:
				break;
		}
	}
	else if ( role == Qt::BackgroundRole ) {
		switch ( index.column() ) {
			case USED:
				if ( index.row() < _backgroundColors.size() )
					return _backgroundColors[index.row()];
			default:
				return QVariant();
		}
	}
	else if ( role == Qt::ForegroundRole ) {
		if ( index.row() < _enableState.size() )
			return _enableState[index.row()]?QVariant():_disabledForeground;
	}
	/*
	else if ( role == Qt::TextColorRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try {
					return a->residual() < 0?Qt::darkRed:Qt::darkGreen;
				}
				catch ( ValueException& ) {}
				break;
			default:
				break;
		}
	}
	*/
	else if ( role == Qt::TextAlignmentRole ) {
		return colAligns[index.column()];
	}
	else if ( role == Qt::ToolTipRole ) {
		QString summary;
		int l = 0;
		pick = Pick::Cast(PublicObject::Find(a->pickID()));

		if ( pick ) {
			if ( l++ ) summary += '\n';
			summary += wfid2qstr(pick->waveformID());
			if ( l++ ) summary += '\n';
			summary += timeToString(pick->time().value(), _pickTimeFormat.c_str());
		}

		/*
		// Phase
		if (l++) summary += '\n';
		summary += "Phase: ";
		try { summary += a->phase().code().c_str(); }
		catch ( ... ) { summary += "-"; }

		// Distance
		if (l++) summary += '\n';
		summary += "Distance: ";
		try {
			double distance = a->distance();
			if ( SCScheme.unit.distanceInKM )
				snprintf(buf, 10, "%.*f", SCScheme.precision.distance, Math::Geo::deg2km(distance));
			else
				snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
			summary += buf;
		}
		catch ( ... ) {
			summary += '-';
		}

		if ( SCScheme.unit.distanceInKM )
			summary += "km";
		else
			summary += degrees;

		// Azimuth
		if (l++) summary += '\n';
		summary += "Azimuth: ";
		try { summary += QString("%1").arg((int)a->azimuth()); }
		catch ( ... ) { summary += '-'; }
		summary += degrees;

		// Takeoff
		if (l++) summary += '\n';
		summary += "Takeoff: ";
		try { summary += QString("%1").arg((int)a->takeOffAngle()); }
		catch ( ... ) { summary += '-'; }
		summary += degrees;

		// Residual
		if (l++) summary += '\n';
		summary += "Residual: ";
		try { summary += QString("%1").arg(a->timeResidual(), 0, 'f', 2); }
		catch ( ... ) { summary += '-'; }
		summary += 's';
		*/

		// Filter
		if ( pick ) {
			if ( !pick->filterID().empty() ) {
				if (l++) summary += '\n';
				summary += "Filter: ";
				summary += pick->filterID().c_str();
			}

			if ( !pick->methodID().empty() ) {
				if (l++) summary += '\n';
				summary += "Method: ";
				summary += pick->methodID().c_str();
			}

			try {
				const CreationInfo &ci = pick->creationInfo();
				if ( !ci.author().empty() ) {
					if (l++) summary += '\n';
					summary += "Author: ";
					summary += ci.author().c_str();
				}
				if ( !ci.agencyID().empty() ) {
					if (l++) summary += '\n';
					summary += "Agency: ";
					summary += ci.agencyID().c_str();
				}
			}
			catch ( ... ) {}
		}

		return summary;
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant ArrivalModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
	if ( orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::TextAlignmentRole:
				return colAligns[section];
				break;
			case Qt::DisplayRole:
				if ( section >= _header.count() )
					return QString("%1").arg(section);
				else
					return _header[section];
				break;
		}
	}
	else {
		return section;
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::ItemFlags ArrivalModel::flags(const QModelIndex &index) const {
	if ( !index.isValid() )
		return Qt::ItemIsEnabled;

	Qt::ItemFlags f = QAbstractTableModel::flags(index);

	/*if ( index.row() < _enableState.size() ) {
		if ( !_enableState[index.row()] ) {
			if ( index.column() == USED )
				f = f & ~(Qt::ItemIsUserCheckable);
		}
		else if ( index.column() == USED ) {
			f = (f | Qt::ItemIsUserCheckable);
		}
	}*/

	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::setData(const QModelIndex &index, const QVariant &value,
                           int role) {
	if ( index.isValid() && index.column() == USED ) {
		if ( !_enableState[index.row()] ) return false;

		if ( role == UsedRole ) {
			_used[index.row()] = value.toInt() & getMask(index);
		}
		else if (role == HoverRole ){
			_hoverState[index.row()] = value.toInt();
		}
		else
			return QAbstractTableModel::setData(index, value, role);

		emit dataChanged(index, index);
		return true;
	}

	return QAbstractTableModel::setData(index, value, role);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setRowEnabled(int row, bool enabled) {
	if ( row >= _enableState.size() ) return;
	_enableState[row] = enabled;
	emit dataChanged(index(row,0), index(row,columnCount()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::isRowEnabled(int row) const {
	return _enableState[row];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setTakeOffAngle(int row, const QVariant &val) {
	if ( row >= _takeOffs.size() ) return;
	_takeOffs[row] = val;
	emit dataChanged(index(row,TAKEOFF), index(row,TAKEOFF));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::useNoArrivals() const {
	for ( int i = 0; i < _used.count(); ++i ) {
		if ( _used[i] != Seismology::LocatorInterface::F_NONE )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::useArrival(int row) const {
	return _used[row] != Seismology::LocatorInterface::F_NONE;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setUseArrival(int row, DataModel::Arrival *arrival) {
	bool used = false;
	try {
		setBackazimuthUsed(row, arrival->backazimuthUsed());
		used = true;
	}
	catch ( ... ) {
		Pick *pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		try {
			if ( pick ) {
				pick->backazimuth().value();
				setBackazimuthUsed(row, true);
			}
			else
				setBackazimuthUsed(row, false);
		}
		catch ( ValueException& ) {
			setBackazimuthUsed(row, false);
		}
	}

	try {
		setHorizontalSlownessUsed(row, arrival->horizontalSlownessUsed());
		used = true;
	}
	catch ( ... ) {
		Pick *pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		try {
			if ( pick ) {
				pick->horizontalSlowness().value();
				setHorizontalSlownessUsed(row, true);
			}
			else
				setHorizontalSlownessUsed(row, false);
		}
		catch ( ValueException& ) {
			setHorizontalSlownessUsed(row, false);
		}
	}

	try {
		setTimeUsed(row, arrival->timeUsed());
		used = true;
	}
	catch ( ... ) {
		setTimeUsed(row, true);
	}

	// TODO check if this is really required for backward compatibility
	try {
		if ( !used && arrival->weight() < 0.5 ) {
			setBackazimuthUsed(row, false);
			setHorizontalSlownessUsed(row, false);
			setTimeUsed(row, false);
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::backazimuthUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;

	return _used[row] & Seismology::LocatorInterface::F_BACKAZIMUTH;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setBackazimuthUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled)
		_used[row] |= Seismology::LocatorInterface::F_BACKAZIMUTH;
	else
		_used[row] &= ~Seismology::LocatorInterface::F_BACKAZIMUTH;

	emit dataChanged(index(row, USED),
	                 index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::horizontalSlownessUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;

	return _used[row] & Seismology::LocatorInterface::F_SLOWNESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setHorizontalSlownessUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled)
		_used[row] |= Seismology::LocatorInterface::F_SLOWNESS;
	else
		_used[row] &= ~Seismology::LocatorInterface::F_SLOWNESS;

	emit dataChanged(index(row, USED),
	                 index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ArrivalModel::timeUsed(int row) const {
	if ( row < 0 || row >= rowCount() ) return false;


	return _used[row] & Seismology::LocatorInterface::F_TIME;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ArrivalModel::setTimeUsed(int row, bool enabled) {
	if ( row < 0 || row >= rowCount() ) return;

	if ( enabled)
		_used[row] |= Seismology::LocatorInterface::F_TIME;
	else
		_used[row] &= ~Seismology::LocatorInterface::F_TIME;

	emit dataChanged(index(row, USED), index(row, USED));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// Implementation of OriginLocatorView

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DiagramFilterSettingsDialog::DiagramFilterSettingsDialog(QWidget *parent) {
	_ui.setupUi(this);

	filterChanged(_ui.comboFilter->currentIndex());

	connect(_ui.comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(filterChanged(int)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DiagramFilterSettingsDialog::filterChanged(int idx) {
	_ui.frameNoFilter->setVisible(false);
	_ui.frameAzimuthAroundEpicenter->setVisible(false);

	switch ( idx ) {
		case 0:
			_ui.frameNoFilter->setVisible(true);
			break;
		case 1:
			_ui.frameAzimuthAroundEpicenter->setVisible(true);
			break;
	};
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct AzimuthFilter : DiagramFilterSettingsDialog::Filter {
	AzimuthFilter(double c, double e) : start(c-e), len(2*e) {
		while ( start < 0 ) start += 360;
		while ( start >= 360 ) start -= 360;
	}

	bool accepts(DiagramWidget *w, int id) {
		float v = w->value(id, PC_AZIMUTH);
		float l = v - start;
		while ( l < 0 ) l += 360;
		while ( l >= 360 ) l -= 360;
		return l <= len;
	}

	double start;
	double len;
};

}

DiagramFilterSettingsDialog::Filter *DiagramFilterSettingsDialog::createFilter() const {
	switch ( _ui.comboFilter->currentIndex() ) {
		default:
		case 0:
			break;
		case 1:
			return new AzimuthFilter(_ui.spinAzimuthCenter->value(), _ui.spinAzimuthExtent->value());
	};

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView::Config::Config() {
	reductionVelocityP = 6.0;
	drawMapLines = true;
	drawGridLines = true;
	computeMissingTakeOffAngles = false;
	defaultEventRadius = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView(const MapsDesc &maps,
                                     const PickerView::Config &pickerConfig,
                                     QWidget * parent, Qt::WFlags f)
 : QWidget(parent, f), _toolMap(NULL), _recordView(NULL), _currentOrigin(NULL),
   _baseOrigin(NULL), _pickerConfig(pickerConfig) {

	_maptree = new Map::ImageTree(maps);

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::OriginLocatorView(Map::ImageTree* mapTree,
                                     const PickerView::Config &pickerConfig,
                                     QWidget * parent, Qt::WFlags f)
: QWidget(parent, f), _toolMap(NULL), _recordView(NULL), _currentOrigin(NULL),
   _baseOrigin(NULL), _pickerConfig(pickerConfig) {

	_maptree = mapTree;

	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorPlot::OriginLocatorPlot(QWidget *parent) : DiagramWidget(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorPlot::linkClicked() {}
void OriginLocatorPlot::commitButtonClicked(bool) {}
void OriginLocatorPlot::commitWithMTTriggered(bool) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::init() {
	_ui.setupUi(this);

	_blinkWidget = NULL;
	_newOriginStatus = CONFIRMED;

	QObject *drawFilter = new ElideFadeDrawer(this);

	_ui.labelRegion->setFont(SCScheme.fonts.heading3);
	_ui.labelRegion->installEventFilter(drawFilter);
	_ui.label_15->setFont(SCScheme.fonts.normal);
	_ui.label_12->setFont(SCScheme.fonts.normal);
	_ui.label_10->setFont(SCScheme.fonts.normal);
	_ui.label_11->setFont(SCScheme.fonts.normal);
	_ui.label_8->setFont(SCScheme.fonts.normal);
	_ui.label_13->setFont(SCScheme.fonts.normal);
	_ui.label_7->setFont(SCScheme.fonts.normal);
	_ui.label_9->setFont(SCScheme.fonts.normal);
	_ui.labelTime->setFont(SCScheme.fonts.highlight);
	_ui.labelDepth->setFont(SCScheme.fonts.highlight);
	_ui.labelDepthUnit->setFont(SCScheme.fonts.normal);
	_ui.labelDepthError->setFont(SCScheme.fonts.normal);
	_ui.labelDepthErrorUnit->setFont(SCScheme.fonts.normal);
	_ui.labelLatitude->setFont(SCScheme.fonts.highlight);
	_ui.labelLatitudeUnit->setFont(SCScheme.fonts.normal);
	_ui.labelLatitudeError->setFont(SCScheme.fonts.normal);
	_ui.labelLatitudeErrorUnit->setFont(SCScheme.fonts.normal);
	_ui.labelLongitude->setFont(SCScheme.fonts.highlight);
	_ui.labelLongitudeUnit->setFont(SCScheme.fonts.normal);
	_ui.labelLongitudeError->setFont(SCScheme.fonts.normal);
	_ui.labelLongitudeErrorUnit->setFont(SCScheme.fonts.normal);
	_ui.labelNumPhases->setFont(SCScheme.fonts.highlight);
	_ui.labelNumPhasesUnit->setFont(SCScheme.fonts.normal);
	_ui.labelNumPhasesError->setFont(SCScheme.fonts.normal);
	_ui.labelStdError->setFont(SCScheme.fonts.normal);
	_ui.labelStdErrorUnit->setFont(SCScheme.fonts.normal);
	_ui.lbComment->setFont(SCScheme.fonts.normal);
	_ui.labelComment->setFont(SCScheme.fonts.normal);
	_ui.labelAzimuthGap->setFont(SCScheme.fonts.normal);
	_ui.labelAzimuthGapUnit->setFont(SCScheme.fonts.normal);
	_ui.labelMinDist->setFont(SCScheme.fonts.normal);
	_ui.labelMinDistUnit->setFont(SCScheme.fonts.normal);

	ElideFadeDrawer *elider = new ElideFadeDrawer(this);

	/*
	_ui.lbEventID->setFont(SCScheme.fonts.normal);
	_ui.lbAgencyID->setFont(SCScheme.fonts.normal);
	_ui.lbUser->setFont(SCScheme.fonts.normal);
	_ui.lbEvaluation->setFont(SCScheme.fonts.normal);
	_ui.labelEventID->setFont(SCScheme.fonts.normal);
	_ui.labelAgency->setFont(SCScheme.fonts.highlight);
	//setBold(_ui.labelAgency, true);
	_ui.labelEvaluation->setFont(SCScheme.fonts.normal);
	_ui.labelUser->setFont(SCScheme.fonts.highlight);
	//setBold(_ui.labelUser, true);

	_ui.lbMethod->setFont(SCScheme.fonts.normal);
	_ui.lbEarthModel->setFont(SCScheme.fonts.normal);
	_ui.labelMethod->setFont(SCScheme.fonts.normal);
	_ui.labelEarthModel->setFont(SCScheme.fonts.normal);
	*/

	setBold(_ui.labelAgency, true);
	setBold(_ui.labelUser, true);

	_ui.labelEventID->installEventFilter(elider);
	_ui.labelAgency->installEventFilter(elider);
	_ui.labelUser->installEventFilter(elider);
	_ui.labelEvaluation->installEventFilter(elider);

	_ui.labelMethod->installEventFilter(elider);
	_ui.labelEarthModel->installEventFilter(elider);

	if ( SCScheme.unit.distanceInKM )
		_ui.labelMinDistUnit->setText("km");

	_ui.btnMagnitudes->setEnabled(false);
	_ui.btnMagnitudes->setVisible(false);

	_reader = NULL;
	_plotFilter = NULL;
	_plotFilterSettings = NULL;

	_ui.btnCustom0->setVisible(false);
	_ui.btnCustom1->setVisible(false);

	_commitMenu = new QMenu(this);
	_actionCommitOptions = _commitMenu->addAction("With additional options...");

	_ui.editFixedDepth->setValidator(new QDoubleValidator(0, 1000.0, 3, _ui.editFixedDepth));
	_ui.editDistanceCutOff->setValidator(new QDoubleValidator(0, 25000.0, 3, _ui.editFixedDepth));
	_ui.editDistanceCutOff->setText("1000");

	_modelArrivalsProxy = NULL;
	_modelArrivals.setDisabledForeground(palette().color(QPalette::Disabled, QPalette::Text));

	ArrivalDelegate *delegate = new ArrivalDelegate(_ui.tableArrivals);
	_ui.tableArrivals->horizontalHeader()->setMovable(true);
	_ui.tableArrivals->setItemDelegate(delegate);
	_ui.tableArrivals->setMouseTracking(true);
	_ui.tableArrivals->resizeColumnToContents(0);

	connect(_ui.tableArrivals->horizontalHeader(), SIGNAL(sectionClicked(int)),
	        _ui.tableArrivals, SLOT(sortByColumn(int)));

	connect(_ui.tableArrivals, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableArrivalsContextMenuRequested(const QPoint &)));

	connect(&_modelArrivals, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	connect(_ui.tableArrivals->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableArrivalsHeaderContextMenuRequested(const QPoint &)));

	_ui.tableArrivals->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.tableArrivals->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.tableArrivals->horizontalHeader()->setSortIndicatorShown(true);
	_ui.tableArrivals->horizontalHeader()->setSortIndicator(DISTANCE, Qt::AscendingOrder);
	//_ui.tableArrivals->horizontalHeader()->setStretchLastSection(true);
	_ui.tableArrivals->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	_ui.tableArrivals->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QAction *action = new QAction(this);
	action->setShortcut(Qt::Key_Escape);
	connect(action, SIGNAL(triggered()),
	        _ui.tableArrivals, SLOT(clearSelection()));

	addAction(action);

	_residuals = new PlotWidget(_ui.groupResiduals, &_modelArrivals);
	_residuals->setEnabled(false);
	_residuals->setColumnCount(PlotCols::Quantity);

	_residuals->setValueDisabledColor(SCScheme.colors.arrivals.disabled);
	_residuals->setDisplayRect(QRectF(0,-10,180,20));

	_map = new OriginLocatorMap(_maptree.get(), _ui.frameMap);
	_map->setMouseTracking(true);
	_map->setOriginCreationEnabled(true);

	try {
		_map->setStationsMaxDist(SCApp->configGetDouble("olv.map.stations.unassociatedMaxDist"));
	}
	catch ( ... ) {
		_map->setStationsMaxDist(360);
	}

	// Read custom column configuration
	try {
		std::vector<std::string> processProfiles =
			SCApp->configGetStrings("display.origin.addons");

		QGridLayout *grid = static_cast<QGridLayout*>(_ui.groupBox->layout());
		int row, col, rowSpan, colSpan;

		if ( !processProfiles.empty() ) {
			grid->getItemPosition(grid->indexOf(_ui.frameInfoSeparator),
			                      &row, &col, &rowSpan, &colSpan);
		}

		for ( size_t i = 0; i < processProfiles.size(); ++i ) {
			QString label, script;
			try {
				label = SCApp->configGetString("display.origin.addon." + processProfiles[i] + ".label").c_str();
			}
			catch ( ... ) { label = ""; }

			try {
				script = Environment::Instance()->absolutePath(SCApp->configGetString("display.origin.addon." + processProfiles[i] + ".script")).c_str();
			}
			catch ( ... ) {}

			if ( script.isEmpty() ) {
				std::cerr << "WARNING: display.origin.addon."
				          << processProfiles[i] << ".script is not set: ignoring"
				          << std::endl;
				continue;
			}

			QLabel *addonLabel = new QLabel;
			addonLabel->setText(label + ":");
			addonLabel->setAlignment(_ui.lbEventID->alignment());
			QLabel *addonText = new QLabel;

			row = grid->rowCount();
			grid->addWidget(addonLabel, row, 0, 1, 1);
			grid->addWidget(addonText, row, 1, 1, grid->columnCount()-1);
			++row;

			_scriptLabelMap[script] = ScriptLabel(addonLabel, addonText);
		}
	}
	catch ( ... ) {}

	if ( !_scriptLabelMap.isEmpty() ) {
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultAvailable(const QString &, const QString &, const QString &, const QString &)),
		        this, SLOT(evalResultAvailable(const QString &, const QString &, const QString &, const QString &)));
		connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultError(const QString &, const QString &, const QString &, int)),
		        this, SLOT(evalResultError(const QString &, const QString &, const QString &, int)));
		connect(this, SIGNAL(newOriginSet(Seiscomp::DataModel::Origin *,
		                                  Seiscomp::DataModel::Event *,
		                                  bool, bool)),
		        this, SLOT(evaluateOrigin(Seiscomp::DataModel::Origin *,
		                                  Seiscomp::DataModel::Event *,
		                                  bool, bool)));
	}

	QHBoxLayout* hboxLayout = new QHBoxLayout(_ui.frameMap);
	hboxLayout->setObjectName("hboxLayoutMap");
	hboxLayout->setSpacing(6);
	hboxLayout->setMargin(0);
	hboxLayout->addWidget(_map);

	_plotTab = new QTabBar(_ui.groupResiduals);
	_plotTab->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	_plotTab->setShape(QTabBar::RoundedNorth);

	for ( int i = 0; i < PlotTabs::Quantity; ++i )
		_plotTab->addTab(EPlotTabsNames::name(i));

	_plotTab->setCurrentIndex(0);

	QLayoutItem *item = _ui.groupResiduals->layout()->takeAt(0);

	_ui.groupResiduals->layout()->addWidget(_plotTab);
	_ui.groupResiduals->layout()->addItem(item);
	_ui.groupResiduals->layout()->addWidget(_residuals);

	connect(_plotTab, SIGNAL(currentChanged(int)), this, SLOT(plotTabChanged(int)));

	plotTabChanged(_plotTab->currentIndex());
	connect(_ui.labelPlotFilter, SIGNAL(linkActivated(const QString &)), this, SLOT(changePlotFilter()));

	connect(_residuals, SIGNAL(valueActiveStateChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(_residuals, SIGNAL(endSelection()), this, SLOT(residualsSelected()));
	connect(_residuals, SIGNAL(adjustZoomRect(QRectF&)), this, SLOT(adjustResidualsRect(QRectF&)));
	connect(_residuals, SIGNAL(hover(int)), this, SLOT(hoverArrival(int)));
	connect(_residuals, SIGNAL(clicked(int)), this, SLOT(selectArrival(int)));
	connect(_residuals, SIGNAL(focalMechanismCommitted(bool, QPoint)),
	        this, SLOT(commitFocalMechanism(bool)));

	connect(_map, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(_map, SIGNAL(hoverArrival(int)), this, SLOT(hoverArrival(int)));
	connect(_map, SIGNAL(clickedArrival(int)), this, SLOT(selectArrival(int)));
	connect(_map, SIGNAL(clickedStation(const std::string &, const std::string &)),
	        this, SLOT(selectStation(const std::string &, const std::string &)));
	connect(_map, SIGNAL(artificialOriginRequested(const QPointF &, const QPoint &)),
	        this, SLOT(createArtificialOrigin(const QPointF &, const QPoint &)));

	//connect(_ui.btnZoom, SIGNAL(clicked()), this, SLOT(zoomMap()));

	connect(_ui.btnImportAllArrivals, SIGNAL(clicked()), this, SLOT(importArrivals()));
	connect(_ui.btnShowWaveforms, SIGNAL(clicked()), this, SLOT(showWaveforms()));
	//connect(_ui.btnShowWaveforms, SIGNAL(clicked()), this, SIGNAL(waveformsRequested()));
	connect(_ui.btnRelocate, SIGNAL(clicked()), this, SLOT(relocate()));
	connect(_ui.btnMagnitudes, SIGNAL(clicked()), this, SLOT(computeMagnitudes()));
	connect(_ui.buttonEditComment, SIGNAL(clicked()), this, SLOT(editComment()));
	connect(_ui.btnCommit, SIGNAL(clicked()), this, SLOT(commit()));
	connect(_actionCommitOptions, SIGNAL(triggered()), this, SLOT(commitWithOptions()));

	connect(&_blinkTimer, SIGNAL(timeout()), this, SLOT(updateBlinkState()));

	/*
	QFontMetrics fm = fontMetrics();
	int width = _ui.lbAgencyID->width() + 6 + fm.boundingRect("WWWWWWWWWW").width();

	_ui.groupBox->setFixedWidth(width);
	*/

	_displayComment = false;
	try {
		_displayCommentID = SCApp->configGetString("olv.display.origin.comment.id");
		_displayComment = true;
	}
	catch ( ... ) {}

	_ui.lbComment->setVisible(_displayComment);
	_ui.labelComment->setVisible(_displayComment);

	try {
		_displayCommentDefault = SCApp->configGetString("display.origin.comment.default");
	}
	catch ( ... ) {
		_displayCommentDefault = _ui.labelComment->text().toStdString();
	}

	try {
		_ui.lbComment->setText(QString("%1:").arg(SCApp->configGetString("display.origin.comment.label").c_str()));
	}
	catch ( ... ) {
		_ui.lbComment->setText(_displayCommentID.c_str());
	}

	try {
		EventType et;
		if ( et.fromString(SCApp->configGetString("olv.defaultEventType").c_str()) )
			_defaultEventType = et;
		else
			cerr << "ERROR: unknown type '" << SCApp->configGetString("olv.defaultEventType")
			     << "' in olv.defaultEventType" << endl;
	}
	catch ( ... ) {}

	try {
		_defaultEarthModel = SCApp->configGetString("olv.locator.defaultProfile");
	}
	catch ( ... ) {}

	std::string defaultLocator = "LOCSAT";
	try {
		defaultLocator = SCApp->configGetString("olv.locator.interface");
	}
	catch ( ... ) {
		try {
			defaultLocator = SCApp->configGetString("olv.locator");
		}
		catch ( ... ) {}
	}

	vector<string> *locatorInterfaces = Seismology::LocatorInterfaceFactory::Services();
	if ( locatorInterfaces ) {
		for ( size_t i = 0; i < locatorInterfaces->size(); ++i )
			_ui.cbLocator->addItem((*locatorInterfaces)[i].c_str());
		delete locatorInterfaces;
	}

	int defaultLocatorIdx = _ui.cbLocator->findText(defaultLocator.c_str());
	if ( defaultLocatorIdx < 0 )
		defaultLocatorIdx = _ui.cbLocator->findText("LOCSAT");

	_ui.cbLocator->setCurrentIndex(defaultLocatorIdx);

	connect(_ui.cbLocator, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(locatorChanged(const QString &)));

	locatorChanged(_ui.cbLocator->currentText());

	connect(_ui.cbLocatorProfile, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(locatorProfileChanged(const QString &)));

	connect(_ui.btnLocatorSettings, SIGNAL(clicked()),
	        this, SLOT(configureLocator()));

	_minimumDepth = -999;

	try {
		// "locator.minimumDepth" preferred
		_minimumDepth = SCApp->configGetDouble("olv.locator.minimumDepth");
	}
	catch ( ... ) {
		try {
			_minimumDepth = SCApp->configGetDouble("locator.minimumDepth");
		}
		catch ( ... ) {}
	}

	try {
		_ui.btnCustom0->setText(SCApp->configGetString("button0").c_str());
	}
	catch ( ... ) {}

	try {
		_ui.btnCustom1->setText(SCApp->configGetString("button1").c_str());
	}
	catch ( ... ) {}

	connect(_ui.btnCustom0, SIGNAL(clicked()), this, SLOT(runScript0()));
	connect(_ui.btnCustom1, SIGNAL(clicked()), this, SLOT(runScript1()));

	try {
		vector<string> cols = SCApp->configGetStrings("olv.arrivalTable.visibleColumns");
		for ( int i = 0; i < ArrivalListColumns::Quantity; ++i )
			colVisibility[i] = false;

		for ( size_t i = 0; i < cols.size(); ++i ) {
			ArrivalListColumns v;
			if ( !v.fromString(cols[i]) ) {
				cerr << "ERROR: olv.arrivalTable.visibleColumns: invalid column name '"
				     << cols[i] << "' at index " << i << ", ignoring" << endl;
				continue;
			}

			colVisibility[v] = true;
		}
	}
	catch ( ... ) {}

	resetCustomLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginLocatorView::~OriginLocatorView() {
	if ( _plotFilter ) delete _plotFilter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::locatorChanged(const QString &text) {
	_ui.cbLocatorProfile->clear();

	_locator = Seismology::LocatorInterfaceFactory::Create(_ui.cbLocator->currentText().toStdString().c_str());
	if ( !_locator ) {
		_ui.cbLocatorProfile->clear();
		_ui.cbLocatorProfile->setEnabled(false);
	}

	_locator->init(SCApp->configuration());

	set<string> models;
	Seismology::LocatorInterface::IDList profiles = _locator->profiles();
	for ( Seismology::LocatorInterface::IDList::iterator it = profiles.begin();
	      it != profiles.end(); ++it ) {
		if ( models.find(*it) != models.end() ) continue;
		_ui.cbLocatorProfile->addItem(it->c_str());
	}

	int defaultIndex = _ui.cbLocatorProfile->findText(_defaultEarthModel.c_str());
	if ( defaultIndex >= 0 )
		_ui.cbLocatorProfile->setCurrentIndex(defaultIndex);
	else
		_ui.cbLocatorProfile->setCurrentIndex(0);

	_ui.cbLocatorProfile->setEnabled(_ui.cbLocatorProfile->count() > 0);

	_ui.cbFixedDepth->setEnabled(_locator->supports(Seismology::LocatorInterface::FixedDepth));
	_ui.cbDistanceCutOff->setEnabled(_locator->supports(Seismology::LocatorInterface::DistanceCutOff));
	_ui.cbIgnoreInitialLocation->setEnabled(_locator->supports(Seismology::LocatorInterface::IgnoreInitialLocation));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::configureLocator() {
	if ( !_locator ) {
		QMessageBox::critical(this, "Locator settings",
		                      "No locator selected.");
		return;
	}

	Seismology::LocatorInterface::IDList params = _locator->parameters();
	if ( params.empty() ) {
		QMessageBox::information(this, "Locator settings",
		                         QString("%1 does not provide any "
		                                 "parameters to adjust.")
		                         .arg(_locator->name().c_str()));
		return;
	}

	LocatorSettings dlg;
	dlg.setWindowTitle(QString("%1 settings").arg(_locator->name().c_str()));

	for ( Seismology::LocatorInterface::IDList::iterator it = params.begin();
	      it != params.end(); ++it ) {
		dlg.addRow(it->c_str(), _locator->parameter(*it).c_str());
	}

	if ( dlg.exec() != QDialog::Accepted )
		return;

	LocatorSettings::ContentList res = dlg.content();
	for ( LocatorSettings::ContentList::iterator it = res.begin();
	      it != res.end(); ++it )
		_locator->setParameter(it->first.toStdString(), it->second.toStdString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changePlotFilter() {
	if ( _plotFilterSettings == NULL )
		_plotFilterSettings = new DiagramFilterSettingsDialog(this);

	if ( _plotFilterSettings->exec() == QDialog::Rejected ) return;
	setPlotFilter(_plotFilterSettings->createFilter());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPlotFilter(DiagramFilterSettingsDialog::Filter *f) {
	if ( _plotFilter ) delete _plotFilter;
	_plotFilter = f;
	applyPlotFilter();

	if ( _plotFilter )
		_ui.labelPlotFilter->setText("<a href=\"filter\">active</a>");
	else
		_ui.labelPlotFilter->setText("<a href=\"filter\">not active</a>");

	_ui.labelPlotFilter->setCursor(Qt::PointingHandCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::applyPlotFilter() {
	if ( _plotFilter == NULL ) {
		for ( int i = 0; i < _residuals->count(); ++i )
			_residuals->showValue(i);
	}
	else {
		for ( int i = 0; i < _residuals->count(); ++i )
			_residuals->showValue(i, _plotFilter->accepts(_residuals, i));
	}

	_residuals->updateBoundingRect();
	QRectF rect = _residuals->boundingRect();
	rect.setLeft(std::min(0.0, double(rect.left())));
	adjustResidualsRect(rect);
	_residuals->setDisplayRect(rect);
	_residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript(const QString &script, const QString &name) {
	QString cmd = QString("%1 %2").arg(script).arg(_currentOrigin->publicID().c_str());

	// start as background process w/o any communication channel
	if ( !QProcess::startDetached(cmd) ) {
		QMessageBox::warning(this, name, tr("Can't execute script"));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript0() {
	runScript(_script0.c_str(), _ui.btnCustom0->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::runScript1() {
	runScript(_script1.c_str(), _ui.btnCustom1->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::locatorProfileChanged(const QString &text) {
	if ( _locator )
		_locator->setProfile(text.toStdString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setMagnitudeCalculationEnabled(bool e) {
	_ui.btnMagnitudes->setVisible(e);
	if ( !e )
		_ui.btnMagnitudes->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget* OriginLocatorView::map() const {
	return _map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::closeEvent(QCloseEvent *e) {
	if ( _recordView ) _recordView->close();
	QWidget::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::residualsSelected() {
	QRectF brect = _residuals->getSelectedValuesRect();
	if ( brect.isEmpty() && !brect.isNull() ) {
		for ( int i = 0; i < _modelArrivals.rowCount(); ++i )
			_modelArrivals.setData(_modelArrivals.index(i, 0), 0, UsedRole);

		return;
	}

	QVector<int> selectedIds = _residuals->getSelectedValues();
	int startIndex = 0;
	for ( int i = 0; i < selectedIds.count(); ++i ) {
		for ( int j = startIndex; j < selectedIds[i]; ++j )
			_modelArrivals.setData(_modelArrivals.index(j, 0), 0, UsedRole);
		_modelArrivals.setData(_modelArrivals.index(selectedIds[i], 0),
		                       getMask(_modelArrivals.index(selectedIds[i], 0)),
		                       UsedRole);
		startIndex = selectedIds[i]+1;
	}

	for ( int j = startIndex; j < _modelArrivals.rowCount(); ++j )
		_modelArrivals.setData(_modelArrivals.index(j, 0), 0, UsedRole);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::hoverArrival(int id) {
	QWidget *w = (QWidget*)sender();
	if ( id == -1 )
		w->setToolTip("");
	else {
		QString method = _modelArrivals.data(_modelArrivals.index(id, METHOD), Qt::DisplayRole).toString();

		if ( method.isEmpty() )
			w->setToolTip(
				_modelArrivals.data(_modelArrivals.index(id, NETWORK), Qt::DisplayRole).toString() + "." +
				_modelArrivals.data(_modelArrivals.index(id, STATION), Qt::DisplayRole).toString() + "-" +
				_modelArrivals.data(_modelArrivals.index(id, PHASE), Qt::DisplayRole).toString());
		else
			w->setToolTip(
				_modelArrivals.data(_modelArrivals.index(id, NETWORK), Qt::DisplayRole).toString() + "." +
				_modelArrivals.data(_modelArrivals.index(id, STATION), Qt::DisplayRole).toString() + "-" +
				_modelArrivals.data(_modelArrivals.index(id, PHASE), Qt::DisplayRole).toString() + " (" +
				method + ")");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrival(int id) {
	if ( id >= 0 && _currentOrigin ) {
		if ( _recordView ) {
			Arrival *ar = _currentOrigin->arrival(id);
			Pick *pick = Pick::Find(ar->pickID());
			if ( pick )
				_recordView->selectTrace(pick->waveformID());
		}

		QModelIndex idx = _modelArrivalsProxy->mapFromSource(_modelArrivals.index(id, 0));
		_ui.tableArrivals->setCurrentIndex(idx);
		_ui.tableArrivals->scrollTo(idx);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectStation(const std::string &net,
                                      const std::string &code) {
	if ( _recordView ) _recordView->selectTrace(net, code);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectRow(const QModelIndex &current, const QModelIndex&) {
	QModelIndex idx = _modelArrivalsProxy->mapToSource(current);
	if ( idx.row() >= 0 && _currentOrigin && _recordView ) {
		Arrival *ar = _currentOrigin->arrival(idx.row());
		Pick *pick = Pick::Find(ar->pickID());
		if ( pick )
			_recordView->selectTrace(pick->waveformID());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::adjustResidualsRect(QRectF& rect) {
	//rect.setLeft(std::max(0.0, floor(rect.left()*0.1) * 10));
	//rect.setRight((int)(ceil(rect.right()*0.1)) * 10);
	rect.setLeft(std::max(0.0, double(floor(rect.left()))));
	rect.setRight((int)(ceil(rect.right())));

	if ( _plotTab->currentIndex() == PT_POLAR ||
	     _plotTab->currentIndex() == PT_FM ) {
		rect.setTop(0);
		rect.setBottom(360);

		if ( rect.right() == 0 )
			rect.setRight(1);

		return;
	}

	rect.setTop(-std::max(std::ceil(std::abs(rect.bottom())), std::ceil(std::abs(rect.top()))));
	rect.setBottom(-rect.top());

	if ( _plotTab->currentIndex() == PT_TRAVELTIME )
		rect.setTop(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::plotTabChanged(int tab) {
	static_cast<PlotWidget*>(_residuals)->setCustomDraw(false);

	// Distance / Residual
	if ( tab == PT_DISTANCE ) {
		_residuals->setMarkerDistance(10, 1);
		_residuals->setType(DiagramWidget::Rectangular);
		_residuals->setIndicies(PC_DISTANCE,PC_RESIDUAL);
		if ( SCScheme.unit.distanceInKM )
			_residuals->setAbscissaName("Distance (km)");
		else
			_residuals->setAbscissaName("Distance (deg)");
		_residuals->setOrdinateName("Residual");
	}
	// Azimuth / Residual
	else if ( tab == PT_AZIMUTH ) {
		_residuals->setMarkerDistance(10, 1);
		_residuals->setType(DiagramWidget::Rectangular);
		_residuals->setIndicies(PC_AZIMUTH,PC_RESIDUAL);
		_residuals->setAbscissaName("Azimuth");
		_residuals->setOrdinateName("Residual");
	}
	// Distance / TravelTime
	else if ( tab == PT_TRAVELTIME ) {
		_residuals->setMarkerDistance(10, 10);
		_residuals->setType(DiagramWidget::Rectangular);
		_residuals->setIndicies(PC_DISTANCE,PC_TRAVELTIME);
		if ( SCScheme.unit.distanceInKM )
			_residuals->setAbscissaName("Distance (km)");
		else
			_residuals->setAbscissaName("Distance (deg)");
		_residuals->setOrdinateName("TravelTime");
	}
	else if ( tab == PT_MOVEOUT ) {
		_residuals->setMarkerDistance(10, 10);
		_residuals->setType(DiagramWidget::Rectangular);
		_residuals->setIndicies(PC_DISTANCE,PC_REDUCEDTRAVELTIME);
		if ( SCScheme.unit.distanceInKM )
			_residuals->setAbscissaName("Distance (km)");
		else
			_residuals->setAbscissaName("Distance (deg)");
		_residuals->setOrdinateName(QString("TTred >x/%1").arg(_config.reductionVelocityP));
	}
	else if ( tab == PT_POLAR ) {
		_residuals->setType(DiagramWidget::Spherical);
		_residuals->setIndicies(PC_DISTANCE,PC_AZIMUTH);
	}
	else if ( tab == PT_FM ) {
		static_cast<PlotWidget*>(_residuals)->setCustomDraw(true);
		_residuals->setType(DiagramWidget::Spherical);
		_residuals->setIndicies(PC_FMDIST,PC_FMAZI);
	}

	QRectF rect = _residuals->boundingRect();

	rect.setLeft(std::min(0.0, double(rect.left())));

	if ( tab == PT_AZIMUTH )
		rect.setRight(360.0);

	adjustResidualsRect(rect);

	/*
	std::cout << "displayRect: " << rect.left() << "," << rect.top()
			  << " - " << rect.width() << " x " << rect.height() << std::endl;
	*/

	_residuals->setDisplayRect(rect);
	_residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changeArrival(int id, bool state) {
	QModelIndex idx = _modelArrivals.index(id, 0);
	_modelArrivals.setData(idx, state?getMask(idx):0, UsedRole);

	_residuals->setValueSelected(id, state);
	_map->setArrivalState(id, state);
	if ( _toolMap )
		_toolMap->setArrivalState(id, state);
	if ( _recordView )
		_recordView->setArrivalState(id, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::changeArrivalEnableState(int id,bool state) {
	//changeArrival(id, state);
	_modelArrivals.setRowEnabled(id, state);
	_residuals->setValueEnabled(id, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::zoomMap() {
	if ( _toolMap != NULL ) {
		_toolMap->activateWindow();
		_toolMap->raise();
		return;
	}

	_toolMap = new OriginLocatorMap(_maptree.get(), this, Qt::Window);
	_toolMap->setAttribute(Qt::WA_DeleteOnClose);
	connect(_toolMap, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(mapKeyPressed(QKeyEvent*)));
	connect(_toolMap, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	connect(_toolMap, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	if ( _currentOrigin ) {
		_toolMap->setOrigin(_currentOrigin.get());
		_toolMap->canvas().displayRect(QRectF(_currentOrigin->longitude()-20, _currentOrigin->latitude()-20, 40, 40));
		_toolMap->setDrawStations(_map->drawStations());
	}
	_toolMap->setWindowTitle("OriginLocator::Map");
	_toolMap->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::mapKeyPressed(QKeyEvent* e) {
	if ( _toolMap == NULL ) return;

	switch ( e->key() ) {
		case Qt::Key_Escape:
			_toolMap->close();
			break;
		case Qt::Key_F9:
			drawStations(!_map->drawStations());
			break;
		case Qt::Key_F11:
			if ( _toolMap->isFullScreen() )
				_toolMap->showNormal();
			else
				_toolMap->showFullScreen();
			break;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::objectDestroyed(QObject* o) {
	if ( o == _toolMap ) {
		_toolMap = NULL;
		//_ui.btnZoom->setEnabled(true);
	}

	if ( o == _recordView ) {
		//std::cout << "Number of objects after: " << Core::BaseObject::ObjectCount() << std::endl;
		_recordView = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::drawStations(bool enable) {
	_map->setDrawStations(enable);
	_map->update();

	if ( _toolMap ) {
		_toolMap->setDrawStations(enable);
		_toolMap->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::readPicks(Origin* o) {
	if ( _blockReadPicks ) return;

	_blockReadPicks = true;

	if ( _reader ) {
		if ( o->arrivalCount() == 0 )
			_reader->loadArrivals(o);

		if ( o->magnitudeCount() == 0 )
			_reader->loadMagnitudes(o);

		for ( size_t i = 0; i < o->magnitudeCount(); ++i ) {
			if ( o->magnitude(i)->stationMagnitudeContributionCount() == 0 )
				_reader->loadStationMagnitudeContributions(o->magnitude(i));
		}

		if ( o->stationMagnitudeCount() == 0 )
			_reader->loadStationMagnitudes(o);

		PickMap originPicks;
		std::vector<PickPtr> tmpPicks;

		bool missing = false;
		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			std::string pickID = o->arrival(i)->pickID();
			if ( Pick::Find(pickID) == NULL ) {
				missing = true;
				break;
			}
		}

		if ( missing ) {
			QProgressDialog progress(this);
			progress.setWindowTitle(tr("Please wait..."));
			progress.setRange(0, o->arrivalCount());
			progress.setLabelText(tr("Loading picks..."));
			progress.setCancelButton(NULL);
			DatabaseIterator it = _reader->getPicks(o->publicID());

			while ( *it ) {
				if ( !it.cached() )
					tmpPicks.push_back(Pick::Cast(*it));
				++it;
				progress.setValue(progress.value()+1);
			}
		}

		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			std::string pickID = o->arrival(i)->pickID();

			PickMap::iterator it = _associatedPicks.find(pickID);
			if ( it != _associatedPicks.end() ) {
				originPicks[pickID] = it->second;
				continue;
			}

			// try to find the pick somewhere in the client memory
			PickPtr pick = Pick::Find(pickID);
			if ( pick )
				originPicks[pickID] = pick;
		}

		_associatedPicks = originPicks;
	}

	_blockReadPicks = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateBlinkState() {
	//--_relocateBlinkCounter;
	if ( _blinkCounter <= 0 ) {
		_blinkCounter = 0;
		_blinker = 0;
		_blinkTimer.stop();
	}

	if ( _blinkWidget == NULL ) return;

	QPalette pal = _blinkWidget->palette();

	int percent = (int)(25 * sin(8*(_blinker++) * 2 * M_PI / 100 - M_PI/2) + 25);

	QColor blink = blend(_blinkColor, qApp->palette().color(QPalette::Button), percent);
	pal.setColor(QPalette::Button, blink);
	_blinkWidget->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::startBlinking(QColor c, QWidget *w) {
	if ( _blinkWidget != NULL && _blinkWidget != w )
		stopBlinking();

	_blinkCounter = 50;
	_blinkColor = c;
	_blinker = 0;
	_blinkWidget = w;
	_blinkTimer.start(40);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::stopBlinking() {
	_blinkCounter = 0;
	updateBlinkState();
	_blinkWidget = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setCreatedOrigin(Seiscomp::DataModel::Origin* o) {
	ObjectChangeList<DataModel::Pick> changedPicks;
	_recordView->getChangedPicks(changedPicks);
	SEISCOMP_DEBUG("received new origin with %lu manual picks", (unsigned long)changedPicks.size());

	startBlinking(QColor(255,128,0), _ui.btnRelocate);
	_ui.btnRelocate->setFocus();

	_ui.btnCommit->setEnabled(true);
	_ui.btnCommit->setText("Commit");
//	_ui.btnCommit->setMenu(_baseEvent?_commitMenu:NULL);
	_localOrigin = true;

	// Update pick cache
	for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
		Pick* p = Pick::Find(o->arrival(i)->pickID());
		if ( p ) _associatedPicks[p->publicID()] = p;
	}

	pushUndo();
	_blockReadPicks = true;
	updateOrigin(o);
	_blockReadPicks = false;

	//computeMagnitudes();
	_ui.btnMagnitudes->setEnabled(true);

	_changedPicks.insert(changedPicks.begin(), changedPicks.end());

	emit newOriginSet(o, _baseEvent.get(), _localOrigin, false);
	emit requestRaise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setConfig(const Config &c) {
	bool updateTakeOffAngles = _config.computeMissingTakeOffAngles != c.computeMissingTakeOffAngles;

	_config = c;
	_residuals->setDrawGridLines(_config.drawGridLines);
	_map->setDrawStationLines(c.drawMapLines);

	for ( int i = 0; i < _residuals->count(); ++i ) {
		if ( _residuals->isValueValid(i,PC_DISTANCE) && _residuals->isValueValid(i,PC_TRAVELTIME) ) {
			if ( SCScheme.unit.distanceInKM )
				_residuals->setValue(i,PC_REDUCEDTRAVELTIME, _residuals->value(i,PC_TRAVELTIME) - _residuals->value(i,PC_DISTANCE)/_config.reductionVelocityP);
			else
				_residuals->setValue(i,PC_REDUCEDTRAVELTIME, _residuals->value(i,PC_TRAVELTIME) - Math::Geo::deg2km(_residuals->value(i,PC_DISTANCE))/_config.reductionVelocityP);
		}
		else {
			_residuals->setValue(i, PC_REDUCEDTRAVELTIME, 0.0);
			_residuals->setValueValid(i, PC_REDUCEDTRAVELTIME, false);
		}

		if ( updateTakeOffAngles && _currentOrigin ) {
			char phase = Util::getShortPhaseName(_currentOrigin->arrival(i)->phase().code());
			PlotWidget::PolarityType polarity = (PlotWidget::PolarityType)_residuals->value(i, PC_POLARITY);

			_residuals->setValue(i, PC_FMDIST, 0.0);
			_residuals->setValue(i, PC_FMAZI, 0.0);
			_residuals->setValueValid(i, PC_FMDIST, false);
			_residuals->setValueValid(i, PC_FMAZI, false);

			if ( _residuals->isValueValid(i, PC_DISTANCE) &&
				 _residuals->isValueValid(i, PC_AZIMUTH) &&
				 phase == 'P' ) {

				double beta;
				bool hasTakeOff;
				try {
					beta = _currentOrigin->arrival(i)->takeOffAngle();
					hasTakeOff = true;
				}
				catch ( ... ) {
					hasTakeOff = false;
				}

				if ( !hasTakeOff && _config.computeMissingTakeOffAngles ) {
					double lat, lon;
					double azi = _residuals->value(i, PC_AZIMUTH);

					Math::Geo::delandaz2coord(
						_currentOrigin->arrival(i)->distance(), azi,
						_currentOrigin->latitude(), _currentOrigin->longitude(),
						&lat, &lon
					);

					try {
						TravelTime ttt = _ttTable.computeFirst(
							_currentOrigin->latitude(), _currentOrigin->longitude(),
							_currentOrigin->depth(), lat, lon
						);

						beta = ttt.takeoff;
						_modelArrivals.setTakeOffAngle(i, beta);

						hasTakeOff = true;
					}
					catch ( ... ) {}
				}

				if ( hasTakeOff &&
					 static_cast<PlotWidget*>(_residuals)->shape(polarity).shown ) {
					double azi = _residuals->value(i, PC_AZIMUTH);

					if ( beta > 90 ) {
						beta = 180-beta;
						azi = azi-180;
						if ( azi < 0 ) azi += 360;
					}

					beta = sqrt(2.0) * sin(0.5*deg2rad(beta));

					_residuals->setValue(i, PC_FMAZI, azi);
					_residuals->setValue(i, PC_FMDIST, beta);
					_residuals->setValueValid(i, PC_FMDIST, true);
					_residuals->setValueValid(i, PC_FMAZI, true);
				}
			}
		}
	}

	if ( _plotTab->currentIndex() == PT_MOVEOUT ) {
		_residuals->updateBoundingRect();
		QRectF rect = _residuals->boundingRect();
		rect.setLeft(std::min(0.0, double(rect.left())));
		adjustResidualsRect(rect);
		_residuals->setDisplayRect(rect);
		_residuals->setOrdinateName(QString("TTred >x/%1").arg(_config.reductionVelocityP));
	}

	_residuals->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const OriginLocatorView::Config &OriginLocatorView::config() const {
	return _config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPickerConfig(const PickerView::Config &c) {
	_pickerConfig = c;

	if ( _recordView )
		_recordView->setConfig(_pickerConfig);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const PickerView::Config& OriginLocatorView::pickerConfig() const {
	return _pickerConfig;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	_reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setPickerView(PickerView* picker) {
	_recordView = picker;

	connect(_recordView, SIGNAL(arrivalChanged(int,bool)), this, SLOT(changeArrival(int,bool)));
	connect(_recordView, SIGNAL(arrivalEnableStateChanged(int,bool)), this, SLOT(changeArrivalEnableState(int,bool)));
	connect(_recordView, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	connect(_recordView, SIGNAL(originCreated(Seiscomp::DataModel::Origin*)),
	        this, SLOT(setCreatedOrigin(Seiscomp::DataModel::Origin*)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addObject(const QString& parentID, Seiscomp::DataModel::Object *o) {
	if ( _baseEvent && _currentOrigin ) {
		OriginReferencePtr ref = OriginReference::Cast(o);
		if ( ref && parentID == _baseEvent->publicID().c_str() ) {
			OriginPtr o = Origin::Find(ref->originID());
			if ( o && (o->arrivalCount() > _currentOrigin->arrivalCount()) )
				startBlinking(QColor(128,255,0), _ui.btnImportAllArrivals);
		}
	}

	if ( _displayComment && _currentOrigin ) {
		if ( parentID == _currentOrigin->publicID().c_str() ) {
			Comment *comment = Comment::Cast(o);
			if ( comment && comment->id() == _displayCommentID )
				_ui.labelComment->setText(comment->text().c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateObject(const QString& parentID, Seiscomp::DataModel::Object *o) {
	if ( _baseEvent) {
		Event *evt = Event::Cast(o);
		if ( evt && evt->publicID() == _baseEvent->publicID() ) {
			if ( evt->preferredFocalMechanismID() != _preferredFocMech ) {
				// Trigger preferred FM update
				setBaseEvent(_baseEvent.get());
			}
		}
	}

	if ( _currentOrigin ) {
		Origin *org = Origin::Cast(o);
		if ( org && org->publicID() == _currentOrigin->publicID() )
			updateContent();
		else if ( _displayComment ) {
			if ( parentID == _currentOrigin->publicID().c_str() ) {
				Comment *comment = Comment::Cast(o);
				if ( comment && comment->id() == _displayCommentID )
					_ui.labelComment->setText(comment->text().c_str());
			}
		}
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setOrigin(Seiscomp::DataModel::Origin* o) {
	setOrigin(o, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setBaseEvent(DataModel::Event *e) {
	_baseEvent = e;
	_preferredFocMech = string();

	//static_cast<PlotWidget*>(_residuals)->setPreferredFM(355,60,41);
	//return;

	_ui.btnCommit->setMenu(_baseEvent?_commitMenu:NULL);

	if ( _baseEvent ) {
		_ui.labelEventID->setText(_baseEvent->publicID().c_str());
		_ui.labelEventID->setToolTip(_baseEvent->publicID().c_str());
	}
	else {
		_ui.labelEventID->setText("-");
		_ui.labelEventID->setToolTip("");
	}

	if ( _baseEvent == NULL ) {
		static_cast<PlotWidget*>(_residuals)->set(90,90,0);
		static_cast<PlotWidget*>(_residuals)->resetPreferredFM();
		return;
	}

	_preferredFocMech = e->preferredFocalMechanismID();

	DataModel::FocalMechanismPtr fm = DataModel::FocalMechanism::Find(_preferredFocMech);
	if ( !fm && !e->preferredFocalMechanismID().empty() && _reader )
		fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), _preferredFocMech));

	if ( !fm ) {
		static_cast<PlotWidget*>(_residuals)->set(90,90,0);
		static_cast<PlotWidget*>(_residuals)->resetPreferredFM();
		return;
	}

	try {
		static_cast<PlotWidget*>(_residuals)->setPreferredFM(
			fm->nodalPlanes().nodalPlane1().strike(),
			fm->nodalPlanes().nodalPlane1().dip(),
			fm->nodalPlanes().nodalPlane1().rake()
		);

		static_cast<PlotWidget*>(_residuals)->set(
			fm->nodalPlanes().nodalPlane1().strike(),
			fm->nodalPlanes().nodalPlane1().dip(),
			fm->nodalPlanes().nodalPlane1().rake()
		);
	}
	catch ( ... ) {
		static_cast<PlotWidget*>(_residuals)->set(90,90,0);
		static_cast<PlotWidget*>(_residuals)->resetPreferredFM();
	}
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::setOrigin(DataModel::Origin* o, DataModel::Event* e,
                                  bool local) {
	if ( _currentOrigin == o ) {
		if ( _baseEvent != e )
			setBaseEvent(e);
		return true;
	}

	if ( !_undoList.isEmpty() ) {
		if ( QMessageBox::question(this, "Show origin",
		                           tr("You have uncommitted modifications.\n"
		                              "When setting the new origin your modifications get lost.\n"
		                              "Do you really want to continue?"),
		                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		return false;
	}

	// Reset plot filter if a new event has been loaded
	if ( (e != NULL) && _baseEvent != e )
		setPlotFilter(NULL);

	stopBlinking();

	_changedPicks.clear();
	_baseOrigin = o;
	setBaseEvent(e);

	_undoList.clear();
	_redoList.clear();

	_ui.btnCommit->setText(local?"Commit":"Confirm");
//	_ui.btnCommit->setMenu(_baseEvent?_commitMenu:NULL);

	// Disable distance cutoff when a new origin has been
	// set from external.
	_ui.cbDistanceCutOff->setChecked(false);
	_ui.cbIgnoreInitialLocation->setChecked(false);

	emit undoStateChanged(!_undoList.isEmpty());
	emit redoStateChanged(!_redoList.isEmpty());

	_ui.btnImportAllArrivals->setEnabled(true);

	try {
		if ( o && o->evaluationMode() == AUTOMATIC )
			_ui.btnCommit->setEnabled(false);
		else
			_ui.btnCommit->setEnabled(true);
	}
	catch ( ... ) {
		_ui.btnCommit->setEnabled(false);
	}

	_blockReadPicks = false;
	updateOrigin(o);

	if ( _recordView )
		_recordView->setOrigin(o, -5*60, 30*60);

	_localOrigin = local;
	emit newOriginSet(o, _baseEvent.get(), _localOrigin, false);
	_ui.btnMagnitudes->setEnabled(false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::clear() {
	stopBlinking();

	_associatedPicks.clear();
	_originPicks.clear();
	_changedPicks.clear();
	_baseOrigin = NULL;
	_baseEvent = NULL;

	_undoList.clear();
	_redoList.clear();

	_ui.btnCommit->setText("Confirm");
	_ui.btnCommit->setMenu(NULL);

	emit undoStateChanged(false);
	emit redoStateChanged(false);

	_ui.btnImportAllArrivals->setEnabled(false);
	_ui.btnCommit->setEnabled(false);

	_blockReadPicks = false;
	resetCustomLabels();
	updateOrigin(NULL);

	if ( _recordView )
		_recordView->close();

	_ui.btnMagnitudes->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateOrigin(Seiscomp::DataModel::Origin* o) {
	if ( _currentOrigin == o ) return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( o ) readPicks(o);

	_currentOrigin = o;
	_modelArrivals.setOrigin(o);

	updateContent();

	if ( _currentOrigin ) {
		for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
			Arrival *arrival = _currentOrigin->arrival(i);
			if ( !Client::Inventory::Instance()->getStation(Pick::Find(arrival->pickID())) ) {
				changeArrivalEnableState(i, false);
			}
			else {
			}
		}
	}

	_residuals->setEnabled(_currentOrigin != NULL);

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::resetCustomLabels() {
	ScriptLabelMap::iterator it;
	for ( it = _scriptLabelMap.begin(); it != _scriptLabelMap.end(); ++it ) {
		it.value().first->setEnabled(false);
		it.value().second->setText("");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::updateContent() {
	_residuals->clear();
	//_ui.tableArrivals->setModel(&_modelArrivals);

	if ( _ui.tableArrivals->selectionModel() )
		delete _ui.tableArrivals->selectionModel();

	if ( _ui.tableArrivals->model() )
		delete _ui.tableArrivals->model();

	_modelArrivalsProxy = new ArrivalsSortFilterProxyModel(this);
	_modelArrivalsProxy->setSourceModel(&_modelArrivals);
	_ui.tableArrivals->setModel(_modelArrivalsProxy);
	connect(_ui.tableArrivals->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(selectRow(const QModelIndex&, const QModelIndex&)));

	for ( int i = 0; i < ArrivalListColumns::Quantity; ++i )
		_ui.tableArrivals->setColumnHidden(i, !colVisibility[i]);

	//_ui.tableArrivals->resize(_ui.tableArrivals->size());
	_ui.tableArrivals->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	_ui.buttonEditComment->setEnabled(_baseEvent.get());

	// Reset custom labels and set background
	resetCustomLabels();

	if ( _currentOrigin == NULL ) {
		_ui.cbLocator->setEnabled(false);
		_ui.retranslateUi(this);
		_ui.btnShowWaveforms->setEnabled(false);
		_ui.btnRelocate->setEnabled(false);
		_ui.cbLocatorProfile->setEnabled(false);
		_ui.btnCustom0->setEnabled(false);
		_ui.btnCustom1->setEnabled(false);
		_map->setOrigin(NULL);
		_residuals->update();
		return;
	}

	_ui.btnCustom0->setEnabled(true);
	_ui.btnCustom1->setEnabled(true);

	if ( _ui.cbLocator->count() > 1 )
		_ui.cbLocator->setEnabled(true);

	_ui.btnLocatorSettings->setEnabled(_locator != NULL);

	_ui.btnShowWaveforms->setEnabled(true);
	_ui.btnRelocate->setEnabled(true);
	_ui.cbLocatorProfile->setEnabled(true);
	_ui.btnCommit->setEnabled(true);

	//_ui.cbFixedDepth->setChecked(Qt::Unchecked);
	Time t = _currentOrigin->time();
	Regions regions;
	_ui.labelRegion->setText(regions.getRegionName(_currentOrigin->latitude(), _currentOrigin->longitude()).c_str());
	//timeToLabel(_ui.labelDate, timeToString(t, "%Y-%m-%d");
	timeToLabel(_ui.labelTime, t, "%Y-%m-%d %H:%M:%S");

	double radius;
	if ( _config.defaultEventRadius > 0 )
		radius = _config.defaultEventRadius;
	else {
		radius = 20;
		try {
			radius = std::min(radius, _currentOrigin->quality().maximumDistance()+0.1);
		}
		catch ( ... ) {}
	}

	if ( _undoList.isEmpty() ) {
		_map->canvas().displayRect(QRectF(_currentOrigin->longitude()-radius, _currentOrigin->latitude()-radius, radius*2, radius*2));
	}

	_map->canvas().setMapCenter(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()));
	//_map->setView(QPointF(_currentOrigin->longitude().value(), _currentOrigin->latitude().value()), _map->zoomLevel());
	_map->setOrigin(_currentOrigin.get());
	_map->update();
	if ( _toolMap ) {
		if ( _undoList.isEmpty() )
			_toolMap->canvas().displayRect(QRectF(_currentOrigin->longitude()-radius, _currentOrigin->latitude()-radius, radius*2, radius*2));

		_toolMap->canvas().setMapCenter(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()));
		//_toolMap->setView(QPointF(_currentOrigin->longitude().value(), _currentOrigin->latitude().value()), _toolMap->zoomLevel());
		_toolMap->setOrigin(_currentOrigin.get());
		_toolMap->update();
	}

	_ui.labelLatitude->setText(latitudeToString(_currentOrigin->latitude(), true, false, SCScheme.precision.location));
	_ui.labelLatitudeUnit->setText(latitudeToString(_currentOrigin->latitude(), false, true));
	//_ui.labelLatitudeUnit->setText("deg");
	try {
		_ui.labelLatitudeError->setText(QString("+/- %1").arg(quantityUncertainty(_currentOrigin->latitude()), 0, 'f', SCScheme.precision.uncertainties));
		_ui.labelLatitudeErrorUnit->setText("km");
	}
	catch ( ValueException& ) {
		_ui.labelLatitudeError->setText("");
		_ui.labelLatitudeErrorUnit->setText("");
	}

	_ui.labelLongitude->setText(longitudeToString(_currentOrigin->longitude(), true, false, SCScheme.precision.location));
	_ui.labelLongitudeUnit->setText(longitudeToString(_currentOrigin->longitude(), false, true, SCScheme.precision.location));
	//_ui.labelLongitudeUnit->setText("deg");
	try {
		_ui.labelLongitudeError->setText(QString("+/- %1").arg(quantityUncertainty(_currentOrigin->longitude()), 0, 'f', SCScheme.precision.uncertainties));
		_ui.labelLongitudeErrorUnit->setText("km");
	}
	catch ( ValueException& ) {
		_ui.labelLongitudeError->setText("");
		_ui.labelLongitudeErrorUnit->setText("");
	}

	try {
		_ui.labelDepth->setText(depthToString(_currentOrigin->depth(), SCScheme.precision.depth));
		_ui.editFixedDepth->setText(_ui.labelDepth->text());
		_ui.labelDepthUnit->setText("km");
	}
	catch ( ValueException& ) {
		_ui.labelDepth->setText("-");
		_ui.editFixedDepth->setText("");
	}

	try {
		double err_z = quantityUncertainty(_currentOrigin->depth());
		if (err_z == 0.0) {
			_ui.labelDepthError->setText(QString("fixed"));
			_ui.labelDepthErrorUnit->setText("");

			//_ui.cbFixedDepth->setChecked(true);
		}
		else {
			_ui.labelDepthError->setText(QString("+/- %1").arg(err_z, 0, 'f', SCScheme.precision.uncertainties));
			_ui.labelDepthErrorUnit->setText("km");

			//_ui.cbFixedDepth->setChecked(false);
		}
	}
	catch ( ValueException& ) {
		_ui.labelDepthError->setText(QString("fixed"));
		_ui.labelDepthErrorUnit->setText("");
	}

	// When an origin has been loaded the depth is released
	_ui.cbFixedDepth->setChecked(false);

	try {
		_ui.labelStdError->setText(QString("%1").arg(_currentOrigin->quality().standardError(), 0, 'f', SCScheme.precision.rms));
	}
	catch ( ValueException& ) {
		_ui.labelStdError->setText("-");
	}

	_ui.labelComment->setText(_displayCommentDefault.c_str());
	if ( _displayComment ) {
		if ( _reader && _currentOrigin->commentCount() == 0 )
			_reader->loadComments(_currentOrigin.get());

		for ( size_t i = 0; i < _currentOrigin->commentCount(); ++i ) {
			if ( _currentOrigin->comment(i)->id() == _displayCommentID ) {
				_ui.labelComment->setText(_currentOrigin->comment(i)->text().c_str());
				break;
			}
		}
	}

	try {
		_ui.labelAzimuthGap->setText(QString("%1").arg(_currentOrigin->quality().azimuthalGap(), 0, 'f', 0));
		//_ui.labelAzimuthGapUnit->setText("deg");
	}
	catch ( ValueException& ) {
		_ui.labelAzimuthGap->setText("-");
	}

	try {
		if ( SCScheme.unit.distanceInKM )
			_ui.labelMinDist->setText(QString("%1").arg(Math::Geo::deg2km(_currentOrigin->quality().minimumDistance()), 0, 'f', SCScheme.precision.distance));
		else
			_ui.labelMinDist->setText(QString("%1").arg(_currentOrigin->quality().minimumDistance(), 0, 'f', 1));
		//_ui.labelMinDistUnit->setText("deg");
	}
	catch ( ValueException& ) {
		_ui.labelMinDist->setText("-");
	}

	try {
		try {
			timeToLabel(_ui.labelCreated, _currentOrigin->creationInfo().modificationTime(), "%Y-%m-%d %H:%M:%S");
			try {
				_ui.labelCreated->setToolTip(tr("Creation time: %1").arg(timeToString(_currentOrigin->creationInfo().creationTime(), "%Y-%m-%d %H:%M:%S")));
			}
			catch ( ... ) {}
		}
		catch ( ... ) {
			timeToLabel(_ui.labelCreated, _currentOrigin->creationInfo().creationTime(), "%Y-%m-%d %H:%M:%S");
			_ui.labelCreated->setToolTip(tr("That is actually the creation time"));
		}
	}
	catch ( ValueException& ) {
		_ui.labelCreated->setText("");
	}

	int activeArrivals = 0;
	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		Arrival* arrival = _currentOrigin->arrival(i);

		Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		QColor baseColor, pickColor;
		if ( i%2 )
			baseColor = Qt::gray;
		else
			baseColor = Qt::lightGray;

		Time pickTime;

		if ( pick ) {
			try {
				switch ( pick->evaluationMode() ) {
					case MANUAL:
						pickColor = SCScheme.colors.arrivals.manual;
						break;
					case AUTOMATIC:
						pickColor = SCScheme.colors.arrivals.automatic;
						break;
					default:
						pickColor = SCScheme.colors.arrivals.undefined;
						break;
				};
			}
			catch ( ... ) {
				pickColor = SCScheme.colors.arrivals.undefined;
			}
			pickTime = pick->time().value();
		}
		else
			pickColor = SCScheme.colors.arrivals.undefined;

		addArrival(i, arrival, pickTime, pickColor);

		_modelArrivals.setUseArrival(i, arrival);

		QColor pickStateColor = pickColor;
		if ( !_modelArrivals.useArrival(i) )
			pickStateColor = SCScheme.colors.arrivals.disabled;
		else
			++activeArrivals;

		_modelArrivals.setRowColor(i, pickStateColor);

		/*
		try {
			addArrival(arrival, SCScheme.colors.arrivals.residuals.colorAt(arrival->residual()));
		}
		catch ( Core::ValueException& ) {
			addArrival(arrival, SCScheme.colors.arrivals.undefined);
		}
		*/
	}

	if ( _baseEvent ) {
		_ui.labelEventID->setText(_baseEvent->publicID().c_str());
		_ui.labelEventID->setToolTip(_baseEvent->publicID().c_str());
	}
	else {
		_ui.labelEventID->setText("-");
		_ui.labelEventID->setToolTip("");
	}

	try {
		_ui.labelAgency->setText(_currentOrigin->creationInfo().agencyID().c_str());
		_ui.labelAgency->setToolTip(_currentOrigin->creationInfo().agencyID().c_str());
	}
	catch ( Core::ValueException & ) {
		_ui.labelAgency->setText("-");
		_ui.labelAgency->setToolTip("");
	}

	try {
		_ui.labelUser->setText(_currentOrigin->creationInfo().author().c_str());
		_ui.labelUser->setToolTip(_currentOrigin->creationInfo().author().c_str());
	}
	catch ( Core::ValueException & ) {
		_ui.labelUser->setText("-");
		_ui.labelUser->setToolTip("");
	}

	QPalette pal = _ui.labelEvaluation->palette();
	pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
	_ui.labelEvaluation->setPalette(pal);

	QString evalMode;
	try {
		evalMode = _currentOrigin->evaluationStatus().toString();
		if ( _currentOrigin->evaluationStatus() == REJECTED ) {
			QPalette pal = _ui.labelEvaluation->palette();
			pal.setColor(QPalette::WindowText, Qt::red);
			_ui.labelEvaluation->setPalette(pal);
		}
	}
	catch ( ... ) {
		evalMode = "-";
	}

	try {
		if ( _currentOrigin->evaluationMode() == AUTOMATIC )
			evalMode += " (A)";
		else if ( _currentOrigin->evaluationMode() == MANUAL )
			evalMode += " (M)";
		else
			evalMode += " (-)";
	}
	catch ( ... ) {
		evalMode += " (-)";
	}

	_ui.labelEvaluation->setText(evalMode);
	_ui.labelMethod->setText(_currentOrigin->methodID().c_str());
	_ui.labelEarthModel->setText(_currentOrigin->earthModelID().c_str());

	_ui.labelNumPhases->setText(QString("%1").arg(activeArrivals));
	_ui.labelNumPhasesError->setText(QString("%1").arg(_currentOrigin->arrivalCount()));

	_residuals->updateBoundingRect();

	plotTabChanged(_plotTab->currentIndex());

	//_ui.tableArrivals->resizeColumnsToContents();
	_ui.tableArrivals->resizeRowsToContents();
	_ui.tableArrivals->sortByColumn(_ui.tableArrivals->horizontalHeader()->sortIndicatorSection());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addArrival(int idx, DataModel::Arrival* arrival,
                                   const Core::Time &time, const QColor& c) {
	int id = _residuals->count();
	_residuals->addValue(QPointF());

	_residuals->setValueColor(id, PC_DISTANCE, c);
	_residuals->setValueColor(id, PC_RESIDUAL, c);
	_residuals->setValueColor(id, PC_TRAVELTIME, c);
	_residuals->setValueColor(id, PC_FMAZI, c);
	//_residuals->setValueColor(id, 3, c);

	double dist = -1;
	try { dist = arrival->distance(); } catch ( ... ) {}

	char phase = Util::getShortPhaseName(arrival->phase().code());
	if ( phase == 'S' )
		_residuals->setValueSymbol(id, DiagramWidget::Rectangle);
	else
		_residuals->setValueSymbol(id, DiagramWidget::Circle);

	try {
		_residuals->setValueColor(id, PC_AZIMUTH, SCScheme.colors.arrivals.residuals.colorAt(arrival->timeResidual()));
	}
	catch ( Core::ValueException& ) {
		_residuals->setValueColor(id, PC_AZIMUTH, SCScheme.colors.arrivals.undefined);
		//_residuals->setValueColor(id, 3, SCScheme.colors.arrivals.undefined);
	}

	_residuals->setValueColor(id, PC_REDUCEDTRAVELTIME, c);

	try {
		if ( SCScheme.unit.distanceInKM )
			_residuals->setValue(id, PC_DISTANCE, Math::Geo::deg2km(dist));
		else
			_residuals->setValue(id, PC_DISTANCE, dist);
	}
	catch ( ValueException& ) {
		_residuals->setValue(id, PC_DISTANCE, 0.0);
		_residuals->setValueValid(id, PC_DISTANCE, false);
	}

	try {
		_residuals->setValue(id, PC_RESIDUAL, arrival->timeResidual());
	}
	catch ( ValueException& ) {
		_residuals->setValue(id, PC_RESIDUAL, 0.0);
		_residuals->setValueValid(id, PC_RESIDUAL, false);
	}

	if ( time )
		_residuals->setValue(id, PC_TRAVELTIME, (float)(time - _currentOrigin->time().value()));
	else {
		_residuals->setValue(id, PC_TRAVELTIME, 0.0);
		_residuals->setValueValid(id, PC_TRAVELTIME, false);
	}

	try {
		_residuals->setValue(id, PC_AZIMUTH, arrival->azimuth());
	}
	catch ( ValueException& ) {
		_residuals->setValue(id, PC_AZIMUTH, 0.0);
		_residuals->setValueValid(id, PC_AZIMUTH, false);
	}

	if ( _residuals->isValueValid(id, PC_DISTANCE) && _residuals->isValueValid(id, PC_TRAVELTIME) ) {
		if ( SCScheme.unit.distanceInKM )
			_residuals->setValue(id, PC_REDUCEDTRAVELTIME, _residuals->value(id,PC_TRAVELTIME) - _residuals->value(id,PC_DISTANCE)/_config.reductionVelocityP);
		else
			_residuals->setValue(id, PC_REDUCEDTRAVELTIME, _residuals->value(id,PC_TRAVELTIME) - Math::Geo::deg2km(_residuals->value(id,PC_DISTANCE))/_config.reductionVelocityP);
	}
	else {
		_residuals->setValue(id, PC_REDUCEDTRAVELTIME, 0.0);
		_residuals->setValueValid(id, PC_REDUCEDTRAVELTIME, false);
	}

	if ( _residuals->isValueValid(id, PC_DISTANCE) &&
		 _residuals->isValueValid(id, PC_AZIMUTH) &&
		 phase == 'P' && _currentOrigin ) {

		PlotWidget::PolarityType polarity = PlotWidget::POL_UNSET;
		Pick *pick = Pick::Find(arrival->pickID());
		if ( pick ) {
			try {
				switch ( pick->polarity() ) {
					case POSITIVE:
						polarity = PlotWidget::POL_POSITIVE;
						break;
					case NEGATIVE:
						polarity = PlotWidget::POL_NEGATIVE;
						break;
					case UNDECIDABLE:
						polarity = PlotWidget::POL_UNDECIDABLE;
						break;
					default:
						break;
				}
			}
			catch ( ... ) {}

			if ( Util::getShortPhaseName(arrival->phase().code()) != 'P' )
				polarity = PlotWidget::POL_UNSET;

			_residuals->setValue(id, PC_POLARITY, polarity);
		}

		double beta;
		bool hasTakeOff;
		try {
			beta = arrival->takeOffAngle();
			hasTakeOff = true;
		}
		catch ( ... ) {
			hasTakeOff = false;
		}

		if ( !hasTakeOff && _config.computeMissingTakeOffAngles ) {
			double lat, lon;
			double azi = _residuals->value(id, PC_AZIMUTH);

			Math::Geo::delandaz2coord(
				dist, azi,
				_currentOrigin->latitude(), _currentOrigin->longitude(),
				&lat, &lon
			);

			try {
				TravelTime ttt = _ttTable.computeFirst(
					_currentOrigin->latitude(), _currentOrigin->longitude(),
					_currentOrigin->depth(), lat, lon
				);

				beta = ttt.takeoff;
				_modelArrivals.setTakeOffAngle(idx, beta);

				hasTakeOff = true;
			}
			catch ( ... ) {}
		}

		if ( hasTakeOff && static_cast<PlotWidget*>(_residuals)->shape(polarity).shown ) {
			double azi;
			azi = _residuals->value(id, PC_AZIMUTH);

			if ( beta > 90 ) {
				beta = 180-beta;
				azi = azi-180;
				if ( azi < 0 ) azi += 360;
			}

			beta = sqrt(2.0) * sin(0.5*deg2rad(beta));

			//if ( static_cast<PlotWidget*>(_residuals)->shape(polarity).colorUsed )
			//	_residuals->setValueColor(id, PC_FMAZI, static_cast<PlotWidget*>(_residuals)->shape(polarity).color);

			_residuals->setValue(id, PC_FMAZI, azi);
			_residuals->setValue(id, PC_FMDIST, beta);
		}
		else {
			_residuals->setValue(id, PC_FMDIST, 0.0);
			_residuals->setValue(id, PC_FMAZI, 0.0);
			_residuals->setValueValid(id, PC_FMDIST, false);
			_residuals->setValueValid(id, PC_FMAZI, false);
		}
	}
	else {
		_residuals->setValue(id, PC_FMDIST, 0.0);
		_residuals->setValue(id, PC_FMAZI, 0.0);
		_residuals->setValueValid(id, PC_FMDIST, false);
		_residuals->setValueValid(id, PC_FMAZI, false);
	}

	if ( _plotFilter )
		_residuals->showValue(id, _plotFilter->accepts(_residuals, id));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::addPick(Seiscomp::DataModel::Pick* pick) {
	if ( _recordView )
		_recordView->addPick(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setStationEnabled(const std::string& networkCode,
                                          const std::string& stationCode,
                                          bool state) {
	if ( _recordView )
		_recordView->setStationEnabled(networkCode, stationCode, state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::importArrivals() {
	stopBlinking();

	if ( !_reader ) return;

	EventPtr event = _baseEvent;

	if ( !event ) {
		event = Event::Cast(_reader->getEvent(_baseOrigin->publicID()));
		if ( !event )
			event = Event::Cast(_reader->getEvent(_currentOrigin->publicID()));

		if ( !event ) {
			QMessageBox::critical(this, "ImportPicks::Error",
			                      "This location has not been associated with an event");
			return;
		}
	}

	ImportPicksDialog *dlg = new ImportPicksDialog(this);
	if ( dlg->exec() == QDialog::Rejected ) {
		delete dlg;
		return;
	}

	ImportPicksDialog::Selection sel = dlg->currentSelection();
	bool importAllPicks = dlg->importAllPicks();
	bool importAllPhases = dlg->importAllPhases();
	bool preferTargetPhases = dlg->preferTargetPhases();
	delete dlg;

	qApp->setOverrideCursor(Qt::WaitCursor);

	OriginPtr referenceOrigin;
	bool associateOnly = false;

	DataModel::PublicObjectTimeSpanBuffer cache(_reader, Core::TimeSpan(3600,0));
	typedef std::pair<std::string,int> PhaseWithFlags;
	typedef std::map<std::string, PhaseWithFlags> PhasePicks;

	PhasePicks sourcePhasePicks;

	switch ( sel ) {
		case ImportPicksDialog::LatestOrigin:
			{
				Core::Time maxTime;
				OriginPtr latestOrigin;
				DatabaseIterator or_it = _reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					++or_it;

					try {
						if ( o->creationInfo().creationTime() > maxTime ) {
							latestOrigin = o;
							maxTime = o->creationInfo().creationTime();
							SEISCOMP_DEBUG("MaxTime: %s, Origin: %s", Core::toString(maxTime).c_str(), o->publicID().c_str());
						}
					}
					catch ( ... ) { continue; }
				}
				or_it.close();

				if ( !latestOrigin || latestOrigin->publicID() == _currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is no later origin than the current");
					QMessageBox::information(this, "ImportPicks", "There is no later origin than the current.");
					qApp->restoreOverrideCursor();
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					_reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::LatestAutomaticOrigin:
			{
				Core::Time maxTime;
				OriginPtr latestOrigin;
				DatabaseIterator or_it = _reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					++or_it;

					// try {
					// 	if ( o->status() != AUTOMATIC_ORIGIN ) { continue; }
					// }
					// catch ( ... ) {}

					try {
						if ( o->evaluationMode() != AUTOMATIC )
							continue;
					}
					catch ( ... ) {}

					try {
						if ( o->creationInfo().creationTime() > maxTime ) {
							latestOrigin = o;
							maxTime = o->creationInfo().creationTime();
							SEISCOMP_DEBUG("MaxTime: %s, Origin: %s", Core::toString(maxTime).c_str(), o->publicID().c_str());
						}
					}
					catch ( ... ) { continue; }
				}
				or_it.close();

				if ( !latestOrigin || latestOrigin->publicID() == _currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is no later origin than the current");
					QMessageBox::information(this, "ImportPicks", "There is no later origin than the current.");
					qApp->restoreOverrideCursor();
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					_reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::MaxPhaseOrigin:
			{
				size_t maxPhase = 0;
				OriginPtr latestOrigin;
				std::vector<OriginPtr> origins;
				DatabaseIterator or_it = _reader->getOrigins(event->publicID());
				while ( *or_it ) {
					OriginPtr o = Origin::Cast(*or_it);
					if ( o ) origins.push_back(o);
					++or_it;
				}
				or_it.close();

				for ( size_t i = 0; i < origins.size(); ++i ) {
					OriginPtr o = origins[i];
					if ( o->arrivalCount() == 0 )
						_reader->loadArrivals(o.get());

					if ( o->arrivalCount() > maxPhase ) {
						latestOrigin = o;
						maxPhase = o->arrivalCount();
						SEISCOMP_DEBUG("MaxPhase: %lu, Origin: %s", (unsigned long)maxPhase, o->publicID().c_str());
					}
				}

				if ( !latestOrigin || latestOrigin->publicID() == _currentOrigin->publicID() ) {
					SEISCOMP_DEBUG("There is origin with more phases than the current");
					qApp->restoreOverrideCursor();
					QMessageBox::information(this, "ImportPicks", "There is no origin with more phases than the current.");
					return;
				}

				referenceOrigin = latestOrigin;
				if ( referenceOrigin->arrivalCount() == 0 )
					_reader->loadArrivals(referenceOrigin.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < referenceOrigin->arrivalCount(); ++i ) {
					Arrival *ar = referenceOrigin->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}
			break;

		case ImportPicksDialog::AllOrigins:
		{
			std::vector<OriginPtr> origins;
			DatabaseIterator or_it = _reader->getOrigins(event->publicID());
			while ( *or_it ) {
				OriginPtr o = Origin::Cast(*or_it);
				if ( o ) origins.push_back(o);
				++or_it;
			}
			or_it.close();

			for ( size_t i = 0; i < origins.size(); ++i ) {
				OriginPtr o = origins[i];
				if ( o->arrivalCount() == 0 )
					_reader->loadArrivals(o.get());

				// Collect all picks with phases
				for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
					Arrival *ar = o->arrival(i);
					try { sourcePhasePicks[ar->pickID()] = PhaseWithFlags(ar->phase().code(), Seismology::arrivalToFlags(ar)); }
					catch ( ... ) {}
				}
			}

			associateOnly = true;
			break;
		}
	};

	PickedPhases sourcePhases, targetPhases, *sourcePhasesPtr, *targetPhasesPtr;

	// Collect source phases grouped by stream
	for ( PhasePicks::iterator it = sourcePhasePicks.begin(); it != sourcePhasePicks.end(); ++it ) {
		PickPtr pick = cache.get<Pick>(it->first);
		if ( !pick ) {
			SEISCOMP_WARNING("Pick %s not found: ignoring", it->first.c_str());
			continue;
		}

		// Filter agency
		if ( !importAllPicks && (objectAgencyID(pick.get()) != SCApp->agencyID()) )
			continue;

		char phaseCode[2] = {'\0', '\0'};
		try { phaseCode[0] = Util::getShortPhaseName(it->second.first); }
		catch ( ... ) {}

		if ( phaseCode[0] == '\0' )
			phaseCode[0] = 'P';

		if ( !importAllPhases )
			sourcePhases[PickPhase(pick->waveformID().networkCode() + "." + pick->waveformID().stationCode(), phaseCode)] = PickWithFlags(pick, it->second.second);
		else
			sourcePhases[PickPhase(wfid2str(pick->waveformID()), it->second.first)] = PickWithFlags(pick, it->second.second);
	}

	// Collect target phases grouped by stream
	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		Arrival *ar = _currentOrigin->arrival(i);
		PickPtr p = cache.get<Pick>(ar->pickID());

		if ( !p ) continue;

		WaveformStreamID &wfsi = p->waveformID();

		char phaseCode[2] = {'\0', '\0'};
		try { phaseCode[0] = Util::getShortPhaseName(ar->phase().code()); }
		catch ( ... ) {}

		if ( phaseCode[0] == '\0' )
			phaseCode[0] = 'P';

		int flags = Seismology::arrivalToFlags(ar);

		if ( !importAllPhases )
			targetPhases[PickPhase(wfsi.networkCode() + "." + wfsi.stationCode(), phaseCode)] = PickWithFlags(p, flags);
		else
			targetPhases[PickPhase(wfid2str(wfsi), ar->phase().code())] = PickWithFlags(p, flags);
	}

	sourcePhasesPtr = &sourcePhases;
	targetPhasesPtr = &targetPhases;

	if ( !preferTargetPhases )
		std::swap(sourcePhasesPtr, targetPhasesPtr);

	qApp->restoreOverrideCursor();

	if ( !merge(sourcePhasesPtr, targetPhasesPtr, true, associateOnly, preferTargetPhases) ) {
		SEISCOMP_DEBUG("No additional picks to merge");
		QMessageBox::information(this, "ImportPicks", "There are no additional "
		                         "streams with picks to merge.");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::merge(void *sourcePhases, void *targetPhases,
                              bool checkDuplicates, bool associateOnly,
                              bool failOnNoNewPhases) {
	PickedPhases *sourcePhasesPtr, *targetPhasesPtr;
	set<string> usedPickIDs;

	std::vector<PhasePickWithFlags> additionalPicks;
	typedef std::pair<std::string, std::string> PhaseStream;
	typedef std::map<PhaseStream, PickWithFlags> NewPhases;

	NewPhases newPhases;

	sourcePhasesPtr = reinterpret_cast<PickedPhases*>(sourcePhases);
	targetPhasesPtr = reinterpret_cast<PickedPhases*>(targetPhases);

	for ( PickedPhases::iterator it = targetPhasesPtr->begin(); it != targetPhasesPtr->end(); ++it ) {
		PickPtr p = it->second.first;
		usedPickIDs.insert(p->publicID());
	}

	// Merge source phases with target phases
	for ( PickedPhases::iterator it = sourcePhasesPtr->begin(); it != sourcePhasesPtr->end(); ++it ) {
		PickPtr p = it->second.first;

		// Do we have the same phase already in the target?
		if ( checkDuplicates && targetPhasesPtr->find(it->first) != targetPhasesPtr->end() ) {
			SEISCOMP_INFO("- phase %s for stream %s (already in target)",
			              it->first.second.c_str(), it->first.first.c_str());
			continue;
		}

		// Final check, if the same pick id is already or will be associated
		// with current origin
		if ( usedPickIDs.find(p->publicID()) != usedPickIDs.end() ) {
			SEISCOMP_INFO("- pick %s as phase %s for stream %s (pick already in target)",
			              p->publicID().c_str(), it->first.second.c_str(),
			              it->first.first.c_str());
			continue;
		}

		usedPickIDs.insert(p->publicID());

		SEISCOMP_INFO("+ pick %s as phase %s for stream %s",
		              p->publicID().c_str(), it->first.second.c_str(),
		              it->first.first.c_str());

		PhaseStream ps(it->first);
		PickWithFlags newPick = newPhases[ps];

		if ( newPick.first ) {
			try {
				// Pick is older than the already inserted one: skip it
				if ( p->creationInfo().creationTime() < newPick.first->creationInfo().creationTime() )
					continue;
			}
			catch ( ... ) {
				// No creationTime set: take the first one
				continue;
			}
		}

		newPhases[ps] = PickWithFlags(p, it->second.second);
	}

	if ( failOnNoNewPhases && newPhases.empty() ) return false;

	SEISCOMP_DEBUG("*** Prepare merged origin ***");
	OriginPtr org = Origin::Create();
	org->assign(_currentOrigin.get());
	for ( PickedPhases::iterator it = targetPhasesPtr->begin(); it != targetPhasesPtr->end(); ++it ) {
		ArrivalPtr arrival = new Arrival;
		arrival->setPickID(it->second.first->publicID());
		arrival->setWeight(it->second.second ? 0 : 1);
		Seismology::flagsToArrival(arrival.get(), it->second.second);
		arrival->setPhase(Phase(it->first.second));
		org->add(arrival.get());
		SEISCOMP_DEBUG("! pick %s as phase %s for stream %s with flags %d",
		               it->second.first->publicID().c_str(), it->first.second.c_str(),
		               it->first.first.c_str(), it->second.second);
	}

	for ( NewPhases::iterator it = newPhases.begin(); it != newPhases.end(); ++it ) {
		PhasePickWithFlags ppwf;
		ppwf.pick = it->second.first;
		ppwf.phase = it->first.second;
		ppwf.flags = it->second.second;
		additionalPicks.push_back(ppwf);
		SEISCOMP_DEBUG("A pick %s as phase %s for stream %s with flags %d",
		               it->second.first->publicID().c_str(), it->first.second.c_str(),
		               wfid2str(it->second.first->waveformID()).c_str(),
		               it->second.second);
	}

	if ( org->arrivalCount() == 0 ) {
		for ( size_t i = 0; i < additionalPicks.size(); ++i ) {
			SensorLocation *sloc = _locator->getSensorLocation(Pick::Find(additionalPicks[i].pick->publicID()));
			if ( sloc == NULL ) continue;

			ArrivalPtr arrival = new Arrival();
			arrival->setPickID(additionalPicks[i].pick->publicID());
			Seismology::flagsToArrival(arrival.get(), 0);
			arrival->setWeight(0.0);

			double az, baz, dist;
			Math::Geo::delazi(org->latitude().value(), org->longitude().value(),
			                  sloc->latitude(), sloc->longitude(), &dist, &az, &baz);

			arrival->setDistance(dist);
			arrival->setAzimuth(az);

			try {
				if ( additionalPicks[i].phase != "" )
					arrival->setPhase(additionalPicks[i].phase);
				else
					arrival->setPhase(Phase("P"));
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}

			TravelTime tt;
			double depth = 10.0, elev = 0.0;
			try { depth = org->depth().value(); }
			catch ( ... ) {}
			try { elev = sloc->elevation(); }
			catch ( ... ) {}

			try {
				tt = _ttTable.compute(arrival->phase().code().c_str(),
				                      org->latitude().value(), org->longitude().value(), depth,
				                      sloc->latitude(), sloc->longitude(), elev);

				double at = (double)(additionalPicks[i].pick->time().value()-org->time().value());
				arrival->setTimeResidual(at-tt.time);
			}
			catch ( ... ) {}

			org->add(arrival.get());
		}

		applyNewOrigin(org.get(), true);
	}
	else
		relocate(org.get(), &additionalPicks, associateOnly, false, false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::showWaveforms() {
	if ( _recordView ) {
		_recordView->activateWindow();
		_recordView->raise();
		return;
	}

	if ( !_currentOrigin ) return;

	//std::cout << "Number of objects before: " << Core::BaseObject::ObjectCount() << std::endl;
	_recordView = new PickerView(NULL, Qt::Window);
	_recordView->setDatabase(_reader);

	try {
		_recordView->setBroadBandCodes(SCApp->configGetStrings("picker.velocityChannelCodes"));
	} catch ( ... ) {}

	try {
		_recordView->setStrongMotionCodes(SCApp->configGetStrings("picker.accelerationChannelCodes"));
	} catch ( ... ) {}

	QString errorMsg;
	if ( !_recordView->setConfig(_pickerConfig, &errorMsg) ) {
		QMessageBox::information(this, "Picker Error", errorMsg);
		delete _recordView;
		_recordView = NULL;
		return;
	}

	/*
	for ( QVector<QPair<QString,QString> >::const_iterator it = _filters.begin();
	      it != _filters.end(); ++it ) {
		_recordView->addFilter(it->first, it->second);
	}

	_recordView->activateFilter(0);

	if ( !_recordView->setDataSource(_streamURL.c_str()) ) {
		QMessageBox::information(this, "RecordStream Error",
		                         QString("Setting recordstream '%1' failed.")
		                           .arg(_streamURL.c_str()));
		delete _recordView;
		_recordView = NULL;
		return;
	}
	*/

	_recordView->setAttribute(Qt::WA_DeleteOnClose);

	setPickerView(_recordView);

	_recordView->setOrigin(_currentOrigin.get(), -5*60, 30*60);

	QVector<int> selectedArrivals = _residuals->getSelectedValues();
	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i )
		_recordView->setArrivalState(i, _residuals->isValueSelected(i));

	_recordView->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate() {
	relocate(_currentOrigin.get(), NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate(std::vector<PhasePickWithFlags>* additionalPicks,
                                 bool associateOnly, bool replaceExistingPhases) {
	relocate(_currentOrigin.get(), additionalPicks, associateOnly, replaceExistingPhases);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::relocate(DataModel::Origin *org,
                                 std::vector<PhasePickWithFlags>* additionalPicks,
                                 bool associateOnly, bool replaceExistingPhases,
                                 bool useArrivalTable) {
	OriginPtr oldOrigin = org;
	OriginPtr origin;

	if ( !_locator ) {
		QMessageBox::critical(this, "Locator error", "No locator set.");
		return;
	}

	if ( _modelArrivals.useNoArrivals() ) {
		QMessageBox::critical(this, "Relocation error", "A relocation cannot be done without any arrivals.");
		return;
	}

	_locator->setProfile(_ui.cbLocatorProfile->currentText().toStdString());

	oldOrigin = Origin::Create();
	oldOrigin->assign(org);

	if ( useArrivalTable ) {
		for ( int i = 0; i < _modelArrivals.rowCount(); ++i ) {
			if ( !_modelArrivals.isRowEnabled(i) ) continue;

			ArrivalPtr arrival = new Arrival(*org->arrival(i));
			arrival->setBackazimuthUsed(_modelArrivals.backazimuthUsed(i));
			arrival->setTimeUsed(_modelArrivals.timeUsed(i));
			arrival->setHorizontalSlownessUsed(_modelArrivals.horizontalSlownessUsed(i));

			if ( arrival->timeUsed() || arrival->backazimuthUsed() || arrival->horizontalSlownessUsed() )
				arrival->setWeight(1.0);
			else
				arrival->setWeight(0.0);

			if ( !_locator->getSensorLocation(Pick::Find(arrival->pickID())) )
				continue;

			oldOrigin->add(arrival.get());

			try {
				if ( arrival->phase().code() == "" )
					arrival->setPhase(Phase("P"));
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}
		}
	}
	else {
		for ( size_t i = 0; i < org->arrivalCount(); ++i ) {
			ArrivalPtr ar = org->arrival(i);

			try {
				if ( !_locator->getSensorLocation(Pick::Find(ar->pickID())) &&
					 ar->weight() < 0.5 )
					continue;
			}
			catch ( ... ) {}

			ArrivalPtr ar2 = new Arrival(*ar);

			oldOrigin->add(ar2.get());
		}
	}

	if ( replaceExistingPhases ) {
		while ( oldOrigin->arrivalCount() > 0 )
			oldOrigin->removeArrival(0);
	}

	if ( additionalPicks ) {
		for ( size_t i = 0; i < additionalPicks->size(); ++i ) {
			SensorLocation *sloc = _locator->getSensorLocation(Pick::Find((*additionalPicks)[i].pick->publicID()));
			if ( sloc == NULL ) continue;

			ArrivalPtr arrival = new Arrival();
			arrival->setPickID((*additionalPicks)[i].pick->publicID());

			if ( associateOnly ) {
				Seismology::flagsToArrival(arrival.get(), 0);
				arrival->setWeight(0.0);
			}
			else {
				Seismology::flagsToArrival(arrival.get(), (*additionalPicks)[i].flags);
				arrival->setWeight(1.0);
			}

			try {
				if ( (*additionalPicks)[i].phase != "" )
					arrival->setPhase((*additionalPicks)[i].phase);
				else
					arrival->setPhase(Phase("P"));
			}
			catch ( ... ) {
				arrival->setPhase(Phase("P"));
			}

			oldOrigin->add(arrival.get());
		}
	}

	bool fixedDepth = _ui.cbFixedDepth->isEnabled() && _ui.cbFixedDepth->isChecked();
	bool distanceCutOff = _ui.cbDistanceCutOff->isEnabled() && _ui.cbDistanceCutOff->isChecked();
	bool ignoreInitialLocation = _ui.cbIgnoreInitialLocation->isEnabled() && _ui.cbIgnoreInitialLocation->isChecked();

	if ( distanceCutOff )
		_locator->setDistanceCutOff(_ui.editDistanceCutOff->text().toDouble());
	else
		_locator->releaseDistanceCutOff();

	_locator->setIgnoreInitialLocation(ignoreInitialLocation);

	setCursor(Qt::WaitCursor);

	for ( int loop = 1; loop <= 2; ++loop ) {
		if ( fixedDepth ) {
			double depth = loop == 1 ? _ui.editFixedDepth->text().toDouble() : _minimumDepth;

			_locator->setFixedDepth(depth);

			SEISCOMP_DEBUG("setting depth to %.2f km", depth);
		}
		else
			_locator->releaseDepth();

		try {
			origin = Gui::relocate(_locator.get(), oldOrigin.get());
			/* DEBUG: Just copy the origin without relocating
			origin = Origin::Cast(oldOrigin->clone());
			origin->assign(oldOrigin.get());
			for ( size_t i = 0; i < oldOrigin->arrivalCount(); ++i ) {
				origin->add(Arrival::Cast(oldOrigin->arrival(i)->clone()));
			}
			*/

			if ( !origin ) {
				QMessageBox::critical(this, "Relocation error", "The relocation failed for some reason.");
				unsetCursor();
				return;
			}

			string msgWarning = _locator->lastMessage(Seismology::LocatorInterface::Warning);
			if ( !msgWarning.empty() ) {
				QMessageBox::warning(this, "Relocation warning", msgWarning.c_str());
			}
		}
		catch ( Core::GeneralException& e ) {
			// If relocation is enabled and fails retry it and just
			// associate
			if ( !associateOnly && additionalPicks ) {
				QMessageBox::critical(this, "Relocation error",
				       QString("Relocating failed. The new picks are going to be inserted with zero weight.\n%1").arg(e.what()));
				relocate(org, additionalPicks, true, replaceExistingPhases, useArrivalTable);
			}
			else
				QMessageBox::critical(this, "Relocation error", e.what());

			unsetCursor();
			return;
		}

		if ( !fixedDepth && _locator->supports(Seismology::LocatorInterface::FixedDepth) ) {
			try {
				if ( origin->depth() < _minimumDepth )
					origin->setDepth(RealQuantity(_minimumDepth,0.0,Core::None,Core::None,Core::None));
				else break;
			}
			catch ( ... ) {
				origin->setDepth(RealQuantity(_minimumDepth,0.0,Core::None,Core::None,Core::None));
			}
			oldOrigin = origin;
			fixedDepth = true;
		}
		else
			break;
	}

	unsetCursor();

	if ( fixedDepth ) {
		_ui.cbFixedDepth->setChecked(true);
		origin->setDepthType(OriginDepthType(OPERATOR_ASSIGNED));
	}

	if ( distanceCutOff )
		_ui.cbDistanceCutOff->setChecked(true);

	if ( ignoreInitialLocation )
		_ui.cbIgnoreInitialLocation->setChecked(true);

	applyNewOrigin(origin.get(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::applyNewOrigin(DataModel::Origin *origin, bool relocated) {
	SEISCOMP_DEBUG("Created new origin %s", origin->publicID().c_str());

	origin->setEvaluationMode(EvaluationMode(MANUAL));
	origin->setEvaluationStatus(EvaluationStatus(CONFIRMED));
	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	origin->setCreationInfo(ci);

	pushUndo();

	_localOrigin = true;
	std::vector<DataModel::PickPtr> originPicks;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Pick* p = Pick::Find(origin->arrival(i)->pickID());
		if ( p ) {
			originPicks.push_back(p);
			_associatedPicks[p->publicID()] = p;
		}
	}

	_originPicks = originPicks;

	stopBlinking();

	_blockReadPicks = true;
	updateOrigin(origin);
	_blockReadPicks = false;

	//computeMagnitudes();
	_ui.btnMagnitudes->setEnabled(true);

	if ( _recordView )
		_recordView->setOrigin(origin);

	emit newOriginSet(origin, _baseEvent.get(), _localOrigin, relocated);

	_ui.btnCommit->setText("Commit");
//	_ui.btnCommit->setMenu(_baseEvent?_commitMenu:NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::mergeOrigins(QList<DataModel::Origin*> orgs) {
	DataModel::PublicObjectTimeSpanBuffer cache(_reader, Core::TimeSpan(3600,0));
	typedef std::pair<std::string,double> PhaseWithWeight;
	typedef std::map<std::string, PhaseWithWeight> PhasePicks;

	PhasePicks sourcePhasePicks;

	Ui::MergeOrigins ui;
	QDialog dlg;
	ui.setupUi(&dlg);

	ui.labelInfo->setText(ui.labelInfo->text().arg(orgs.size()));

	if ( dlg.exec() != QDialog::Accepted ) return;

	qApp->setOverrideCursor(Qt::WaitCursor);

	bool importAllPicks = ui.checkAllAgencies->isChecked();

	// First origin is always the target of the drag and drop operation

	// Fill picks
	for ( int i = 0; i < orgs.size(); ++i ) {
		OriginPtr o = orgs[i];
		if ( o->arrivalCount() == 0 )
			_reader->loadArrivals(o.get());

		// Collect all picks with phases
		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
			pair<PhasePicks::iterator,bool> itp;
			Arrival *ar = o->arrival(i);
			itp = sourcePhasePicks.insert(PhasePicks::value_type(ar->pickID(), PhaseWithWeight()));
			// Pick already exists
			if ( itp.second == false ) {
				SEISCOMP_DEBUG("Ignoring pick %s from Origin %s: pick already added to merge list",
				               ar->pickID().c_str(), o->publicID().c_str());
				continue;
			}

			double weight = 1.0; try { weight = ar->weight(); } catch ( ... ) {}
			try { itp.first->second = PhaseWithWeight(ar->phase().code(), weight); }
			catch ( ... ) { itp.first->second = PhaseWithWeight("P", weight); }
		}
	}

	PickedPhases sourcePhases, targetPhases;

	// Collect source phases grouped by stream
	for ( PhasePicks::iterator it = sourcePhasePicks.begin(); it != sourcePhasePicks.end(); ++it ) {
		PickPtr pick = cache.get<Pick>(it->first);
		if ( !pick ) {
			SEISCOMP_WARNING("Pick %s not found: ignoring", it->first.c_str());
			continue;
		}

		// Filter agency
		if ( !importAllPicks && (objectAgencyID(pick.get()) != SCApp->agencyID()) )
			continue;

		sourcePhases[PickPhase(wfid2str(pick->waveformID()), it->second.first)] = PickWithFlags(pick, it->second.second);
	}

	qApp->restoreOverrideCursor();

	Origin *oldCurrent = _currentOrigin.get();

	merge(&sourcePhases, &targetPhases, true, false, false);

	if ( oldCurrent != _currentOrigin ) emit locatorRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setLocalAmplitudes(Seiscomp::DataModel::Origin *org,
                                           AmplitudeSet *amps, StringSet *ampIDs) {
	if ( org != _currentOrigin ) return;

	for ( AmplitudeSet::iterator it = _changedAmplitudes.begin();
	      it != _changedAmplitudes.end(); ++it ) {
		if ( ampIDs->find(it->first->publicID()) != ampIDs->end() )
			amps->insert(*it);
	}

	// Store new amplitudes in current set
	_changedAmplitudes.swap(*amps);
	emit requestRaise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::computeMagnitudes() {
	emit computeMagnitudesRequested();

	_ui.btnMagnitudes->setEnabled(_currentOrigin->magnitudeCount() == 0);

	if ( _currentOrigin->magnitudeCount() > 0 ) {
		emit magnitudesAdded(_currentOrigin.get(), _baseEvent.get());
		evaluateOrigin(_currentOrigin.get(), _baseEvent.get(),
		               _localOrigin, false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::magnitudeRemoved(const QString &id, Seiscomp::DataModel::Object *obj) {
	if ( id != _currentOrigin->publicID().c_str() ) return;

	_ui.btnMagnitudes->setEnabled(_currentOrigin->magnitudeCount() == 0);

	if ( _currentOrigin->magnitudeCount() > 0 ) {
		evaluateOrigin(_currentOrigin.get(), _baseEvent.get(),
		               _localOrigin, false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::pushUndo() {
	_undoList.push(
		OriginMemento(
			_currentOrigin.get(), _changedPicks,
			_changedAmplitudes, _localOrigin
		)
	);

	if ( _undoList.size() > 20 )
		_undoList.pop();

	_redoList.clear();

	emit undoStateChanged(!_undoList.isEmpty());
	emit redoStateChanged(!_redoList.isEmpty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::undo() {
	if ( _undoList.isEmpty() ) return false;

	_redoList.push(
		OriginMemento(
			_currentOrigin.get(), _changedPicks,
			_changedAmplitudes, _localOrigin
		)
	);

	OriginPtr origin = _undoList.top().origin;
	bool newOrigin = _undoList.top().newOrigin;
	_changedPicks = _undoList.top().newPicks;
	_changedAmplitudes = _undoList.top().newAmplitudes;
	_undoList.pop();

	_ui.btnMagnitudes->setEnabled(newOrigin && (origin->magnitudeCount() == 0));

	_blockReadPicks = true;
	updateOrigin(origin.get());
	_blockReadPicks = false;

	if ( _recordView )
		_recordView->setOrigin(origin.get());

	_localOrigin = newOrigin;

	emit newOriginSet(origin.get(), _baseEvent.get(), newOrigin, false);
	emit undoStateChanged(!_undoList.isEmpty());
	emit redoStateChanged(!_redoList.isEmpty());

	stopBlinking();

	return !_undoList.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::redo() {
	if ( _redoList.isEmpty() ) return false;

	_undoList.push(
		OriginMemento(
			_currentOrigin.get(), _changedPicks,
			_changedAmplitudes, _localOrigin
		)
	);

	OriginPtr origin = _redoList.top().origin;
	bool newOrigin = _redoList.top().newOrigin;
	_changedPicks = _redoList.top().newPicks;
	_changedAmplitudes = _redoList.top().newAmplitudes;
	_redoList.pop();

	_ui.btnMagnitudes->setEnabled(newOrigin && (origin->magnitudeCount() == 0));

	_blockReadPicks = true;
	updateOrigin(origin.get());
	_blockReadPicks = false;

	if ( _recordView )
		_recordView->setOrigin(origin.get());

	_localOrigin = newOrigin;

	emit newOriginSet(origin.get(), _baseEvent.get(), newOrigin, false);
	emit undoStateChanged(!_undoList.isEmpty());
	emit redoStateChanged(!_redoList.isEmpty());

	stopBlinking();

	return !_redoList.isEmpty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::createArtificialOrigin() {
	createArtificialOrigin(_map->canvas().mapCenter(), QPoint());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::createArtificialOrigin(const QPointF &epicenter,
                                              const QPoint &dialogPos) {
	OriginDialog dialog(this);
	try {
		if ( SCApp->configGetBool("olv.artificialOriginAdvanced") ) {
			dialog.enableAdvancedOptions();

			// build list of available magnitude types
			QStringList magList;
			Processing::MagnitudeProcessorFactory::ServiceNames *names =
			        Processing::MagnitudeProcessorFactory::Services();
			for ( Processing::MagnitudeProcessorFactory::ServiceNames::const_iterator it = names->begin();
			      it != names->end(); ++it) {
				magList << it->c_str();
			}
			dialog.setMagTypes(magList);
		}
	} catch ( Seiscomp::Config::Exception& ) {}

	dialog.loadSettings();
	dialog.setLongitude(epicenter.x());
	dialog.setLatitude(epicenter.y());

	if ( ! dialogPos.isNull() )
		dialog.move(dialogPos.x(), dialogPos.y());

	if ( dialog.exec() == QDialog::Accepted ) {
		dialog.saveSettings();
		OriginPtr origin = Origin::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());
		origin->setCreationInfo(ci);
		origin->setLongitude(dialog.longitude());
		origin->setLatitude(dialog.latitude());
		origin->setDepth(RealQuantity(dialog.depth()));
		origin->setTime(Core::Time(dialog.getTime_t()));
		origin->setEvaluationMode(EvaluationMode(MANUAL));

		if ( dialog.advanced() ) {
			// magnitude
			std::string type = dialog.magType().toStdString();
			std::string id = origin->publicID() + "#netMag." + type;
			MagnitudePtr mag = Magnitude::Create(id);
			mag->setCreationInfo(ci);
			mag->setMagnitude(RealQuantity(dialog.magValue()));
			mag->setType(type);
			mag->setOriginID(origin->publicID());
			mag->setStationCount(dialog.phaseCount());
			origin->add(mag.get());

			// origin quality
			OriginQuality quality;
			quality.setUsedPhaseCount(dialog.phaseCount());
			origin->setQuality(quality);
		}

		emit artificalOriginCreated(origin.get());
	}
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setScript0(const std::string &script) {
	_script0 = script;
	_ui.btnCustom0->setVisible(!_script0.empty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::setScript1(const std::string &script) {
	_script1 = script;
	_ui.btnCustom1->setVisible(!_script1.empty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::editComment() {
	if ( !_baseEvent ) return;

	CommentEdit dlg;
	dlg.ui.labelHeadline->setFont(SCScheme.fonts.highlight);
	dlg.ui.labelAuthor->setText("-");
	dlg.ui.labelDate->setText("-");
	dlg.ui.labelComment->setText("-");

	setItalic(dlg.ui.labelAuthor);
	setItalic(dlg.ui.labelDate);

	for ( size_t i = 0; i < _baseEvent->commentCount(); ++i ) {
		if ( _baseEvent->comment(i)->id() == "Operator" ) {
			try {
				dlg.ui.labelAuthor->setText(_baseEvent->comment(i)->creationInfo().author().c_str());
			}
			catch ( ... ) {}

			try {
				timeToLabel(dlg.ui.labelDate, _baseEvent->comment(i)->creationInfo().modificationTime(), "%F %T");
			}
			catch ( ... ) {
				try {
					timeToLabel(dlg.ui.labelDate, _baseEvent->comment(i)->creationInfo().creationTime(), "%F %T");
				}
				catch ( ... ) {}
			}

			dlg.ui.labelComment->setText(_baseEvent->comment(i)->text().c_str());
			dlg.ui.editComment->setPlainText(dlg.ui.labelComment->text());

			break;
		}
	}

	if ( dlg.exec() != QDialog::Accepted ) return;

	if ( dlg.ui.labelComment->text() != dlg.ui.editComment->toPlainText() )
		sendJournal(_baseEvent->publicID(), "EvOpComment",
		            dlg.ui.editComment->toPlainText().toStdString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commit(bool associate) {
	if ( _newOriginStatus != EvaluationStatus::Quantity ) {
		try {
			if ( _currentOrigin->evaluationMode() == AUTOMATIC )
				_currentOrigin->setEvaluationStatus(_newOriginStatus);
		}
		catch ( ... ) {
			// if evaluationMode isn't set yet we asume AUTOMATIC
			_currentOrigin->setEvaluationStatus(_newOriginStatus);
		}
	}

	try {
		_currentOrigin->creationInfo();
	}
	catch( ... ) {
		_currentOrigin->setCreationInfo(CreationInfo());
	}

	CreationInfo &ci = _currentOrigin->creationInfo();
	ci.setAuthor( SCApp->author() );
	ci.setModificationTime(Core::Time::GMT());
	_ui.labelUser->setText(ci.author().c_str());
	_ui.labelUser->setToolTip(ci.author().c_str());

	// Update evaluation line
	QPalette pal = _ui.labelEvaluation->palette();
	pal.setColor(QPalette::WindowText, palette().color(QPalette::WindowText));
	_ui.labelEvaluation->setPalette(pal);

	QString evalMode;
	try {
		evalMode = _currentOrigin->evaluationStatus().toString();
		if ( _currentOrigin->evaluationStatus() == REJECTED ) {
			QPalette pal = _ui.labelEvaluation->palette();
			pal.setColor(QPalette::WindowText, Qt::red);
			_ui.labelEvaluation->setPalette(pal);
		}
	}
	catch ( ... ) {
		evalMode = "-";
	}

	try {
		if ( _currentOrigin->evaluationMode() == AUTOMATIC )
			evalMode += " (A)";
		else if ( _currentOrigin->evaluationMode() == MANUAL )
			evalMode += " (M)";
		else
			evalMode += " (-)";
	}
	catch ( ... ) {
		evalMode += " (-)";
	}

	_ui.labelEvaluation->setText(evalMode);

	if ( _recordView )
		_recordView->applyPicks();

	ObjectChangeList<DataModel::Pick> pickCommitList;
	std::vector<AmplitudePtr> amplitudeCommitList;

	// collect all picks belonging to the origin
	std::set<PickPtr> originPicks;
	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		PickPtr p = Pick::Find(_currentOrigin->arrival(i)->pickID());
		if ( p ) originPicks.insert(p);
	}

	// intersect the picks in the origin with the already tracked
	// manual created picks
	for ( PickSet::iterator it = _changedPicks.begin();
	      it != _changedPicks.end(); ++it ) {
		if ( originPicks.find(it->first) == originPicks.end() ) continue;
		pickCommitList.push_back(*it);
	}

	for ( AmplitudeSet::iterator it = _changedAmplitudes.begin();
	      it != _changedAmplitudes.end(); ++it ) {
		amplitudeCommitList.push_back(it->first);
	}

	std::cerr << "PickList for commit:" << std::endl;
	for ( ObjectChangeList<DataModel::Pick>::iterator it = pickCommitList.begin();
		it != pickCommitList.end(); ++it ) {
		if ( it->second )
			std::cerr << "New:";
		else
			std::cerr << "Update:";
		std::cerr << " " << it->first->publicID() << std::endl;
	}
	std::cerr << "--------------------" << std::endl;

	std::cerr << "AmplitudeList for commit:" << std::endl;
	for ( std::vector<AmplitudePtr>::iterator it = amplitudeCommitList.begin();
		it != amplitudeCommitList.end(); ++it ) {
		std::cerr << "New: " << (*it)->publicID() << std::endl;
	}
	std::cerr << "--------------------" << std::endl;

	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		std::string pickID = _currentOrigin->arrival(i)->pickID();

		// try to find the pick somewhere in the client memory
		PickPtr pick = Pick::Find(pickID);
		if ( pick ) _associatedPicks[pickID] = pick;
	}

	if ( _localOrigin ) {
		// Strip invalid magnitudes
		size_t i = 0;
		while ( i < _currentOrigin->magnitudeCount() ) {
			Magnitude *mag = _currentOrigin->magnitude(i);
			try {
				if ( mag->evaluationStatus() == REJECTED ) {
					_currentOrigin->removeMagnitude(i);
					continue;
				}
			}
			catch ( ... ) {}
			++i;
		}
	}

	if ( /*_currentOrigin == _baseOrigin || */ !_localOrigin )
		emit updatedOrigin(_currentOrigin.get());
	else
		emit committedOrigin(_currentOrigin.get(),
		                     associate?_baseEvent.get():NULL,
		                     pickCommitList, amplitudeCommitList);

	if ( _baseEvent && _defaultEventType ) {
		// Check if event type changed
		bool typeSet;
		try { _baseEvent->type(); typeSet = true; }
		catch ( ... ) { typeSet = false; }

		if ( !typeSet )
			sendJournal(_baseEvent->publicID(), "EvType", _defaultEventType->toString());
	}

	_changedPicks.clear();
	_changedAmplitudes.clear();
	_localOrigin = false;

	//_ui.btnCommit->setEnabled(false);
	//_ui.btnCommit->setMenu(NULL);
	_ui.btnMagnitudes->setEnabled(false);

	_undoList.clear();
	_redoList.clear();

	emit undoStateChanged(!_undoList.isEmpty());
	emit redoStateChanged(!_redoList.isEmpty());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commitFocalMechanism(bool withMT, QPoint pos) {
	if ( _localOrigin ) {
		QMessageBox::critical(this, "Commit",
		                      "The origin this focal mechanism uses as "
		                      "trigger is not yet committed.\n"
		                      "Commit the origin before committing the "
		                      "focal mechanism.");
		return;
	}

	MomentTensorPtr mt;
	OriginPtr derived;
	if ( withMT && _currentOrigin ) {
		OriginDialog dialog(_currentOrigin->longitude().value(),
		                    _currentOrigin->latitude().value(), this);
		try { dialog.setDepth(_currentOrigin->depth().value()); }
		catch ( ValueException e ) {}
		dialog.setTime(_currentOrigin->time().value());

		dialog.enableAdvancedOptions(true, false);
		dialog.setPhaseCount(_residuals->count());
		// search for preferred magnitude value
		if ( _baseEvent ) {
			Magnitude *m = Magnitude::Find(_baseEvent->preferredMagnitudeID());
			if ( m )
				dialog.setMagValue(m->magnitude().value());
		}
		dialog.setMagType("Mw");

		if ( ! pos.isNull() )
			dialog.move(pos.x(), pos.y());

		if ( dialog.exec() != QDialog::Accepted ) return; // commit aborted

		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());

		// derive origin
		derived = Origin::Create();
		derived->setType(OriginType(HYPOCENTER));
		derived->setMethodID("FocalMechanism");
		derived->setCreationInfo(ci);
		derived->setEvaluationMode(EvaluationMode(MANUAL));

		try { derived->quality(); } // ensure existing quality object
		catch ( ValueException e ) { derived->setQuality(OriginQuality()); }

		derived->setTime(Time(dialog.getTime_t()));
		derived->setLatitude(dialog.latitude());
		derived->setLongitude(dialog.longitude());
		derived->setDepth(RealQuantity(dialog.depth()));
		derived->quality().setUsedPhaseCount(dialog.phaseCount());

		// moment magnitude
		MagnitudePtr mag = Magnitude::Create();
		mag->setCreationInfo(ci);
		mag->setMagnitude(dialog.magValue());
		mag->setType(dialog.magType().toStdString());
		mag->setOriginID(derived->publicID());
		mag->setStationCount(dialog.phaseCount());
		derived->add(mag.get());

		// moment tensor
		mt = MomentTensor::Create();
		mt->setDerivedOriginID(derived->publicID());
		mt->setMomentMagnitudeID(mag->publicID());
		mt->setCreationInfo(ci);
	}

	// Create FocalMechanism of both nodal planes
	NODAL_PLANE np1 = static_cast<PlotWidget*>(_residuals)->np1();
	NODAL_PLANE np2 = static_cast<PlotWidget*>(_residuals)->np2();

	FocalMechanismPtr fm = FocalMechanism::Create();
	fm->setTriggeringOriginID(_currentOrigin->publicID());
	if ( mt )
		fm->add(mt.get());
	NodalPlanes nps;
	NodalPlane np;
	np.setStrike(np1.str);
	np.setDip(np1.dip);
	np.setRake(np1.rake);
	nps.setNodalPlane1(np);
	np.setStrike(np2.str);
	np.setDip(np2.dip);
	np.setRake(np2.rake);
	nps.setNodalPlane2(np);
	fm->setNodalPlanes(nps);
	fm->setMethodID("first motion");
	fm->setEvaluationMode(EvaluationMode(MANUAL));
	fm->setEvaluationStatus(EvaluationStatus(CONFIRMED));

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());

	fm->setCreationInfo(ci);

	if ( fm )
		emit committedFocalMechanism(fm.get(), _baseEvent.get(),
		                             derived?derived.get():NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::commitWithOptions() {
	CommitOptions dlg;
	int idx;
	string eqName, eqComment;

	try {
		if ( SCApp->configGetBool("olv.commit.forceEventAssociation") == false ) {
			dlg.ui.cbAssociate->setChecked(false);
		}
	}
	catch ( ... ) {}

	try {
		if ( SCApp->configGetBool("olv.commit.fixOrigin") == false ) {
			dlg.ui.cbFixSolution->setChecked(false);
		}
	}
	catch ( ... ) {}

	if ( _defaultEventType ) {
		int idx = dlg.ui.comboEventTypes->findText(_defaultEventType->toString());
		if ( idx >= 0 ) dlg.ui.comboEventTypes->setCurrentIndex(idx);
	}

	if ( !_baseEvent ) {
		dlg.ui.cbAssociate->setText(QString(dlg.ui.cbAssociate->text()).arg("[]"));
		dlg.ui.frameEventOptions->setEnabled(false);
	}
	else {
		dlg.ui.cbAssociate->setText(QString(dlg.ui.cbAssociate->text()).arg(_baseEvent->publicID().c_str()));

		idx = -1;
		try {
			idx = dlg.ui.comboEventTypes->findText(_baseEvent->type().toString());
		}
		catch ( ... ) {}

		if ( idx != -1 )
			dlg.ui.comboEventTypes->setCurrentIndex(idx);

		idx = -1;
		try {
			idx = dlg.ui.comboEventTypeCertainty->findText(_baseEvent->typeCertainty().toString());
		}
		catch ( ... ) {}

		if ( idx != -1 )
			dlg.ui.comboEventTypeCertainty->setCurrentIndex(idx);

		if ( _reader && _baseEvent->eventDescriptionCount() == 0 )
			_reader->loadEventDescriptions(_baseEvent.get());

		if ( _reader && _baseEvent->commentCount() == 0 )
			_reader->loadComments(_baseEvent.get());

		// Fill earthquake name
		for ( size_t i = 0; i < _baseEvent->eventDescriptionCount(); ++i ) {
			if ( _baseEvent->eventDescription(i)->type() == EARTHQUAKE_NAME ) {
				eqName = _baseEvent->eventDescription(i)->text();
				dlg.ui.editEQName->setText(eqName.c_str());
				break;
			}
		}

		// Fill operator's comment
		for ( size_t i = 0; i < _baseEvent->commentCount(); ++i ) {
			if ( _baseEvent->comment(i)->id() == "Operator" ) {
				eqComment = _baseEvent->comment(i)->text();
				dlg.ui.editEQComment->setText(eqComment.c_str());
				break;
			}
		}
	}

	idx = -1;
	try {
		idx = dlg.ui.comboOriginStates->findText(_currentOrigin->evaluationStatus().toString());
		if ( idx == -1 )
			idx = dlg.ui.comboOriginStates->findText(EvaluationStatus::NameDispatcher::name(CONFIRMED));
	}
	catch ( ... ) {
		idx = dlg.ui.comboOriginStates->findText(EvaluationStatus::NameDispatcher::name(CONFIRMED));
	}

	if ( idx != -1 )
		dlg.ui.comboOriginStates->setCurrentIndex(idx);

	// If a new origin is about to be sent, deactive the
	// update option since updating is not possible.
	if ( _currentOrigin != _baseOrigin )
		dlg.ui.cbUpdateOrigin->setEnabled(false);

	if ( dlg.exec() == QDialog::Rejected )
		return;

	if ( dlg.ui.cbUpdateOrigin->isChecked() ) {
		if ( !_newOriginStatus.fromString(qPrintable(dlg.ui.comboOriginStates->currentText())) )
			_newOriginStatus = CONFIRMED;

		if ( dlg.ui.comboOriginStates->currentIndex() > 0 )
			_currentOrigin->setEvaluationStatus(_newOriginStatus);
		else
			_currentOrigin->setEvaluationStatus(Core::None);

		// Do not override the status in commit
		_newOriginStatus = EvaluationStatus::Quantity;
		commit(dlg.ui.cbAssociate->isChecked());
		_newOriginStatus = CONFIRMED;
	}

	// Do event specific things
	if ( _baseEvent ) {
		std::string type, newType, typeCertainty, newTypeCertainty;

		if ( dlg.ui.comboEventTypes->currentIndex() > 0 )
			newType = dlg.ui.comboEventTypes->currentText().toStdString();

		try {
			type = _baseEvent->type().toString();
		}
		catch ( ... ) {}

		if ( dlg.ui.comboEventTypeCertainty->currentIndex() > 0 )
			newTypeCertainty = dlg.ui.comboEventTypeCertainty->currentText().toStdString();

		try {
			typeCertainty = _baseEvent->typeCertainty().toString();
		}
		catch ( ... ) {}

		bool ok = true;

		if ( ok && eqComment != dlg.ui.editEQComment->text().toStdString() ) {
			eqComment = dlg.ui.editEQComment->text().toStdString();
			ok = sendJournal(_baseEvent->publicID(), "EvOpComment", eqComment);
		}

		if ( ok && eqName != dlg.ui.editEQName->text().toStdString() ) {
			eqName = dlg.ui.editEQName->text().toStdString();
			ok = sendJournal(_baseEvent->publicID(), "EvName", eqName);
		}

		if ( ok && (type != newType) )
			ok = sendJournal(_baseEvent->publicID(), "EvType", newType);

		if ( ok && (typeCertainty != newTypeCertainty) )
			ok = sendJournal(_baseEvent->publicID(), "EvTypeCertainty", newTypeCertainty);

		if ( ok && dlg.ui.cbFixSolution->isChecked() && dlg.ui.cbAssociate->isChecked() )
			sendJournal(_baseEvent->publicID(), "EvPrefOrgID", _currentOrigin->publicID());
	}

	if ( dlg.ui.cbBackToEventList->isChecked() )
		emit eventListRequested();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool OriginLocatorView::sendJournal(const std::string &objectID,
                                    const std::string &action,
                                    const std::string &params) {
	/*
	if ( _updateLocalEPInstance ) {
		NotifierPtr notifier;

		if ( action == "EvType" ) {
			EventType et;
			if ( et.fromString(params) )
				_currentEvent->setType(et);
			else
				_currentEvent->setType(Core::None);

			notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
		}
		else if ( action == "EvPrefOrgID" ) {
			if ( params.empty() )
				QMessageBox::critical(this, "Error", "Releasing the preferred origin in offline mode is not supported.");
			else if ( _currentEvent->preferredOriginID() != params ) {
				_currentEvent->setPreferredOriginID(params);
				_currentEvent->setPreferredMagnitudeID("");
				notifier = new Notifier("EventParameters", OP_UPDATE, _currentEvent.get());
			}
		}

		if ( notifier ) SCApp->emitNotifier(notifier.get());
	}
	else */{
		JournalEntryPtr entry = new JournalEntry;
		entry->setObjectID(objectID);
		entry->setAction(action);
		entry->setParameters(params);
		entry->setSender(SCApp->author());
		entry->setCreated(Core::Time::GMT());

		NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
		NotifierMessagePtr nm = new NotifierMessage;
		nm->attach(n.get());
		return SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get());
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct InvertFilter : ArrivalModel::Filter {
	InvertFilter(QItemSelectionModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *arr) const {
		return !_model->isRowSelected(row, QModelIndex());
	}

	QItemSelectionModel *_model;
};


struct AutomaticPickFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		Pick *p = Pick::Find(arr->pickID());
		if ( p == NULL ) return false;

		try {
			return p->evaluationMode() == AUTOMATIC;
		}
		catch ( ... ) {
			return true;
		}
	}
};


struct ManualPickFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		Pick *p = Pick::Find(arr->pickID());
		if ( p == NULL ) return false;

		try {
			return p->evaluationMode() == MANUAL;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct ZeroWeightFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		try {
			return arr->weight() == 0.0;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct NonZeroWeightFilter : ArrivalModel::Filter {
	bool accepts(int, int, DataModel::Arrival *arr) const {
		try {
			return arr->weight() != 0.0;
		}
		catch ( ... ) {
			return false;
		}
	}
};


struct ActivatedArrivalFilter : ArrivalModel::Filter {
	ActivatedArrivalFilter(QAbstractItemModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *) const {
		return _model->data(_model->index(row, 0), UsedRole).toInt() != 0;
	}

	QAbstractItemModel *_model;
};


struct DeactivatedArrivalFilter : ArrivalModel::Filter {
	DeactivatedArrivalFilter(QAbstractItemModel *model) : _model(model) {}

	bool accepts(int row, int, DataModel::Arrival *) const {
		return _model->data(_model->index(row, 0), UsedRole).toInt() == 0;
	}

	QAbstractItemModel *_model;
};


}

void OriginLocatorView::tableArrivalsContextMenuRequested(const QPoint &pos) {
	QMenu menu;

	if ( !_ui.tableArrivals->selectionModel() ) return;

	bool hasSelection = _ui.tableArrivals->selectionModel()->hasSelection();

	QAction *actionInvertSelection = menu.addAction("Invert selection");
	QMenu *subSelection = menu.addMenu("Select");
	QAction *actionSelectAutomatic = subSelection->addAction("Automatic picks");
	QAction *actionSelectManual = subSelection->addAction("Manual picks");
	subSelection->addSeparator();
	QAction *actionSelectWeight0 = subSelection->addAction("Zero weight");
	QAction *actionSelectWeightNon0 = subSelection->addAction("Non-zero weight");
	subSelection->addSeparator();
	QAction *actionSelectActivated = subSelection->addAction("Activated");
	QAction *actionSelectDeactivated = subSelection->addAction("Deactivated");

	menu.addSeparator();

	QMenu *subActivate = menu.addMenu("Activate");
	QMenu *subDeactivate = menu.addMenu("Deactivate");

	QAction *actionActivate = subActivate->addAction("All");
	QAction *actionActivateTime = subActivate->addAction("Time");
	QAction *actionActivateBaz = subActivate->addAction("Backazimuth");
	QAction *actionActivateSlow = subActivate->addAction("Slowness");

	QAction *actionDeactivate = subDeactivate->addAction("All");
	QAction *actionDeactivateTime = subDeactivate->addAction("Time");
	QAction *actionDeactivateBaz = subDeactivate->addAction("Backazimuth");
	QAction *actionDeactivateSlow = subDeactivate->addAction("Slowness");

	if ( !hasSelection ) {
		actionActivate->setEnabled(false);
		actionDeactivate->setEnabled(false);
	}

	menu.addSeparator();

	QAction *actionRename = menu.addAction("Rename selected arrivals");

	if ( !hasSelection )
		actionRename->setEnabled(false);

	menu.addSeparator();

	QAction *actionDeleteSelectedArrivals = menu.addAction("Delete selected arrivals");
	if ( !hasSelection )
		actionDeleteSelectedArrivals->setEnabled(false);

	menu.addSeparator();
	QAction *actionCopyToClipboard = menu.addAction("Copy selected rows to clipboard");

	QAction *result = menu.exec(_ui.tableArrivals->mapToGlobal(pos));

	if ( result == actionDeleteSelectedArrivals )
		deleteSelectedArrivals();
	else if ( result == actionActivate )
		activateSelectedArrivals(Seismology::LocatorInterface::F_ALL, true);
	else if ( result == actionActivateTime )
		activateSelectedArrivals(Seismology::LocatorInterface::F_TIME, true);
	else if ( result == actionActivateBaz )
		activateSelectedArrivals(Seismology::LocatorInterface::F_BACKAZIMUTH, true);
	else if ( result == actionActivateSlow )
		activateSelectedArrivals(Seismology::LocatorInterface::F_SLOWNESS, true);
	else if ( result == actionDeactivate )
		activateSelectedArrivals(Seismology::LocatorInterface::F_ALL, false);
	else if ( result == actionDeactivateTime )
		activateSelectedArrivals(Seismology::LocatorInterface::F_TIME, false);
	else if ( result == actionDeactivateBaz )
		activateSelectedArrivals(Seismology::LocatorInterface::F_BACKAZIMUTH, false);
	else if ( result == actionDeactivateSlow )
		activateSelectedArrivals(Seismology::LocatorInterface::F_SLOWNESS, false);

	else if ( result == actionInvertSelection )
		selectArrivals(InvertFilter(_ui.tableArrivals->selectionModel()));
	else if ( result == actionSelectAutomatic )
		selectArrivals(AutomaticPickFilter());
	else if ( result == actionSelectManual )
		selectArrivals(ManualPickFilter());
	else if ( result == actionSelectWeight0 )
		selectArrivals(ZeroWeightFilter());
	else if ( result == actionSelectWeightNon0 )
		selectArrivals(NonZeroWeightFilter());
	else if ( result == actionSelectActivated )
		selectArrivals(ActivatedArrivalFilter(_modelArrivalsProxy));
	else if ( result == actionRename )
		renameArrivals();
	else if ( result == actionSelectDeactivated )
		selectArrivals(DeactivatedArrivalFilter(_modelArrivalsProxy));
	else if ( result == actionCopyToClipboard )
		SCApp->copyToClipboard(_ui.tableArrivals);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::tableArrivalsHeaderContextMenuRequested(const QPoint &pos) {
	int count = _ui.tableArrivals->horizontalHeader()->count();
	QAbstractItemModel *model = _ui.tableArrivals->horizontalHeader()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!_ui.tableArrivals->horizontalHeader()->isSectionHidden(i));
	}

	QAction *result = menu.exec(_ui.tableArrivals->horizontalHeader()->mapToGlobal(pos));
	if ( result == NULL ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		colVisibility[i] = actions[i]->isChecked();

	_ui.tableArrivals->horizontalHeader()->setSectionHidden(section, !colVisibility[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrivals(const ArrivalModel::Filter *f) {
	if ( _ui.tableArrivals->selectionModel() == NULL )
		return;

	QItemSelection selection;

	for ( int i = 0; i < _modelArrivalsProxy->rowCount(); ++i ) {
		Arrival *arr = _currentOrigin->arrival(i);
		QModelIndex idx = _modelArrivalsProxy->mapFromSource(_modelArrivals.index(i,0));

		if ( f != NULL && !f->accepts(idx.row(), i, arr) ) continue;

		selection.append(QItemSelectionRange(idx));
	}

	//selection = _modelArrivalsProxy->mapSelectionFromSource(selection);
	_ui.tableArrivals->selectionModel()->select(selection, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::selectArrivals(const ArrivalModel::Filter &f) {
	selectArrivals(&f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::deleteSelectedArrivals() {
	if ( _ui.tableArrivals->selectionModel() == NULL )
		return;

	QModelIndexList rows = _ui.tableArrivals->selectionModel()->selectedRows();
	if ( rows.empty() ) return;

	if ( _currentOrigin == NULL ) return;

	OriginPtr origin = Origin::Create();
	*origin = *_currentOrigin;

	vector<bool> flags;
	flags.resize(_currentOrigin->arrivalCount(), false);

	foreach ( const QModelIndex &idx, rows ) {
		int row = _modelArrivalsProxy->mapToSource(idx).row();
		if ( row >= (int)_currentOrigin->arrivalCount() ) {
			cerr << "Delete arrivals: invalid idx " << row
			     << ": only " << _currentOrigin->arrivalCount() << " available"
			     << endl;
			continue;
		}

		flags[row] = true;
	}

	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		// Skip arrivals to be deleted
		if ( flags[i] ) continue;

		// Copy existing arrivals
		Arrival *arr = _currentOrigin->arrival(i);
		ArrivalPtr newArr = new Arrival(*arr);
		origin->add(newArr.get());
	}

	applyNewOrigin(origin.get(), false);
	startBlinking(QColor(255,128,0), _ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::activateSelectedArrivals(Seismology::LocatorInterface::Flags flags,
                                                 bool activate) {
	if ( _ui.tableArrivals->selectionModel() == NULL )
		return;

	QModelIndexList rows = _ui.tableArrivals->selectionModel()->selectedRows();
	if ( rows.empty() ) return;

	bool changed = false;

	foreach ( const QModelIndex &idx, rows ) {
		int mask = getMask(idx);
		int oldFlags = idx.data(UsedRole).toInt();
		int newFlags = oldFlags;
		if ( activate )
			newFlags |= flags;
		else
			newFlags &= ~flags;
		newFlags &= mask;

		if ( oldFlags != newFlags ) {
			_modelArrivalsProxy->setData(idx, newFlags, UsedRole);
			changed = true;
		}
	}

	if ( changed )
		startBlinking(QColor(255,128,0), _ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::renameArrivals() {
	RenamePhases dlg(this);

	QSet<QString> phases;
	QSet<Arrival*> arrivals;

	for ( int i = 0; i < _modelArrivalsProxy->rowCount(); ++i ) {
		Arrival *arr = _currentOrigin->arrival(i);
		QModelIndex idx = _modelArrivalsProxy->mapFromSource(_modelArrivals.index(i,0));

		if ( _ui.tableArrivals->selectionModel()->isRowSelected(idx.row(), QModelIndex()) ) {
			phases.insert(arr->phase().code().c_str());
			arrivals.insert(arr);
		}
	}

	foreach (const QString &ph, phases)
		dlg.ui.listSourcePhases->addItem(ph);

	PickerView::Config::StringList pickPhases;
	_pickerConfig.getPickPhases(pickPhases);
	foreach (const QString &ph, pickPhases)
		dlg.ui.listTargetPhase->addItem(ph);

	if ( dlg.exec() != QDialog::Accepted )
		return;

	QList<QListWidgetItem*> sourceItems = dlg.ui.listSourcePhases->selectedItems();
	if ( sourceItems.empty() ) {
		QMessageBox::information(this, "Rename arrivals",
		                         "No source phases selected: nothing to do.");
		return;
	}

	QList<QListWidgetItem*> targetItems = dlg.ui.listTargetPhase->selectedItems();
	if ( targetItems.empty() ) {
		QMessageBox::information(this, "Rename arrivals",
		                         "No target phase selected: nothing to do.");
		return;
	}

	if ( targetItems.count() > 1 ) {
		QMessageBox::critical(this, "Rename arrivals",
		                      "Internal error: More than one target phase selected.");
		return;
	}

	if ( _currentOrigin == NULL ) return;


	OriginPtr origin = Origin::Create();
	*origin = *_currentOrigin;

	for ( size_t i = 0; i < _currentOrigin->arrivalCount(); ++i ) {
		Arrival *arr = _currentOrigin->arrival(i);

		// Copy existing arrivals
		ArrivalPtr newArr = new Arrival(*arr);

		if ( arrivals.contains(arr) && phases.contains(arr->phase().code().c_str()) )
			newArr->setPhase(Phase(targetItems[0]->text().toStdString()));

		origin->add(newArr.get());
	}

	applyNewOrigin(origin.get(), false);
	startBlinking(QColor(255,128,0), _ui.btnRelocate);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::dataChanged(const QModelIndex& topLeft, const QModelIndex&) {
	if ( topLeft.column() != USED ) return;

	int flags = _modelArrivals.data(topLeft, UsedRole).toInt();
	bool used = flags != 0;
	_residuals->setValueSelected(topLeft.row(), used);
	_map->setArrivalState(topLeft.row(), used);
	if ( _toolMap )
		_toolMap->setArrivalState(topLeft.row(), used);
	if ( _recordView )
		_recordView->setArrivalState(topLeft.row(), used);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evalResultAvailable(const QString &oid,
                                            const QString &className,
                                            const QString &script,
                                            const QString &result) {
	if ( !_currentOrigin || _currentOrigin->publicID() != oid.toStdString() )
		return;

	ScriptLabelMap::iterator it = _scriptLabelMap.find(script);
	if ( it == _scriptLabelMap.end() ) return;

	it.value().first->setEnabled(true);
	it.value().second->setText(result);

	it.value().second->setPalette(_ui.labelEventID->palette());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evalResultError(const QString &oid,
                                        const QString &className,
                                        const QString &script,
                                        int error) {
	if ( !_currentOrigin || _currentOrigin->publicID() != oid.toStdString() )
		return;

	ScriptLabelMap::iterator it = _scriptLabelMap.find(script);
	if ( it == _scriptLabelMap.end() ) return;

	it.value().first->setEnabled(true);
	it.value().second->setText("ERROR");
	QPalette p = it.value().second->palette();
	p.setColor(QPalette::WindowText, Qt::darkRed);
	it.value().second->setPalette(p);
	it.value().second->setToolTip(PublicObjectEvaluator::Instance().errorMsg(error));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void OriginLocatorView::evaluateOrigin(Seiscomp::DataModel::Origin *org,
                                       Seiscomp::DataModel::Event *event,
                                       bool localOrigin, bool relocated) {
	QStringList scripts;
	ScriptLabelMap::iterator it;
	for ( it = _scriptLabelMap.begin(); it != _scriptLabelMap.end(); ++it )
		scripts << it.key();

	// Local origins need special handling
	if ( localOrigin )
		PublicObjectEvaluator::Instance().eval(org, scripts);
	else
		PublicObjectEvaluator::Instance().prepend(this, org->publicID().c_str(),
		                                          org->typeInfo(), scripts);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
