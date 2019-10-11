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
#include "ilwisdata.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "attributedefinition.h"
#include "raster.h"
#include "connectorinterface.h"
#include "symboltable.h"
#include "conventionalcoordinatesystem.h"
#include "ilwisoperation.h"

#include <QThread>
using namespace Ilwis;

OperationHelperRaster::OperationHelperRaster()
{
}

BoundingBox OperationHelperRaster::initialize(const IRasterCoverage &inputRaster, IRasterCoverage &outputRaster, quint64 what)
{
	Resource resource(itRASTER);
	Size<> sz = inputRaster->size();
	BoundingBox box(sz);

	if (what & itRASTERSIZE) {
		resource.addProperty("size", IVARIANT(sz.toString()), true);
	}
	if (what & itENVELOPE) {
		if (box.isNull() || !box.isValid()) {
			sz = inputRaster->size();
			box = BoundingBox(sz);
		}
		Envelope bounds = inputRaster->georeference()->pixel2Coord(box);
		resource.addProperty("envelope", IVARIANT(bounds.toString()));
	}
	if (what & itCOORDSYSTEM) {
		QUrl url = inputRaster->coordinateSystem()->resourceRef().url(true);
		QFileInfo inf(url.toLocalFile());
		if (!addCsyFromInput(inputRaster.ptr(), resource))
			return BoundingBox();

        
    }
    if ( what & itDOMAIN) {
        resource.addProperty("domain", IVARIANT(inputRaster->datadef().domain<>()->id()));
    }
    resource.prepare();

    outputRaster.prepare(resource);
    if ( what & itTABLE) {
        if ( inputRaster->attributeTable().isValid())    {
            if ( inputRaster->datadef().domain<>() == outputRaster->datadef().domain<>()) {
                if ( outputRaster.isValid())
                    outputRaster->setAttributes(inputRaster->attributeTable());
            }
        }
    }
    if ( what & itDOMAIN){
        for(quint32 i = 0; i < outputRaster->size().zsize(); ++i){
            QString index = outputRaster->stackDefinition().index(i);
            outputRaster->setBandDefinition(index,DataDefinition(outputRaster->datadef().domain()));
        }
    }
	if (what & itGEOREF) {
		outputRaster->georeference(inputRaster->georeference());
	}


    return box;
}

IIlwisObject OperationHelperRaster::initialize(const IIlwisObject &inputObject, IlwisTypes tp, quint64 what)
{
    Resource resource(tp);
    if (inputObject->ilwisType() & itCOVERAGE) {
        ICoverage cov = inputObject.as<Coverage>();
        if (inputObject->ilwisType() == itRASTER) {
            IRasterCoverage gcInput = inputObject.as<RasterCoverage>();
            if ( what & itRASTERSIZE) {
                Size<> sz = gcInput->size();
                BoundingBox box(sz);
                resource.addProperty("size", IVARIANT(box.size().toString()));
            }
            if ( what & itGEOREF) {
				addGrfFromInput(gcInput.ptr(), resource);
            }
            if ( what & itDOMAIN) {
                resource.addProperty("domain", IVARIANT(gcInput->datadef().domain<>()->id()));
            }
        }
        if ( what & itCOORDSYSTEM) {
			if (!addCsyFromInput(cov.ptr(), resource))
				return IIlwisObject();
        }

     }

    resource.prepare();
    IIlwisObject obj;
    obj.prepare(resource);
    if (inputObject->ilwisType() & itCOVERAGE) {
        OperationHelper::initialize(inputObject, obj, tp, what);
    }

    return obj;
}

bool OperationHelperRaster::addCsyFromInput(const Coverage* cov, Resource& resource) {
	QUrl url = cov->coordinateSystem()->resourceRef().url(true);
	QFileInfo inf(url.toLocalFile());
	if (inf.exists()) {
		resource.addProperty("coordinatesystem", url.toString(), true);
		return true;
	}
	CoordinateSystem::addCsyProperty(cov->coordinateSystem(), resource);
	return true;
}

