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

/*
** SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008) 
** Copyright (C) [dates of first publication] Silicon Graphics, Inc.
** All Rights Reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
** of the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
** 
** The above copyright notice including the dates of first publication and either this
** permission notice or a reference to http://oss.sgi.com/projects/FreeB/ shall be
** included in all copies or substantial portions of the Software. 
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
** INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
** PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL SILICON GRAPHICS, INC.
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
** OR OTHER DEALINGS IN THE SOFTWARE.
** 
** Except as contained in this notice, the name of Silicon Graphics, Inc. shall not
** be used in advertising or otherwise to promote the sale, use or other dealings in
** this Software without prior written authorization from Silicon Graphics, Inc.
*/
/*
** Author: Mikko Mononen, July 2009.
*/

#include <qglobal.h>

#ifndef TESSELATOR_H
#define TESSELATOR_H

#ifdef __cplusplus
extern "C" {
#endif

// See OpenGL Red Book for description of the winding rules
// http://www.glprogramming.com/red/chapter11.html
enum TessWindingRule
{
	TESS_WINDING_ODD,
	TESS_WINDING_NONZERO,
	TESS_WINDING_POSITIVE,
	TESS_WINDING_NEGATIVE,
	TESS_WINDING_ABS_GEQ_TWO,
};

// The contents of the tessGetElements() depends on element type being passed to tessTesselate().
// Tesselation result element types:
// TESS_POLYGONS
//   Each element in the element array is polygon defined as 'polySize' number of vertex indices.
//   If a polygon has than 'polySize' vertices, the remaining indices are stored as TESS_UNDEF.
//   Example, drawing a polygon:
//     const int nelems = tessGetElementCount(tess);
//     const TESSindex* elems = tessGetElements(tess);
//     for (int i = 0; i < nelems; i++) {
//         const TESSindex* poly = &elems[i * polySize];
//         glBegin(GL_POLYGON);
//         for (int j = 0; j < polySize; j++) {
//             if (poly[j] == TESS_UNDEF) break;
//             glVertex2fv(&verts[poly[j]*vertexSize]);
//         }
//         glEnd();
//     }
//
// TESS_CONNECTED_POLYGONS
//   Each element in the element array is polygon defined as 'polySize' number of vertex indices,
//   followed by 'polySize' indices to neighour polygons, that is each element is 'polySize' * 2 indices.
//   If a polygon has than 'polySize' vertices, the remaining indices are stored as TESS_UNDEF.
//   If a polygon edge is a boundary, that is, not connected to another polygon, the neighbour index is TESS_UNDEF.
//   Example, flood fill based on seed polygon:
//     const int nelems = tessGetElementCount(tess);
//     const TESSindex* elems = tessGetElements(tess);
//     unsigned char* visited = (unsigned char*)calloc(nelems);
//     TESSindex stack[50];
//     int nstack = 0;
//     stack[nstack++] = seedPoly;
//     visited[startPoly] = 1;
//     while (nstack > 0) {
//         TESSindex idx = stack[--nstack];
//			const TESSindex* poly = &elems[idx * polySize * 2];
//			const TESSindex* nei = &poly[polySize];
//          for (int i = 0; i < polySize; i++) {
//              if (poly[i] == TESS_UNDEF) break;
//              if (nei[i] != TESS_UNDEF && !visited[nei[i]])
//	                stack[nstack++] = nei[i];
//                  visited[nei[i]] = 1;
//              }
//          }
//     }
//
// TESS_BOUNDARY_CONTOURS
//   Each element in the element array is [base index, count] pair defining a range of vertices for a contour.
//   The first value is index to first vertex in contour and the second value is number of vertices in the contour.
//   Example, drawing contours:
//     const int nelems = tessGetElementCount(tess);
//     const TESSindex* elems = tessGetElements(tess);
//     for (int i = 0; i < nelems; i++) {
//         const TESSindex base = elems[i * 2];
//         const TESSindex count = elems[i * 2 + 1];
//         glBegin(GL_LINE_LOOP);
//         for (int j = 0; j < count; j++) {
//             glVertex2fv(&verts[(base+j) * vertexSize]);
//         }
//         glEnd();
//     }
//
enum TessElementType
{
	TESS_POLYGONS,
	TESS_CONNECTED_POLYGONS,
	TESS_BOUNDARY_CONTOURS,
};

typedef float TESSreal;
typedef qint32 TESSindex;
typedef struct TESStesselator TESStesselator;
typedef struct TESSalloc TESSalloc;

#define TESS_UNDEF (~(TESSindex)0)

#define TESS_NOTUSED(v) do { (void)(1 ? (void)0 : ( (void)(v) ) ); } while(0)

// Custom memory allocator interface.
// The internal memory allocator allocates mesh edges, vertices and faces
// as well as dictionary nodes and active regions in buckets and uses simple
// freelist to speed up the allocation. The bucket size should roughly match your
// expected input data. For example if you process only hundreds of vertices,
// a bucket size of 128 might be ok, where as when processing thousands of vertices
// bucket size of 1024 might be approproate. The bucket size is a compromise between
// how often to allocate memory from the system versus how much extra space the system
// should allocate. Reasonable defaults are show in commects below, they will be used if
// the bucket sizes are zero.
// 
// The use may left the memrealloc to be null. In that case, the tesselator will not try to
// dynamically grow int's internal arrays. The tesselator only needs the reallocation when it
// has found intersecting segments and needs to add new vertex. This defency can be cured by
// allocating some extra vertices beforehand. The 'extraVertices' variable allows to specify
// number of expected extra vertices.  
struct TESSalloc
{
	void *(*memalloc)( void *userData, quint32 size );
	void *(*memrealloc)( void *userData, void* ptr, quint32 size );
	void (*memfree)( void *userData, void *ptr );
	void* userData;				// User data passed to the allocator functions.
	qint32 meshEdgeBucketSize;		// 512
    qint32 meshVertexBucketSize;	// 512
    qint32 meshFaceBucketSize;		// 256
    qint32 dictNodeBucketSize;		// 512
    qint32 regionBucketSize;		// 256
    qint32 extraVertices;			// Number of extra vertices allocated for the priority queue.
};


//
// Example use:
//
//
//
//

// tessNewTess() - Creates a new tesselator.
// Use tessDeleteTess() to delete the tesselator.
// Parameters:
//   alloc - pointer to a filled TESSalloc struct or NULL to use default malloc based allocator.
// Returns:
//   new tesselator object.
TESStesselator* tessNewTess( TESSalloc* alloc );
void tessReinitialize(TESStesselator *tess);

// tessDeleteTess() - Deletes a tesselator.
// Parameters:
//   tess - pointer to tesselator object to be deleted.
void tessDeleteTess( TESStesselator *tess );

// tessAddContour() - Adds a contour to be tesselated.
// The type of the vertex coordinates is assumed to be TESSreal.
// Parameters:
//   tess - pointer to tesselator object.
//   size - number of coordinates per vertex. Must be 2 or 3.
//   pointer - pointer to the first coordinate of the first vertex in the array.
//   stride - defines offset in bytes between consecutive vertices.
//   count - number of vertices in contour.
void tessAddContour( TESStesselator *tess, qint32 size, const void* pointer, qint32 stride, qint32 count );

// tessTesselate() - tesselate contours.
// Parameters:
//   tess - pointer to tesselator object.
//   windingRule - winding rules used for tesselation, must be one of TessWindingRule.
//   elementType - defines the tesselation result element type, must be one of TessElementType.
//   polySize - defines maximum vertices per polygons if output is polygons.
//   vertexSize - defines the number of coordinates in tesselation result vertex, must be 2 or 3.
//   normal - defines the normal of the input contours, of null the normal is calculated automatically.
// Returns:
//   1 if succeed, 0 if failed.
qint32 tessTesselate( TESStesselator *tess, qint32 windingRule, qint32 elementType, qint32 polySize, qint32 vertexSize, const TESSreal* normal );

// tessGetVertexCount() - Returns number of vertices in the tesselated output.
qint32 tessGetVertexCount( TESStesselator *tess );

// tessGetVertices() - Returns pointer to first coordinate of first vertex.
const TESSreal* tessGetVertices( TESStesselator *tess );

// tessGetVertexIndices() - Returns pointer to first vertex index.
// Vertex indices can be used to map the generated vertices to the original vertices.
// Every point added using tessAddContour() will get a new index starting at 0.
// New vertices generated at the intersections of segments are assigned value TESS_UNDEF.
const TESSindex* tessGetVertexIndices( TESStesselator *tess );
	
// tessGetElementCount() - Returns number of elements in the the tesselated output.
qint32 tessGetElementCount( TESStesselator *tess );

// tessGetElements() - Returns pointer to the first element.
const TESSindex* tessGetElements( TESStesselator *tess );

#ifdef __cplusplus
};
#endif

#endif // TESSELATOR_H
