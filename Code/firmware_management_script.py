#!/usr/bin/python3

# Adds PlatformIO post-processing to merge all the ESP flash images into a single image.

import os
from shutil import copyfile

Import("env", "projenv")

board_config = env.BoardConfig()
firmware_build_bin = "${BUILD_DIR}/${PROGNAME}.bin"
standard_bin = "firmware.bin"
merged_bin = "firmware-merged.bin"


def create_binaries(source, target, env):
  flash_images = [
    *env.Flatten(env.get("FLASH_EXTRA_IMAGES", [])),
    "$ESP32_APP_OFFSET",
    source[0].get_abspath(),
  ]
  merge_cmd = " ".join(
    [
      '"$PYTHONEXE"',
      '"$OBJCOPY"',
      "--chip",
      board_config.get("build.mcu", "esp32"),
      "merge_bin",
      "-o",
      merged_bin,
      "--flash_mode",
      board_config.get("build.flash_mode", "dio"),
      "--flash_freq",
      "${__get_board_f_flash(__env__)}",
      "--flash_size",
      board_config.get("upload.flash_size", "4MB"),
      *flash_images,
    ]
  )

  copyfile(source[0].get_abspath(), standard_bin)
  env.Execute(merge_cmd)

env.AddCustomTarget(
  name="create-binaries",
  dependencies=firmware_build_bin,
  actions=create_binaries,
  title="Create binaries",
  description="Build firmware images",
  always_build=True,
)
