#include "ShellCommands.h"

#include "globals.h"
#include "plugin/ButtonComboManager.h"
#include "plugin/FunctionData.h"
#include "plugin/HookData.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginData.h"
#include "plugin/SectionInfo.h"
#include "utils/StringTools.h"
#include "utils/utils.h"

#include <iopshell/api.h>

#include <coreinit/debug.h>

#include <memory>
#include <numeric>
#include <optional>
#include <zconf.h>

namespace {
    std::string toString(WUPSButtonCombo_ComboType type) {
        switch (type) {
            case WUPS_BUTTON_COMBO_COMBO_TYPE_INVALID:
                return "Invalid";
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD:
                return "Hold";
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD_OBSERVER:
                return "Hold (Observer)";
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN:
                return "Press Down";
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN_OBSERVER:
                return "Press Down (Observer)";
        }
        return "invalid";
    }

    std::string toString(WUPSButtonCombo_ComboStatus type) {
        switch (type) {
            case WUPS_BUTTON_COMBO_COMBO_STATUS_INVALID_STATUS:
                return "Invalid";
            case WUPS_BUTTON_COMBO_COMBO_STATUS_VALID:
                return "Valid";
            case WUPS_BUTTON_COMBO_COMBO_STATUS_CONFLICT:
                return "Conflict";
        }
        return "Invalid";
    }


    std::string toString(const function_replacement_library_type_t type) {
        switch (type) {
            case LIBRARY_AVM:
                return "AVM";
            case LIBRARY_CAMERA:
                return "CAMERA";
            case LIBRARY_COREINIT:
                return "COREINIT";
            case LIBRARY_DC:
                return "DC";
            case LIBRARY_DMAE:
                return "DMAE";
            case LIBRARY_DRMAPP:
                return "DRMAPP";
            case LIBRARY_ERREULA:
                return "ERREULA";
            case LIBRARY_GX2:
                return "GX2";
            case LIBRARY_H264:
                return "H264";
            case LIBRARY_LZMA920:
                return "LZMA920";
            case LIBRARY_MIC:
                return "MIC";
            case LIBRARY_NFC:
                return "NFC";
            case LIBRARY_NIO_PROF:
                return "NIO_PROF";
            case LIBRARY_NLIBCURL:
                return "NLIBCURL";
            case LIBRARY_NLIBNSS:
                return "NLIBNSS";
            case LIBRARY_NLIBNSS2:
                return "NLIBNSS2";
            case LIBRARY_NN_AC:
                return "NN_AC";
            case LIBRARY_NN_ACP:
                return "NN_ACP";
            case LIBRARY_NN_ACT:
                return "NN_ACT";
            case LIBRARY_NN_AOC:
                return "NN_AOC";
            case LIBRARY_NN_BOSS:
                return "NN_BOSS";
            case LIBRARY_NN_CCR:
                return "NN_CCR";
            case LIBRARY_NN_CMPT:
                return "NN_CMPT";
            case LIBRARY_NN_DLP:
                return "NN_DLP";
            case LIBRARY_NN_EC:
                return "NN_EC";
            case LIBRARY_NN_FP:
                return "NN_FP";
            case LIBRARY_NN_HAI:
                return "NN_HAI";
            case LIBRARY_NN_HPAD:
                return "NN_HPAD";
            case LIBRARY_NN_IDBE:
                return "NN_IDBE";
            case LIBRARY_NN_NDM:
                return "NN_NDM";
            case LIBRARY_NN_NETS2:
                return "NN_NETS2";
            case LIBRARY_NN_NFP:
                return "NN_NFP";
            case LIBRARY_NN_NIM:
                return "NN_NIM";
            case LIBRARY_NN_OLV:
                return "NN_OLV";
            case LIBRARY_NN_PDM:
                return "NN_PDM";
            case LIBRARY_NN_SAVE:
                return "NN_SAVE";
            case LIBRARY_NN_SL:
                return "NN_SL";
            case LIBRARY_NN_SPM:
                return "NN_SPM";
            case LIBRARY_NN_TEMP:
                return "NN_TEMP";
            case LIBRARY_NN_UDS:
                return "NN_UDS";
            case LIBRARY_NN_VCTL:
                return "NN_VCTL";
            case LIBRARY_NSYSCCR:
                return "NSYSCCR";
            case LIBRARY_NSYSHID:
                return "NSYSHID";
            case LIBRARY_NSYSKBD:
                return "NSYSKBD";
            case LIBRARY_NSYSNET:
                return "NSYSNET";
            case LIBRARY_NSYSUHS:
                return "NSYSUHS";
            case LIBRARY_NSYSUVD:
                return "NSYSUVD";
            case LIBRARY_NTAG:
                return "NTAG";
            case LIBRARY_PADSCORE:
                return "PADSCORE";
            case LIBRARY_PROC_UI:
                return "PROC_UI";
            case LIBRARY_SND_CORE:
                return "SND_CORE";
            case LIBRARY_SND_USER:
                return "SND_USER";
            case LIBRARY_SNDCORE2:
                return "SNDCORE2";
            case LIBRARY_SNDUSER2:
                return "SNDUSER2";
            case LIBRARY_SWKBD:
                return "SWKBD";
            case LIBRARY_SYSAPP:
                return "SYSAPP";
            case LIBRARY_TCL:
                return "TCL";
            case LIBRARY_TVE:
                return "TVE";
            case LIBRARY_UAC:
                return "UAC";
            case LIBRARY_UAC_RPL:
                return "UAC_RPL";
            case LIBRARY_USB_MIC:
                return "USB_MIC";
            case LIBRARY_UVC:
                return "UVC";
            case LIBRARY_UVD:
                return "UVD";
            case LIBRARY_VPAD:
                return "VPAD";
            case LIBRARY_VPADBASE:
                return "VPADBASE";
            case LIBRARY_ZLIB125:
                return "ZLIB125";
            case LIBRARY_OTHER:
                return "OTHER";
        }
        return "OTHER";
    }

