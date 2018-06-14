#include "kernel.h"
#include "ilwisdata.h"
#include "symboltable.h"
#include "operationExpression.h"
#include "operationmetadata.h"
#include "domain.h"
#include "table.h"
#include "commandhandler.h"
#include "operation.h"
#include "mastercatalog.h"
#include "operationhelper.h"
#include "uicontextmodel.h"
#include "modelregistry.h"
#include "chartmodel.h"
#include "dataseriesmodel.h"
#include "chartoperation.h"
#include "chartseriescolor.h"

using namespace Ilwis;
using namespace Ui; 

REGISTER_OPERATION(ChartSeriesColor)

ChartSeriesColor::ChartSeriesColor()
{

}

ChartSeriesColor::ChartSeriesColor(quint64 metaid, const Ilwis::OperationExpression &expr) : ChartOperation(metaid, expr)
{

}

Ilwis::OperationImplementation *ChartSeriesColor::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
	return new ChartSeriesColor(metaid, expr);
}

Ilwis::OperationImplementation::State ChartSeriesColor::prepare(ExecutionContext *ctx, const SymbolTable &st)
{
	ChartOperation::prepare(ctx, st);

	if (!getModel()) {
		kernel()->issues()->log(TR("Invalid model id used for chart. Chart can not assign new color"));
		return sPREPAREFAILED;
	}

	_seriesName = _expression.input<QString>(1);
	_newColor = _expression.input<QString>(2);

	return sPREPARED;
}

bool ChartSeriesColor::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
	if (_prepState == sNOTPREPARED)
		if ((_prepState = prepare(ctx, symTable)) != sPREPARED)
			return false;

	auto serie = _chartmodel->getSeriesByName(_seriesName);
	serie->setColor(_newColor);

	return true;
}

quint64 ChartSeriesColor::createMetadata()
{
	OperationResource resource({ "ilwis://operations/ChartSeriesColor" });
	resource.setLongName("Change color of a chart series");
	resource.setSyntax("ChartSeriesColor(parameters))");
	resource.setInParameterCount({ 2 });
	resource.addInParameter(0, itINT32, TR("Chart ID"), TR("The ID identifying the chart containing the series."));
	resource.addInParameter(1, itSTRING, TR("Series Name"), TR(""));
	resource.addInParameter(2, itSTRING, TR("Color"), TR("String describing the new color"));
	resource.setKeywords("chart series, table, chart, color");

	mastercatalog()->addItems({ resource });
	return resource.id();
}