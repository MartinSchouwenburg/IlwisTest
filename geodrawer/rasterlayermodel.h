#ifndef RASTERLAYERMODEL_H
#define RASTERLAYERMODEL_H
#include "ilwiscoreui_global.h"
#include "coveragedisplay\coveragelayermodel.h"
#include "location.h"
#include "geos/geom/Coordinate.h"
#include "ilwiscoordinate.h"
#include "box.h"

namespace Ilwis {
class RasterCoverage;
typedef IlwisData<RasterCoverage> IRasterCoverage;

namespace Ui {

class Quad {
public:
    Quad(const unsigned int imageOffsetX, const unsigned int imageOffsetY, const unsigned int imageSizeX, const unsigned int imageSizeY, const unsigned int zoomFactor);
    void addQuad(const Coordinate & c1, const Coordinate & c2, const Coordinate & c3, const Coordinate & c4, const double s1, const double t1, const double s2, const double t2);
    void setId(qint32 id);
    bool operator ==(const Ilwis::Ui::Quad &_quad);
    unsigned int imageOffsetX;
    unsigned int imageOffsetY;
    unsigned int imageSizeX;
    unsigned int imageSizeY;
    unsigned int zoomFactor;
    qint32 id;
	QVector<qreal> vertices;
    QVector<qreal> uvs;
	QVector<int> indices;
    bool active;
    bool dirty;
};

class TextureHeap;

class RasterLayerModel : public CoverageLayerModel
{
    Q_OBJECT
public:
	RasterLayerModel();
    RasterLayerModel(LayerManager *manager, QStandardItem *parent, const QString &name, const QString &desc, const IOOptions& options);
    ~RasterLayerModel();
    Q_PROPERTY(QVariantMap palette READ palette NOTIFY paletteChanged);
    Q_PROPERTY(QVector<qint32> removeQuads READ removeQuads)
    Q_PROPERTY(QVariantMap stretch READ stretch NOTIFY stretchChanged);

	void coverage(const ICoverage& cov) override;
    void fillAttributes() ;
	bool prepare(int);
    QString value2string(const QVariant &value, const QString &attrName) const;
    QString layerData(const Coordinate &crdIn, const QString& attrName, QVariantList &items) const;
    static LayerModel *create(LayerManager *manager, QStandardItem *parentLayer, const QString &name, const QString &desc, const IOOptions& options);
	bool usesColorData() const;
    QVariantList linkProperties() const;
    virtual bool renderReady();
    virtual void renderReady(bool yesno);
    std::vector<qint32> ccBands() const;

    Q_INVOKABLE virtual int numberOfBuffers(const QString&) const;
    Q_INVOKABLE virtual QVector<qreal> vertices(qint32 bufferIndex, const QString& ) const;
    Q_INVOKABLE virtual QVector<qreal> uvs(qint32 bufferIndex) const;
    Q_INVOKABLE virtual QVector<int> indices(qint32 bufferIndex, const QString& ) const;
    Q_INVOKABLE virtual QVariantMap texture(qint32 bufferIndex);
    Q_INVOKABLE void setQuadId(qint32 bufferIndex, qint32 id);

protected:
    void DivideImage(unsigned int imageOffsetX, unsigned int imageOffsetY, unsigned int imageSizeX, unsigned int imageSizeY);
    void init();
    QVariantMap stretch(); // for "live" RGB stretching of a Color Composite; thus not for Palette rasters (Palette rasters "live" stretch their palette)
    void refreshStretch();
    TextureHeap * textureHeap;
    std::vector<Quad> _quads;
    std::vector<qint32> _addQuads;
    QVector<qint32> _removeQuads;
    std::vector<qint32> _ccBands;
    IRasterCoverage _raster;
    NumericRange _currentStretchRange;
    int _maxTextureSize;
    int _paletteSize;
    unsigned long _width;
    unsigned long _height;
    unsigned long _imageWidth;
    unsigned long _imageHeight;
    bool _initDone;
    bool _linear;

private:
    void GenerateQuad(Coordinate & c1, Coordinate & c2, Coordinate & c3, Coordinate & c4, unsigned int imageOffsetX, unsigned int imageOffsetY, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int zoomFactor);
    QVariantMap palette();
    void refreshPalette();
    QVector<qint32> removeQuads();
    QVariantMap _palette;
    QVariantMap _stretch;
    bool _refreshPaletteAtNextCycle;
    bool _renderReady = false;

public slots:
    virtual void requestRedraw();

signals:
    void paletteChanged();
    void stretchChanged();

    NEW_LAYERMODEL(RasterLayerModel);
};

class CCRasterLayerModel : public RasterLayerModel
{
    Q_OBJECT
public:
	CCRasterLayerModel();
    CCRasterLayerModel(LayerManager *manager, QStandardItem *parent, const QString &name, const QString &desc, const IOOptions& options);

	bool prepare(int);
    void init();
    QString value2string(const QVariant &value, const QString &attrName) const;
	bool usesColorData() const;

public slots:
    virtual void requestRedraw();

    NEW_LAYERMODEL(CCRasterLayerModel);
};
}
}

#endif // RASTERLAYERMODEL_H
