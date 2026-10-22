#!/bin/sh

set -eu

version="1.0.0"
revision="1"

project_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
source_directory="$project_directory/src"
test_directory="$project_directory/tests"
template="$project_directory/packaging/PackageInfo.in"
output_directory="$project_directory/dist"

if [ "$(uname -s)" != "Haiku" ]; then
	echo "Error: packages must be built on Haiku." >&2
	exit 1
fi

for command_name in findpaths getarch make package sed; do
	if ! command -v "$command_name" >/dev/null 2>&1; then
		echo "Error: required command '$command_name' was not found." >&2
		exit 1
	fi
done

architecture=$(getarch)
package_name="ttfx_text_screensaver-$version-$revision-$architecture.hpkg"
output="$output_directory/$package_name"

if [ -e "$output" ]; then
	echo "Error: $output already exists." >&2
	echo "Increase the package revision or remove the old local artifact explicitly." >&2
	exit 1
fi

make -C "$test_directory" clean
make -C "$test_directory" test
make -C "$source_directory" clean
make -C "$source_directory"

binary="$source_directory/ttfx-text"
if [ ! -f "$binary" ]; then
	echo "Error: the expected screen saver binary was not produced at $binary." >&2
	exit 1
fi

staging_directory=$(mktemp -d "${TMPDIR:-/tmp}/ttfx-text-package.XXXXXX")
trap 'rm -rf -- "$staging_directory"' EXIT HUP INT TERM

mkdir -p "$staging_directory/add-ons/Screen Savers"
mkdir -p "$staging_directory/documentation/packages/ttfx_text_screensaver"
mkdir -p "$output_directory"

cp "$binary" "$staging_directory/add-ons/Screen Savers/ttfx-text"
cp "$project_directory/README.md" \
	"$staging_directory/documentation/packages/ttfx_text_screensaver/README.md"
cp "$project_directory/LICENSE" \
	"$staging_directory/documentation/packages/ttfx_text_screensaver/LICENSE"

sed \
	-e "s/@VERSION@/$version/g" \
	-e "s/@REVISION@/$revision/g" \
	-e "s/@ARCHITECTURE@/$architecture/g" \
	"$template" > "$staging_directory/.PackageInfo"

package create -C "$staging_directory" "$output"
package list -i "$output"

echo
echo "Created $output"
