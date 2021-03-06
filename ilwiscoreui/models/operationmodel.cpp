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

#include <QString>
#include <QFileInfo>
#include <QUrl>
#include "kernel.h"
#include "ilwisdata.h"
#include "ilwiscontext.h"
#include "connectorinterface.h"
#include "catalog/resource.h"
#include "ilwisobject.h"
#include "operationmetadata.h"
#include "mastercatalog.h"
#include "resourcemodel.h"
#include "operationmodel.h"

using namespace Ilwis;
using namespace Ui;

OperationModel::OperationModel()
{
}

OperationModel::OperationModel(const Ilwis::Resource& source, QObject *parent) : ResourceModel(source, parent)
{
    if ( item().hasProperty("longname"))
        _displayName =  item()["longname"].toString();
    else
        _displayName = item().name();
}

OperationModel::OperationModel(quint64 id, QObject *parent) : ResourceModel(mastercatalog()->id2Resource(id), parent)
{

}


QString OperationModel::inputparameterName(quint32 index) const
{
    return getProperty("pin_" + QString::number(index + 1) + "_name");

}

QString OperationModel::inputparameterTypeNames(quint32 index) const
{
    quint64 ilwtype = getProperty("pin_" + QString::number(index + 1) + "_type").toULongLong();
    QString type;
    for(quint64 i =0; i < 64; ++i){
       quint64 result = 1LL << i;
        if ( hasType(ilwtype, result)) {
            if ( type != "")
                type += ",";
            type += TypeHelper::type2HumanReadable(result);
        }
        if ( result > ilwtype)
            break;
    }
    return type;
}

QString OperationModel::inputparameterTypeInternalNames(quint32 index) const
{
	quint64 ilwtype = getProperty("pin_" + QString::number(index + 1) + "_type").toULongLong();
	QString type;
	for (quint64 i = 0; i < 64; ++i) {
		quint64 result = 1LL << i;
		if (hasType(ilwtype, result)) {
			if (type != "")
				type += "|";
			type += TypeHelper::type2name(result);
		}
		if (result > ilwtype)
			break;
	}
	return type;
}

QString OperationModel::inputparameterType(quint32 index) const
{
    return getProperty("pin_" + QString::number(index + 1) + "_type");
}

QString OperationModel::inputparameterDescription(quint32 index) const
{
    QString ret = getProperty("pin_" + QString::number(index + 1) + "_desc");
    if ( ret == sUNDEF)
        return "";
    return ret;
}

QString OperationModel::outputparameterName(quint32 index) const
{
    return getProperty("pout_" + QString::number(index + 1) + "_name");
}

QString OperationModel::outputparameterTypeNames(quint32 index) const
{
    quint64 ilwtype = getProperty("pout_" + QString::number(index + 1) + "_type").toULongLong();
    return TypeHelper::type2HumanReadable(ilwtype);
}

QString OperationModel::outputparameterType(quint32 index) const
{
    return getProperty("pout_" + QString::number(index + 1) + "_type");
}

QString OperationModel::outputparameterDescription(quint32 index) const
{
    QString ret = getProperty("pout_" + QString::number(index + 1) + "_desc");
    if ( ret == sUNDEF)
        return "";
    return ret;
}

QString OperationModel::syntax() const
{
    QString syn = getProperty("syntax");
    if ( syn.indexOf("ilwis://")!= -1){
        syn = syn.mid(QString("ilwis://operations/").size());
    }
    return syn;
}

QString OperationModel::pythonSyntax() const
{
    QString expr("ilwis.Engine.do(");
    expr += "'" + name() + "'";
    QStringList names = inParamNames();
    for(int i=0; i < names.size(); ++i){
        expr += ",";
        expr += names[i];
    }
    expr += ")";
    names = outParamNames();
    if ( names.size() == 1) {
        expr = names[0] + "=" + expr;
    }else if ( names.size() > 1){
        expr = "out_tuple=" + expr;
    }
    return expr;
}

QString OperationModel::provider() const
{
    QString kw =   getProperty("namespace");
    if ( kw == sUNDEF)
        return "";
    return kw;
}

int OperationModel::maxParameterCount(bool inputCount) const
{
    QString inParams = getProperty(inputCount ? "inparameters" : "outparameters");
    QStringList parts = inParams.split("|");
    int maxp = 0;
    for(QString p : parts){
        maxp = std::max(maxp , p.toInt());
    }
    return maxp;
}

