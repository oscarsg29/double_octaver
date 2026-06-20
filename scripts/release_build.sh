#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

PROJECT_FILE="Builds/MacOSX/DoubleOctaver.xcodeproj"
DERIVED_DATA="Builds/MacOSX/DerivedData"
BUILD_DIR="Builds/MacOSX/build/Release"
VST3_NAME="DoubleOctaver.vst3"
AU_NAME="DoubleOctaver.component"
INSTALLED_VST3="$HOME/Library/Audio/Plug-Ins/VST3/$VST3_NAME"
INSTALLED_AU="$HOME/Library/Audio/Plug-Ins/Components/$AU_NAME"

usage() {
    cat <<EOF
Usage:
  scripts/release_build.sh [--version x.y.z] [--no-bump] [--skip-build]

Defaults:
  Bumps the patch version by one, stamps the current git short commit hash,
  builds Release VST3 and AU, installs them into ~/Library/Audio/Plug-Ins,
  and signs the installed bundles ad-hoc.

Examples:
  scripts/release_build.sh
  scripts/release_build.sh --version 1.0.7
  scripts/release_build.sh --no-bump
EOF
}

REQUESTED_VERSION=""
NO_BUMP=0
SKIP_BUILD=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version)
            [[ $# -ge 2 ]] || { echo "Missing value for --version" >&2; exit 2; }
            REQUESTED_VERSION="$2"
            shift 2
            ;;
        --no-bump)
            NO_BUMP=1
            shift
            ;;
        --skip-build)
            SKIP_BUILD=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

pre_release_dirty="$(git status --porcelain)"

current_version="$(perl -ne 'if (/version="([0-9]+\.[0-9]+\.[0-9]+)"/) { print $1; exit }' doubleOctaver.jucer)"
if [[ -z "$current_version" ]]; then
    echo "Could not read current version from doubleOctaver.jucer" >&2
    exit 1
fi

if [[ -n "$REQUESTED_VERSION" ]]; then
    new_version="$REQUESTED_VERSION"
elif [[ "$NO_BUMP" -eq 1 ]]; then
    new_version="$current_version"
else
    IFS=. read -r major minor patch <<< "$current_version"
    new_version="${major}.${minor}.$((patch + 1))"
fi

if [[ ! "$new_version" =~ ^[0-9]+[.][0-9]+[.][0-9]+$ ]]; then
    echo "Invalid version '$new_version'. Use x.y.z, for example 1.0.2." >&2
    exit 1
fi

IFS=. read -r version_major version_minor version_patch <<< "$new_version"
version_code_decimal=$((version_major * 65536 + version_minor * 256 + version_patch))
version_code_hex="$(printf '0x%x' "$version_code_decimal")"

commit_hash="$(git rev-parse --short HEAD)"
if [[ -n "$pre_release_dirty" ]]; then
    build_commit="${commit_hash}+"
else
    build_commit="$commit_hash"
fi

echo "Current version: $current_version"
echo "New version:     $new_version"
echo "Version code:    $version_code_hex ($version_code_decimal)"
echo "Build commit:    $build_commit"

perl -0pi -e "s/version=\"[0-9]+\\.[0-9]+\\.[0-9]+\"/version=\"$new_version\"/" doubleOctaver.jucer

perl -0pi -e "s/constexpr auto buildCommit = \"[^\"]+\";/constexpr auto buildCommit = \"$build_commit\";/" \
    Source/PluginEditor.cpp

NEW_VERSION="$new_version" VERSION_CODE_HEX="$version_code_hex" perl -0pi -e '
               s/JucePlugin_Version=\d+\.\d+\.\d+/JucePlugin_Version=$ENV{NEW_VERSION}/g;
               s/JucePlugin_VersionCode=0x[0-9a-fA-F]+/JucePlugin_VersionCode=$ENV{VERSION_CODE_HEX}/g;
               s/JucePlugin_VersionString=(?:\\\\)*\\"[0-9.]+(?:\\\\)*\\"/JucePlugin_VersionString=\\\\\\"$ENV{NEW_VERSION}\\\\\\"/g;
               s/JUCE_APP_VERSION=\d+\.\d+\.\d+/JUCE_APP_VERSION=$ENV{NEW_VERSION}/g;
               s/JUCE_APP_VERSION_HEX=0x[0-9a-fA-F]+/JUCE_APP_VERSION_HEX=$ENV{VERSION_CODE_HEX}/g;
               s/com\.OscarOsorio\.DoubleOctaver\.aradocumentarchive\.\d+\.\d+\.\d+/com.OscarOsorio.DoubleOctaver.aradocumentarchive.$ENV{NEW_VERSION}/g' \
    Builds/MacOSX/DoubleOctaver.xcodeproj/project.pbxproj

