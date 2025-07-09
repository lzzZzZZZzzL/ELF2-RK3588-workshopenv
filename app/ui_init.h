#ifndef UI_INIT_H
#define UI_INIT_H

/*****************************************************************************/
/* 头文件                                                                     */
/*****************************************************************************/
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QFrame>
#include <QFormLayout>
#include <QTimer>

/*****************************************************************************/
/* 声明                                                                      */
/*****************************************************************************/
class Widget;

class UIInit
{
public:
    static void initUI(Widget *widget);
};

#endif
