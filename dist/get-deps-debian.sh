#!/bin/bash


function echoPackageInstall() {
  local name="$1"

  echo
  echo "============================================================"
  echo "Installing $name..."
  echo "============================================================"
  echo
}

function extractGitHubSource() {
  local author="$1"
  local name="$2"
  local branch_name="$3"
  local extract_dir="$4"

  local dep_full_name="$author/$name"
  local download_dir="download/$author"
  local dest_dir="$extract_dir/$author"
  local archive="$name-$branch_name.tar.gz"
  local sources_url="https://github.com/$author/$name/archive/$branch_name.tar.gz"

  echo "Create download directory $download_dir..." && \
  mkdir -p "$download_dir" && \
  echo "Download sources from $sources_url..." && \
  wget -O "$download_dir/$archive" "$sources_url" && \
  echo "Create destination directory $dest_dir..." && \
  mkdir -p "$dest_dir" && \
  echo "Extract downloaded sources to $dest_dir..." && \
  tar -xzf "$download_dir/$archive" -C "$dest_dir" && \
  echo "Extracting done for $dep_full_name." || \
  echo "Error : unable to extract sources for $dep_full_name."
}


function tryAptGet() {
  local name="$1"

  echoPackageInstall "$name"

  if apt-get install -y "$name"; then
    echo "Successfully get $name."
  else
    echo "Error : unable to get $name." >&2
  fi
}

function tryHeaderOnlyGet() {
  local author="$1"
  local name="$2"
  local branch_name="$3"

  local dep_full_name="$author/$name"
  echoPackageInstall "$dep_full_name"

  caller_dir="$(pwd)" # Must be absolute for working-directory restore
  local caller_dir

  local install_dir="$caller_dir/install" # Must be absolute for CMake configuration
  local deps_dir="build"
  local project_dir="$deps_dir/$author/$name-$branch_name"

  echo "Create deps build directory at $deps_dir..." && \
  mkdir -p $deps_dir && \
  echo "Extract $dep_full_name sources..." && \
  extractGitHubSource "$author" "$name" "$branch_name" "$deps_dir" && \
  echo "Go to $dep_full_name project directory $project_dir..." && \
  cd "$project_dir" && \
  echo "Configure project..." && \
  cmake "-DCMAKE_INSTALL_PREFIX=$install_dir" . && \
  echo "Install project to $install_dir..." && \
  cmake --install . && \
  echo "Successfully get $dep_full_name." || \
  echo "Error : unable to get $dep_full_name."

  cd "$caller_dir" || exit 1
}

function trySourceGet() {
  local author="$1"
  local name="$2"
  local branch_name="$3"
  local special_build_dir="$4"

  local dep_full_name="$author/$name"
  echoPackageInstall "$dep_full_name"

  caller_dir="$(pwd)" # Must be absolute for working-directory restore
  local caller_dir

  local install_dir="$caller_dir/install" # Must be absolute for CMake configuration
  local deps_dir="build"
  local project_dir="$deps_dir/$author/$name-$branch_name"

  if [ "$special_build_dir" ]; then
    build_dir="$deps_dir/$author/$special_build_dir/build"
  else
    build_dir="$deps_dir/$author/$name-$branch_name/build"
  fi

  echo "Create deps build directory at $deps_dir..." && \
  mkdir -p "$deps_dir" && \
  extractGitHubSource "$author" "$name" "$branch_name" "$deps_dir"
  echo "Create $dep_full_name build directory at $build_dir..."
  mkdir -p "$build_dir" && \
  echo "Go to $dep_full_name project directory $project_dir..." && \
  cd "$build_dir" && \
  echo "Configure project..." && \
  cmake "-DCMAKE_INSTALL_PREFIX=$install_dir" "-DCMAKE_AR=$(which "$AR")" "-DCMAKE_RANLIB=$(which "$RANLIB")" .. && \
  echo "Build project..." && \
  cmake --build . -- "-j$(nproc)" && \
  echo "Install project to $install_dir..." && \
  cmake --install . && \
  echo "Successfully get $dep_full_name." || \
  echo "Error : unable to get $dep_full_name."

  cd "$caller_dir" || exit 1
}


if [ ! "$AR" ]; then
  AR="ar"
fi

if [ ! "$RANLIB" ]; then
  RANLIB="ranlib"
fi

tryAptGet libboost-all-dev
tryAptGet liblua5.3-dev
tryHeaderOnlyGet nlohmann json master
tryHeaderOnlyGet ThePhD sol2 main
trySourceGet gabime spdlog v1.x spdlog-1.x
