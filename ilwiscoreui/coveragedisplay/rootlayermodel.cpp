#include "kernel.h"
#include "ilwisdata.h"
#include "representation.h"
#include "geometries.h"
#include "georeference.h"
#include "coordinatesystem.h"
#include "attributemodel.h"
#include "visualattribute.h"
#include "ilwisobjectmodel.h"
#include "util/size.h"
#include "rootlayermodel.h"
#include "coveragelayermodel.h"
#include "raster.h"


using namespace Ilwis;
using namespace Ui;

//REGISTER_LAYERMODEL("globallayermodel",GlobalLayerModel);

RootLayerModel::RootLayerModel() : LayerModel()
{
	_layerType = itROOTLAYER;
}

RootLayerModel::RootLayerModel(LayerManager *lm, QStandardItem *parent) :
    LayerModel(lm, parent, TR("Global Properties"),"", IOOptions())
{
	_layerType = itROOTLAYER;
 
	_cameraPosition = { 0,0,0 };
	_isValid = true;
	_icon = "settings_green.png";

	fillData();
}

void RootLayerModel::setActiveAttribute(int idx)
{
    if (idx < _visualAttributes.size()) {
        _activeAttribute = _visualAttributes[idx]->attributename();
        add2ChangedProperties("buffers", true);
        emit activeAttributeChanged();
    }
}

QVariant RootLayerModel::vproperty(const QString &attrName) const
{
    return QVariant();
}

void RootLayerModel::vproperty(const QString &attrName, const QVariant &value)
{

}

QString RootLayerModel::url() const
{
  return "";
}

Envelope RootLayerModel::zoomEnvelope() const
{
    return _zoomEnvelope;
}

void RootLayerModel::zoomEnvelope(const Envelope &zoomEnvelope)
{
    _zoomEnvelope = zoomEnvelope;
	if (_screenGrf.isValid()) {
		_screenGrf->envelope(_zoomEnvelope);
		_screenGrf->compute();
		emit zoomEnvelopeChanged();
        layerManager()->updateAxis();
	}
}

Envelope RootLayerModel::coverageEnvelope() const
{
	return _coverageEnvelope;
}

void RootLayerModel::coverageEnvelope(const Envelope &cEnvelope)
{
	_coverageEnvelope = cEnvelope;
}

bool RootLayerModel::showLayerInfo() const
{
    return _showLayerInfo;
}

void RootLayerModel::showLayerInfo(bool showLayerInfo)
{
    _showLayerInfo = showLayerInfo;
}

void RootLayerModel::scrollInfo(const QVariantMap & si)
{
	if (si.size() > 0) {
		Coordinate center = _zoomEnvelope.center();
		if (si.contains("leftpositionperc")) {
			double xold = _zoomEnvelope.min_corner().x;
			_zoomEnvelope.min_corner().x = (_viewEnvelope.xlength() - 1) * si["leftpositionperc"].toDouble() + _viewEnvelope.min_corner().x;
			double dx = xold - _zoomEnvelope.min_corner().x;
			_zoomEnvelope.max_corner().x -= dx; // keep the width the same
			center = _zoomEnvelope.center();
		}
		if (si.contains("bottompositionperc")) {
			double yold = _zoomEnvelope.max_corner().y;
			_zoomEnvelope.max_corner().y = _viewEnvelope.max_corner().y - (_viewEnvelope.ylength() - 1) * si["toppositionperc"].toDouble();
			double dy = yold - _zoomEnvelope.max_corner().y;
			_zoomEnvelope.min_corner().y -= dy; // keep the height the same
			center = _zoomEnvelope.center();
		}
		zoomEnvelope(_zoomEnvelope);
		_cameraPosition.x = center.x - _viewEnvelope.center().x;
		_cameraPosition.y = center.y - _viewEnvelope.center().y;
		layerManager()->refresh();
	}
}

