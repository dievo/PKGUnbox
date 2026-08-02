// PKGUnbox CLI — extracts PS4 .pkg files
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#include "common/path_util.h"
#include "common/string_util.h"
#include "core/file_format/pkg.h"
#include "core/file_format/psf.h"
#include "core/loader.h"

static constexpr int RET_BASE_GAME = 101;
static constexpr int RET_PATCH = 102;
static constexpr int RET_DLC = 103;

static void PrintUsage(std::string_view programName) {
    std::cout << "PKGUnbox — PS4 PKG Extractor\n\n"
              << "Usage:\n"
              << "  " << programName << " <file.pkg> [output_dir]\n"
              << "  " << programName << " <file1.pkg> <file2.pkg> ... [output_dir]\n"
              << "  " << programName << " <file.pkg> --check-type\n"
              << "  " << programName << " --help\n\n"
              << "Arguments:\n"
              << "  file.pkg       One or more PS4 .pkg files to extract\n"
              << "  output_dir     Output directory (default: next to each .pkg file)\n"
              << "  --check-type   Print the package type and exit (base=101, patch=102, dlc=103)\n"
              << "  --output DIR   Explicit output directory for all files\n"
              << "  --help         Show this help message\n\n"
              << "Exit codes:\n"
              << "  0   Success or no file provided\n"
              << "  1   Error (invalid file, open failure, etc.)\n"
              << "  101 Base game\n"
              << "  102 Game update (patch)\n"
              << "  103 DLC\n";
}

static bool IsInteractive() {
#ifdef _WIN32
    return false; // Windows batch scripts expect stdin.get()
#else
    return isatty(fileno(stdin)) != 0;
#endif
}

struct ExtractResult {
    std::filesystem::path file;
    bool success;
    int type;
};

static int ExtractSingle(const std::filesystem::path& file,
                         const std::filesystem::path& outputDir,
                         bool checkTypeOnly) {
    // Detect file type
    if (Loader::DetectFileType(file) != Loader::FileTypes::Pkg) {
        std::cerr << "Error: " << file << " is not a valid PKG file\n";
        return 1;
    }

    // Open PKG
    PKG pkg;
    std::string failreason;
    if (!pkg.Open(file, failreason)) {
        std::cerr << "Error: Cannot open PKG file: " << failreason << "\n";
        return 1;
    }

    // Parse SFO metadata
    PSF psf;
    if (!psf.Open(pkg.sfo)) {
        std::cerr << "Error: Could not read SFO metadata\n";
        return 1;
    }

    // Determine output path from title ID
    std::filesystem::path output_folder_path = outputDir;
    output_folder_path /= pkg.GetTitleID();

    // Detect package type
    int pkgType = RET_BASE_GAME;
    auto category = psf.GetString("CATEGORY");
    auto pkgFlags = pkg.GetPkgFlags();

    if (pkgFlags.contains("PATCH")) {
        pkgType = RET_PATCH;
        output_folder_path += "-patch";
    } else if (category.has_value() && *category == "ac") {
        pkgType = RET_DLC;
        if (auto contentId = psf.GetString("CONTENT_ID"); contentId.has_value()) {
            auto parts = Common::SplitString(std::string(*contentId), '-');
            if (parts.size() >= 3) {
                output_folder_path /= parts[2];
            }
        }
    }

    if (checkTypeOnly) {
        return pkgType;
    }

    // Extract
    std::cout << "  Target: " << output_folder_path << "\n";

    if (!pkg.Extract(file, output_folder_path, failreason)) {
        std::cerr << "Error: Cannot extract PKG: " << failreason << "\n";
        return 1;
    }

    u32 nfiles = pkg.GetNumberOfFiles();
    for (u32 i = 0; i < nfiles; i++) {
        std::cout << "\r      Extracting file " << (i + 1) << " of " << nfiles << std::flush;
        pkg.ExtractFiles(i);
    }

    std::cout << "\n";
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    // Check for --help
    for (int i = 1; i < argc; i++) {
        if (std::string_view(argv[i]) == "--help" || std::string_view(argv[i]) == "-h") {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    // Parse arguments
    std::vector<std::filesystem::path> pkgFiles;
    std::filesystem::path outputDir;
    bool checkTypeOnly = false;
    bool hasExplicitOutput = false;

    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];

        if (arg == "--check-type") {
            checkTypeOnly = true;
        } else if (arg == "--output") {
            if (i + 1 < argc) {
                outputDir = argv[++i];
                hasExplicitOutput = true;
            } else {
                std::cerr << "Error: --output requires a directory argument\n";
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            // Already handled above
        } else {
            // Check if it's a directory (output_dir without --output flag)
            std::filesystem::path p(arg);
            if (std::filesystem::is_directory(p) && !hasExplicitOutput && pkgFiles.empty()) {
                outputDir = p;
                hasExplicitOutput = true;
            } else {
                pkgFiles.push_back(p);
            }
        }
    }

    if (pkgFiles.empty()) {
        std::cerr << "Error: No .pkg files provided\n";
        PrintUsage(argv[0]);
        return 1;
    }

    // Single file with --check-type
    if (checkTypeOnly && pkgFiles.size() == 1) {
        return ExtractSingle(pkgFiles[0], outputDir, true);
    }

    // Multiple files with --check-type not supported
    if (checkTypeOnly) {
        std::cerr << "Error: --check-type only works with a single file\n";
        return 1;
    }

    // Default output: current directory
    if (!hasExplicitOutput) {
        outputDir = std::filesystem::current_path();
    }

    // Extract all files
    int total = static_cast<int>(pkgFiles.size());
    int succeeded = 0;
    int failed = 0;

    for (int i = 0; i < total; i++) {
        std::cout << "[" << (i + 1) << "/" << total << "] Extracting " << pkgFiles[i].filename() << "...\n";

        int result = ExtractSingle(pkgFiles[i], outputDir, false);
        if (result == 0) {
            succeeded++;
        } else {
            failed++;
        }
    }

    // Summary
    if (total > 1) {
        std::cout << "\n--- Summary ---\n";
        std::cout << "Total: " << total << " | Succeeded: " << succeeded << " | Failed: " << failed << "\n";
    }

    if (argc == 2 && IsInteractive()) {
        std::cout << "Press [enter] to exit." << std::endl;
        std::cin.get();
    }

    return failed > 0 ? 1 : 0;
}
