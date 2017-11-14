#ifndef MAPINFORMATIONPROPERTYSETTER_H
#define MAPINFORMATIONPROPERTYSETTER_H
#include <QObject>
#include <QQmlListProperty>
#include "propertyeditors/visualpropertyeditor.h"

namespace Ilwis {
namespace Ui{
class MapInformationPropertySetter : public VisualPropertyEditor
{
    Q_OBJECT

       Q_PROPERTY(float showInfo READ showInfo WRITE setShowInfo NOTIFY showInfoChanged)
public:
    MapInformationPropertySetter(VisualAttribute *p);
    MapInformationPropertySetter();

    bool canUse(const IIlwisObject &obj, const QString &name) const;
    static VisualPropertyEditor *create(VisualAttribute *p);

    bool showInfo() const;
    void setShowInfo(bool yesno);

    NEW_PROPERTYEDITOR(MapInformationPropertySetter)

signals:
    void showInfoChanged();

};
}
}

#endif // MAPINFORMATIONPROPERTYSETTER_H
