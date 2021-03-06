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

#include <QApplication>
#include <QtQml>
#include <QtQuick/QQuickView>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSqlQuery>
#include <QFileSystemModel>
#include "kernel.h"
#include "errorobject.h"
#include "catalog/resource.h"
#include "ilwistype.h"
#include "catalog.h"
#include "mastercatalog.h"
#include "catalogview.h"
#include "ilwiscontext.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "table.h"
#include "operationmetadata.h"
#include "script.h"
#include "models/catalogfiltermodel.h"
#include "models/mastercatalogmodel.h"
#include "models/operationmodel.h"
#include "models/operationcatalogmodel.h"
#include "models/usermessagehandler.h"
#include "applicationformexpressionparser.h"
#include "workflowmetadataformbuilder.h"
#include "models/tranquilizerhandler.h"
#include "coveragedisplay/layermanager.h"
#include "coveragedisplay/coveragelayermodel.h"
#include "models/ilwisobjectmodel.h"
#include "models/attributemodel.h"
#include "models/domainitemmodel.h"
#include "models/operationsbykeymodel.h"
#include "models/uicontextmodel.h"
#include "models/projectionparametermodel.h"
#include "models/workflow/workflowmodel.h"
#include "models/workflow/modelbuilder.h"
#include "models/workflow/errormodel.h"
#include "coveragedisplay/visualattribute.h"
#include "coveragedisplay/propertyeditors/visualpropertyeditor.h"
#include "models/tablemodel.h"
#include "coveragedisplay/layerinfoitem.h"
#include "models/catalogmapitem.h"
#include "models/columnmodel.h"
#include "models/chartmodel.h"
#include "dataseriesmodel.h"
#include "chartoperationeditor.h"
#include "models/tabmodel.h"
#include "models/datapanemodel.h"
#include "models/objectcreator.h"
#include "models/ilwisobjectcreatormodel.h"
#include "models/preferencesmodel.h"
#include "models/internaldatabasemodel.h"
#include "coveragedisplay/propertyeditors/numericrepresentationsetter.h"
#include "coveragedisplay/propertyeditors/itemrepresentationsetter.h"
#include "coveragedisplay/propertyeditors/crosssection.h"
#include "ilwiscoreui/tableoperations/tableoperation.h"
#include "keyfilter.h"
#include "startilwis.h"
#include "scriptmodel.h"
#include "applicationmodel.h"
#include "analysispattern.h"
#include "modelbuilder.h"
#include "modeldesigner.h"
#include "modelregistry.h"
#include "controlpointmodel.h"
#include "controlpointslistmodel.h"
#include "workflow/analysismodel.h"
#include "workflow/conceptmodel.h"
#include "workflow/applicationmodelui.h"
#include "catalogoperationeditor.h"
#include "menumodel.h"
#include "filesystem.h"
#include "mousefilter.h"
#include "../core/buildnumber.h"

using namespace Ilwis;
using namespace Ui;
StartIlwis::StartIlwis()
{

}

StartIlwis::StartIlwis(QObject *parent, QQmlApplicationEngine *eng) : QObject(parent), _engine(eng)
{

}

