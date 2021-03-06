import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.1
import "../../../Global.js" as Global
import "../../../controls" as Controls
import "../../.." as Base

Column {
	id : histedit
    width: 100
    height: 62
    property var editor
	property var modelid : null
	spacing : 5
	property var currentPolygon : []
	objectName : uicontext.uniqueName("histogram_editor_")

	function handleMousePressed(mx,my) {
		if ( useAOI.checked && editor.editState)
			editor.addPoint(mx,my)	
		else
			editor.updateChart(mx,my)
	}

    Button{
        id : usesInfo
        x : 5
        y : 5
        text: editor.chartModelId == 10000000 ? "Open Histogram" : "Update Chart"
		width : 100
		height : 22
 

		onClicked : {
			var mid = editor.visualAttribute.layer.modelId()
			var createInfo
			var ydata = editor.isColorComposite ? 'histogram_red|histogram_green|histogram_blue' : 'histogram|-histogram_cumulative' 

			if ( editor.chartModelId == 10000000){
				if ( editor.useAOI) {
					if ( editor.aggregateAOIs){
						ydata = 'histogram_0|-histogram_cumulative_0'
					}else{
						for(var i=0; i < editor.polyCount; ++i) {
							if ( ydata != "")
								ydata += "|"
							ydata += 'histogram_' + i + '|-histogram_cumulative_' + i
						}
					}
				}
				var xc = editor.isColorComposite ? 'min_red' : 'min'
				var extras = 'resx=2|resy=2|specialtype=histogram|linkedid=' + mid;
				if ( useAOI.checked)
					extras += '|aoi=true'
				createInfo = {type : "chart", url : editor.tableUrl, ctype : 'line', name : editor.editorName , xaxis : xc, yaxis : ydata, zaxis : '', extraparameters : extras }
				modelid = objectcreator.createObject(createInfo)
				editor.chartModelId = modelid
				var filter = "itemid=" + modelid
				bigthing.newCatalog(filter, "chart", "","other")
				editor.editState = false
			}else {
				if ( editor.useAOI)
					editor.updateAOIs()
			}
		}
    }

	Column {
		x : 4
		spacing : 4
		CheckBox {
			id : useAOI
			height : 22
			width : 120
			text : qsTr("Use Area of interest")
			checked : false

			onCheckedChanged : {
				editor.useAOI = checked
			}
			CheckBox{
				id : aggrAOI
				height : 22
				width : 120
				text : qsTr("Aggregate Areas of interest")
				checked : true

				onCheckedChanged : {
					editor.aggregateAOIs = checked
				}
				anchors.left : useAOI.right
				anchors.leftMargin : 8
				visible : useAOI.checked
			}
		}
		Controls.PushButton {
			text : editor.editState ? qsTr("End current AOI") : qsTr("Add new AOI")
			width : 140
			height : 22
			visible : useAOI.checked
			checkable : true
			checked : editor.editState
			highlighted : editor.editState

			onClicked : {
				if (!editor.editState)
					editor.addEmptyPolygon()
				else {
//					editor.deleteAllAOIs()
				}
				editor.editState = !editor.editState
			}
		}

		Controls.PushButton {
			text : qsTr("Delete last AOI")
			width : 140
			height : 22
			visible : useAOI.checked

			onClicked : {
				editor.editState = false
				editor.deleteLastAOI()
			}
		}

		Controls.PushButton {
			text : qsTr("Delete all AOI's")
			width : 140
			height : 22
			visible : useAOI.checked

			onClicked : {
				editor.editState = false
				editor.deleteAllAOIs()
			}
		}
	}


}

