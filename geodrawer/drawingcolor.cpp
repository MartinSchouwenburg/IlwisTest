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

#include <QColor>
#include "kernel.h"
#include "ilwisdata.h"
#include "coveragedisplay/coveragelayermodel.h"
#include "drawingcolor.h"

using namespace Ilwis;
using namespace Geodrawer;

DrawingColor::DrawingColor(CoverageLayerModel *spatialdrawer) : _spatialdrawer(spatialdrawer)
{
}

QColor DrawingColor::clrVal(double rVal) const
{
    return QColor(0,0,0);
}