QVariantMap RootLayerModel::scrollInfo() const
{
	double xPercL = 0;
	double xPercR = 1;
	double yPercT = 1;
	double yPercB = 0;

	if (_zoomEnvelope.isValid() && _viewEnvelope.isValid()) {
		xPercL = (_zoomEnvelope.min_corner().x - _viewEnvelope.min_corner().x) / (_viewEnvelope.xlength() - 1);
		yPercB = 1.0 - (_zoomEnvelope.min_corner().y - _viewEnvelope.min_corner().y) / (_viewEnvelope.ylength() - 1);
		xPercR = 1.0 - (_viewEnvelope.max_corner().x - _zoomEnvelope.max_corner().x) / (_viewEnvelope.xlength() - 1);
		yPercT = (_viewEnvelope.max_corner().y - _zoomEnvelope.max_corner().y) / (_viewEnvelope.ylength() - 1);
	}

	QVariantMap data;
	data["leftpositionperc"] = xPercL;
	data["bottompositionperc"] = yPercB;
	data["rightpositionperc"] = xPercR;
	data["toppositionperc"] = yPercT;
	data["xsizeperc"] = std::abs(xPercL - xPercR);
	data["ysizeperc"] = std::abs(yPercT - yPercB);

	return data;
}

bool Ilwis::Ui::RootLayerModel::prepare(int prepType)
{
    if (!hasType(prepType, LayerModel::ptGEOMETRY | LayerModel::ptRENDER)) {
        addVisualAttribute(new GlobalAttributeModel(TR("Geometry"), "", this));
        addVisualAttribute(new GlobalAttributeModel(TR("3D"), "", this));

        LayerManager::create(this, "gridlayer", layerManager(), sUNDEF, sUNDEF);
        LayerManager::create(0, "backgroundlayer", layerManager(), sUNDEF, sUNDEF);

        _prepared |= (LayerModel::ptGEOMETRY | LayerModel::ptRENDER);
    }

    return true;
}

QVariantMap RootLayerModel::latlonEnvelope() const
{
    Envelope env;
    if (_screenCsy->isLatLon()) {
        env = _viewEnvelope;
    }
    else {
        ICoordinateSystem csyWgs84("code=epsg:4326");
        env = csyWgs84->convertEnvelope(_screenCsy, _viewEnvelope);
    }
    return env.toMap();
}

IlwisObjectModel *RootLayerModel::screenCsyPrivate()
{
    return _csy;
}

IlwisObjectModel *RootLayerModel::screenGrfPrivate()
{
    return _grf;
}

QString Ilwis::Ui::RootLayerModel::projectionInfoPrivate() const
{
    if (_csy) {
        return _csy->projectionInfo();
    }
    return "";
}

double Ilwis::Ui::RootLayerModel::width() const
{
	return _viewEnvelope.xlength() - 1;
}

double Ilwis::Ui::RootLayerModel::height() const
{
	return _viewEnvelope.ylength() - 1;
}

QVariantMap RootLayerModel::viewEnvelopePrivate() const
{
    if ( _viewEnvelope.isValid())
        return _viewEnvelope.toMap();
    return QVariantMap();
}

QVariantMap RootLayerModel::zoomEnvelopePrivate() const
{
    if ( _zoomEnvelope.isValid())
        return _zoomEnvelope.toMap();
    return viewEnvelopePrivate();
}

void RootLayerModel::viewBox(const BoundingBox& box) {
	_viewBox = box;
}
BoundingBox RootLayerModel::viewBox() const {
	return _viewBox;
}

bool RootLayerModel::zoomInMode() const
{
    return _zoomInMode;
}

void RootLayerModel::setZoomInMode(bool yesno)
{
    _zoomInMode = yesno;
}

bool RootLayerModel::panningMode() const
{
    return _panningMode;
}

void RootLayerModel::setPanningMode(bool yesno)
{
    _panningMode = yesno;
}

void RootLayerModel::viewEnvelope(const Envelope &env)
{
	if (!_viewEnvelope.isValid())
		_coverageEnvelope = env;
    _viewEnvelope = env;
    emit viewEnvelopeChanged();
    emit latlonEnvelopeChanged();
    emit xGridAxisValuesChanged();
}

Envelope Ilwis::Ui::RootLayerModel::viewEnvelope() const
{
	return _viewEnvelope;
}



