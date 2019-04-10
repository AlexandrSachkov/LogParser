#include "CircularLineBuffer.h"

namespace PLP {
    CircularLineBuffer::CircularLineBuffer() {}

    bool CircularLineBuffer::initialize(unsigned int lineSize, unsigned int numLines) {
        if (lineSize == 0 || numLines == 0) {
            return false;
        }

        try {
            for (unsigned int i = 0; i < numLines; i++) {
                _lineBuffers.emplace_back(lineSize);
            }
            _lineNumFileOffset.resize(numLines);
        } catch (std::bad_alloc&) {
            return false;
        }

        return true;
    }

    bool CircularLineBuffer::pushBack(const char* data, unsigned int size) {
        return pushBack(0, 0, data, size);
    }

    bool CircularLineBuffer::pushBack(unsigned long long lineNum, unsigned long long fileOffset, const char* data, unsigned int size) {
        _lineNumFileOffset[_currPos] = { lineNum, fileOffset };
        if (!_lineBuffers[_currPos].set(data, size)) {
            return false;
        }

        if (_currPos == _startPos) {
            _startPos = (_startPos + 1) % _lineBuffers.size();
        }

        _currPos = (_currPos + 1) % _lineBuffers.size();
        return true;
    }

    bool CircularLineBuffer::get(unsigned int linePos, char*& data, unsigned int& size) {
        unsigned long long lineNum;
        unsigned long long fileOffset;

        return get(linePos, lineNum, fileOffset, data, size);
    }

    bool CircularLineBuffer::get(
        unsigned int linePos, 
        unsigned long long& lineNum, 
        unsigned long long& fileOffset, 
        char*& data, 
        unsigned int& size
    ) {
        if (linePos >= _lineBuffers.size()) {
            return false;
        }

        const unsigned int index = (_startPos + linePos) % _lineBuffers.size();
        lineNum = _lineNumFileOffset[index].first;
        fileOffset = _lineNumFileOffset[index].second;
        _lineBuffers[index].get(data, size);

        return true;
    }

    unsigned int CircularLineBuffer::getSize() {
        return (unsigned int)_lineBuffers.size();
    }
}