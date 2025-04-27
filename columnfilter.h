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
    
    //dgm
    int columnIndex_myy[30];
    int column_counter_myy = 0;
    QList<QList<QString>>  twoSelectedItems;
    QString colName;
    int selectedItemscount = 0;
    int columnind;
    QVector<QVector<bool>> m_selectedStates;
    QHash<QString, QList<QVariant>> new_checks;
    int header_count = 0;
    //dgm
    void on_confirmFilterButton_clicked();
    
    int resetbuttonflag = 0;
    int confirmbuttonflag = 0;
    int comfirm_button_flag[30] = { 0 };
    int reset_column_flag[30] = { 0 };
    int columnButton;

private:
    void init();
    bool modifiedFlag = false;
    void closeEvent(QCloseEvent*) override;
    bool connected = false;

private slots:
    void on_resetFilterButton_clicked();


signals:
    void filterConfig(QList<QString> filterconfig);
    void resetFilterFlag(int colunmindex);
    void filter_apply(QList<QList<QString>> items, int count);


};



#endif // COLUMNFILTER_H