void StartIlwis::init() {

    try{
        QQmlContext *ctx =_engine->rootContext();
        if(!Ilwis::initIlwis(Ilwis::rmDESKTOP)){
            exit(-1);
        }

        QFileInfo ilwisroot = context()->ilwisFolder();
        QString qmlpluginpath = ilwisroot.absoluteFilePath() + "/extensions/ui";

        _engine->addImportPath(qmlpluginpath);
        _engine->addPluginPath(qmlpluginpath);

        qmlRegisterType<MasterCatalogModel>("MasterCatalogModel",1,0,"MasterCatalogModel");
        qmlRegisterType<CatalogModel>("CatalogModel",1,0,"CatalogModel");
        qmlRegisterType<WorkSpaceModel>("WorkSpaceModel",1,0,"WorkSpaceModel");
        qmlRegisterType<ResourceModel>("ResourceModel",1,0,"ResourceModel");
        qmlRegisterType<OperationCatalogModel>("OperationCatalogModel",1,0,"OperationCatalogModel");
        qmlRegisterType<OperationModel>("OperationModel",1,0,"OperationModel");
        qmlRegisterType<ApplicationFormExpressionParser>("ApplicationFormExpressionParser",1,0,"FormBuilder");
        qmlRegisterType<WorkflowMetadataFormBuilder>("WorkflowMetadataFormBuilder",1,0,"WorkflowMetadataFormBuilder");
        qmlRegisterType<UserMessageHandler>("UserMessageHandler",1,0,"UserMessageHandler");
        qmlRegisterType<MessageModel>("MessageModel",1,0,"MessageModel");
        qmlRegisterType<Ilwis::Ui::TranquilizerHandler>("TranquilizerHandler",1,0, "TranquilizerHandler");
        qmlRegisterType<TranquilizerModel>("TranquilizerModel",1,0, "TranquilizerModel");
        qmlRegisterType<LayerManager>("LayerManager",1,0,"LayerManager");
        qmlRegisterType<Ilwis::Ui::LayerModel>("LayerModel",1,0,"LayerModel");
        qmlRegisterType<RootLayerModel>("RootLayerModel",1,0,"GlobalLayerModel");
        qmlRegisterType<IlwisObjectModel>("IlwisObjectModel",1,0,"IlwisObjectModel");
        qmlRegisterType<AttributeModel>("AttributeModel",1,0,"AttributeModel");
        qmlRegisterType<VisualAttribute>("VisualAttribute",1,0,"VisualAttribute");
        qmlRegisterType<Ilwis::Ui::DomainItemModel>("DomainItemModel",1,0,"DomainItemModel");
        qmlRegisterType<Ilwis::Ui::OperationsByKeyModel>("OperationsByKeyModel",1,0,"OperationsByKeyModel");
        qmlRegisterType<UIContextModel>("UIContextModel", 1,0, "UIContextModel");
        qmlRegisterType<VisualPropertyEditor>("VisualPropertyEditor", 1,0, "VisualPropertyEditor");
        qmlRegisterType<NumericRepresentationSetter>("NumericRepresentationSetter", 1,0, "NumericRepresentationSetter");
        qmlRegisterType<ItemRepresentationSetter>("ItemRepresentationSetter", 1,0, "ItemRepresentationSetter");
        qmlRegisterType<RepresentationElementModel>("RepresentationElement", 1,0, "RepresentationElement");
        qmlRegisterType<ProjectionParameterModel>("ProjectionParameterModel", 1,0, "ProjectionParameterModel");
        qmlRegisterType<WorkflowModel>("WorkflowModel", 1,0, "WorkflowModel");
		qmlRegisterType<CatalogOperationEditor>("CatalogOperationEditor", 1, 0, "CatalogOperationEditor");
       
        qmlRegisterType<TableModel>("TableModel", 1,0,"TableModel");
        qmlRegisterType<TableOperation>("TableOperation",1,0,"TableOperation");
        qmlRegisterType<ColumnModel>("ColumnModel", 1,0,"ColumnModel");
        qmlRegisterType<LayerInfoItem>("LayerInfoItem", 1,0,"LayerInfoItem");
        qmlRegisterType<CatalogMapItem>("CatalogMapItem", 1,0,"CatalogMapItem");

		qmlRegisterType<ChartModel>("ChartModel", 1, 0, "ChartModel");
		qmlRegisterType<DataseriesModel>("DataseriesModel", 1, 0, "DataseriesModel");
        qmlRegisterType<ChartOperationEditor>("ChartOperationEditor", 1, 0, "ChartOperationEditor");

        qmlRegisterType<CatalogFilterModel>("CatalogFilterModel", 1,0,"CatalogFilterModel");
        qmlRegisterType<DataPaneModel>("DataPaneModel", 1,0,"DataPaneModel");
        qmlRegisterType<TabModel>("TabModel", 1,0,"TabModel");
        qmlRegisterType<SidePanelModel>("SidePanelModel", 1,0,"SidePanelModel");
        qmlRegisterType<ObjectCreator>("ObjectCreator", 1,0,"ObjectCreator");
        qmlRegisterType<IlwisObjectCreatorModel>("IlwisObjectCreatorModel", 1,0,"IlwisObjectCreatorModel");
        qmlRegisterType<ModelBuilder>("ModelBuilder", 1,0,"ModelBuilder");
        qmlRegisterType<ModelDesigner>("ModelDesigner", 1,0,"ModelDesigner");
        qmlRegisterType<ErrorModel>("ErrorModel", 1,0,"ErrorModel");
        qmlRegisterType<PreferencesModel>("PreferencesModel",1,0,"PreferencesModel");
        qmlRegisterType<InternalDatabaseModel>("InternalDatabaseModel",1,0,"InternalDatabaseModel");
        qmlRegisterType<ScriptModel>("ScriptModel",1,0,"ScriptModel");
        qmlRegisterType<ApplicationModelUI>("ApplicationModel",1,0,"ApplicationModel");
        qmlRegisterType<AnalysisModel>("AnalysisModel",1,0,"AnalysisModel");
        qmlRegisterType<ConceptModel>("ConceptModel",1,0,"ConceptModel");
        qmlRegisterType<CoverageLayerModel>("CoverageLayerModel", 1, 0, "CoverageLayerModel");
		qmlRegisterType<ModelRegistry>("ModelRegistry", 1, 0, "ModelRegistry");
        qmlRegisterType<CrossSectionPin>("CrossSectionPin", 1, 0, "CrossSectionPin");
        qmlRegisterType<PinDataSource>("PinDataSource", 1, 0, "PinDataSource");
        qmlRegisterType<ControlPointModel>("ControlPointModel", 1, 0, "ControlPointModel");
        qmlRegisterType<ControlPointsListModel>("ControlPointsListModel", 1, 0, "ControlPointsListModel");
		qmlRegisterType<FileSystem>("FileSystem", 1, 0, "FileSystem");
		qmlRegisterType<MenuModel>("MenuModel", 1, 0, "MenuModel");


        _mastercatalogmodel = new MasterCatalogModel(ctx);
        _formbuilder = new ApplicationFormExpressionParser();
        _messageHandler = new UserMessageHandler();
        _operations = new OperationCatalogModel();
        _tranquilizers =new TranquilizerHandler();
        _modelBuilder =new ModelBuilder();
        _datapane =new DataPaneModel();
        _objcreator =new ObjectCreator();
        _preferences =new PreferencesModel();
        _database = new InternalDatabaseModel();
        _trqthread = new QThread;
		_mousePosition = new CursorPosProvider();

		/*_filesystemmodel = new QFileSystemModel();
		_filesystemmodel->setRootPath("");
		_filesystemmodel->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot);*/
		//tree->setCurrentIndex(fsModel->index(QDir::currentPath()));

        ctx->setContextProperty("mastercatalog", _mastercatalogmodel);
        ctx->setContextProperty("formbuilder", _formbuilder);
        ctx->setContextProperty("messagehandler", _messageHandler);
        ctx->setContextProperty("tranquilizerHandler", _tranquilizers);
        ctx->setContextProperty("operations", _operations);
        ctx->setContextProperty("modelbuilder", modelbuilder().get());
        ctx->setContextProperty("datapane", _datapane);
        ctx->setContextProperty("objectcreator", _objcreator);
        ctx->setContextProperty("preferences",_preferences);
        ctx->setContextProperty("internaldatabase",_database);
        ctx->setContextProperty("uicontext", uicontext().get());
        ctx->setContextProperty("models", modelregistry().get());
		ctx->setContextProperty("mouseposition", _mousePosition);
		//ctx->setContextProperty("filesystem", _filesystemmodel);
		//ctx->setContextProperty("rootPathIndex", _filesystemmodel->index(_filesystemmodel->rootPath()));

		uicontext()->qmlContext(ctx);
        uicontext()->prepare();

        _mastercatalogmodel->prepare();
        _operations->prepare({"globaloperationscatalog", true});

        qRegisterMetaType<IssueObject>("IssueObject");
        qRegisterMetaType<UrlSet>("UrlSet");


        _operations->connect(uicontext().get(),&UIContextModel::currentWorkSpaceChanged, _operations, &OperationCatalogModel::workSpaceChanged);
        _messageHandler->connect(kernel()->issues().data(), &IssueLogger::updateIssues,_messageHandler, &UserMessageHandler::addMessage );


        TranquilizerWorker *trw = new TranquilizerWorker;
        trw->moveToThread(_trqthread);
        _trqthread->setProperty("workingcatalog", qVariantFromValue(context()->workingCatalog()));
        _trqthread->connect(kernel(), &Kernel::updateTranquilizer, trw, &TranquilizerWorker::updateTranquilizer);
        _trqthread->connect(kernel(), &Kernel::createTranquilizer, trw, &TranquilizerWorker::createTranquilizer);
        _trqthread->connect(kernel(), &Kernel::removeTranquilizer, trw, &TranquilizerWorker::removeTranquilizer);
        _trqthread->connect(trw, &TranquilizerWorker::sendUpdateTranquilizer, _tranquilizers, &TranquilizerHandler::updateTranquilizer);
        _trqthread->connect(trw, &TranquilizerWorker::sendCreateTranquilizer, _tranquilizers, &TranquilizerHandler::createTranquilizer);
        _trqthread->connect(trw, &TranquilizerWorker::sendRemoveTranquilizer, _tranquilizers, &TranquilizerHandler::removeTranquilizer);

        _trqthread->start();


        _keys = new KeyFilter() ;
        _mastercatalogmodel->connect(_keys, &KeyFilter::keyPressed, _mastercatalogmodel, &MasterCatalogModel::keyPressed);
        _mastercatalogmodel->connect(_keys, &KeyFilter::keyReleased, _mastercatalogmodel, &MasterCatalogModel::keyReleased);
        QCoreApplication::instance()->installEventFilter(_keys);
    } catch(ErrorObject&){

    } catch(...){

    }
}

