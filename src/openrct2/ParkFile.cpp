#include "Context.h"
#include "GameState.h"
#include "OpenRCT2.h"
#include "ParkImporter.h"
#include "Version.h"
#include "core/Crypt.h"
#include "drawing/Drawing.h"
#include "interface/Viewport.h"
#include "interface/Window.h"
#include "localisation/Date.h"
#include "localisation/Localisation.h"
#include "management/Award.h"
#include "management/Finance.h"
#include "management/NewsItem.h"
#include "object/Object.h"
#include "object/ObjectManager.h"
#include "object/ObjectRepository.h"
#include "ride/ShopItem.h"
#include "scenario/Scenario.h"
#include "world/Climate.h"
#include "world/Entrance.h"
#include "world/Map.h"
#include "world/Park.h"

#include <array>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>

namespace OpenRCT2
{
    constexpr uint32_t PARK_FILE_MAGIC = 0x4B524150; // PARK

    // Current version that is saved.
    constexpr uint32_t PARK_FILE_CURRENT_VERSION = 0x0;

    // The minimum version that is forwards compatible with the current version.
    constexpr uint32_t PARK_FILE_MIN_VERSION = 0x0;

    constexpr uint32_t COMPRESSION_NONE = 0;
    constexpr uint32_t COMPRESSION_GZIP = 1;

    namespace ParkFileChunkType
    {
        // clang-format off
        constexpr uint32_t RESERVED_0       = 0x00;

        constexpr uint32_t AUTHORING        = 0x01;
        constexpr uint32_t OBJECTS          = 0x02;
        constexpr uint32_t SCENARIO         = 0x03;
        constexpr uint32_t GENERAL          = 0x04;
        constexpr uint32_t CLIMATE          = 0x05;
        constexpr uint32_t PARK             = 0x06;
        constexpr uint32_t HISTORY          = 0x07;
        constexpr uint32_t RESEARCH         = 0x08;
        constexpr uint32_t NOTIFICATIONS    = 0x09;

        constexpr uint32_t INTERFACE        = 0x20;
        constexpr uint32_t EDITOR           = 0x21;

        constexpr uint32_t TILES            = 0x30;
        constexpr uint32_t THINGS           = 0x31;
        constexpr uint32_t RIDES            = 0x32;
        constexpr uint32_t BANNERS          = 0x33;
        constexpr uint32_t ANIMATIONS       = 0x34;
        constexpr uint32_t STAFF            = 0x35;
        constexpr uint32_t STRINGS          = 0x36;

        constexpr uint32_t DERIVED          = 0x50;
        // clang-format on
    }; // namespace ParkFileChunkType

    class ParkFile
    {
    public:
        std::vector<rct_object_entry> RequiredObjects;

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

    private:
        template<typename T> void WriteValue(T v)
        {
            WriteBuffer(&v, sizeof(T));
        }

        template<typename T> T ReadValue()
        {
            T v{};
            ReadBuffer(&v, sizeof(v));
            return v;
        }

    public:
        void Save(const std::string_view& path)
        {
            _header = {};
            _header.Magic = PARK_FILE_MAGIC;
            _header.TargetVersion = PARK_FILE_CURRENT_VERSION;
            _header.MinVersion = PARK_FILE_MIN_VERSION;
            _header.Compression = COMPRESSION_NONE;

            _buffer = std::stringstream(std::ios::out | std::ios::binary);

            WriteAuthoringChunk();
            WriteObjectsChunk();
            WriteScenarioChunk();
            WriteGeneralChunk();
            WriteInterfaceChunk();
            WriteClimateChunk();
            WriteTilesChunk();

            // TODO avoid copying the buffer
            auto uncompressedData = _buffer.str();

            _header.NumChunks = (uint32_t)_chunks.size();
            _header.UncompressedSize = _buffer.tellp();
            _header.Sha1 = Crypt::SHA1(uncompressedData.data(), uncompressedData.size());

            std::ofstream fs(std::string(path).c_str(), std::ios::binary);
            WriteHeader(fs);
            fs.write(uncompressedData.data(), uncompressedData.size());
        }

    private:
        void WriteHeader(std::ostream& fs)
        {
            fs.seekp(0);
            fs.write((const char*)&_header, sizeof(_header));
            for (const auto& chunk : _chunks)
            {
                fs.write((const char*)&chunk, sizeof(chunk));
            }
        }