void RootLayerModel::clearLayers()
{
    LayerModel::clearLayers();
    _zoomInMode = false;
    _panningMode = false;
    _viewEnvelope = Envelope();
	_viewBox = BoundingBox();
	_coverageEnvelope = Envelope();
    _showLayerInfo = true;
    _layerInfoItems = QVariantList();
}

QVariantMap RootLayerModel::coord2Screen(const QVariantMap &var) const
{
    Envelope env(var);

    BoundingBox bb = screenGrf()->coord2Pixel(env);

    return bb.toMap();
}

QVariantMap RootLayerModel::screen2Coord(const QVariantMap &var) const
{
    Pixeld p(var);

    Coordinate c = screenGrf()->pixel2Coord(p);

    return c.toMap();
}

IGeoReference RootLayerModel::screenGrf() const
{
    return _screenGrf;
}

void RootLayerModel::screenGrf(const IGeoReference &screenGrf)
{
    _screenGrf = screenGrf;
    _csy = new IlwisObjectModel(_screenGrf->resource(),this);

    emit georefChanged();
}

ICoordinateSystem RootLayerModel::screenCsy() const
{
    return _screenCsy;
}

void RootLayerModel::screenCsy(const ICoordinateSystem &screenCsy)
{
    _screenCsy = screenCsy;
    _csy = new IlwisObjectModel(_screenCsy->resource(),this);

    emit coordinateSystemChanged();
    emit latlonEnvelopeChanged();
    emit projectionInfoChanged();
    emit zoomEnvelopeChanged();
}

void RootLayerModel::setCurrentCoordinate(const QString &var)
{
	if (var != "") {
		QStringList parts = var.split("|");
		if (parts.size() == 2 && _screenCsy.isValid() && _screenGrf.isValid()) {
			_currentCoordinate = _screenGrf->pixel2Coord(Ilwis::Pixel(parts[0].toDouble(), parts[1].toDouble()));
			emit currentcurrentCoordinateChanged();
			emit currentLatLonChanged();
		}
	}
}

QString RootLayerModel::currentCoordinate() const
{
	if (_screenCsy.isValid()) {
		if (_screenCsy->isLatLon()) {
			QString crd =  _currentCoordinate.toString(6);
			return crd;
		}
	}
	return _currentCoordinate.toString(2);
}

QString RootLayerModel::currentLatLon() const
{
	if (_screenCsy.isValid()) {
		if (_screenCsy->isLatLon()) {
			LatLon ll(_currentCoordinate.y, _currentCoordinate.x);
			return ll.toString();
		}
		else if (_screenCsy->canConvertToLatLon())
			return _screenCsy->coord2latlon(_currentCoordinate).toString();
	}
	return "";
}

QVariantMap RootLayerModel::drawEnvelope(const QString& envelope) const{
    QVariantMap vmap;
    try {
        if ( envelope == sUNDEF)
            return QVariantMap();

        Ilwis::Envelope llenv(envelope);
        if ( llenv.isValid() && !llenv.isNull() && screenGrf().isValid())    {

            Ilwis::BoundingBox bb = screenGrf()->coord2Pixel(llenv);
            vmap["minx"] = bb.min_corner().x;
            vmap["miny"] = bb.min_corner().y;
            vmap["width"] = bb.xlength();
            vmap["height"] = bb.ylength();
        }
        return vmap;
    } catch (const Ilwis::ErrorObject& ){

    } catch (std::exception& ex){
        Ilwis::kernel()->issues()->log(ex.what());
    }
    return vmap;
}

/*QString RootLayerModel::layerInfo(const Coordinate &crdIn, const QString &attrName, QVariantList &items) 
{
	QString result;
	auto layers = layersManager()->layerList2();
	for (LayerModel *layer : layers) {
		if (result != "")
			result += ";";
		result += layer->layerInfo(crdIn, attrName, items);
	}
	return result;

	
}*/