bool OperationHelperRaster::addGrfFromInput(const RasterCoverage* raster, Resource& resource) {
	if (raster && raster->georeference().isValid()) {
		QUrl url = raster->georeference()->resourceRef().url(true);
		QFileInfo inf(url.toLocalFile());
		if (inf.exists()) {
			resource.addProperty("georeference", url.toString(), true);
			return true;
		}
		else {
			QString codeCsy;
			if (raster->coordinateSystem()->ilwisType() == itCONVENTIONALCOORDSYSTEM) {
				codeCsy = raster->coordinateSystem().as<ConventionalCoordinateSystem>()->toProj4();


			}
			else if (raster->coordinateSystem()->ilwisType() == itBOUNDSONLYCSY) {
				if (url.toString().indexOf("undetermined") < 0)
					codeCsy = raster->coordinateSystem()->envelope().toString();
				else
					codeCsy = "unknown";
			}
			QString grfType = raster->georeference()->grfType();
			if (grfType == "corners" || grfType == "undeterminedGeoReference") {
				QString grfs = QString("code=georef:type=corners,csy=%1,envelope=%2,gridsize=%3").arg(codeCsy).arg(raster->envelope().toString()).arg(raster->size().twod().toString());
				resource.addProperty("georeference", grfs, true);
			}
		}
	}
	return false;
}

int OperationHelperRaster::subdivideTasks(ExecutionContext *ctx,const IRasterCoverage& raster, const BoundingBox &bnds, std::vector<BoundingBox > &boxes)
{
    if ( !raster.isValid() || raster->size().isNull() || raster->size().ysize() == 0) {
        return ERROR1(ERR_NO_INITIALIZED_1, "Grid size");
        return iUNDEF;
    }

    int cores = std::min(QThread::idealThreadCount(),(int)raster->size().ysize());
    if (raster->size().linearSize() < 10000 || ctx->_threaded == false)
        cores = 1;

    boxes.clear();
    boxes.resize(cores);
    BoundingBox bounds = bnds;
    if ( bounds.isNull() || !bounds.isValid())
        bounds = BoundingBox(raster->size());
    int left = 0; //bounds.min_corner().x;
    int right = bounds.size().xsize();
    int top = bounds.size().ysize();
    int step = bounds.size().ysize() / cores;
    int currentY = 0;

    for(int i=0 ; i < cores; ++i){
        BoundingBox smallBox(Pixel(left, currentY,0), Pixel(right - 1, std::min(top - 1,currentY + step),bounds.zlength()) );
        boxes[i] = smallBox;
        currentY = currentY + step  ;
    }
    return cores;
}

IRasterCoverage OperationHelperRaster::resample(const IRasterCoverage& sourceRaster, const IGeoReference& targetGrf) {
    if (!sourceRaster.isValid() || !targetGrf.isValid())
        return IRasterCoverage();

        Resource res;
        res.prepare();
        QString expr = QString("%3=resample(%1,%2,nearestneighbour)").arg(sourceRaster->resource().url().toString()).arg(targetGrf->resource().url().toString()).arg(res.name());
        ExecutionContext ctxLocal;
        SymbolTable symtabLocal;
        if (!commandhandler()->execute(expr, &ctxLocal, symtabLocal))
            return false;
        QVariant var = symtabLocal.getValue(res.name());
        return var.value<IRasterCoverage>();
}

bool OperationHelperRaster::resample(IRasterCoverage& raster1, IRasterCoverage& raster2, ExecutionContext *ctx) {
    if ( !raster1.isValid())
        return false;

    IGeoReference commonGeoref = raster1->georeference();
    if ( ctx->_masterGeoref != sUNDEF) {
        if(!commonGeoref.prepare(ctx->_masterGeoref))
            return false;
    }
    if (raster1->georeference()!= commonGeoref ){
        Resource res;
        res.prepare();
        QString expr = QString("%3=resample(%1,%2,nearestneighbour)").arg(raster1->resource().url().toString()).arg(commonGeoref->resource().url().toString()).arg(res.name());
        ExecutionContext ctxLocal;
        SymbolTable symtabLocal;
        if(!commandhandler()->execute(expr,&ctxLocal,symtabLocal))
            return false;
        QVariant var = symtabLocal.getValue(res.name());
        raster1 = var.value<IRasterCoverage>();
    }
    if ( raster2.isValid() && raster2->georeference()!= commonGeoref ){
        Resource res;
        res.prepare();
        QString expr = QString("%3=resample(%1,%2,nearestneighbour)").arg(raster2->resource().url().toString()).arg(commonGeoref->resource().url().toString()).arg(res.name());
        ExecutionContext ctxLocal;
        SymbolTable symtabLocal;
        if(!commandhandler()->execute(expr,&ctxLocal,symtabLocal))
            return false;
        QVariant var = symtabLocal.getValue(res.name());
        IRasterCoverage outRaster = var.value<IRasterCoverage>();
        raster2.assign(outRaster);
    }
    return true;
}

