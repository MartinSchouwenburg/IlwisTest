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

#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include "kernel.h"
#include "ilwiscontext.h"
#include "workspacemodel.h"

using namespace Ilwis;
using namespace Ui;

WorkSpaceModel::WorkSpaceModel(const QString &name, QObject *parent) :
    CatalogModel( Ilwis::Resource("ilwis://internalcatalog/workspaces/" + name,itWORKSPACE),
        CatalogModel::ctINTERNAL |
        CatalogModel::ctDATA |
        CatalogModel::ctOPERATION | 
        (name == "default" ? CatalogModel::ctFIXED : CatalogModel::ctMUTABLE),
        parent)
{
}

WorkSpaceModel::~WorkSpaceModel()
{

}

void WorkSpaceModel::addItems(const QString& ids)
{
    auto putInConfig = [&](quint64 id, int count) {

        QString basekey = "users/" + Ilwis::context()->currentUser() + "/workspace-" + name();
        Ilwis::Resource res = Ilwis::mastercatalog()->id2Resource(id);
        if ( res.isValid()){
            QString key;
            if ( res.ilwisType() & itOPERATIONMETADATA){
                key = QString("%1/operation-%2").arg(basekey).arg(count);
                Ilwis::context()->configurationRef().addValue(QString("%1/operation-count").arg(basekey),QString::number(count + 1));
            }
            else{
                key = QString("%1/data-%2").arg(basekey).arg(count);
                Ilwis::context()->configurationRef().addValue(QString("%1/data-count").arg(basekey),QString::number(count + 1));
            }
            Ilwis::context()->configurationRef().addValue(key + "/url", res.url().toString());
            Ilwis::context()->configurationRef().addValue(key + "/type", QString::number(res.ilwisType()));
        }
    };


    Ilwis::InternalDatabaseConnection stmt;
    int count = 0;
    if (_view.fixedItemCount() == 0) {
        QString query = QString("Insert into workspaces (workspaceid) values(%1)").arg(id());
        if(!stmt.exec(query))
            return;
    }
    // we dont want to add ids that are already in the list so we collect all ids that we have laready added
    std::set<QString> presentids;
    for(auto item : _operations){
        presentids.insert(item->id());
    }
    for(auto item : _data){
        presentids.insert(item->id());
    }
    QStringList idlist = ids.split("|");
    for(auto id : idlist){
        if ( presentids.find(id) != presentids.end())
            continue;

        quint64 idn = id.toULongLong();
        putInConfig(idn, count);
        _view.addFixedItem(idn);
        ++count;

        QString query = QString("Select workspaceid from workspace where workspaceid=%1 and itemid=%2").arg(resource().id()).arg(idn);
        if (stmt.exec(query)) { // already there
            if ( stmt.next()) {
                continue;
            }else{
                query = QString("Insert into workspace (workspaceid, itemid) values(%1,%2)").arg(resource().id()).arg(idn);
                if(!stmt.exec(query))
                    return;
            }
        }
    }
    QString availableWorkspaces = Ilwis::ilwisconfig("users/" + Ilwis::context()->currentUser() + "/workspaces",QString(""));
    if ( availableWorkspaces.indexOf(name()) == -1){
        if ( availableWorkspaces.size() > 0){
            availableWorkspaces += "|";
        }
        availableWorkspaces += name();
        Ilwis::context()->configurationRef().addValue("users/" + Ilwis::context()->currentUser() + "/workspaces",availableWorkspaces);
    }

    refresh(true);
    emit dataChanged();
}

void WorkSpaceModel::removeItems(const QString& ids)
{
    QStringList idlist = ids.split("|");
    for(auto id : idlist){
        _view.removeFixedItem(id.toULongLong());
    }
}

bool WorkSpaceModel::isDefault() const
{
    return resource().url().toString() == Ilwis::Catalog::DEFAULT_WORKSPACE;
}

void WorkSpaceModel::gatherItems() {
    bool needRefresh = _refresh;
    CatalogModel::gatherItems();
    if ( needRefresh){
        _operations.clear();
        _data.clear();

        for(auto iter=_allItems.begin(); iter != _allItems.end(); ++iter){
            if ( (*iter)->type() & itOPERATIONMETADATA){
                _operations.push_back(new OperationModel((*iter)->resource(),this));
            }else if ( hasType((*iter)->type(), itILWISOBJECT)){
                _data.push_back(*iter);
            }
        }
    }
    refresh(false);
}

QQmlListProperty<OperationModel> WorkSpaceModel::operations()
{
    refresh(true);
    gatherItems();
    return  QQmlListProperty<OperationModel>(this, _operations);
}

QQmlListProperty<ResourceModel> WorkSpaceModel::data()
{
    refresh(true);
    gatherItems();
    return  QQmlListProperty<ResourceModel>(this, _data);
}

QString WorkSpaceModel::modelType() const
{
    return "workspacemodel";
}