        void BeginChunk(uint32_t id)
        {
            _currentChunk.Id = id;
            _currentChunk.Offset = _buffer.tellp();
            _currentChunk.Length = 0;
        }

        void EndChunk()
        {
            _currentChunk.Length = (uint64_t)_buffer.tellp() - _currentChunk.Offset;
            _chunks.push_back(_currentChunk);
        }

        void BeginArray()
        {
            _currentArrayCount = 0;
            _currentArrayElementSize = 0;
            _currentArrayStartPos = _buffer.tellp();
            WriteValue<uint32_t>(0);
            WriteValue<uint32_t>(0);
            _currentArrayLastPos = _buffer.tellp();
        }

        void NextArrayElement()
        {
            auto lastElSize = (size_t)_buffer.tellp() - _currentArrayLastPos;
            if (_currentArrayCount == 0)
            {
                // Set array element size based on first element size
                _currentArrayElementSize = lastElSize;
            }
            else if (_currentArrayElementSize != lastElSize)
            {
                // Array element size was different from first element so reset it
                // to dynamic
                _currentArrayElementSize = 0;
            }
            _currentArrayCount++;
            _currentArrayLastPos = _buffer.tellp();
        }

        void EndArray()
        {
            auto backupPos = _buffer.tellp();
            if ((size_t)backupPos != (size_t)_currentArrayStartPos + 8 && _currentArrayCount == 0)
            {
                throw std::runtime_error("Array data was written but no elements were added.");
            }
            _buffer.seekp(_currentArrayStartPos);
            WriteValue((uint32_t)_currentArrayCount);
            WriteValue((uint32_t)_currentArrayElementSize);
            _buffer.seekp(backupPos);
        }

        void WriteBuffer(const void* buffer, size_t len)
        {
            _buffer.write((char*)buffer, len);
        }

        void WriteString(const std::string_view& s)
        {
            char nullt = '\0';
            auto len = s.find('\0');
            if (len == std::string_view::npos)
            {
                len = s.size();
            }
            _buffer.write(s.data(), len);
            _buffer.write(&nullt, sizeof(nullt));
        }

        void WriteStringTable(const std::string_view& lcode, const std::string_view& value)
        {
            BeginArray();
            WriteString(lcode);
            WriteString(value);
            NextArrayElement();
            EndArray();
        }

        void WriteAuthoringChunk()
        {
            BeginChunk(ParkFileChunkType::AUTHORING);
            WriteString(gVersionInfoFull);
            BeginArray();
            EndArray();
            WriteString("");                    // custom notes that can be attached to the save
            WriteValue<uint64_t>(std::time(0)); // date started
            WriteValue<uint64_t>(std::time(0)); // date modified
            EndChunk();
        }

        void WriteObjectsChunk()
        {
            BeginChunk(ParkFileChunkType::OBJECTS);
            BeginArray();
            // TODO do not hard code object count
            auto& objManager = GetContext()->GetObjectManager();
            for (size_t i = 0; i < OBJECT_ENTRY_COUNT; i++)
            {
                auto obj = objManager.GetLoadedObject(i);
                if (obj != nullptr)
                {
                    auto entry = obj->GetObjectEntry();
                    auto type = (uint16_t)(entry->flags & 0x0F);
                    type |= 0x8000; // Make as legacy object
                    WriteValue<uint16_t>(type);
                    WriteString(std::string_view(entry->name, 8));
                    WriteString("");
                }
                else
                {
                    WriteValue<uint16_t>(0);
                    WriteString("");
                    WriteString("");
                }
                NextArrayElement();
            }
            EndArray();
            EndChunk();
        }

        void WriteScenarioChunk()
        {
            BeginChunk(ParkFileChunkType::SCENARIO);
            WriteValue<uint32_t>(gS6Info.category);
            WriteStringTable("en-GB", gScenarioName);
            char parkName[128];
            format_string(parkName, sizeof(parkName), gParkName, &gParkNameArgs);
            WriteStringTable("en-GB", parkName);
            WriteStringTable("en-GB", gScenarioDetails);

            WriteValue<uint32_t>(gScenarioObjectiveType);
            WriteValue<uint16_t>(gScenarioObjectiveYear);      // year
            WriteValue<uint32_t>(gScenarioObjectiveNumGuests); // guests
            WriteValue<uint16_t>(600);                         // rating
            WriteValue<uint16_t>(gScenarioObjectiveCurrency);  // excitement
            WriteValue<uint16_t>(gScenarioObjectiveNumGuests); // length
            WriteValue<money32>(gScenarioObjectiveCurrency);   // park value
            WriteValue<money32>(gScenarioObjectiveCurrency);   // ride profit
            WriteValue<money32>(gScenarioObjectiveCurrency);   // shop profit

            WriteValue(gScenarioParkRatingWarningDays);

            WriteValue<money32>(gScenarioCompletedCompanyValue);
            if (gScenarioCompletedCompanyValue == MONEY32_UNDEFINED || gScenarioCompletedCompanyValue == (money32)0x80000001)
            {
                WriteString("");
            }
            else
            {
                WriteString(gScenarioCompletedBy);
            }
            EndChunk();
        }

