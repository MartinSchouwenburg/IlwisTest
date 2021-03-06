import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.1
import UIContextModel 1.0
import "../../controls" as Controls
import "../../Global.js" as Global
import "../.." as Base

Controls.DropableItem {
    id : dropContainer
    width : 155
    maxHeight: 100
    property var panelCallBack
    Rectangle {

        anchors.fill: parent
        color : "white"
        border.width: 1
        border.color: Global.edgecolor
        radius: 2
        ExclusiveGroup { id: layoutTypes }
        Grid {
            id : layouts
            columns : 4
            spacing : 2
            y : 4
            x : 4
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; checked : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels301.png"; callBack : panelCallBack;type :"1"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels30h2.png"; callBack : panelCallBack;type:"2 horizontal"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels30h3.png"; callBack : panelCallBack;type:"3 horizontal"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels30v2.png"; callBack : panelCallBack;type:"2 vertical"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels30v3.png"; callBack : panelCallBack;type:"3 vertical"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels3021.png"; callBack : panelCallBack;type:"3: 2 by 1"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels3021v.png"; callBack : panelCallBack;type:"3: 2 by 1 v"}
            Controls.ToolButton { exclusiveGroup : layoutTypes; checkable : true; width : 35; height : 35; iconH : 30; iconW : 30; iconSource : "../../images/mapanels304.png"; callBack : panelCallBack;type:"4"}
        }
         CheckBox {
             width : 150
             height : 25
             text : "Panels are linked"
             checked : true
             anchors.top : layouts.bottom
             anchors.topMargin : 4
             x : 4

             onCheckedChanged : {
                 if (!checked ){
                     for(var i=0; i < layersview.layermanagers.length; ++i){
                         var layerm1 = layersview.layermanagers[i]
                         for(var j=0; j < layersview.layermanagers.length; ++j){
                            var layerm2 = layersview.layermanagers[j]
                            if ( layerm1.modelId() != layerm2.modelId()){
                                layerm1.unLink(layerm2, "zoomextent")
                                layerm2.unlink(layerm1,"zoomextent")
                            }
                            
                         }
                     }
                }
             }
         }
     }

}