perl -0pi -e "s/#define JucePlugin_Version\\s+\\d+\\.\\d+\\.\\d+/#define JucePlugin_Version                $new_version/;
               s/#define JucePlugin_VersionCode\\s+0x[0-9a-fA-F]+/#define JucePlugin_VersionCode            $version_code_hex/;
               s/#define JucePlugin_VersionString\\s+\"\\d+\\.\\d+\\.\\d+\"/#define JucePlugin_VersionString          \"$new_version\"/;
               s/#define JucePlugin_ARADocumentArchiveID\\s+\"com\\.OscarOsorio\\.DoubleOctaver\\.aradocumentarchive\\.\\d+\\.\\d+\\.\\d+\"/#define JucePlugin_ARADocumentArchiveID   \"com.OscarOsorio.DoubleOctaver.aradocumentarchive.$new_version\"/" \
    JuceLibraryCode/JucePluginDefines.h

perl -0pi -e "s/#define  JucePlugin_Version\\s+\\d+\\.\\d+\\.\\d+/#define  JucePlugin_Version                $new_version/;
               s/#define  JucePlugin_VersionCode\\s+0x[0-9a-fA-F]+/#define  JucePlugin_VersionCode            $version_code_hex/;
               s/#define  JucePlugin_VersionString\\s+\"\\d+\\.\\d+\\.\\d+\"/#define  JucePlugin_VersionString          \"$new_version\"/;
               s/#define  JucePlugin_ARADocumentArchiveID\\s+\"com\\.OscarOsorio\\.DoubleOctaver\\.aradocumentarchive\\.\\d+\\.\\d+\\.\\d+\"/#define  JucePlugin_ARADocumentArchiveID   \"com.OscarOsorio.DoubleOctaver.aradocumentarchive.$new_version\"/" \
    JuceLibraryCode/JuceHeader.h

perl -0pi -e "s/<string>\\d+\\.\\d+\\.\\d+<\\/string>/<string>$new_version<\\/string>/g;
               s/<integer>\\d+<\\/integer>/<integer>$version_code_decimal<\\/integer>/g" \
    Builds/MacOSX/Info-AU.plist

perl -0pi -e "s/<string>\\d+\\.\\d+\\.\\d+<\\/string>/<string>$new_version<\\/string>/g" \
    Builds/MacOSX/Info-VST3.plist \
    Builds/MacOSX/Info-VST3_Manifest_Helper.plist \
    Builds/MacOSX/Info-Standalone_Plugin.plist

if [[ "$SKIP_BUILD" -eq 1 ]]; then
    echo "Skipped build."
    exit 0
fi

xcodebuild -project "$PROJECT_FILE" -scheme "DoubleOctaver - VST3" -configuration Release -derivedDataPath "$DERIVED_DATA" build
xcodebuild -project "$PROJECT_FILE" -scheme "DoubleOctaver - AU" -configuration Release -derivedDataPath "$DERIVED_DATA" build

mkdir -p "$(dirname "$INSTALLED_VST3")" "$(dirname "$INSTALLED_AU")"

# The generated Xcode copy phase can run before ProcessInfoPlistFile. Copy the
# finalized build products after xcodebuild so installed bundles carry the new version.
ditto "$BUILD_DIR/$VST3_NAME" "$INSTALLED_VST3"
ditto "$BUILD_DIR/$AU_NAME" "$INSTALLED_AU"

codesign --force --sign - --timestamp "$INSTALLED_VST3"
codesign --force --sign - --timestamp "$INSTALLED_AU"
codesign --verify --deep --strict "$INSTALLED_VST3" "$INSTALLED_AU"

echo
echo "Installed VST3:"
/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" -c "Print :CFBundleVersion" \
    "$INSTALLED_VST3/Contents/Info.plist"

echo
echo "Installed AU:"
/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" -c "Print :CFBundleVersion" -c "Print :AudioComponents:0:version" \
    "$INSTALLED_AU/Contents/Info.plist"

echo
echo "Release build complete: v$new_version $build_commit"