        void WriteGeneralChunk()
        {
            BeginChunk(ParkFileChunkType::GENERAL);
            WriteValue<uint64_t>(gScenarioTicks);
            WriteValue<uint32_t>(gDateMonthTicks);
            WriteValue(gDateMonthsElapsed);
            WriteValue(gScenarioSrand0);
            WriteValue(gScenarioSrand1);
            WriteValue(gGuestInitialCash);
            WriteValue(gGuestInitialHunger);
            WriteValue(gGuestInitialThirst);

            WriteValue(gNextGuestNumber);
            BeginArray();
            for (const auto& spawn : gPeepSpawns)
            {
                if (!gPeepSpawns->isNull())
                {
                    WriteValue(spawn.x);
                    WriteValue(spawn.y);
                    WriteValue(spawn.z);
                    WriteValue(spawn.direction);
                }
                NextArrayElement();
            }
            EndArray();

            WriteValue(gLandPrice);
            WriteValue(gConstructionRightsPrice);
            WriteValue(gGrassSceneryTileLoopPosition); // TODO (this needs to be xy32)

            EndChunk();
        }

        void WriteInterfaceChunk()
        {
            BeginChunk(ParkFileChunkType::INTERFACE);
            WriteValue(gSavedViewX);
            WriteValue(gSavedViewY);
            WriteValue(gSavedViewZoom);
            WriteValue(gSavedViewRotation);
            WriteValue<uint32_t>(gLastEntranceStyle);
            EndChunk();
        }

        void WriteClimateChunk()
        {
            BeginChunk(ParkFileChunkType::CLIMATE);
            WriteValue(gClimate);
            WriteValue(gClimateUpdateTimer);
            for (const auto* cs : { &gClimateCurrent, &gClimateNext })
            {
                WriteValue(cs->Weather);
                WriteValue(cs->Temperature);
                WriteValue(cs->WeatherEffect);
                WriteValue(cs->WeatherGloom);
                WriteValue(cs->RainLevel);
            }
            EndChunk();
        }

        void WriteParkChunk()
        {
            BeginChunk(ParkFileChunkType::PARK);
            WriteValue<uint32_t>(gParkNameArgs);
            WriteValue<money32>(gCash);
            WriteValue<money32>(gBankLoan);
            WriteValue<money32>(gMaxBankLoan);
            WriteValue<uint16_t>(gBankLoanInterestRate);
            WriteValue<uint64_t>(gParkFlags);
            WriteValue<money32>(gParkEntranceFee);
            WriteValue(gStaffHandymanColour);
            WriteValue(gStaffMechanicColour);
            WriteValue(gStaffSecurityColour);

            WriteValue(gSamePriceThroughoutParkA);
            WriteValue(gSamePriceThroughoutParkB);

            // Marketing
            BeginArray();
            for (size_t i = 0; i < std::size(gMarketingCampaignDaysLeft); i++)
            {
                WriteValue<uint32_t>(gMarketingCampaignDaysLeft[i]);
                WriteValue<uint32_t>(gMarketingCampaignRideIndex[i]);
                NextArrayElement();
            }
            EndArray();

            // Awards
            BeginArray();
            for (const auto& award : gCurrentAwards)
            {
                WriteValue(award.Time);
                WriteValue(award.Type);
                NextArrayElement();
            }
            EndArray();

            BeginArray();
            for (const auto& t : gPeepWarningThrottle)
            {
                WriteValue(t);
                NextArrayElement();
            }
            EndArray();
            WriteValue(gParkRatingCasualtyPenalty);
            WriteValue(gCurrentExpenditure);
            WriteValue(gCurrentProfit);
            WriteValue(gTotalAdmissions);
            WriteValue(gTotalIncomeFromAdmissions);
        }

