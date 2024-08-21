#ifndef COLUMNFILTER_H
#define COLUMNFILTER_H
#include <QDialog>
#include "columnfilterdialog.h"

class columnFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit columnFilterDialog(QWidget* parent = nullptr);
    ~columnFilterDialog();
    Ui::columnFilterDialog* ui;
    int columnIndex;

private:
    void init();
    bool modifiedFlag = false;
    void closeEvent(QCloseEvent*) override;

private slots:
    void on_resetFilterButton_clicked();


signals:
    void filterConfig(QList<QString> filterconfig);
    void resetFilterFlag(int colunmindex);


};



#endif // COLUMNFILTER_H
