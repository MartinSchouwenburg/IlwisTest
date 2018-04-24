#pragma once

#include <memory>
#include <QObject>
#include <QAbstractItemModel>
#include "kernel.h"
#include "ilwisdata.h"
#include "coverage.h"
#include "layermodel.h"
#include "rootlayermodel.h"
#include "ilwiscoreui_global.h"

class QQuickItem;

namespace Ilwis {
class GeoReference;
class CoordinateSystem;
typedef IlwisData<GeoReference> IGeoReference;
typedef IlwisData<CoordinateSystem> ICoordinateSystem;


namespace Ui {

class UIContextModel;
class RootLayerModel;
class LayerModel;
class CoverageLayerModel;
class LayerInfoItem;
class LayerManager;
class CompositeLayerModel;

typedef std::function<LayerModel *(LayerManager *, QStandardItem *,const QString&, const QString&, const IOOptions&)> CreateLayer;

class ILWISCOREUISHARED_EXPORT LayerManager : public QObject
{
   
    Q_OBJECT

    Q_PROPERTY(QStandardItemModel * layerTree READ layerTree NOTIFY layerTreeChanged)
    Q_PROPERTY(QQmlListProperty<Ilwis::Ui::LayerModel> topLevelLayers READ childLayersPrivate NOTIFY topLevelLayersChanged)
    Q_PROPERTY(QQmlListProperty<Ilwis::Ui::LayerModel> allCoverages READ allCoveragesPrivate NOTIFY allCoveragesChanged)
    Q_PROPERTY(quint32 viewid READ viewid CONSTANT)
    Q_PROPERTY(bool hasSelectionDrawer READ hasSelectionDrawer WRITE setHasSelectionDrawer NOTIFY hasSelectionDrawerChanged)
    Q_PROPERTY(bool zoomInMode READ zoomInMode WRITE setZoomInMode NOTIFY zoomInModeChanged)
    Q_PROPERTY(bool zoomOutMode READ zoomOutMode WRITE setZoomOutMode NOTIFY zoomOutModeChanged)
    Q_PROPERTY(bool panningMode READ panningMode WRITE setPanningMode NOTIFY panningModeChanged)
    Q_PROPERTY(Ilwis::Ui::RootLayerModel* rootLayer READ rootLayer CONSTANT)
    Q_PROPERTY(bool isValid READ isValid NOTIFY isValidChanged)
    Q_PROPERTY(bool needUpdate READ needUpdate WRITE needUpdate NOTIFY needUpdateChanged)
    Q_PROPERTY(LayerModel *lastAddedCoverageLayer READ lastAddedCoverageLayer CONSTANT)

public:
	enum LayerMovement { lmUP, lmDOWN, lmREMOVE };
    enum ManagerType { mtMAIN, mtOVERVIEW, mtTHUMB, mtUNKNOWN };

    explicit LayerManager();
    LayerManager(QObject *parent, QQuickItem *viewContainer);
    ~LayerManager();

    Q_INVOKABLE void setLayerListName(const QString name);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addCommand(const QString& expression);
	Q_INVOKABLE void viewArea(QQuickItem *area);
	Q_INVOKABLE void wholeMap();
	Q_INVOKABLE  Ilwis::Ui::LayerModel* findLayer(int nodeid);
    Q_INVOKABLE int nodeid(QModelIndex idx) const;
    Q_INVOKABLE void move(int nodeId, const QModelIndex& targetLocation);
    Q_INVOKABLE quint32 modelId() const;
    Q_INVOKABLE quint32 viewid() const; // modelid == viewid, for historical reasons (viewid is older) it remains in the interface as it is used in qml
    Q_INVOKABLE void setSelection(const QString &pixelpair);

    RootLayerModel *rootLayer() const;
    
    bool hasSelectionDrawer() const;
    void setHasSelectionDrawer(bool yesno);
	bool zoomInMode() const;
	void setZoomInMode(bool yesno);
	bool zoomOutMode() const;
	void setZoomOutMode(bool yesno);
	bool panningMode() const;
	void setPanningMode(bool yesno);

    QString layerListName() const;
    QStandardItemModel *layerTree();
	LayerModel *lastAddedCoverageLayer() const;
	void lastAddedCoverageLayer(LayerModel *lyr);
	QString layerData(const Coordinate &crdIn, const QString& attrName, QVariantList &items);


    static LayerModel *create(QStandardItem *parentLayer,  const ICoverage& cov, LayerManager *lm, const IOOptions& options = IOOptions());

   

    static LayerModel *create(QStandardItem *parentLayer,  const QString& type, LayerManager *lm, const QString& layername, const QString& description="", const IOOptions& options= IOOptions());
    static int registerLayerModel(const QString& modelname, CreateLayer);

    void moveLayer(LayerModel *parentLayer, LayerModel *layer, LayerMovement type);

    void clearLayers(LayerModel *parentLayer);
    bool isValid() const;
	QQuickItem *viewArea() const;

	void needUpdate(bool yesno);
	bool needUpdate() const;
    int nextId();
    QVariantList linkProperties() const;
    ManagerType managerType() const;

public slots:
        void linkAcceptMessage(quint32 id, const QString& sourceType, const QVariantMap& parameters);
signals:
    void removeLayer(const Ilwis::Resource& resource);
	void layerTreeChanged();
    void hasSelectionDrawerChanged();
	void zoomInModeChanged();
    void zoomOutModeChanged();
	void panningModeChanged();
    void isValidChanged();
	void layerCountChanged();
	void needUpdateChanged();
    void topLevelLayersChanged();
    void allCoveragesChanged();
    void linkSendMessage(const QVariantMap& parameters);

private:
    RootLayerModel *_globalLayer = 0;
    QStandardItemModel  *_tree = 0;
    static std::map<QString, CreateLayer> _createLayers;
    bool _hasSelectionDrawer = false;
	bool _zoomInMode = false;
    bool _zoomOutMode = false;
	bool _panningMode = false;
    QString _layerListName = sUNDEF;
    quint32 _modelId = iUNDEF;
	QQuickItem *_viewContainer = 0;
	int _nodeCounter = 0;
    static quint32 _baseViewId;
    ManagerType _managerType = mtUNKNOWN;

	bool _needUpdate = false; // needed when a property of the whole rendering changed (e.g. zoom)
	LayerModel *_lastAddedCoverageLayer = 0;
    QList<LayerModel *> _childeren; //this list is filled on the fly in  childLayersPrivate, don't rely on it to have contents
    QList<LayerModel *> _coverages; //this list is filled on the fly in  allCoveragesPrivate, don't rely on it to have contents

    static void addLayer(QStandardItem * parentLayer, LayerModel * layer, LayerManager * lm, int lowernodid);
    QQmlListProperty<LayerModel> childLayersPrivate();
    QQmlListProperty<LayerModel> allCoveragesPrivate();
    void setSelectionPrivate(const Coordinate& crd, LayerModel * layer);

};

}
}