        void WriteResearch()
        {
            BeginChunk(ParkFileChunkType::RESEARCH);

            // Research status
            WriteValue(gResearchFundingLevel);
            WriteValue(gResearchPriorities);
            WriteValue(gResearchProgressStage);
            WriteValue(gResearchProgress);
            WriteValue(gResearchExpectedMonth);
            WriteValue(gResearchExpectedDay);
            WriteValue(gResearchLastItem);
            WriteValue(gResearchNextItem);

            // Research order
            BeginArray();
            // type (uint8_t)
            // flags (uint8_t)
            // entry (uint32_t)
            EndArray();

            EndChunk();
        }

        void WriteNotifications()
        {
            BeginChunk(ParkFileChunkType::NOTIFICATIONS);
            BeginArray();
            for (const auto& notification : gNewsItems)
            {
                WriteValue(notification.Type);
                WriteValue(notification.Flags);
                WriteValue(notification.Assoc);
                WriteValue(notification.Ticks);
                WriteValue(notification.MonthYear);
                WriteValue(notification.Day);
                WriteString(notification.Text);
                NextArrayElement();
            }
            EndArray();
            EndChunk();
        }

        void WriteDerivedChunk()
        {
            BeginChunk(ParkFileChunkType::DERIVED);
            WriteValue<uint32_t>(gParkSize);
            WriteValue<uint32_t>(gNumGuestsInPark);
            WriteValue<uint32_t>(gNumGuestsHeadingForPark);
            WriteValue<uint32_t>(gCompanyValue);
            WriteValue<uint32_t>(gParkValue);
            WriteValue<uint32_t>(gParkRating);
            EndChunk();
        }

        void WriteTilesChunk()
        {
            BeginChunk(ParkFileChunkType::TILES);
            WriteValue<uint32_t>(gMapSize);
            WriteValue<uint32_t>(gMapSize);
            BeginArray();
            auto numTiles = std::size(gTileElements);
            for (size_t i = 0; i < numTiles; i++)
            {
                WriteBuffer(&gTileElements[i], sizeof(gTileElements[i]));
                NextArrayElement();
            }
            EndArray();
            EndChunk();
        }

    public:
        void Load(const std::string_view& path)
        {
            std::ifstream fs(std::string(path).c_str(), std::ios::binary);

            _header = ReadHeader(fs);

            _chunks.clear();
            for (uint32_t i = 0; i < _header.NumChunks; i++)
            {
                ChunkEntry entry;
                fs.read((char*)&entry, sizeof(entry));
                _chunks.push_back(entry);
            }

            _buffer = std::stringstream(std::ios::in | std::ios::out | std::ios::binary);
            _buffer.clear();

            char temp[2048];
            size_t read = 0;
            do
            {
                fs.read(temp, sizeof(temp));
                read = fs.gcount();
                _buffer.write(temp, read);
            } while (read != 0);

            RequiredObjects.clear();
            if (SeekChunk(ParkFileChunkType::OBJECTS))
            {
                auto len = ReadArray();
                for (size_t i = 0; i < len; i++)
                {
                    auto type = ReadValue<uint16_t>();
                    auto id = ReadString();
                    auto version = ReadString();

                    rct_object_entry entry{};
                    entry.flags = type & 0x7FFF;
                    strncpy(entry.name, id.c_str(), 8);
                    RequiredObjects.push_back(entry);

                    ReadNextArrayElement();
                }
            }
        }

        void Import()
        {
            ReadTilesChunk();
            ReadScenarioChunk();
            ReadGeneralChunk();
            ReadParkChunk();
            ReadClimateChunk();
            ReadResearchChunk();
            ReadInterfaceChunk();

            // Initial cash will eventually be removed
            gInitialCash = gCash;
        }

    private:
        Header ReadHeader(std::istream& fs)
        {
            Header header;
            fs.read((char*)&header, sizeof(header));
            return header;
        }

        bool SeekChunk(uint32_t id)
        {
            auto result = std::find_if(_chunks.begin(), _chunks.end(), [id](const ChunkEntry& e) { return e.Id == id; });
            if (result != _chunks.end())
            {
                auto offset = result->Offset;
                _buffer.seekg(offset);
                return true;
            }
            return false;
        }

        size_t ReadArray()
        {
            _currentArrayCount = ReadValue<uint32_t>();
            _currentArrayElementSize = ReadValue<uint32_t>();
            _currentArrayLastPos = _buffer.tellg();
            return _currentArrayCount;
        }