QStringList OperationModel::inParamNames() const
{
    int maxp = maxParameterCount(true);
    QStringList names;
    for(int i = 0; i < maxp; ++i){
        names.append(inputparameterName(i));
    }
    return names;
}

QStringList OperationModel::outParamNames() const
{
    int maxp = maxParameterCount(false);
    QStringList names;
    for(int i = 0; i < maxp; ++i){
        names.append(outputparameterName(i));
    }
    return names;
}

QString OperationModel::inParameterCount() const
{
    QString inParams = getProperty("inparameters");
    if ( inParams != sUNDEF){
        inParams.replace("|"," or ");
    }else
        inParams = "";
    return inParams;
}

QString OperationModel::outParameterCount() const
{
    QString outParams = getProperty("outparameters");
    if ( outParams != sUNDEF){
        outParams.replace("|"," or ");
    }else
        outParams = "";
    return outParams;
}

QStringList OperationModel::inParameterIconList() const
{
    int mx = maxParameterCount(true);
    QStringList icons;
    for(int i = 0; i < mx; ++i){
        QString p = "pin_" + QString::number(i+1) + "_type";
        IlwisTypes type = item()[p].toULongLong();
        if ( type == iUNDEF)
            continue;
        icons.append(iconPath(type));

    }
    return icons;
}

QStringList OperationModel::outParameterIconList() const
{
    int mx = maxParameterCount(false);
    QStringList icons;
    for(int i = 0; i < mx; ++i){
        QString p = "pin_" + QString::number(i+1) + "_type";
        IlwisTypes type = item()[p].toULongLong();
        if ( type == iUNDEF)
            continue;
        icons.append(iconPath(type));

    }
    return icons;
}

QString OperationModel::getProperty(const QString &name) const
{
    if ( item().hasProperty(name))
        return item()[name].toString();
    return sUNDEF;
}

bool OperationModel::needChoice(OperationModel *other) const
{
    const auto check = [](const QString& type, const OperationModel *operation) ->bool const {
        bool ok = false;
        QString parms = operation->getProperty(type);
        QStringList parts = parms.split("|");
        for(const auto& part : parts){
            if ( part.toInt() > 1) {
                ok = true;
                break;
            }
        }
        return ok;
    };

    if (!other)
        return false;
    if ( check("inparameters", other))
        return true;
    return check("outparameters",this);
}

bool OperationModel::isLegalFlow(OperationModel *from, OperationModel *to) const
{

    if ( !to || !from)
        return false;

    int outParamCount = from->outParamNames().size();

    for(int i = 0; i < outParamCount; i++)
    {
        if(to->parameterIndexes(from->outputparameterType(i), true).size() != 0)
        {
            return true;
        }
    }
    //TODO: Error gooien
    return false;
}

QStringList OperationModel::choiceList(quint32 paramIndex) const
{
    Resource res = mastercatalog()->id2Resource(id().toULongLong());
    if (res.isValid()){
        Ilwis::IOperationMetaData metadata;
        if ( metadata.prepare(res)){
            return metadata->choiceList(paramIndex);
        }
    }
    return QStringList();
}

QStringList OperationModel::parameterIndexes(const QString &typefilter, bool fromOperation)
{
	QStringList indexes;
	bool ok;
	IlwisTypes tp1;
	if (typefilter == sUNDEF)
		tp1 = itANY;
	else {
		tp1 = typefilter.toULongLong(&ok);
		if (!ok) {
			QStringList types = typefilter.split("|");

			for (auto s : types) {
				quint64 tp = TypeHelper::name2type(s.trimmed());
				if (tp != itUNKNOWN)
					tp1 |= tp;
			}
		}
	}

    bool found = true;
    for(int i = 0; i < maxParameterCount(!fromOperation); ++i){
        if ( typefilter != ""){
            found = false;
            if ( typefilter != ""){
                quint64 tp2 = getProperty((fromOperation ? "pout_" : "pin_") + QString::number(i + 1) + "_type").toULongLong();
                //output == column is in reality a table
                if ( hasType(tp1, tp2) || (hasType(tp2,itTABLE) && tp1 == itCOLUMN) ||  (hasType(tp1,itTABLE) && tp2 == itCOLUMN) ){
                    found = true;
                }
            }
        }
        if ( found)
        {
            // for from flow you want to get the output names of the from operation and vice versa for the other case
            indexes.push_back(QString::number(i) + ": " + (fromOperation ? outputparameterName(i) : inputparameterName(i)));
        }
    }
    return indexes;
}

