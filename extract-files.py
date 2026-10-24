#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from pathlib import Path
import re

import extract_utils.tools

extract_utils.tools.DEFAULT_PATCHELF_VERSION = '0_17_2'

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)
from extract_utils.makefiles import MakefilesCtx, ProductPackagesCtx

namespace_imports = [
    'device/asus/sdm660-common',
    'hardware/qcom-caf/msm8998',
    'hardware/qcom-caf/wlan',
    'vendor/qcom/opensource/dataservices',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


def fix_prebuilt_app_uses_libraries(
    ctx: MakefilesCtx, _packages_ctx: ProductPackagesCtx
):
    # Record the libraries declared by the retained APK manifests. The two
    # apps using DEX-only imports cannot express uses_libs dependencies in A15;
    # their runtime libraries are declared in the installed permissions XMLs.
    apps = {
        'dpmserviceapp': {'enforce_uses_libs': False},
        'QtiTelephonyService': {'enforce_uses_libs': False},
        'ims': {'uses_libs': [
            'qti-telephony-hidl-wrapper', 'qti-telephony-utils', 'ims-ext-common',
        ]},
        'imssettings': {'uses_libs': ['ims-ext-common', 'qti-telephony-utils']},
        'qcrilmsgtunnel': {'uses_libs': [
            'qti-telephony-hidl-wrapper', 'qti-telephony-utils',
        ]},
    }
    pattern = r'^android_app_import \{\n    name: "(' + '|'.join(apps) + r')",\n.*?^\}'

    def fix_app(match):
        block = match.group(0)
        lines = []
        for key, value in apps[match.group(1)].items():
            if f'    {key}:' in block:
                continue
            if isinstance(value, list):
                lines.append(f'    {key}: [')
                lines.extend(f'        "{lib}",' for lib in value)
                lines.append('    ],')
            else:
                lines.append(f'    {key}: {str(value).lower()},')
        if lines:
            block = block.replace('    owner:', '\n'.join(lines) + '\n    owner:', 1)
        return block

    ctx.bp_out.flush()
    text = Path(ctx.bp_out.name).read_text()
    text = re.sub(pattern, fix_app, text, flags=re.MULTILINE | re.DOTALL)
    ctx.bp_out.seek(0)
    ctx.bp_out.write(text)
    ctx.bp_out.truncate()


blob_fixups: blob_fixups_user_type = {
    (
        'system_ext/lib/libfm-hci.so',
        'system_ext/lib64/libfm-hci.so',
        'system_ext/lib/libwfdnative.so',
        'system_ext/lib64/libwfdnative.so',
    ): blob_fixup()
        .remove_needed('android.hidl.base@1.0.so'),
    (
        'system_ext/lib64/lib-imscamera.so',
        'system_ext/lib64/lib-imsvideocodec.so',
        'system_ext/lib64/libimsmedia_jni.so',
    ): blob_fixup()
        .add_needed('libgui_shim.so'),
    'vendor/bin/pm-service': blob_fixup()
        .add_needed('libutils-v33.so'),
    'vendor/lib64/libwvhidl.so': blob_fixup()
        .add_needed('libcrypto_shim.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'sdm660-common',
    'asus',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    check_elf=False,
    skip_main_proprietary_file=True,
)
module.add_proprietary_file(
    'proprietary-files.txt',
    post_makefile_generation_fn=fix_prebuilt_app_uses_libraries,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
