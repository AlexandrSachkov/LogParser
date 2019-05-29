#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "pagedfileviewwidget.h"
#include "ullspinbox.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>

#include "coreobjptr.h"
#include "FileReaderI.h"
#include "IndexReaderI.h"

#include <memory>

class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(CoreObjPtr<PLP::FileReaderI> fileReader, QWidget *parent = nullptr);
    ~FileView();

    const QString& getFilePath();
    void openIndex(CoreObjPtr<PLP::IndexReaderI> indexReader);
signals:

private slots:
    void closeTab(int index);

private:
    PagedFileViewWidget* _dataView;
    QTabWidget* _indexViewer;
    ULLSpinBox* _currLineNumBox;

    QString _filePath;
};

#endif // FILEVIEW_H
