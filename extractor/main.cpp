// PKGUnbox CLI — extracts PS4 .pkg files
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdlib>
#include <iostream>
#include <string_view>

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
              << "  " << programName << " <file.pkg> --check-type\n"
              << "  " << programName << " --help\n\n"
              << "Arguments:\n"
              << "  file.pkg       Path to the PS4 .pkg file to extract\n"
              << "  output_dir     Output directory (default: next to the .pkg file)\n"
              << "  --check-type   Print the package type and exit (base=101, patch=102, dlc=103)\n"
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

int main(int argc, char** argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    std::string_view arg1 = argv[1];
    if (arg1 == "--help" || arg1 == "-h") {
        PrintUsage(argv[0]);
        return 0;
    }

    std::filesystem::path file = argv[1];
    std::filesystem::path output_folder_path;
    bool checkTypeOnly = false;

    if (argc > 2) {
        if (std::string_view(argv[2]) == "--check-type") {
            checkTypeOnly = true;
        } else {
            output_folder_path = argv[2];
        }
    }

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
    std::cout << "Extracting " << file << " to " << output_folder_path << "\n";

    if (!pkg.Extract(file, output_folder_path, failreason)) {
        std::cerr << "Error: Cannot extract PKG: " << failreason << "\n";
        return 1;
    }

    u32 nfiles = pkg.GetNumberOfFiles();
    for (u32 i = 0; i < nfiles; i++) {
        std::cout << "\r      Extracting file " << (i + 1) << " of " << nfiles << std::flush;
        pkg.ExtractFiles(i);
    }

    std::cout << "\nDone.\n";

    if (argc == 2 && IsInteractive()) {
        std::cout << "Press [enter] to exit." << std::endl;
        std::cin.get();
    }

    return 0;
}
