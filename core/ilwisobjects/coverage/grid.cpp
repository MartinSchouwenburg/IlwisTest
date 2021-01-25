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

#include "raster.h"
#include "ilwiscontext.h"
#include "connectorinterface.h"
#include "geometries.h"
#include "grid.h"

using namespace Ilwis;


GridBlockInternal::GridBlockInternal(quint32 blocknr, quint64 rasterid,quint32 lines , quint32 width) :  _undef(undef<PIXVALUETYPE>()), _size(Size<>(width, lines,1)), _id(blocknr), _rasterid(rasterid), _inMemory(false), _dataInMemory(false)
{
    _blockSize = _size.xsize()* _size.ysize();
	
}

GridBlockInternal::~GridBlockInternal()
{

}

Size<> GridBlockInternal::size() const
{
    return _size;
}

GridBlockInternal *GridBlockInternal::clone(quint64 newRasterId)
{

    GridBlockInternal *block = new GridBlockInternal(blockNr(), newRasterId, _size.xsize(), _size.ysize());
    block->init();
    std::copy(_data.begin(), _data.end(), block->_data.begin());
    block->_inMemory = true;
    block->_dataInMemory = true;
    block->save2Cache(); // as a result, block->_inMemory becomes false
    return block;
}

char *GridBlockInternal::blockAsMemory()
{
    return (char *)&_data[0];
}

void GridBlockInternal::fill(const std::vector<PIXVALUETYPE>& values)
{
    copy(values.begin(), values.end(), _data.begin());
    _dataInMemory = true;
}

quint32 GridBlockInternal::blockSize() {
    return _blockSize;
}

/**
 * @brief GridBlockInternal::save2Cache
 * Saves the content to disk-cache and unloads the gridblock from memory
 * @return true if successful
 */

bool GridBlockInternal::save2Cache() {
    if (!_inMemory) // nothing to do
        return true;
    Locker<> lock(_mutex);
    if (!_dataInMemory)
        return true; // still nothing to do
    if ( _gridblockFileName == sUNDEF) {
        QString name = QString("gridblock_%1").arg(_id);
        QDir localDir(context()->cacheLocation().toLocalFile());
        if ( !localDir.exists()) {
            localDir.mkpath(localDir.absolutePath());
        }
        _gridblockFileName = localDir.absolutePath() + "/" + name;
    }
    if ( _gridblockFile.isNull()) {
        _gridblockFile.reset(new QTemporaryFile(_gridblockFileName));
    }
    if(!_gridblockFile->open() ){
        return ERROR1(ERR_COULD_NOT_OPEN_WRITING_1,_gridblockFileName);
    }
    _gridblockFileName = _gridblockFile->fileName();
    quint64 bytesNeeded = _data.size() * sizeof(PIXVALUETYPE);
    quint64 total =_gridblockFile->write((char *)&_data[0], bytesNeeded);
    _gridblockFile->close();
    if ( total != bytesNeeded) {
        return ERROR1(ERR_COULD_NOT_OPEN_WRITING_1,_gridblockFileName);
    }
    _inMemory = false;
    _dataInMemory = false;
    _data = std::vector<PIXVALUETYPE>();
	
    return true;
}

/**
 * @brief GridBlockInternal::loadDiskDataToMemory
 * Loads the content of _data from either the cache file or the original source (in that order of preference)
 */

void GridBlockInternal::loadDiskDataToMemory()
{
    if (_gridblockFileName != sUNDEF)
        loadFromCache();
    else
        fetchFromSource();
    _dataInMemory = true;
}

quint64 GridBlockInternal::blockNr()
{
    return _id;
}

/**
 * @brief GridBlockInternal::init
 * Allocates the data array
 */

