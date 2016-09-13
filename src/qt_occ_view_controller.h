#pragma once

#include <QtCore/QObject>
#include <QtCore/QPoint>
class QCursor;

namespace Mayo {

class WidgetOccView;

class QtOccViewController : public QObject
{
    Q_OBJECT

public:
    QtOccViewController(WidgetOccView* view = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setViewCursor(const QCursor& cursor);

private:
    QPoint m_prevPos;
    class WidgetOccView* m_view = nullptr;
};

} // namespace Mayo
