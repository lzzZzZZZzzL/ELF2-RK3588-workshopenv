/*****************************************************************************/
/* 头文件                                                                     */
/*****************************************************************************/
#include "facedialog.h"

/*****************************************************************************/
/* 函数定义                                                                   */
/*****************************************************************************/
FaceRegisterDialog::FaceRegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("人脸注册");

    //设置字体
    QFont font;
    font.setPointSize(14);

    //ID输入行
    QLabel *idLabel = new QLabel("ID:", this);
    idLabel->setFont(font);
    idLineEdit = new QLineEdit(this);
    idLineEdit->setFont(font);

    //姓名输入行
    QLabel *nameLabel = new QLabel("姓名:", this);
    nameLabel->setFont(font);
    nameLineEdit = new QLineEdit(this);
    nameLineEdit->setFont(font);

    //按钮
    okButton = new QPushButton("确认", this);
    okButton->setFont(font);
    cancelButton = new QPushButton("取消", this);
    cancelButton->setFont(font);

    //布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(idLabel);
    mainLayout->addWidget(idLineEdit);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(nameLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);      //确认按钮
    buttonLayout->addWidget(cancelButton);  //取消按钮
    mainLayout->addLayout(buttonLayout);

    //按钮槽函数连接
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

QString FaceRegisterDialog::getID() const {
    return idLineEdit->text();
}

QString FaceRegisterDialog::getName() const {
    return nameLineEdit->text();
}

void FaceRegisterDialog::accept()
{
    //检查ID和姓名是否为空
    if(idLineEdit->text().isEmpty() || nameLineEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "输入错误", "ID和姓名不能为空！");
        return;
    }
    QDialog::accept();  //调用基类的accept()函数
}

void FaceRegisterDialog::reject()
{
    QDialog::reject();  //调用基类的reject()函数
}