void GridBlockInternal::init() {
    if (!_inMemory) {
        Locker<> lock(_mutex);
        if (_inMemory) // may happen due to multithreading
            return;
        try{
        _data.resize(blockSize(), _undef);
        _inMemory = true;
        } catch(const std::bad_alloc& ) {
            qDebug() << "err mem ";
            throw OutOfMemoryError( TR("Couldnt allocate memory for raster"), false);
        }
    }
}

/**
 * @brief GridBlockInternal::loadFromCache
 * Loads the _data array from disk if it is available, otherwise fills it with _undef values
 * @return true if successful
 */

bool GridBlockInternal::loadFromCache() {
    if ( _gridblockFileName == sUNDEF) {
        std::fill(_data.begin(), _data.end(), _undef);
        return true; // totaly new block; never been swapped so no load needed
    }
    quint64 bytesNeeded = _data.size() * sizeof(PIXVALUETYPE);
    if(!_gridblockFile->open() ){
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_gridblockFileName);
    }
    quint64 total =_gridblockFile->read((char *)&_data[0], bytesNeeded);
    _gridblockFile->close();
    if ( total != bytesNeeded) {
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_gridblockFileName);
    }
    return true;
}

/**
 * @brief GridBlockInternal::fetchFromSource
 * Instructs the rastercoverage to "push" its data to the grid, through setBlockData() calls
 */

void GridBlockInternal::fetchFromSource()
{
    IIlwisObject obj = mastercatalog()->get(_rasterid);
    if ( obj.isValid()){
        IRasterCoverage raster = obj.as<RasterCoverage>();
        raster->getData(_id);
    }
}

/**
 * @brief GridBlockInternal::dispose
 * Cleans up the memory content, without saving it to cache; practically the opposite of init()
 */

void GridBlockInternal::dispose()
{
    _data = std::vector<PIXVALUETYPE>();
    _inMemory = false;
    _dataInMemory = false;
}

//----------------------------------------------------------------------

Grid::Grid(int maxlines) : _maxCacheBlocks(1), _memUsed(0),_blocksPerBand(0), _maxLines(maxlines) {
    //Locker lock(_mutex);
    if ( _maxLines == iUNDEF){
		_maxLines =  context()->configurationRef()("system-settings/grid-blocksize", 500);
         if ( _maxLines > 1 && (_maxLines * size().xsize() * 8 > 1e7)) {
            _maxLines = max(1, 1e7 / (size().xsize() * 8));
         }
    }
	_cache.resize(1);
    //qDebug() << "grid created:" << this;
}

quint32 Grid::blockSize(quint32 index) const {
    if ( index < _blockSizes.size() )
        return _blockSizes[index];
    return iUNDEF;
}

Grid::~Grid() {
    clear();
    context()->changeMemoryLeft(_memUsed);
   // qDebug() << "grid deleted:" << this;
}

Size<> Grid::size() const {
    return _size;
}

int Grid::maxLines() const
{
    return _maxLines;
}

Grid *Grid::clone(quint64 newRasterId, quint32 index1, quint32 index2)
{
    Locker<> lock(_mutex);
    if ( index2 < index1){
        ERROR2(ERR_INVALID_INIT_FOR_2,TR("grid limits"),TR("clone grid"));
        return 0;
    }
    quint32 start = index1 == iUNDEF ? 0 : index1;
    quint32 end = index2 == iUNDEF ? (quint32)_blocks.size() / _blocksPerBand : index2 + 1;

    Grid *grid = new Grid(_maxLines);
    grid->prepare(0,Size<>(_size.xsize(), _size.ysize(), end - start));

    quint32 startBlock = start * _blocksPerBand;
    quint32 endBlock = std::min(end * _blocksPerBand, (quint32)_blocks.size());
    for(int i=startBlock, j=0; i < endBlock; ++i, ++j) {

        if (!_blocks[i]->dataInMemory()) {
            update(i, true);
        }
        auto b = _blocks[i]->clone(newRasterId);
        grid->setBlock(j, b);
    }
    return grid;
}