    const char *getButtonChar(const WUPSButtonCombo_Buttons value) {
        std::string combo;
        if (value & WUPS_BUTTON_COMBO_BUTTON_A) {
            return "A";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_B) {
            return "B";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_X) {
            return "X";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_Y) {
            return "Y";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_L) {
            return "L";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_R) {
            return "R";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_ZL) {
            return "ZL";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_ZR) {
            return "ZR";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_UP) {
            return "UP";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_DOWN) {
            return "DOWN";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_LEFT) {
            return "LEFT";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_RIGHT) {
            return "RIGHT";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_STICK_L) {
            return "STICK-L";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_STICK_R) {
            return "STICK-R";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_PLUS) {
            return "PLUS";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_MINUS) {
            return "MINUS";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_TV) {
            return "TV";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_RESERVED_BIT) {
            return "RESERVED-BIT";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_1) {
            return "1";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_2) {
            return "2";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_C) {
            return "C";
        }
        if (value & WUPS_BUTTON_COMBO_BUTTON_Z) {
            return "Z";
        }
        return "";
    }

    std::string getComboAsString(const uint32_t value) {
        char comboString[60] = {};

        for (uint32_t i = 0; i < 32; i++) {
            uint32_t bitMask = 1 << i;
            if (value & bitMask) {
                auto val = getButtonChar(static_cast<WUPSButtonCombo_Buttons>(bitMask));
                if (val[0] != '\0') {
                    strcat(comboString, val);
                    strcat(comboString, "+");
                }
            }
        }
        std::string res(comboString);
        if (res.ends_with("+")) {
            res.pop_back();
        }
        return res;
    }

