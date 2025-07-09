#ifndef Facedialog_H
#define Facedialog_H

/*****************************************************************************/
/* 头文件                                                                   */
/*****************************************************************************/
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QObject>
#include <QMessageBox>

/*****************************************************************************/
/* 声明                                                                       */
/*****************************************************************************/
class FaceRegisterDialog : public QDialog
{
    Q_OBJECT

public:
    FaceRegisterDialog(QWidget *parent = nullptr);
    QString getID() const;
    QString getName() const;

private slots:
    void accept() override;
    void reject() override;

private:
    QLineEdit *idLineEdit;
    QLineEdit *nameLineEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif
