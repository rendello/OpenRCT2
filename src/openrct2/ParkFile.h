#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>
#include "object/Object.h"

namespace OpenRCT2
{
    class ParkFile
    {
    public:
        std::vector<rct_object_entry> RequiredObjects;

        void Load(const std::string_view& path);
        void Save(const std::string_view& path);
        void Import();

    private:
#pragma pack(push, 1)
        struct Header
        {
            uint32_t Magic{};
            uint32_t TargetVersion{};
            uint32_t MinVersion{};
            uint32_t NumChunks{};
            uint64_t UncompressedSize{};
            uint32_t Compression{};
            std::array<uint8_t, 20> Sha1{};
        };

        struct ChunkEntry
        {
            uint32_t Id{};
            uint64_t Offset{};
            uint64_t Length{};
        };
#pragma pack(pop)

        Header _header;
        std::vector<ChunkEntry> _chunks;
        std::stringstream _buffer;
        ChunkEntry _currentChunk;
        std::streampos _currentArrayStartPos;
        std::streampos _currentArrayLastPos;
        size_t _currentArrayCount;
        size_t _currentArrayElementSize;

        void WriteHeader(std::ostream& fs);
        void BeginChunk(uint32_t id);
        void EndChunk();

        void WriteBuffer(const void* buffer, size_t len);
        void WriteString(const std::string_view& s);
        void BeginArray();
        void NextArrayElement();
        void EndArray();

        template<typename T>
        void WriteValue(T v)
        {
            WriteBuffer(&v, sizeof(T));
        }

        void WriteAuthoringChunk();
        void WriteObjectsChunk();
        void WriteGeneralChunk();
        void WriteTilesChunk();

        Header ReadHeader(std::istream& fs);
        bool SeekChunk(uint32_t id);
        size_t ReadArray();
        bool ReadNextArrayElement();
        void ReadBuffer(void* dst, size_t len);
        std::string ReadString();

        template<typename T>
        T ReadValue()
        {
            T v{};
            ReadBuffer(&v, sizeof(v));
            return v;
        }
    };
} // namespace OpenRCT2