QString OperationModel::modelType() const
{
    return "operationmodel";
}

bool OperationModel::booleanOperation() const
{
    QString ret = getProperty("pout_" + QString::number(1) + "_type");
    if ( ret == sUNDEF)
        return false;
    return ret == "16777216"; // bool
}

QString OperationModel::specialIcon() const
{
    QString keyw = keywords();
    if ( keyw == "")
        return "";
    if (keyw.indexOf("workflow") != -1){
        return "workflow20.png";
    }
    if ( keyw.indexOf("visualization") != -1){
        return "vector_fill.png";
    }
    return "";
}
QString OperationModel::customForm() const {
	QString frm =  getProperty("customform");
	if (frm == sUNDEF)
		return frm;
	

	return QUrl::fromLocalFile(context()->ilwisFolder().absoluteFilePath() + "/qml/workbench/customforms/" + frm).toString();
}
QString OperationModel::fullDescription() const {

	auto breakLine = [](const QString& inp, int maxLine)->QString {
		QString result;
		bool possibleBreak = false;
		bool breakNeeded = false;
		int count = 0;
		for (int i = 0; i < inp.size(); ++i) {
			QChar c = inp[i];
			if (breakNeeded && (c == ' ' || c == ',')) {
				possibleBreak = true;
			}
			if (count > 50) {
				breakNeeded = true;
			}
			result += c;
			++count;
			if (breakNeeded && possibleBreak) {
				result += "<br>";
				possibleBreak = breakNeeded = false;
				count = 0;
			}
		}
		return result;
	};

	QString result = "<p><table>";
	result += QString("<tr><td><b>Name</b></td>") + "<td>" + name() + "</td></tr>";
	if (name() != displayName()) {
		result += QString("<tr><td><b>Display Name</b></td>") + "<td>" + displayName() + "</td></tr>";
	}

	result += QString("<tr><td><b>Description</b></td>") + "<td>" + breakLine(description(), 50) + "</td></tr>";
	result += QString("<tr><td><b>Python Syntax</b></td>") + "<td>" + pythonSyntax() + "</td></tr>";
	result += QString("<tr><td><b>Provider</b></td>") + "<td>" + provider() + "</td></tr>";
	result += QString("<tr><td><b>Keywords</b></td>") + "<td>" + keywords() + "</td></tr>";
	result += "<hr>";
	result += QString("<tr><td><b>Input parameters</b></td>") + "<td>" + QString::number(maxParameterCount(true)) + "</td></tr>";

	for (int i = 0; i < maxParameterCount(true); ++i) {
		result += QString("<tr><td><b>Parameter</b></td>") + "<td>" + QString::number(i) + "</td></tr>";
		result += QString("<tr><td><b>Name</b></td>") + "<td><i>" + inputparameterName(i) + "</i></td></tr>";
		result += QString("<tr><td><b>Types</b></td>") + "<td><i>" + breakLine(inputparameterTypeNames(i),50) + "</i></td></tr>";
		result += QString("<tr><td><b>Descrption</b></td>") + "<td><i>" + breakLine(inputparameterDescription(i), 50) + "</i></td></tr>";
	}
	result += "<hr>";
	result += QString("<tr><td><b>Output parameters</b></td>") + "<td>" + QString::number(maxParameterCount(false)) + "</td></tr>";
	for (int i = 0; i < maxParameterCount(false); ++i) {
		result += QString("<tr><td><b>Parameter</b></td>") + "<td>" + QString::number(i) + "</td></tr>";
		result += QString("<tr><td><b>Name</b></td>") + "<td><i>" + outputparameterName(i) + "</i></td></tr>";
		result += QString("<tr><td><b>Types</b></td>") + "<td><i>" + breakLine(outputparameterTypeNames(i), 50) + "</i></td></tr>";
		result += QString("<tr><td><b>Descrption</b></td>") + "<td><i>" + breakLine(outputparameterTypeNames(i), 50) + "</i></td></tr>";
	}
	return result + "</table></p>";
}

