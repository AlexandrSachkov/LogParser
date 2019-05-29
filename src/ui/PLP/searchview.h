#ifndef STANDARDSEARCHVIEW_H
#define STANDARDSEARCHVIEW_H

#include <QWidget>
#include <QProgressDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

#include "ullspinbox.h"
#include "CoreI.h"

class SearchView : public QWidget
{
    Q_OBJECT
public:
    explicit SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent = nullptr);

    void createSourceContent(QLayout* mainLayout);
    void createDestinationContent(QLayout* mainLayout);
    void createSearchLimiterContent(QLayout* mainLayout);
    void createSearchOptionContent(QLayout* mainLayout);
    void createMultilineSearchOptionContent(QLayout* mainLayout);
signals:
    void progressUpdate(int percent, unsigned long long numResults);
    void searchCompleted(bool success);
private slots:
    void openFile();
    void openIndex();
    void openDestinationDir();
    void startSearch();
    void onProgressUpdate(int percent, unsigned long long numResults);
    void onSearchCancelled();
    void onSearchCompletion(bool success);
private:
    void startRegularSearch(
            PLP::FileReaderI* fileReader,
            PLP::IndexReaderI* indexReader,
            PLP::IndexWriterI* indexWriter,
            unsigned long long startLine,
            unsigned long long endLine,
            unsigned long long maxNumResults
            );

    void startMultilineSearch(
            PLP::FileReaderI* fileReader,
            PLP::IndexReaderI* indexReader,
            PLP::IndexWriterI* indexWriter,
            unsigned long long startLine,
            unsigned long long endLine,
            unsigned long long maxNumResults
            );

    const int ROW_LABEL_WIDTH = 300;
    const int NUM_ROWS = 5;

    PLP::CoreI* _plpCore;
    QProgressDialog* _progressDialog = nullptr;
    bool _multiline = false;

    QLineEdit* _filePath;
    QLineEdit* _indexPath;
    QLineEdit* _destDir;
    QLineEdit* _destName;
    ULLSpinBox* _fromLineBox;
    ULLSpinBox* _toLineBox;
    ULLSpinBox* _numResultsBox;
    QLineEdit* _searchField;
    QCheckBox* _regex;
    QCheckBox* _ignoreCase;

    std::vector<QCheckBox*> _lineEnabledCheckBoxes;
    std::vector<QSpinBox*> _lineOffsetBoxes;
    std::vector<QLineEdit*> _searchPatternBoxes;
    std::vector<QCheckBox*> _regexCheckBoxes;
    std::vector<QCheckBox*> _ignoreCaseCheckBoxes;
};

#endif // STANDARDSEARCHVIEW_H