    std::string getMaskAsString(WUPSButtonCombo_ControllerTypes controller_mask) {
        if (controller_mask == WUPS_BUTTON_COMBO_CONTROLLER_NONE) {
            return "NONE";
        }
        if (controller_mask == WUPS_BUTTON_COMBO_CONTROLLER_ALL) {
            return "ALL";
        }
        // Optional: Check strictly for full groups if you prefer that over listing individual bits
        if (controller_mask == WUPS_BUTTON_COMBO_CONTROLLER_VPAD) {
            return "VPAD (All)";
        }
        if (controller_mask == WUPS_BUTTON_COMBO_CONTROLLER_WPAD) {
            return "WPAD (All)";
        }
        std::vector<std::string> parts;

        // VPADs
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_VPAD_0) parts.emplace_back("VPAD_0");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_VPAD_1) parts.emplace_back("VPAD_1");

        // WPADs
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_0) parts.emplace_back("WPAD_0");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_1) parts.emplace_back("WPAD_1");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_2) parts.emplace_back("WPAD_2");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_3) parts.emplace_back("WPAD_3");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_4) parts.emplace_back("WPAD_4");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_5) parts.emplace_back("WPAD_5");
        if (controller_mask & WUPS_BUTTON_COMBO_CONTROLLER_WPAD_6) parts.emplace_back("WPAD_6");


        if (parts.empty()) {
            return "UNKNOWN";
        }

        std::string result;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) {
                result += " | ";
            }
            result += parts[i];
        }

        return result;
    }

    bool isHoldCombo(WUPSButtonCombo_ComboType type) {
        switch (type) {
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD:
            case WUPS_BUTTON_COMBO_COMBO_TYPE_HOLD_OBSERVER:
                return true;
            case WUPS_BUTTON_COMBO_COMBO_TYPE_INVALID:
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN:
            case WUPS_BUTTON_COMBO_COMBO_TYPE_PRESS_DOWN_OBSERVER:
                return false;
        }
        return false;
    }
} // namespace
namespace ShellCommands {
    namespace {
        class ConsoleTable {
        public:
            enum Alignment { LEFT,
                             RIGHT };

            void AddColumn(const std::string &title, const Alignment align = LEFT) {
                mCols.push_back({title, align, title.length()});
            }

            void AddRow(const std::vector<std::string> &rowData) {
                if (rowData.size() != mCols.size()) return;
                mRows.push_back(rowData);
                updateWidths(rowData);
            }

            void AddFooter(const std::vector<std::string> &footerData) {
                if (footerData.size() != mCols.size()) return;
                mFooter = footerData;
                updateWidths(footerData);
            }

            void Print() {
                OSReport("\n");
                PrintSeparator();

                OSReport("|");
                for (const auto &[title, align, width] : mCols) printCell(title, width, align);
                OSReport("\n");

                PrintSeparator();

                for (const auto &row : mRows) {
                    OSReport("|");
                    for (size_t i = 0; i < row.size(); ++i) printCell(row[i], mCols[i].width, mCols[i].align);
                    OSReport("\n");
                }

                if (!mFooter.empty()) {
                    PrintSeparator();
                    OSReport("|");
                    for (size_t i = 0; i < mFooter.size(); ++i) printCell(mFooter[i], mCols[i].width, mCols[i].align);
                    OSReport("\n");
                }

                size_t totalWidth = 1;
                for (const auto &col : mCols) {
                    totalWidth += col.width + 3;
                }

                std::string border(totalWidth, '-');
                OSReport("%s\n", border.c_str());
            }

        private:
            void updateWidths(const std::vector<std::string> &data) {
                for (size_t i = 0; i < mCols.size(); ++i) {
                    if (data[i].length() > mCols[i].width) {
                        mCols[i].width = data[i].length();
                    }
                }
            }

            void PrintSeparator() const {
                OSReport("|");
                for (const auto &col : mCols) {
                    // Separator is width + 2 spaces padding
                    std::string line(col.width + 2, '-');
                    OSReport("%s|", line.c_str());
                }
                OSReport("\n");
            }

