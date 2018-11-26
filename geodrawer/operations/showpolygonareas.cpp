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

#include "kernel.h"
#include "ilwisdata.h"
#include "symboltable.h"
#include "operationExpression.h"
#include "operationmetadata.h"
#include "commandhandler.h"
#include "geometries.h"
#include "georeference.h"
#include "operation.h"
#include "coveragedisplay/draweroperation.h"
#include "coveragedisplay/layermodel.h"
#include "coveragedisplay/layermanager.h"
#include "coveragedisplay/rootlayermodel.h"
#include "showpolygonareas.h"

using namespace Ilwis;
using namespace Ui;

REGISTER_OPERATION(ShowPolygonAreas)

ShowPolygonAreas::ShowPolygonAreas()
{}

ShowPolygonAreas::ShowPolygonAreas(quint64 metaid, const Ilwis::OperationExpression &expr) : DrawerOperation(metaid, expr)
{}

bool ShowPolygonAreas::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
    try {
        if (_prepState == sNOTPREPARED)
            if((_prepState = prepare(ctx,symTable)) != sPREPARED)
                return false;

        _layer->vproperty("showareas", _areaVisibility);

        return true;
    }catch(const VisualizationError& ){
    }
    return false;
}

Ilwis::OperationImplementation *ShowPolygonAreas::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new ShowPolygonAreas(metaid, expr)    ;
}

Ilwis::OperationImplementation::State ShowPolygonAreas::prepare(ExecutionContext *ctx, const SymbolTable &t)
{
    OperationImplementation::prepare(ctx, t);
    if (!getViewId(_expression.input<QString>(0))){
        return sPREPAREFAILED;
    }

    _layer =  layerManager()->findLayer(_expression.input<QString>(1).toInt());
    if (!_layer)
        return sPREPAREFAILED;

   _areaVisibility = _expression.input<bool>(2);

    return sPREPARED;
}

quint64 ShowPolygonAreas::createMetadata()
{
    OperationResource operation({"ilwis://operations/showpolygonareas"});
    operation.setSyntax("showpolygonareas(viewid, drawername-index,visibility=!true | false[,type])");
    operation.setDescription(TR("sets or resets the visibility of a layer"));
    operation.setInParameterCount({3});
    operation.addInParameter(0,itINTEGER , TR("view id"),TR("id of the view to which this drawer has to be added"));
    operation.addInParameter(1,itSTRING , TR("layer index/code"), TR("location of the layer to be used"));
    operation.addInParameter(2,itBOOL , TR("visibility"));
    operation.setOutParameterCount({0});
    operation.setKeywords("visualization");

    mastercatalog()->addItems({operation});
    return operation.id();
}


