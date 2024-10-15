#pragma once

#include <QPlainTextEdit>
#include <QKeyEvent>

class LineEditFilter : public QObject {
protected:
    bool eventFilter(QObject* obj, QEvent* event) override {
        QPlainTextEdit* editor = qobject_cast<QPlainTextEdit*>(obj);
        if (editor) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                    return true; // 拦截回车键事件
                }
            }
        }
        return QObject::eventFilter(obj, event);
    }
};