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
#include "connectorinterface.h"
#include "ilwisobjectconnector.h"
#include "factory.h"
#include "abstractfactory.h"
#include "connectorfactory.h"
#include "ilwisobjectfactory.h"
#include "pythonscriptconnector.h"
#include "pythonobjectfactory.h"

using namespace Ilwis;
using namespace PythonScript;

PythonObjectFactory::PythonObjectFactory() : IlwisObjectFactory("IlwisObjectFactory","python","")
{

}

IlwisObject *Ilwis::PythonScript::PythonObjectFactory::create(const Ilwis::Resource &resource, const IOOptions &options) const
{
    const ConnectorFactory *factory = kernel()->factory<ConnectorFactory>("ilwis::ConnectorFactory");
    PythonScriptConnector *connector = factory->createFromResource<PythonScriptConnector>(resource, "python", options);

   if(!connector) {
       kernel()->issues()->log(TR(ERR_COULDNT_CREATE_OBJECT_FOR_2).arg("Connector",resource.name()));
       return 0;
   }
   IlwisObject *object = createObject(connector, options);
   if ( object)
       return object;

   delete connector;
   return 0;
}

bool PythonObjectFactory::canUse(const Resource &resource) const
{
    if( resource.url().scheme() == "ilwis")  {
        return false;
    }
    QString filename = resource.url(true).toLocalFile();
    if ( filename == "")
        return false;
    QString ext = QFileInfo(filename).suffix().toLower();
    if ( ext != "py")
        return false;


    return hasType(resource.ilwisType(),itSCRIPT);
}

bool PythonObjectFactory::prepare()
{
    return true;
}