void Grid::clear() {
    _size = Size<>();
    _blockSizes = std::vector<quint32>();
    int next = true;
    while(next ){
        next = false; // next will become true again af a next iteration is needed
        for(int i=0; i < _cache[0].size(); ++i){
            if (_cache[0][i]._grid == this){
                _cache[0].erase(_cache[0].begin() + i);
                next = true;
                break;
            }
        }
    }
    for(quint32 i = 0; i < _blocks.size(); ++i)
        delete _blocks[i];
    _blocks = std::vector< GridBlockInternal *>();
    _blockSizes =  std::vector<quint32>();
    _blockOffsets = std::vector<quint32>();
    _size = Size<>();
}

PIXVALUETYPE Grid::value(const Pixel &pix, int threadIndex) {
    if (pix.x <0 || pix.y < 0 || pix.x >= _size.xsize() || pix.y >= _size.ysize() )
        return PIXVALUEUNDEF;
   if ( pix.is3D() && (pix.z < 0 || pix.z >= _size.zsize()))
        return PIXVALUEUNDEF;
    quint32 yoff = (qint32)pix.y % _maxLines;
    quint32 block = pix.y / _maxLines;
    quint32 bandBlocks = _blocksPerBand * (pix.is3D() ? pix.z : 0);
	quint32 offset = yoff * _size.xsize() + pix.x;; // _offsets[yoff][pix.x];
    return value(bandBlocks + block, offset, threadIndex);
}

PIXVALUETYPE &Grid::value(quint32 block, int offset, int threadIndex)  {
	//Locker<> lock(_mutex);
    if ( _blocks[block]->dataInMemory() ) // no load needed
        return _blocks[block]->at(offset);
  //  Locker<> lock(_mutex); // slower case. must prevent other threads to messup admin
    if ( !_blocks[block]->dataInMemory())
      if(!update(block, true, threadIndex))
          throw ErrorObject(TR("Grid block is out of bounds"));
    return _blocks[block]->at(offset); // block is now in memory
}

void Grid::setValue(quint32 block, int offset, PIXVALUETYPE v ) {
	//Locker<> lock(_mutex);
    if ( _blocks[block]->dataInMemory() ) { 
        _blocks[block]->at(offset) = v;
        return;
    }
    Locker<> lock(_mutex);
    if ( !_blocks[block]->dataInMemory())
        if(!update(block, true))
            return;
    _blocks[block]->at(offset) = v;
}

quint32 Grid::blocks() const {
    return (quint32)_blocks.size();
}

quint32 Grid::blocksPerBand() const {
    return _blocksPerBand;
}

void Grid::setBlockData(quint32 block, const std::vector<PIXVALUETYPE>& data) { // this is the central function that brings in data from a raster coverage
    if ( _blocks[block]->inMemory() ) {
        _blocks[block]->fill(data);
        return;
    }
    if(!update(block, false))
        return;
    //block will be in memory now
    _blocks[block]->fill(data);
}

char *Grid::blockAsMemory(quint32 block) {
    if ( _blocks[block]->dataInMemory() ) { // no load needed
        GridBlockInternal *du = _blocks[block];
        char * p = du->blockAsMemory();
        return p;
    }
    Locker<> lock(_mutex);
    if(!update(block, true))
        return 0;
    GridBlockInternal *du = _blocks[block];
    char * p = du->blockAsMemory();
    return p;
}

void Grid::setBandProperties(RasterCoverage *raster, int n){
    _size.zsize(_size.zsize() + n);
    quint32 oldBlocks = (quint32)_blocks.size();
    quint32 newBlocks = numberOfBlocks();
    _blocks.resize(newBlocks);
    _blockSizes.resize(newBlocks);
    _blockOffsets.resize(newBlocks);
    qint32 totalLines = _size.ysize();

    for(quint32 block = oldBlocks; block < _blocks.size(); ++block) {
        int linesPerBlock = std::min((qint32)_maxLines, totalLines);
        _blocks[block] = new GridBlockInternal(block,raster ? raster->id() : i64UNDEF,linesPerBlock, _size.xsize());
        _blockSizes[block] = linesPerBlock * _size.xsize();
        _blockOffsets[block] = block == 0 ? 0 : _blockOffsets[block-1] +  _blockSizes[block];
        totalLines -= _maxLines;
        if ( totalLines <= 0) // to next band
            totalLines = _size.ysize();
    }
}

