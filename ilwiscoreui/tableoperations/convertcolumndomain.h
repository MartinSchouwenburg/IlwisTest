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

#ifndef CONVERTCOLUMNDOMAIN_H
#define CONVERTCOLUMNDOMAIN_H
#include "kernel.h"
#include "ilwisdata.h"
#include "tableoperation.h"

namespace Ilwis {
namespace Ui {
class TableModel;

class ILWISCOREUISHARED_EXPORT ConvertColumnDomain : public TableOperation
{
    Q_OBJECT
public:
    ConvertColumnDomain();

    bool canUse(TableModel *tblModel, const QVariantMap &parameter) const;

    Q_INVOKABLE void execute(const QVariantMap &parameters);
    static Ilwis::Ui::TableOperation *create() { return new ConvertColumnDomain(); }

};
}
}

#endif // CONVERTCOLUMNDOMAIN_H
