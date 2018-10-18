#pragma once

namespace Ilwis {
    namespace Ui {

        class ChartLegendProperties : public ChartOperation
        {
        public:
            ChartLegendProperties();
            ChartLegendProperties(quint64 metaid, const Ilwis::OperationExpression &expr);

            bool execute(ExecutionContext *ctx, SymbolTable& symTable);
            static Ilwis::OperationImplementation *create(quint64 metaid, const Ilwis::OperationExpression& expr);
            Ilwis::OperationImplementation::State prepare(ExecutionContext *ctx, const SymbolTable &);

            static quint64 createMetadata();

        private:
            bool _show = true;
            QString _align = "top";

            NEW_OPERATION(ChartLegendProperties);
        };
    }
}