#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QLineEdit>

#include "Core.h"

class ScriptView : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptView(PLP::CoreI* plpCore, QWidget *parent = nullptr);
    ~ScriptView();
signals:

public slots:
    void openScript();
    void loadScript();
    void saveScript();
    void runScript();

private:
    QPushButton* _open;
    QPushButton* _load;
    QPushButton* _save;
    QPushButton* _run;
    QLineEdit* _scriptPath;
    QPlainTextEdit* _scriptEditor;
    QPlainTextEdit* _console;

    PLP::CoreI* _plpCore;
};

#endif // SCRIPTVIEW_H