void Grid::resetBlocksPerBand(quint64 rasterid, quint32 blockCount, int maxlines) {
	Size<> keepSize = _size;
	clear();
	_maxLines = maxlines;
	_blocksPerBand = blockCount;
	prepare(rasterid, keepSize);
}

bool Grid::prepare(quint64 rasterid, const Size<> &sz) {
    Locker<> lock(_mutex);

    clear();
    _size = sz;

    if ( _size.isNull()|| !_size.isValid() || _maxLines == 0)
        return false;

    if ( _size.zsize() == 0)
        _size.zsize(1);

    //if ( _maxLines > 1 && (_maxLines * sz.xsize() * 8 > 1e7))
    //   _maxLines = max(1, 1e7 / (sz.xsize() * 8));

    quint64 bytesNeeded = _size.linearSize() * sizeof(PIXVALUETYPE);
    quint64 mleft = context()->memoryLeft();
    if ( _memUsed != 0) // reszing a grid may reuse an older grid; in this case the memory has to be correctly given back
        context()->changeMemoryLeft(_memUsed);
    _memUsed = std::min(bytesNeeded, mleft/2);
    context()->changeMemoryLeft(-_memUsed);
    int nblocks = numberOfBlocks();
    _maxCacheBlocks = std::min(_size.zsize() > 1 ? _size.zsize() * 2 : 10, (quint32)20); // allow more blocks when using maplists
    _blocksPerBand = nblocks / sz.zsize();

    qint32 totalLines = _size.ysize();
    _blocks.resize(nblocks);
    _blockSizes.resize(nblocks);
    _blockOffsets.resize(nblocks);

    for(quint32 i = 0; i < _blocks.size(); ++i) {
        int linesPerBlock = std::min((qint32)_maxLines, totalLines);
        _blocks[i] = new GridBlockInternal(i, rasterid,linesPerBlock, _size.xsize());
        _blockSizes[i] = linesPerBlock * _size.xsize();
        _blockOffsets[i] = i == 0 ? 0 : _blockOffsets[i-1] +  _blockSizes[i];
        totalLines -= _maxLines;
        if ( totalLines <= 0) // to next band
            totalLines = _size.ysize();
    }
    return true;
}

int Grid::numberOfBlocks() {
    PIXVALUETYPE rblocks = (PIXVALUETYPE)_size.ysize() / _maxLines;
    int nblocks = (int)rblocks;
    PIXVALUETYPE rest = rblocks - nblocks;
    if ( rest >= (1.0 / ( _maxLines + 1))) {
        nblocks++;
    }
    return nblocks * _size.zsize();
}

bool Grid::update(quint32 block, bool loadDiskData, int threadIndex) {
    if ( block >= _blocks.size() ) // illegal, blocknumber is outside the allowed range
        return false;
    // update the _cache[_threadIndex] array to reflect the Most-Recently-Used blocks; the first position in the array is the MRU-block, the last position is the first candidate to be eliminated
    auto iter = std::find(_cache[threadIndex].begin(), _cache[threadIndex].end(), GridBlockNrPair(this, block));
    if ( iter != _cache[threadIndex].end()){
        auto gbnp = (*iter);
        _cache[threadIndex].erase(iter);
        _cache[threadIndex].insert(_cache[threadIndex].begin(), gbnp);
        if ( !_blocks[block]->dataInMemory()){
            _blocks[block]->init();
            _blocks[block]->loadDiskDataToMemory();
        }
    } else { // block is not in memory, bring it in
        if (_cache[threadIndex].size() >= _maxCacheBlocks) { // keep list same size
            _cache[threadIndex].back()._grid->_blocks[_cache[threadIndex].back()._blocknr]->save2Cache(); // least used element is saved to disk
            _cache[threadIndex].pop_back(); // least used element is eliminated from the cache list
        }
        _blocks[block]->init(); // the data will be overwritten entirely by either loadFromCache or setBlockData
        if (loadDiskData)
            _blocks[block]->loadDiskDataToMemory();
		if (_blocks[block]->dataInMemory())
			_cache[threadIndex].insert(_cache[threadIndex].begin(), GridBlockNrPair(this, block));
    }
    return true;

}

