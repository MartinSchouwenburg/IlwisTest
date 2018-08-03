#include "kernel.h"
#include "ilwisdata.h"
#include "factory.h"
#include "abstractfactory.h"
#include "chartmodel.h"
#include "chartoperationeditor.h"
#include "chartoperationfactory.h"

using namespace Ilwis;
using namespace Ui;


ChartOperationFactory::ChartOperationFactory() : AbstractFactory("chartoperationfactory", "ilwis", "Generates operation objects for UI accessible chart operations")
{
}

int ChartOperationFactory::registerChartOperation(const QString &name, ChartOperationCreate createFunc)
{
    _creators[name] = createFunc;

    return 42;
}

QList<ChartOperationEditor *> ChartOperationFactory::selectedOperations(ChartModel *chartModel, const QVariantMap& parameters)
{
    QList<ChartOperationEditor *> operations;
    for (auto creator : _creators) {
        auto *operation = creator.second();
        if (operation) {
            if (operation->canUse(chartModel, parameters)) {
                operation->prepare(chartModel);
                operations.append(operation);
            }
            else
                delete operation;
        }
    }
    return operations;
}
