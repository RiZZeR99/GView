#pragma once

#include "Internal.hpp"

#include <unordered_map>
#include <deque>
#include <list>

namespace GView
{
namespace View
{
    namespace DissasmViewer
    {
        using namespace AppCUI;

        struct Config
        {
            struct
            {
                ColorPair Normal;
                ColorPair Highlight;
                ColorPair Inactive;
                ColorPair Cursor;
                ColorPair Line;
                ColorPair Selection;
                ColorPair OutsideZone;
                ColorPair StructureColor;
                ColorPair DataTypeColor;
            } Colors;
            struct
            {
                AppCUI::Input::Key AddNewType;
            } Keys;
            bool Loaded;

            static void Update(IniSection sect);
            void Initialize();
        };

        struct DissasemblyZone
        {
            uint64 offset;
            uint64 size;
            DissamblyLanguage language;
        };

        struct ParseZone
        {
            uint32 startLineIndex;
            uint32 endingLineIndex;
            uint32 extendedSize;
            uint32 textLinesOffset;
            bool isCollapsed;
        };

        enum class InternalDissasmType : uint8
        {
            UInt8,
            UInt16,
            UInt32,
            UInt64,
            Int8,
            Int16,
            Int32,
            Int64,
            AsciiZ,
            Utf16Z,
            Utf32Z,
            UnidimnsionalArray,
            BidimensionalArray,
            UserDefined
        };

        struct DissasmType
        {
            InternalDissasmType primaryType;
            std::string_view name;

            uint32 secondaryType;
            uint32 width;
            uint32 height;

            std::vector<DissasmType> internalTypes;
            uint32 GetExpandedSize() const;
        };

        struct SettingsData
        {
            DissamblyLanguage defaultLanguage;
            std::vector<DissasemblyZone> zones;
            std::deque<char*> buffersToDelete;
            uint32 availableID;

            std::unordered_map<uint64, string_view> memoryMappings; // memmory locations to functions
            std::vector<uint64> offsetsToSearch;
            std::vector<ParseZone> parseZones;
            std::unordered_map<uint64, bool> collapsed;
            std::map<uint64, DissasmType> dissasmTypeMapped;          // mapped types against the offset of the file
            std::unordered_map<TypeID, DissasmType> userDeginedTypes; // user defined typess
            SettingsData();
        };

        class Instance : public View::ViewControl
        {
            struct DrawLineInfo
            {
                uint64 textFileOffset;
                uint64 viewOffset;
                uint32 lineOffset;
                uint32 textSize;
                const uint8* start;
                const uint8* end;
                Character* chNameAndSize;
                Character* chText;
                bool recomputeOffsets;
                bool shouldSearchMapping;
                const DissasmType* dissasmType;
                bool insideStructure;

                uint32 currentLineFromOffset;
                uint32 lineToDraw;
                DrawLineInfo()
                    : recomputeOffsets(true), shouldSearchMapping(true), insideStructure(false), currentLineFromOffset(0), lineToDraw(0)
                {
                }
            };

            enum class MouseLocation : uint8
            {
                OnView,
                OnHeader,
                Outside
            };
            struct MousePositionInfo
            {
                MouseLocation location;
                uint64 bufferOffset;
            };

            struct
            {
                uint64 startView, currentPos;
                uint32 base;
            } Cursor;

            struct
            {
                ColorPair Normal, Line, Highlighted;
            } CursorColors;

            struct
            {
                uint32 visibleRows;
                uint32 charactersPerLine;
                uint32 startingTextLineOffset;

                uint32 structureLinesDisplayed;
                uint32 charactersToDelay;
            } Layout;

            struct ButtonsData
            {
                int x;
                int y;
                SpecialChars c;
                ColorPair color;
                uint64 offsetStructure;
            };

            struct
            {
                uint8 buffer[1024];
                uint32 length = 1024;
                std::list<std::reference_wrapper<const DissasmType>> types;
                std::list<int32> levels;
                uint64 offset;
                uint64 initialOffset;
                bool isCollapsed;

                uint32 currentLineToDraw;
                uint32 skipLines;

                std::vector<ButtonsData> buttons;
            } MyLine;

            FixSizeString<16> name;

            Reference<GView::Object> obj;
            Pointer<SettingsData> settings;
            static Config config;
            CharacterBuffer chars;
            Utils::Selection selection;

            void RecomputeDissasmLayout();
            void WriteLineToChars(DrawLineInfo& dli);
            bool PrepareDrawLineInfo(DrawLineInfo& dli);
            bool StructureViewToLines(DrawLineInfo& dli);
            void RegisterStructureCollapseButton(DrawLineInfo& dli, SpecialChars c);

            void AddStringToChars(DrawLineInfo& dli, ColorPair pair, const char* fmt, ...);
            void AddStringToChars(DrawLineInfo& dli, ColorPair pair, string_view stringToAdd);
            void FillRestWithSpaces(DrawLineInfo& dli);

            void AnalyzeMousePosition(int x, int y, MousePositionInfo& mpInfo);

            void MoveTo(uint64 offset, bool select);
            void MoveScrollTo(uint64 offset);

            int PrintCursorPosInfo(int x, int y, uint32 width, bool addSeparator, Renderer& r);

          public:
            Instance(const std::string_view& name, Reference<GView::Object> obj, Settings* settings);

            bool GetPropertyValue(uint32 propertyID, PropertyValue& value) override;
            bool SetPropertyValue(uint32 propertyID, const PropertyValue& value, String& error) override;
            void SetCustomPropertyValue(uint32 propertyID) override;
            bool IsPropertyValueReadOnly(uint32 propertyID) override;
            const vector<Property> GetPropertiesList() override;
            bool GoTo(uint64 offset) override;
            bool Select(uint64 offset, uint64 size) override;
            std::string_view GetName() override;
            void PaintCursorInformation(AppCUI::Graphics::Renderer& renderer, uint32 width, uint32 height) override;
            void Paint(AppCUI::Graphics::Renderer& renderer) override;

            bool OnUpdateCommandBar(AppCUI::Application::CommandBar& commandBar) override;
            bool OnEvent(Reference<Control>, Event eventType, int ID) override;

            void OnAfterResize(int newWidth, int newHeight) override;
            void OnStart() override;

            // Mouse events
            bool OnMouseWheel(int x, int y, AppCUI::Input::MouseWheel direction) override;
            void OnMousePressed(int x, int y, AppCUI::Input::MouseButton button) override;

            virtual bool OnKeyEvent(AppCUI::Input::Key keyCode, char16 characterCode) override;
        };
    } // namespace DissasmViewer
} // namespace View

}; // namespace GView