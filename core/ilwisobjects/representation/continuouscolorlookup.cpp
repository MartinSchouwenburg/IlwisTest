/*IlwisObjects is a framework for analysis, processing and visualization of remote sensing and gis data
Copyright (C) 2018  52n North

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlField>
#include "kernel.h"
#include "ilwisdata.h"
#include "range.h"
#include "itemrange.h"
#include "colorrange.h"
#include "numericrange.h"
#include "continuouscolorlookup.h"

using namespace Ilwis;



ContinuousColorLookup::ContinuousColorLookup()
{

}

ContinuousColorLookup::ContinuousColorLookup(const QString &definition, const QString& mode) : _relative(mode == "relative")
{
    fromDefinition(definition);
}

ContinuousColorLookup::ContinuousColorLookup(const IDomain &, const QString& rprCode)
{
    InternalDatabaseConnection db;
    QString query = QString("Select * from representation where code='%1'").arg(rprCode != "" ? rprCode : "pseudo");
    if (db.exec(query)) {
        if ( db.next()){
            QSqlRecord rec = db.record();
            QString  definition = rec.field("definition").value().toString();
            fromDefinition(definition);
        }
    }


}

QColor ContinuousColorLookup::value2color(double value, const NumericRange& actualRange, const NumericRange& stretchRange) const
{
    if ( value == rUNDEF)
        return QColor("transparent");

    if ( stretchRange.isValid())  {
        if ( _linear){
            if ( value < stretchRange.center()){
                double stretchFraction = (value - stretchRange.min())/ stretchRange.distance();
                value = actualRange.min() + stretchFraction * actualRange.distance();
            }else{
                if ( value >= stretchRange.center()){
                    double stretchFraction = (stretchRange.max() - value)/ stretchRange.distance();
                    value = actualRange.max() - stretchFraction * actualRange.distance(); 
                }
            }
        }
    }
	if (_relative)
		value = min(1.0, max(0.0, (value - actualRange.min()) / actualRange.distance())); // scale it between 0..1 
	else
		value = min(actualRange.max(), max(actualRange.min(), (value - actualRange.min()))); // scale it between 0..1
    for(int i = 0; i < _colorranges.size(); ++i){
		double delta = std::abs(_colorranges[i].first._last - _colorranges[i].first._first);
		double position = 0;
		if (_colorranges[i].first._reversed) {
			if (value < _colorranges[i].first._first && value >= _colorranges[i].first._last) {
				
				if (_step == 0) {
					position = (value - _colorranges[i].first._last) / delta;
				}
				else
					position = ((quint32)(value - _colorranges[i].first._last) / _step) / ((quint32)(delta / _step));

				position = 1.0 - position;
				return ContinuousColorRange::valueAt(position, &_colorranges[i].second);
			}
		} else if (value <= _colorranges[i].first._last) {
            double position = 0;
            if ( _step == 0){
                position = (value - _colorranges[i].first._first)/ delta;
            }else
                position = ((quint32)(value - _colorranges[i].first._first)/ _step)/( (quint32)(delta / _step));

            return ContinuousColorRange::valueAt(position,&_colorranges[i].second);
        }
    }
    return QColor();
}

void ContinuousColorLookup::addGroup(const ValueRange &range, const ContinuousColorRange &colorrange)
{
	if (_colorranges.size() == 0) {
		_colorranges.push_back(std::pair< ValueRange, ContinuousColorRange>(range, colorrange));
	}else if (!_colorranges.back().first.overlaps(range)) {
		_colorranges.push_back(std::pair< ValueRange, ContinuousColorRange>(range, colorrange));
	}
}

NumericRange ContinuousColorLookup::numericRange() const {
	NumericRange rng;
	for (auto& item : _colorranges) {
		rng += item.first._first;
		rng += item.first._last;
	}
	return rng;
}

void ContinuousColorLookup::setColor(double value, const QColor &clr)
{
    // TODO:
}

ColorLookUp *ContinuousColorLookup::clone() const
{
    ContinuousColorLookup *newlookup = new ContinuousColorLookup();
    newlookup->_colorranges = _colorranges;
    newlookup->_linear = _linear;
    newlookup->_numericRange = _numericRange;
    newlookup->_step = _step;
	newlookup->_definition = _definition;

    return newlookup;
}

void ContinuousColorLookup::store(QDataStream& stream) const {
	stream << (quint32)_colorranges.size();
	for (auto item : _colorranges) {
		stream << item.first._first;
		stream << item.first._last;
		stream << item.first._reversed;
		item.second.store(stream);
	}
	_numericRange.store(stream);
	stream << _step;
	stream << _linear;
	stream << _relative;
	stream << _definition;
}
void ContinuousColorLookup::load(QDataStream& stream) {
	quint32 n;
	stream >> n;
	_colorranges.resize(n);
	for (auto& vr : _colorranges) {
		stream >> vr.first._first;
		stream >> vr.first._last;
		stream >> vr.first._reversed;
		vr.second.load(stream);
	}
	_numericRange.load(stream);
	stream >> _step;
	stream >> _linear;
	stream >> _relative;
	stream >> _definition;
}

QString ContinuousColorLookup::definition(const IDomain& dom, bool& hasChanged)  {
	QString def;
	for (auto crange : _colorranges) {
		if (def != "")
			def += ";";
		def += QString("%1:%2|%3|%4").arg(crange.first._first).arg(crange.first._last).arg(crange.second.limitColor1().name()).arg(crange.second.limitColor2().name());
	}
	hasChanged = def != _definition;
	if (hasChanged)
		_definition = def;
	return def;
}

void ContinuousColorLookup::reset(const IDomain& dom) {
}

void ContinuousColorLookup::fromDefinition(const QString &definition, const IDomain&)
{
	_colorranges.clear();
    QStringList parts = definition.split(";");
    for( QString group : parts){
        QStringList groupdef = group.split("|");
        if ( groupdef.size() != 3) {
            ERROR2(ERR_ILLEGAL_VALUE_2,TR("Representation definition"), definition);
            return;
        }
        QStringList limits = groupdef[0].split(":");
        if ( limits.size() != 2){
            ERROR2(ERR_ILLEGAL_VALUE_2,TR("Representation definition"), definition);
            return;
        }
        bool ok1, ok2;
		ValueRange vr;
		vr._first = limits[0].toDouble(&ok1);
		vr._last = limits[1].toDouble(&ok2);
        if ( !(ok1 && ok2)){
            ERROR2(ERR_ILLEGAL_VALUE_2,TR("Representation definition"), definition);
            return;
        }
		vr._reversed = vr._last < vr._first;
        QColor color1 = string2color(groupdef[1]);
        if ( !(color1.isValid())){
            ERROR2(ERR_ILLEGAL_VALUE_2,TR("Representation definition"), definition);
            return;
        }


        QColor color2 = string2color(groupdef[2]);
        if ( !(color2.isValid())){
            ERROR2(ERR_ILLEGAL_VALUE_2,TR("Representation definition"), definition);
            return;
        }
        ContinuousColorRange colorrange(color1, color2);
        addGroup(vr,colorrange);

    }
	_definition = definition;

}

void ContinuousColorLookup::addGroup(const NumericRange& range, const ContinuousColorRange& colorrange) {
	ValueRange vr;
	vr._first = range.min();
	vr._last = range.max();
	addGroup(vr, colorrange);
}

void ContinuousColorLookup::steps(int st) {
	if (st > 0) {
		_step = st;
	}
}

int ContinuousColorLookup::steps() const {
	return _step;
}

void ContinuousColorLookup::relative(bool yesno) {
	_relative = yesno;

}

bool ContinuousColorLookup::relative() const {
	return _relative;
}

QString ContinuousColorLookup::definition() const {
	return _definition;
}