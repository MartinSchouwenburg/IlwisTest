import QtQuick 2.2
import QtGraphicalEffects 1.0
import UIContextModel 1.0
import LayerManager 1.0
import "../Global.js" as Global

MouseArea {
    id : mapArea
    width : parent.width
    height : parent.height
    hoverEnabled: true
    property LayerManager layerManager
    property bool zoomToExtents : true
    property bool showInfo : true
    property bool hasPermanence : false
    property bool zoomStarted : false
    property bool panStarted : false
    property bool zoomOutModeInWheelMode : false
    property bool panModeInWheelMode : false
    property int panPrevMouseX : -1
    property int panPrevMouseY : -1
    property string selectiondrawerColor : "basic"
    property var pStart
    property var pEnd
	property int wheelCounter : 0
    signal zoomEnded(string envelope)
	signal panChanged(string envelope, var updatebackground)
    signal setZoomPanButton(bool enablePanAndZoomOut)
    signal checkZoomNormalButton(bool enablePanAndZoomOut)
    signal checkZoomOutButton(bool checked)
    signal checkNormalButton(bool checked)
    signal checkPanButton(bool checked)
    signal click(int mx, int my)
    signal mousePressed(int mx, int my)
    signal mouseMoved(int mx, int my)
    signal mouseReleased(int mx, int my)
    signal selectTab()

    FloatingRectangle{
        id : floatrect
		text : layerManager.rootLayer.layerInfoString
    }

	  Timer {
		id : wheelTimer
        interval: 500; running: false; repeat: true
		property int prevCounter : 0
		property var lastEnvelope

        onTriggered: {
			if ( wheelCounter != 0){
				if ( wheelTimer.prevCounter - wheelCounter  == 0){
					running = false
					wheelTimer.prevCounter = 0
					wheelCounter = 0
					if ( lastEnvelope)
						zoomEnded(wheelTimer.lastEnvelope + ",true")
				}else {
					prevCounter = wheelCounter	
				}
			}
		}


    }

    Rectangle {
	    id : targetRectangle
	    x : 0
	    y : 0
	    width : 0
	    height : 0
	    visible : false
	    border.width : 1
	    border.color : "#e6f9ff"
	    color : "transparent"
	    z : 100

	    function disable() {
            if (hasPermanence)
                return
		    x = 0
		    y = 0
		    width = 0
		    height = 0
		    visible = false
	    }
	}

	Rectangle {
		anchors.fill : targetRectangle
		color : "grey"
		opacity : 0.2
		z: 101
	}

    onPressed:  {
        if ( !layerManager)
            return
        if(!changeSubPanel()) { // normal press, we can process normall else we only change the subpanel and nothing else
            zoomOutModeInWheelMode = false
            panModeInWheelMode = false

            selectTab()
            if ( layerManager.zoomInMode || layerManager.zoomOutMode){
			    if (!zoomStarted){
				    pStart = {x : mouseX, y : mouseY}
				    pEnd = pStart
				    setRect()
				    targetRectangle.visible = true
				    showInfo = false
			    }
                zoomStarted = true
            } else if ( layerManager.panningMode ){
                panStarted = true
                panPrevMouseX = mouseX
                panPrevMouseY = mouseY
                showInfo = false
                cursorShape = Qt.ClosedHandCursor

            }
            if ( showInfo && layerManager.rootLayer.showLayerInfo && !zoomStarted && !panStarted){
              floatrect.enabled = true
              floatrect.opacity = 1
              floatrect.x = mouseX
              floatrect.y = mouseY
              var mposition = mouseX + "|" + mouseY
              layerManager.setSelection(mposition);
              mousePressed(mouseX, mouseY)
            }
        }
    }

    function resetWheelPanZoomFlags() {
        zoomOutModeInWheelMode = false
        panModeInWheelMode = false
    }

    function setRectangle(envelope){
        if ( !layerManager)
            return
        var coords = envelope
        if ( typeof envelope === 'string'){
            var point = envelope.split(",")
            var pminx = parseFloat(point[0])   
            var pmaxx = parseFloat(point[2]) 
            var pminy = parseFloat(point[1]) 
            var pmaxy = parseFloat(point[3])  
            var coords = {minx : pminx, maxx : pmaxx, miny : pminy, maxy : pmaxy } 
        }                   
        var bb = layerManager.rootLayer.coord2Screen(coords)
		if ( bb && ("minx" in bb)){
			targetRectangle.x = bb.minx
			targetRectangle.width = bb.maxx - bb.minx
			targetRectangle.y = bb.miny
			targetRectangle.height = bb.maxy - bb.miny
			targetRectangle.visible = true;
		}
    }

    function setRect() {
        if(!pStart)
            return
        var x1 = pStart.x;
        var x2 = pEnd.x;
        var y1 = pStart.y;
        var y2 = pEnd.y;
        var dx = x2 - x1;
        var dy = y2 - y1;
        var rFactX = Math.abs(dx / width);
        var rFactY = Math.abs(dy / height);
        var rFact = width / height;
        if (rFactX > rFactY)
            y2 = y1 + Math.round(Math.abs(dx) * Math.sign(dy) / rFact);
        else
            x2 = x1 + Math.round(Math.abs(dy) * Math.sign(dx) * rFact);
        if (Math.abs(x1-x2) < 3) x2 = x1;
        if (Math.abs(y1-y2) < 3) y2 = y1;
        if (x1 > x2) {
            var temp = x1
            x1 = x2
            x2 = temp
        }
        if (y1 > y2) {
            var temp = y1
            y1 = y2
            y2 = temp
        }
        targetRectangle.x = x1
        targetRectangle.width = x2 - x1
        targetRectangle.y = y1
        targetRectangle.height = y2 - y1
    }

    onPositionChanged: {
        if ( !layerManager)
            return
        if (zoomStarted){
            pEnd = {x : mouseX, y : mouseY }
            setRect()
        } else if (panStarted){
            var dX = mouseX - panPrevMouseX
            var dY = mouseY - panPrevMouseY
            panPrevMouseX = mouseX
            panPrevMouseY = mouseY
            var cbZoom = layerManager.rootLayer.zoomEnvelope
            var deltax = (cbZoom.minx - cbZoom.maxx) * dX / width;
            var deltay = (cbZoom.maxy - cbZoom.miny) * dY / height;
            cbZoom.minx += deltax;
            cbZoom.maxx += deltax;
            cbZoom.miny += deltay;
            cbZoom.maxy += deltay;
            var envelope = cbZoom.minx + "," + cbZoom.miny + "," + cbZoom.maxx + "," + cbZoom.maxy
            panChanged(envelope, false)
        }
		var mposition = mouseX + "|" + mouseY
		layerManager.rootLayer.currentCoordinate = mposition
        if ( showInfo && floatrect.opacity > 0){
            floatrect.x = mouseX
            floatrect.y = mouseY
            mouseMoved(mouseX, mouseY)
        }
    }
    onReleased: {
    
        if ( !layerManager)
            return


        if (layerManager.zoomInMode ){
            pEnd = {x : mouseX , y : mouseY }
            setRect()
            if (zoomStarted) {
				
                if (targetRectangle.width < 3 && targetRectangle.height < 3) { // case of clicking on the map in zoom mode
                    var cbZoom = layerManager.rootLayer.zoomEnvelope
                    var posx = cbZoom.minx + (cbZoom.maxx - cbZoom.minx) * targetRectangle.x / width; // determine click point
                    var posy = cbZoom.maxy - (cbZoom.maxy - cbZoom.miny) * targetRectangle.y / height;
                    var w = (cbZoom.maxx - cbZoom.minx) / (2.0 * 1.41); // determine new window size
                    var h = (cbZoom.maxy - cbZoom.miny) / (2.0 * 1.41);
                    var envelope = (posx - w) + "," + (posy - h) + "," + (posx + w) + "," + (posy + h) // determine new bounds
                    zoomEnded(envelope)
                } else if (targetRectangle.width >= 3 && targetRectangle.height >= 3) {
		            var minPos = {x: targetRectangle.x  , y: targetRectangle.y , z: 0}
			        var maxPos = {x: targetRectangle.width + targetRectangle.x  , y: targetRectangle.height + targetRectangle.y, z: 0}
			        var minCoord = layerManager.rootLayer.screen2Coord(minPos)
                    var maxCoord = layerManager.rootLayer.screen2Coord(maxPos)
			        var envelope = minCoord.x + "," + minCoord.y + "," + maxCoord.x + "," + maxCoord.y
                    zoomEnded(envelope)
                } // else if targetRectangle is a horizontal or a vertical strip, do nothing
            }
			targetRectangle.disable()
            zoomStarted = false
			showInfo = true
			layerManager.refresh()
            var enablePanAndZoomOut = layerManager.rootLayer.scrollInfo.xsizeperc < 1.0 || layerManager.rootLayer.scrollInfo.ysizeperc < 1.0
            setZoomPanButton(enablePanAndZoomOut)
        } else if ( layerManager.zoomOutMode ){
            pEnd = {x : mouseX , y : mouseY}
            setRect()
            if (zoomStarted) {
                if (targetRectangle.width < 3 && targetRectangle.height < 3) { // case of clicking on the map in zoom mode
                    var cbZoom = layerManager.rootLayer.zoomEnvelope
                    var posx = cbZoom.minx + (cbZoom.maxx - cbZoom.minx) * targetRectangle.x / width; // determine click point
                    var posy = cbZoom.maxy - (cbZoom.maxy - cbZoom.miny) * targetRectangle.y / height;
                    var w = (cbZoom.maxx - cbZoom.minx) * 1.41 / 2.0; // determine new window size
                    var h = (cbZoom.maxy - cbZoom.miny) * 1.41 / 2.0;
                    var envelope = (posx - w) + "," + (posy - h) + "," + (posx + w) + "," + (posy + h) // determine new bounds
                    zoomEnded(envelope)
                } else if (targetRectangle.width >= 3 && targetRectangle.height >= 3) {
                    var top = targetRectangle.y 
                    var bottom = targetRectangle.y + targetRectangle.height
                    var left = targetRectangle.x
                    var right = targetRectangle.x + targetRectangle.width
                    top = - top;
                    bottom = height + (height - bottom);
                    left = - left;
                    right = width + (width - right);
                    var minPos = {x: left, y: top, z: 0}
                    var maxPos = {x: right, y: bottom, z: 0}
                    var minCoord = layerManager.rootLayer.screen2Coord(minPos)
                    var maxCoord = layerManager.rootLayer.screen2Coord(maxPos)
                    var envelope = minCoord.x + "," + minCoord.y + "," + maxCoord.x + "," + maxCoord.y
                    zoomEnded(envelope)
                } // else if targetRectangle is a horizontal or a vertical strip, do nothing
            }
            targetRectangle.disable()
            zoomStarted = false
            showInfo = true
            layerManager.refresh()
            var enablePanAndZoomOut = layerManager.rootLayer.scrollInfo.xsizeperc < 1.0 || layerManager.rootLayer.scrollInfo.ysizeperc < 1.0
            setZoomPanButton(enablePanAndZoomOut)
            checkZoomNormalButton(enablePanAndZoomOut)
            layerManager.zoomOutMode = enablePanAndZoomOut
        } else if ( layerManager.panningMode ){
            if (panStarted){
                var dX = mouseX - panPrevMouseX
                var dY = mouseY - panPrevMouseY
                panPrevMouseX = mouseX
                panPrevMouseY = mouseY
                var cbZoom = layerManager.rootLayer.zoomEnvelope
                var deltax = (cbZoom.minx - cbZoom.maxx) * dX / width;
                var deltay = (cbZoom.maxy - cbZoom.miny) * dY / height;
                cbZoom.minx += deltax;
                cbZoom.maxx += deltax;
                cbZoom.miny += deltay;
                cbZoom.maxy += deltay;
                var envelope = cbZoom.minx + "," + cbZoom.miny + "," + cbZoom.maxx + "," + cbZoom.maxy
                panChanged(envelope, true)
            }
            panStarted = false
            panPrevMouseX = -1
            panPrevMouseY = -1
            showInfo = true
            cursorShape = Qt.ArrowCursor
        }else{
            click(mouseX, mouseY)
            mouseReleased(mouseX, mouseY)
        }

        floatrect.enabled = false
        floatrect.opacity = 0
        floatrect.x = 0
        floatrect.y = 0
    }
    onWheel: {
        if ( layerManager){
            var envelope = layerManager.rootLayer.zoomEnvelope
            var zoomposition = {x: mouseX / width, y: 1.0 - mouseY / height};
            envelope = Global.calcZoomOutEnvelope(envelope, zoomposition, viewmanager, wheel.angleDelta.y < 0 ? 1.1 : 1.0/1.1 );
            envelope = envelope.minx + "," + envelope.miny + "," + envelope.maxx + "," + envelope.maxy
			if ( !wheelTimer.running)
				wheelTimer.running = true
			wheelTimer.lastEnvelope = envelope
			++wheelCounter
            zoomEnded(envelope + ",false");
            var enablePanAndZoomOut = layerManager.rootLayer.scrollInfo.xsizeperc < 1.0 || layerManager.rootLayer.scrollInfo.ysizeperc < 1.0
            setZoomPanButton(enablePanAndZoomOut)
            if (!enablePanAndZoomOut && (layerManager.zoomOutMode || layerManager.panningMode)) {
                checkZoomOutButton(false)
                checkPanButton(false)
                checkNormalButton(true)
                zoomOutModeInWheelMode = layerManager.zoomOutMode
                panModeInWheelMode = layerManager.panningMode
                layerManager.zoomOutMode = false
                layerManager.panningMode = false
            } else if (enablePanAndZoomOut && (zoomOutModeInWheelMode || panModeInWheelMode)) {
                layerManager.zoomOutMode = zoomOutModeInWheelMode
                layerManager.panningMode = panModeInWheelMode
                checkZoomOutButton(zoomOutModeInWheelMode)
                checkPanButton(panModeInWheelMode)
                checkNormalButton(false)
            }
        }
    }
}



