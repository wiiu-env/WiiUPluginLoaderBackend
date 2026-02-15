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

        table.AddColumn("Plugin Name", ConsoleTable::LEFT);
        table.AddColumn("Current usage", ConsoleTable::RIGHT);
        table.AddColumn("Peak usage", ConsoleTable::RIGHT);
        table.AddColumn("Currently allocated", ConsoleTable::RIGHT);
        table.AddColumn("Total allocated", ConsoleTable::RIGHT);
        table.AddColumn("Total freed", ConsoleTable::RIGHT);

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


    void Init() {
        sPluginGroup = std::make_unique<IOPShellModule::CommandGroup>("plugins", "Manage aroma plugins");

        sPluginGroup->AddCommand("heap_usage", PrintHeapUsage, "Show current heap usage for tracked plugins");

        sPluginCmdHandle = sPluginGroup->Register();
    }
} // namespace ShellCommands