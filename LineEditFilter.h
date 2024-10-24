#pragma once

#include <QPlainTextEdit>
#include <QKeyEvent>

class LineEditFilter : public QObject {
    Q_OBJECT

protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        QPlainTextEdit* editor = qobject_cast<QPlainTextEdit*>(obj);
        if (editor) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                    emit buttonClicked();
                    return true;
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }

signals:
    void buttonClicked();
};