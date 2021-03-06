/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "TextComparator.h"

#include <string>
#include <functional>
#include <vector>

namespace PLP {
    class FileReaderI;
    class FileWriterI;
    class IndexReaderI;
    class IndexWriterI;

    class CoreI {
    public:
        virtual ~CoreI() {}

        virtual bool initialize() = 0;
        virtual void cleanupGeneratedFilesOnRelease(bool val) = 0;
        virtual bool runScript(const std::wstring* scriptLua) = 0;
        virtual void cancelOperation() = 0;
        virtual bool isCancelled() = 0;
        virtual bool attachLogOutput(const char* name, const std::function<void(int, const char*)>* func) = 0;
        virtual void detachLogOutput(const char* name) = 0;

        virtual FileReaderI* createFileReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const std::function<void(int percent)>* progressUpdate
        ) = 0;

        virtual FileWriterI* createFileWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            bool overwriteIfExists
        ) = 0;

        virtual IndexReaderI* createIndexReader(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes
        ) = 0;

        virtual IndexWriterI* createIndexWriter(
            const std::string& path,
            unsigned long long preferredBuffSizeBytes,
            const FileReaderI* fReader,
            bool overwriteIfExists
        ) = 0;

        virtual void release(FileReaderI*) = 0;
        virtual void release(FileWriterI*) = 0;
        virtual void release(IndexReaderI*) = 0;
        virtual void release(IndexWriterI*) = 0;

        virtual bool search(
            FileReaderI* fileReader,
            IndexReaderI* indexReader,
            IndexWriterI* indexWriter,
            unsigned long long start,
            unsigned long long end, //0 for end of file, inclusive
            unsigned long long maxNumResults,
            TextComparator* comparator,
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) = 0;

        virtual bool searchMultiline(
            FileReaderI* fileReader,
            IndexReaderI* indexReader,
            IndexWriterI* indexWriter,
            unsigned long long start,
            unsigned long long end, //0 for end of file, inclusive
            unsigned long long maxNumResults,
            const std::unordered_map<int, TextComparator*>& lineComparators,
            const std::function<void(int percent, unsigned long long numResults)>* progressUpdate
        ) = 0;
    };
}