        bool ReadNextArrayElement()
        {
            if (_currentArrayCount == 0)
            {
                return false;
            }
            if (_currentArrayElementSize != 0)
            {
                _buffer.seekg((size_t)_currentArrayLastPos + _currentArrayElementSize);
            }
            _currentArrayCount--;
            return _currentArrayCount == 0;
        }

        void ReadBuffer(void* dst, size_t len)
        {
            _buffer.read((char*)dst, len);
        }

        std::string ReadString()
        {
            std::string buffer;
            buffer.reserve(64);
            char c;
            while ((c = ReadValue<char>()) != 0)
            {
                buffer.push_back(c);
            }
            buffer.shrink_to_fit();
            return buffer;
        }

        std::string ReadStringTable()
        {
            std::string result;
            auto len = ReadArray();
            for (size_t i = 0; i < len; i++)
            {
                auto lcode = ReadString();
                auto value = ReadString();
                if (i == 0)
                {
                    result = value;
                }
            }
            return result;
        }

        void ReadScenarioChunk()
        {
            if (SeekChunk(ParkFileChunkType::SCENARIO))
            {
                ReadValue<uint32_t>();

                String::Set(gScenarioName, sizeof(gScenarioName), ReadStringTable().c_str());
                String::Set(gS6Info.name, sizeof(gS6Info.name), gScenarioName);
                ReadStringTable(); // park name
                String::Set(gScenarioDetails, sizeof(gScenarioDetails), ReadStringTable().c_str());
                String::Set(gS6Info.details, sizeof(gS6Info.details), gScenarioName);

                gScenarioObjectiveType = ReadValue<uint32_t>();
                gScenarioObjectiveYear = ReadValue<uint16_t>();      // year
                gScenarioObjectiveNumGuests = ReadValue<uint32_t>(); // guests
                ReadValue<uint16_t>();                               // rating
                ReadValue<uint16_t>();                               // excitement
                ReadValue<uint16_t>();                               // length
                gScenarioObjectiveCurrency = ReadValue<money32>();   // park value
                ReadValue<money32>();                                // ride profit
                ReadValue<money32>();                                // shop profit

                gScenarioParkRatingWarningDays = ReadValue<uint16_t>();

                gScenarioCompletedCompanyValue = ReadValue<money32>();
                String::Set(gScenarioCompletedBy, sizeof(gScenarioCompletedBy), ReadString().c_str());
                EndChunk();
            }
            else
            {
                throw std::runtime_error("No scenario chunk found.");
            }
        }

        void ReadGeneralChunk()
        {
            if (SeekChunk(ParkFileChunkType::GENERAL))
            {
                gScenarioTicks = ReadValue<uint64_t>();
                gDateMonthTicks = ReadValue<uint32_t>();
                gDateMonthsElapsed = ReadValue<uint16_t>();
                gScenarioSrand0 = ReadValue<uint32_t>();
                gScenarioSrand1 = ReadValue<uint32_t>();
                gGuestInitialCash = ReadValue<money16>();
                gGuestInitialHunger = ReadValue<uint8_t>();
                gGuestInitialThirst = ReadValue<uint8_t>();

                gNextGuestNumber = ReadValue<uint32_t>();
                size_t numPeepSpawns = ReadArray();
                for (size_t i = 0; i < numPeepSpawns; i++)
                {
                    gPeepSpawns[i].x = ReadValue<uint32_t>();
                    gPeepSpawns[i].y = ReadValue<uint32_t>();
                    gPeepSpawns[i].z = ReadValue<uint32_t>();
                    gPeepSpawns[i].direction = ReadValue<uint8_t>();
                    ReadNextArrayElement();
                }

                gLandPrice = ReadValue<money16>();
                gConstructionRightsPrice = ReadValue<money16>();
                gGrassSceneryTileLoopPosition = ReadValue<uint16_t>();
            }
            else
            {
                throw std::runtime_error("No general chunk found.");
            }
        }

        void ReadInterfaceChunk()
        {
            if (SeekChunk(ParkFileChunkType::INTERFACE))
            {
                gSavedViewX = ReadValue<uint16_t>();
                gSavedViewY = ReadValue<uint16_t>();
                gSavedViewZoom = ReadValue<uint8_t>();
                gSavedViewRotation = ReadValue<uint8_t>();
                gLastEntranceStyle = ReadValue<uint32_t>();
            }
        }

