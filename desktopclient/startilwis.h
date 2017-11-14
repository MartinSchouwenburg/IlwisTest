#ifndef STARTILWIS_H
#define STARTILWIS_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>

class ApplicationFormExpressionParser;
class WorkflowMetadataFormBuilder;


class PreferencesModel;

class KeyFilter;
class QThread;
class InternalDatabaseModel;

namespace Ilwis {
	namespace Ui {
		class MasterCatalogModel;
		class OperationCatalogModel;
		class TranquilizerHandler;
		class DataPaneModel;
		class ObjectCreator;
		class ModelBuilder;
		class UserMessageHandler;
	}
}



class StartIlwis : public QObject
{
    Q_OBJECT
public:
    explicit StartIlwis();
    explicit StartIlwis(QObject *parent, QQmlApplicationEngine *eng);

    Q_INVOKABLE void init();
    Q_INVOKABLE void initMainWindow();
    Q_INVOKABLE QString buildType() const;
    Q_INVOKABLE QString verionNumber() const;
    Q_INVOKABLE QString buildNumber() const;

    void stop();
signals:

public slots:

private:
	Ilwis::Ui::MasterCatalogModel *_mastercatalogmodel;
	ApplicationFormExpressionParser *_formbuilder;
	Ilwis::Ui::UserMessageHandler *_messageHandler;
	Ilwis::Ui::OperationCatalogModel *_operations;
	Ilwis::Ui::TranquilizerHandler *_tranquilizers;
	Ilwis::Ui::ModelBuilder *_modelBuilder;
	Ilwis::Ui::DataPaneModel *_datapane;
	Ilwis::Ui::ObjectCreator *_objcreator;
	PreferencesModel *_preferences;
	InternalDatabaseModel *_database;
	KeyFilter *_keys;

    QQmlApplicationEngine *_engine;
    QThread *_trqthread;

//    std::unique_ptr<MasterCatalogModel> _mastercatalogmodel;
//    std::unique_ptr<ApplicationFormExpressionParser> _formbuilder;
//    std::unique_ptr<WorkflowMetadataFormBuilder> workflowmetadataformbuilder;
//    std::unique_ptr<UserMessageHandler> _messageHandler;
//    std::unique_ptr<OperationCatalogModel> operations;
//    std::unique_ptr<TranquilizerHandler> tranquilizers;
//    std::unique_ptr<ScenarioBuilderModel> scenarios;
//    std::unique_ptr<DataPaneModel> datapane;
//    std::unique_ptr<ObjectCreator> objcreator;
//    std::unique_ptr<PreferencesModel> preferences;
//    std::unique_ptr<KeyFilter> _keys;
};


#endif // STARTILWIS_H