QString RootLayerModel::layerInfo(const QString& pixelpair)  
{
    try {
        if ( zoomInMode() || panningMode()) // when zooming we dont don' give info. costs too much performance
            return "";

        if ( layerManager()){
			_layerInfoItems.clear();
            QStringList parts = pixelpair.split("|");
            if ( parts.size() == 2 ){
                Ilwis::Coordinate crd = _screenGrf->pixel2Coord(Ilwis::Pixel(parts[0].toDouble(), parts[1].toDouble()));
               QString ret = layerManager()->layerData(crd,"", _layerInfoItems);
			   emit layerInfoItemsChanged();
			   return ret;
            }
        }
        return "";
    }
    catch(const ErrorObject& ){}
    catch(const std::exception& ex){
        kernel()->issues()->log(ex.what());
    }
    return "";
}

QVariantList RootLayerModel::layerInfoItems()
{
    return _layerInfoItems;
}

void RootLayerModel::initSizes(int newwidth, int newheight, bool initial) {

	auto CalcNewSize = [](double oldDim1, double dim2, double aspect)->double {
		double viewdim = dim2 * aspect;
		return (viewdim - oldDim1) / 2.0;
	};
	Size<> sz = (initial || !_screenGrf.isValid()) ? Size<>() : _screenGrf->size();

	double aspectRatioView = (double)newwidth / (double)newheight;
	double deltaX = 0, deltaY = 0;
	double cwidth = _coverageEnvelope.xlength() - 1;
	double cheight = _coverageEnvelope.ylength() - 1;
    double aspectRatioCoverage = cwidth / cheight;

	if (aspectRatioCoverage < aspectRatioView) {
		deltaX = CalcNewSize(cwidth, cheight, aspectRatioView);
	}
	else {
		deltaY = CalcNewSize(cheight, cwidth, 1.0 / aspectRatioView);
	}

	Coordinate pmin = _coverageEnvelope.min_corner();
	Coordinate pmax = _coverageEnvelope.max_corner();
	pmin = { pmin.x - deltaX, pmin.y - deltaY, 0 };
	pmax = { pmax.x + deltaX, pmax.y + deltaY, 0 };
	//qDebug() << "dx="<< deltaX << "dy="<< deltaY << "arv=" << aspectRatioView << "atc=" << aspectRatioCoverage << "minx=" << pmin.x << "miny=" << pmin.y << "maxx=" << pmax.x << "maxyx=" << pmax.y;
	//_viewEnvelope = { pmin, pmax };
    viewEnvelope({ pmin, pmax });

    kernel()->issues()->log(QString("init sizes %1 %2").arg(pmin.x).arg(pmin.y) , IssueObject::itMessage);

	if (!_screenGrf.isValid()) {
		IGeoReference grf;
		grf.prepare();
		grf->create("corners");
		grf->coordinateSystem(_screenCsy);
		screenGrf(grf);
	}
	_screenGrf->size(Size<>(newwidth, newheight, 1));
    if (initial)
        zoomEnvelope(_viewEnvelope);
	
}

double RootLayerModel::zoomFactor() const
{
	return _zoomFactor;
}

void RootLayerModel::zoomFactor(double zf)
{
	if (zf > 0.0001)
		_zoomFactor = zf;
}

QVariantMap RootLayerModel::cameraPosition() const
{
	QVariantMap vmap;
	vmap["x"] = _cameraPosition.x;
	vmap["y"] = _cameraPosition.y;
	vmap["z"] = _cameraPosition.z;

	return vmap;
}

void RootLayerModel::cameraPosition(const QVariantMap & coord)
{
	if (coord.size() == 0) {
		_cameraPosition = { 0,0,0 };
		return;
	}
	bool ok;
	if (coord.contains("x")) {
		double v = coord["x"].toDouble(&ok);
		if (ok)
			_cameraPosition.x = v;

	}
	if (coord.contains("y")) {
		double v = coord["y"].toDouble(&ok);
		if (ok)
			_cameraPosition.y = v;
	}
	if (coord.contains("z")) {
		double v = coord["z"].toDouble(&ok);
		if (ok)
			_cameraPosition.z = v;
	}

}

void Ilwis::Ui::RootLayerModel::active(bool yesno)
{
    // you can not set this state of the root, always true
}

bool RootLayerModel::active() const
{
    return true;
}