void Grid::unloadInternal() {
    for (auto b : _blocks){
        b->save2Cache();
    }
   // qDebug() << "grid unloaded:" << this;
}

void Grid::setBlock(int index, GridBlockInternal *block)
{
    Locker<> lock(_mutex);
    if (_cache[0].size() >= _maxCacheBlocks) { // keep list same size
        _cache[0].back()._grid->_blocks[_cache[0].back()._blocknr]->save2Cache(); // least used element is saved to disk
        _cache[0].pop_back(); // least used element is eliminated from the cache list
    }
    _cache[0].insert(_cache[0].begin(), GridBlockNrPair(this, block->blockNr()));
    if ( index >= _blocks.size())
        _blocks.resize(index + 1);
    _blocks[index] = block;
}

void Grid::unload(bool uselock) {
    if ( uselock) {
        Locker<> lock(_mutex);
        unloadInternal();
    }else
        unloadInternal();
}

std::map<quint32, std::vector<quint32> > Grid::calcBlockLimits(const IOOptions& options ){
    std::map<quint32, std::vector<quint32> > result;
    int blockplayer = blocksPerBand();
    if ( options.size() == 0) {
        quint32 lastblock = 0;
        for(int layer = 0; layer < size().zsize(); ++layer) {
            for(int block = 0; block < blockplayer; ++block) {
                result[layer].push_back(lastblock + block);
            }
            lastblock += blockplayer;
        }
    } else {
        auto bindex = options["blockindex"];
        if ( bindex.isValid()) {
            quint32 index = bindex.toUInt();
            int layer = index / blockplayer;
            result[layer].push_back(index);
        }
    }

    return result;
}

bool Grid::isValid() const
{
    return !(_size.isNull() || _size.isValid());
}

qint64 Grid::memUsed() const
{
    return _memUsed;
}

PIXVALUETYPE Grid::findBigger(PIXVALUETYPE v)
{
    for(int i=0; i < _blocks.size(); ++i){
        if ( !_blocks[i]->dataInMemory() )
            update(i,true);
        for(int j=0; j < _blocks[i]->blockSize(); ++j){
            PIXVALUETYPE v2 = _blocks[i]->at(j);
            if ( v2 >= v){
                return v2;
            }
        }
    }
    return PIXVALUEUNDEF;
}

void Grid::prepare4Operation(int nThreads) {
	//entry 0 remains untouched unless nthreads is 1 in which case nothing happens; the threads relevant cache will be loaded into 1...n threads
	if (nThreads == 1)
		return;
	_cache.resize(nThreads + 1);
	for (int idx = 0; idx < _cache[0].size(); ++idx) {
		int threadIdx = _cache[0][idx]._blocknr / _blocksPerBand + 1;
		_cache[threadIdx].push_back({ _cache[0][idx]._grid, _cache[0][idx]._blocknr });
	}
	_maxCacheBlocks = std::max(1, (int)_maxCacheBlocks / nThreads);
}

void Grid::unprepare4Operation() {
	_cache.resize(1);
	_maxCacheBlocks = std::min(_size.zsize() > 1 ? _size.zsize() * 2 : 10, (quint32)20);
}



