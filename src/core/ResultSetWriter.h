#pragma once

#include "ResultSetWriterI.h"

#include <string>
#include <memory>
#include <sstream>

namespace PLP {
    class PagedWriter;
    class TaskRunner;

    class ResultSetWriter : public ResultSetWriterI {
    public:
        ResultSetWriter();
        ~ResultSetWriter();

        bool initialize(
            std::wstring& path,
            std::wstring& dataFilePath,
            unsigned long long preferredBufferSizeBytes,
            bool overwriteIfExists,
            TaskRunner& asyncTaskRunner
        );

        //C++ interface
        bool appendCurrLine(const FileReaderI* fReader) override;

        //Lua interface
        bool appendCurrLine(std::shared_ptr<FileReaderI> fReader);

        //Shared interface
        unsigned long long getNumResults() const override;
        void release() override;

    private:
        bool flush();
        bool updateResultCount();

        std::unique_ptr<PagedWriter> _writer = nullptr;
        std::wstring _path;
        std::string _dataFilePath;
        
        unsigned long long _prevLineNum = 0;
        unsigned long long _resultCount = 0;
    };
}