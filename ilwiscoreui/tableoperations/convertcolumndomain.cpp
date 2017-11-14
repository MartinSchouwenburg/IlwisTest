#include "kernel.h"
#include "ilwisdata.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "table.h"
#include "models/tablemodel.h"
#include "tableoperation.h"
#include "symboltable.h"
#include "operationmetadata.h"
#include "commandhandler.h"
#include "operationExpression.h"
#include "operation.h"
#include "convertcolumndomain.h"

using namespace Ilwis;
using namespace Ui;

ConvertColumnDomain::ConvertColumnDomain(): TableOperation("Convert Column", QUrl("ConvertColumn.qml"),TR("converts the domain of a column to another appropriate domain"))
{
}

bool ConvertColumnDomain::canUse(TableModel *tblModel, const QVariantMap &parameter) const 
{
    bool ok;
    quint32 colIndex = parameter["columnindex"].toInt(&ok);
    if ( !ok || colIndex >= tblModel->getColumnCount())
        return false;
    IDomain dom = tblModel->table()->columndefinition(colIndex).datadef().domain();

    return hasType(dom->ilwisType(), itTEXTDOMAIN);
}

void ConvertColumnDomain::execute(const QVariantMap &parameters)
{
    try{
        QString colName = parameters["columnname"].toString();
        QString targetdom = parameters["targetdomaintype"].toString();
        QString expr = QString("convertcolumndomain(%1,%2,%3)").arg(table()->resource().url().toString()).arg(colName).arg(targetdom);

        Ilwis::OperationExpression ex(expr);
        Ilwis::Operation op(ex);
        ExecutionContext ctx;
        SymbolTable tbl;
        if (op->execute(&ctx, tbl)){
            tableModel()->updateColumns();
        }
    } catch(const ErrorObject& ){

    }
}

/*TableOperation *ConvertColumnDomain::create()
{
    return new ConvertColumnDomain();
}*/