void StartIlwis::initMainWindow()
{
	QObject *top = _engine->rootObjects().value(0);
	QObject *topLevel = top->findChild<QObject *>("root__mainui");
	if (topLevel) {
		uicontext()->rootObject(topLevel);
	}
	QQuickWindow *window = qobject_cast<QQuickWindow *>(top);
	window->setIcon(QIcon("./qml/images/ilwis4.png"));

	window->setTitle(" ");
	if (!window) {
		qWarning("Error: Your root item has to be a Window.");
		return;
	}
	window->show();

    uicontext()->initializeDataPane();
}

QString StartIlwis::buildType() const
{
#ifdef ILWIS_VERSION_TYPE
    return ILWIS_VERSION_TYPE;
#endif
    return "";
}

QString StartIlwis::verionNumber() const
{
#ifdef ILWIS_VERSION_NUMBER
    return QString::number(ILWIS_VERSION_NUMBER);
#endif
return "";
}

QString StartIlwis::buildNumber() const
{
#ifdef ILWIS_BUILDNR
    return QString::number(ILWIS_BUILDNR);
#endif
    return "";
}

void StartIlwis::stop()
{

    Ilwis::exitIlwis();
    delete _mastercatalogmodel;
    delete _formbuilder;
    delete _messageHandler;
    delete _operations;
    delete _tranquilizers;
    delete _modelBuilder;
    delete _datapane;
    delete _objcreator;
    delete _preferences;
    delete _keys;
    _trqthread->deleteLater();

}