            static void printCell(const std::string &text, const size_t width, const Alignment align) {
                if (align == LEFT) OSReport(" %-*s |", static_cast<int>(width), text.c_str());
                else
                    OSReport(" %*s |", static_cast<int>(width), text.c_str());
            }

            struct Column {
                std::string title;
                Alignment align;
                size_t width;
            };

            std::vector<Column> mCols;
            std::vector<std::vector<std::string>> mRows;
            std::vector<std::string> mFooter;
        };
    } // namespace

    std::unique_ptr<IOPShellModule::CommandGroup> sPluginGroup;
    std::optional<IOPShellModule::Command> sPluginCmdHandle;

    std::string BytesToHumanReadable(uint32_t bytes) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f KiB", bytes / 1024.0f);
        return std::string(buffer);
    }

    void PrintHeapUsage() {
        ConsoleTable table;

        table.AddColumn("Plugin name", ConsoleTable::LEFT);
        table.AddColumn("Current usage", ConsoleTable::RIGHT);
        table.AddColumn("Peak usage", ConsoleTable::RIGHT);
        table.AddColumn("Current allocations", ConsoleTable::RIGHT);
        table.AddColumn("Total allocations", ConsoleTable::RIGHT);
        table.AddColumn("Total frees", ConsoleTable::RIGHT);

        uint32_t totalCurrentBytes = 0;
        uint32_t totalPeakBytes    = 0;
        uint32_t totalChunks       = 0;

        for (const auto &plugin : gLoadedPlugins) {
            if (!plugin.isLinkedAndLoaded() || !plugin.isUsingTrackingPluginHeapMemoryAllocator()) {
                continue;
            }

            const auto tracking = plugin.getTrackingMemoryAllocator();
            if (!tracking) { continue; }

            const auto stats = tracking->GetHeapMemoryUsageSnapshot();
            if (!stats) { continue; }

            auto activeChunks = stats->allocationMap.size();

            table.AddRow({stats->pluginName,
                          BytesToHumanReadable(stats->currentAllocated),
                          BytesToHumanReadable(stats->peakAllocated),
                          std::to_string(activeChunks),
                          std::to_string(stats->allocCount),
                          std::to_string(stats->freeCount)});

            // Accumulate Totals
            totalCurrentBytes += stats->currentAllocated;
            totalPeakBytes += stats->peakAllocated;
            totalChunks += activeChunks;
        }

        // Add the Footer Row
        table.AddFooter({"TOTAL (tracked plugins)",
                         BytesToHumanReadable(totalCurrentBytes),
                         BytesToHumanReadable(totalPeakBytes),
                         std::to_string(totalChunks),
                         "-",
                         "-"});

        table.Print();
    }

    void ListPlugins(int argc, char **argv) {
        bool showAll = false;
        if (argc > 1 && (std::string_view(argv[1]) == "-a" || std::string_view(argv[1]) == "--all")) {
            showAll = true;
        }

        ConsoleTable table;

        table.AddColumn("Index", ConsoleTable::LEFT);
        table.AddColumn("Name", ConsoleTable::LEFT);
        table.AddColumn("Author", ConsoleTable::LEFT);
        table.AddColumn("Version", ConsoleTable::LEFT);
        if (showAll) {
            table.AddColumn("Active", ConsoleTable::LEFT);
        }
        table.AddColumn("API", ConsoleTable::LEFT);
        table.AddColumn("Memory footprint", ConsoleTable::RIGHT);
        table.AddColumn("Heap usage", ConsoleTable::RIGHT);
        uint32_t totalSizeOther = 0;
        uint32_t id             = 0;
        for (const auto &plugin : gLoadedPlugins) {
            if (!showAll && !plugin.isLinkedAndLoaded()) {
                totalSizeOther += plugin.getMemoryFootprint();
                id++;
                continue;
            }

            auto &meta = plugin.getMetaInformation();

            std::string heapUsage = "unknown";
            if (const auto tracking = plugin.getTrackingMemoryAllocator()) {
                uint32_t heapUsageSize = 0;
                if (const auto stats = tracking->GetHeapMemoryUsageSnapshot()) {
                    heapUsageSize = stats->currentAllocated;
                }
                heapUsage = BytesToHumanReadable(heapUsageSize);
            }

            std::vector<std::string> rowData;
            rowData.emplace_back(std::to_string(id));
            rowData.emplace_back(meta.getName());
            rowData.emplace_back(meta.getAuthor());
            rowData.emplace_back(meta.getVersion());
            if (showAll) {
                rowData.emplace_back(plugin.isLinkedAndLoaded() ? "Yes" : "No");
            }
            rowData.emplace_back(meta.getWUPSVersion().toString());
            rowData.emplace_back("~ " + BytesToHumanReadable(plugin.getMemoryFootprint()));
            rowData.emplace_back(heapUsage);

            table.AddRow(rowData);
            id++;
        }

        if (totalSizeOther > 0 && !showAll) {
            std::vector<std::string> rowData;
            rowData.emplace_back("Inactive Plugins");
            rowData.emplace_back("-");
            rowData.emplace_back("-");

            rowData.emplace_back("-");
            rowData.emplace_back("~ " + BytesToHumanReadable(totalSizeOther));
            rowData.emplace_back("-");

            table.AddFooter(rowData);
        }

        table.Print();
    }

    void PluginDetails(std::optional<uint32_t> idOpt) {
        uint32_t id = 0;
        for (const auto &plugin : gLoadedPlugins) {
            if (idOpt) {
                if (*idOpt != id) {
                    id++;
                    continue;
                }
            }
            auto &meta = plugin.getMetaInformation();
            OSReport("\n");
            OSReport("Index:            %d\n", id);
            OSReport("Name:             %s\n", meta.getName().c_str());
            OSReport("Description:      %s\n", meta.getDescription().c_str());
            OSReport("Author:           %s\n", meta.getAuthor().c_str());
            OSReport("Version:          %s\n", meta.getVersion().c_str());
            OSReport("Build date:       %s\n", meta.getBuildTimestamp().c_str());
            OSReport("Storage Id:       %s\n", meta.getStorageId().c_str());
            OSReport("Active:           %s\n", plugin.isLinkedAndLoaded() ? "Yes" : "No");
            OSReport("API:              %s\n", meta.getWUPSVersion().toString().c_str());

            const auto memoryFootprint = plugin.getMemoryFootprint();
            const auto pluginSize      = plugin.getPluginDataCopy()->getBuffer().size_bytes();
            size_t textSize            = 0;
            size_t dataSize            = 0;

            OSReport("Memory footprint: ~%s\n", BytesToHumanReadable(memoryFootprint).c_str());
            OSReport("\t- .wps:  ~%s\n", BytesToHumanReadable(pluginSize).c_str());
            if (plugin.isLinkedAndLoaded()) {
                textSize = plugin.getPluginLinkInformation().getTextMemory().size();
                dataSize = plugin.getPluginLinkInformation().getDataMemory().size();
                OSReport("\t- CODE:  ~%s\n", BytesToHumanReadable(textSize).c_str());
                OSReport("\t- DATA:  ~%s\n", BytesToHumanReadable(dataSize).c_str());
            }
            const auto otherSize = memoryFootprint - pluginSize - textSize - dataSize;
            OSReport("\t- Other: ~%s\n", BytesToHumanReadable(otherSize).c_str());
            OSReport("\n");
            const auto &sectionInfoList = plugin.getPluginLinkInformation().getSectionInfoList();
            OSReport("Sections:         %d\n", sectionInfoList.size());
            for (const auto &sectionInfo : sectionInfoList) {
                OSReport("\t- 0x%08X - 0x%08X %-15s %-11s\n", sectionInfo.second.getAddress(), sectionInfo.second.getAddress() + sectionInfo.second.getSize(), sectionInfo.first.c_str(), BytesToHumanReadable(sectionInfo.second.getSize()).c_str());
            }
            OSReport("\n");

            const auto &hookList = plugin.getPluginLinkInformation().getHookDataList();
            OSReport("WUPS Hooks:       %d\n", hookList.size());
            for (const auto &hook : hookList) {
                OSReport("\t- %p - %s\n", hook.getFunctionPointer(), hookNameToString(hook.getType()).c_str());
            }
            OSReport("\n");
            const auto &buttonCombos = plugin.GetButtonComboData();
            OSReport("Button combos: %d\n", buttonCombos.size());
            for (const auto &combo : buttonCombos) {
                OSReport("\t- \"%s\" \n", combo.label.c_str());
                OSReport("\t\tStatus:          %s\n", toString(combo.status).c_str());
                OSReport("\t\tCallback:        %p (%s)\n", combo.callbackOptions.callback, combo.callbackOptions.callback != nullptr ? getModuleAndSymbolName(reinterpret_cast<uint32_t>(combo.callbackOptions.callback)).c_str() : "");
                OSReport("\t\tContext:         %p\n", combo.callbackOptions.context);
                OSReport("\t\tType:            %s\n", toString(combo.buttonComboOptions.type).c_str());
                if (isHoldCombo(combo.buttonComboOptions.type)) {
                    OSReport("\t\tHold duration:   %d\n", combo.buttonComboOptions.optionalHoldForXMs);
                }
                OSReport("\t\tCombo:           %s\n", getComboAsString(combo.buttonComboOptions.basicCombo.combo).c_str());
                OSReport("\t\tController Mask: %s\n", getMaskAsString(combo.buttonComboOptions.basicCombo.controllerMask).c_str());
            }
            OSReport("\n");
            const auto &functionPatches = plugin.getPluginLinkInformation().getFunctionDataList();
            OSReport("Function patches: %d\n", functionPatches.size());
            for (const auto &function : functionPatches) {
                if (function.getPhysicalAddress() != nullptr) {
                    OSReport("\t- Hook: %p -             PA: %p VA: %p\n", function.getReplaceAddress(), function.getPhysicalAddress(), function.getVirtualAddress());
                } else {
                    OSReport("\t- Hook: %p - %-9s - %s\n", function.getReplaceAddress(), toString(function.getLibrary()).c_str(), function.getName().c_str());
                }
            }

            OSReport("\n");
            OSReport("Heap usage:\n");
            if (const auto tracking = plugin.getTrackingMemoryAllocator(); tracking != nullptr) {
                if (const auto stats = tracking->GetHeapMemoryUsageSnapshot(); stats) {
                    OSReport("\t- Currently allocated: %s\n", BytesToHumanReadable(stats->currentAllocated).c_str());
                    OSReport("\t- Peak allocated:      %s\n", BytesToHumanReadable(stats->peakAllocated).c_str());
                    OSReport("\t- Current allocations: %d\n", stats->allocationMap.size());
                    OSReport("\t- Total allocations:   %d\n", stats->allocCount);
                    OSReport("\t- Total frees:         %d\n", stats->freeCount);
                }
            } else {
                OSReport("\t Not tracked.\n");
            }

            OSReport("\n=================\n");

            id++;
            if (idOpt) {
                break;
            }
        }
    }

    void Init() {
        sPluginGroup = std::make_unique<IOPShellModule::CommandGroup>("plugins", "Manage aroma plugins");

        sPluginGroup->AddCommand("heap_usage", PrintHeapUsage, "Show current heap usage for tracked plugins");

        sPluginGroup->AddRawCommand("list", ListPlugins, "Lists active plugins", "Usage: \"list -a\" to list all plugins");
        sPluginGroup->AddCommand("details", PluginDetails, "Shows details for plugins");

        sPluginCmdHandle = sPluginGroup->Register();
    }
} // namespace ShellCommands