        void ReadClimateChunk()
        {
            if (SeekChunk(ParkFileChunkType::CLIMATE))
            {
                gClimate = ReadValue<uint8_t>();
                gClimateUpdateTimer = ReadValue<uint16_t>();
                for (auto cs : { &gClimateCurrent, &gClimateNext })
                {
                    cs->Weather = ReadValue<uint8_t>();
                    cs->Temperature = ReadValue<int8_t>();
                    cs->WeatherEffect = ReadValue<uint8_t>();
                    cs->WeatherGloom = ReadValue<uint8_t>();
                    cs->RainLevel = ReadValue<uint8_t>();
                }
            }
        }

        void ReadParkChunk()
        {
        }

        void ReadResearchChunk()
        {
        }

        void ReadTilesChunk()
        {
            if (SeekChunk(ParkFileChunkType::TILES))
            {
                auto mapWidth = ReadValue<uint32_t>();
                [[maybe_unused]] auto mapHeight = ReadValue<uint32_t>();

                OpenRCT2::GetContext()->GetGameState()->InitAll(mapWidth);

                auto numElements = ReadArray();
                ReadBuffer(gTileElements, numElements * sizeof(TileElement));

                map_update_tile_pointers();
                UpdateParkEntranceLocations();
            }
            else
            {
                throw std::runtime_error("No tiles chunk found.");
            }
        }
    };
} // namespace OpenRCT2

enum : uint32_t
{
    S6_SAVE_FLAG_EXPORT = 1 << 0,
    S6_SAVE_FLAG_SCENARIO = 1 << 1,
    S6_SAVE_FLAG_AUTOMATIC = 1u << 31,
};

int32_t scenario_save(const utf8* path, int32_t flags)
{
    if (flags & S6_SAVE_FLAG_SCENARIO)
    {
        log_verbose("saving scenario");
    }
    else
    {
        log_verbose("saving game");
    }

    if (!(flags & S6_SAVE_FLAG_AUTOMATIC))
    {
        window_close_construction_windows();
    }

    map_reorganise_elements();
    viewport_set_saved_view();

    bool result = false;
    auto parkFile = std::make_unique<OpenRCT2::ParkFile>();
    try
    {
        // if (flags & S6_SAVE_FLAG_EXPORT)
        // {
        //     auto& objManager = OpenRCT2::GetContext()->GetObjectManager();
        //     s6exporter->ExportObjectsList = objManager.GetPackableObjects();
        // }
        // s6exporter->RemoveTracklessRides = true;
        // s6exporter->Export();
        if (flags & S6_SAVE_FLAG_SCENARIO)
        {
            // s6exporter->SaveScenario(path);
        }
        else
        {
            // s6exporter->SaveGame(path);
        }
        parkFile->Save(path);
        result = true;
    }
    catch (const std::exception&)
    {
    }

    gfx_invalidate_screen();

    if (result && !(flags & S6_SAVE_FLAG_AUTOMATIC))
    {
        gScreenAge = 0;
    }
    return result;
}

class ParkFileImporter : public IParkImporter
{
private:
    const IObjectRepository& _objectRepository;
    std::unique_ptr<OpenRCT2::ParkFile> _parkFile;

public:
    ParkFileImporter(IObjectRepository& objectRepository)
        : _objectRepository(objectRepository)
    {
    }

    ParkLoadResult Load(const utf8* path) override
    {
        _parkFile = std::make_unique<OpenRCT2::ParkFile>();
        _parkFile->Load(path);
        return ParkLoadResult(std::move(_parkFile->RequiredObjects));
    }

    ParkLoadResult LoadSavedGame(const utf8* path, bool skipObjectCheck = false) override
    {
        return Load(path);
    }

    ParkLoadResult LoadScenario(const utf8* path, bool skipObjectCheck = false) override
    {
        return Load(path);
    }

    ParkLoadResult LoadFromStream(
        IStream* stream, bool isScenario, bool skipObjectCheck = false, const utf8* path = String::Empty) override
    {
        return Load(path);
    }

    void Import() override
    {
        _parkFile->Import();
    }

    bool GetDetails(scenario_index_entry* dst) override
    {
        return false;
    }
};

std::unique_ptr<IParkImporter> ParkImporter::CreateParkFile(IObjectRepository& objectRepository)
{
    return std::make_unique<ParkFileImporter>(objectRepository);
}
