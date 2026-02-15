#include "ShellCommands.h"

#include "globals.h"
#include "plugin/PluginContainer.h"
#include "plugin/PluginData.h"

#include <iopshell/api.h>

#include <coreinit/debug.h>

#include <memory>
#include <numeric>
#include <optional>

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

        // Define Columns
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
        for (const auto &plugin : gLoadedPlugins) {
            if (!showAll && !plugin.isLinkedAndLoaded()) {
                totalSizeOther += plugin.getMemoryFootprint();
                continue;
            }

            auto meta = plugin.getMetaInformation();

            std::string heapUsage = "unknown";
            if (const auto tracking = plugin.getTrackingMemoryAllocator()) {
                uint32_t heapUsageSize = 0;
                if (const auto stats = tracking->GetHeapMemoryUsageSnapshot()) {
                    heapUsageSize = stats->currentAllocated;
                }
                heapUsage = BytesToHumanReadable(heapUsageSize);
            }

            std::vector<std::string> rowData;
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

    void Init() {
        sPluginGroup = std::make_unique<IOPShellModule::CommandGroup>("plugins", "Manage aroma plugins");

        sPluginGroup->AddCommand("heap_usage", PrintHeapUsage, "Show current heap usage for tracked plugins");

        sPluginGroup->AddRawCommand("list", ListPlugins, "Lists active plugins", "Usage: \"list -a\" to list all plugins");

        sPluginCmdHandle = sPluginGroup->Register();
    }
} // namespace ShellCommands