int RootLayerModel::numberOfBuffers(const QString&) const {
    return 0;
}

QVariantList Ilwis::Ui::RootLayerModel::gridAxis(const QString & type) const
{
    if (!screenCsy().isValid() || !screenGrf().isValid())
        return QVariantList();
    
    return calcAxisValues(type, _zoomEnvelope.min_corner(), _zoomEnvelope.max_corner());
}

QVariantList RootLayerModel::calcAxisValues(const QString& axisType, const Coordinate& cmin, const Coordinate& cmax) const {
    QVariantList axisValues; 
    const LayerModel *grid = findLayerByName("Primary Grid");
    if (!grid)
        return QVariantList();
    double cellDistance = grid->vproperty("celldistance").toDouble();
    if (cellDistance == rUNDEF)
        return axisValues;

    auto RoundNumber = [](double num, double prec)->QString {
        return QString("%1").arg(round(num, prec), 0, 'f', (prec != 0 ? -std::log10(prec) : 0));
    };

    auto SavePoints = [&](double a, double b, double newValue, double prec, double rest) -> double {

        auto data = axisValues.back().toMap();
        auto dist = std::abs(a - b);
        double nextrest = dist - (int)dist;
        data["size"] = (int)(dist + rest);
        axisValues.back() = data;
        data["value"] = RoundNumber(newValue, prec);
        axisValues.push_back(data);
        return nextrest;
    };

    double precision = 0;
    precision = (int)log10(std::abs(cellDistance));
    precision = precision >= 0 ? 0 : pow(10, precision);

    Coordinate center = layerManager()->rootLayer()->viewEnvelope().center();

    QVariantMap data;
    double rest = 0;
  
    if (axisType == "xaxisvaluestop") {
        data["value"] = "";//RoundNumber(cmin.x, 0.001);
        axisValues.push_back(data);
        auto pixPrev1 = screenGrf()->coord2Pixel(cmin);
        for (double x = ceil(cmin.x / cellDistance) * cellDistance; x < cmax.x; x += cellDistance)
        {
            auto p = screenGrf()->coord2Pixel(Coordinate(x, cmin.y));
            rest = SavePoints(p.x, pixPrev1.x, x, precision, rest);
            pixPrev1 = p;
        }
    }else if (axisType == "xaxisvaluesbottom"){
        rest = 0;
        data["value"] = ""; // RoundNumber(cmax.x, 0.001);
        axisValues.push_back(data); 
        rest = 0;
        auto pixPrev2 = screenGrf()->coord2Pixel(cmin);
        for (double x = ceil(cmin.x / cellDistance) * cellDistance; x < cmax.x; x += cellDistance)
        {
         auto p = screenGrf()->coord2Pixel(Coordinate(x, cmax.y));
            rest = SavePoints(p.x, pixPrev2.x, x, precision, rest);
            pixPrev2 = p;
        }
        
        
    } else if (axisType == "yaxisvaluesleft") {
        data["value"] = "";  // RoundNumber(cmin.y, 0.001);
        axisValues.push_back(data);
        auto pixPrev1 = screenGrf()->coord2Pixel(cmax);
        rest = 0;
        for (double y = ceil(cmax.y / cellDistance) * cellDistance - cellDistance; y >= cmin.y; y -= cellDistance)
        {
            auto p = screenGrf()->coord2Pixel(Coordinate(cmin.x, y));
            rest = SavePoints(p.y, pixPrev1.y, y, precision, rest);
            pixPrev1 = p;
        }
    }
    else if (axisType == "yaxisvaluesright") {

        rest = 0;
        data["value"] = ""; //RoundNumber(cmax.y, 0.001);
        axisValues.push_back(data);
        auto pixPrev2 = screenGrf()->coord2Pixel(cmax);
        for (double y = ceil(cmax.y / cellDistance) * cellDistance - cellDistance; y >= cmin.y; y -= cellDistance)
        {
            auto p = screenGrf()->coord2Pixel(Coordinate(cmax.x, y));
            rest = SavePoints(p.y, pixPrev2.y, y, precision, rest);
            pixPrev2 = p;
        }
    }
    return axisValues;
}











