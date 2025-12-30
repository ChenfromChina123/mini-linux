# -*- mode: python ; coding: utf-8 -*-
"""
PyInstaller配置文件 - 用于打包xiaochen_agent为Linux可执行文件
使用方法: pyinstaller build_linux.spec
"""

import sys
from PyInstaller.utils.hooks import collect_data_files, collect_submodules

block_cipher = None

# 收集所有子模块
hiddenimports = [
    'xiaochen_agent_v2',
    'xiaochen_agent_v2.core',
    'xiaochen_agent_v2.core.agent',
    'xiaochen_agent_v2.core.config',
    'xiaochen_agent_v2.core.config_manager',
    'xiaochen_agent_v2.core.metrics',
    'xiaochen_agent_v2.core.session',
    'xiaochen_agent_v2.ui',
    'xiaochen_agent_v2.ui.cli',
    'xiaochen_agent_v2.utils',
    'xiaochen_agent_v2.utils.console',
    'xiaochen_agent_v2.utils.display',
    'xiaochen_agent_v2.utils.files',
    'xiaochen_agent_v2.utils.interrupt',
    'xiaochen_agent_v2.utils.logs',
    'xiaochen_agent_v2.utils.tags',
    'xiaochen_agent_v2.utils.terminal',
    'requests',
    'urllib3',
    'colorama',
    'json',
    'os',
    'sys',
    'signal',
    'readline',
]

# 数据文件
datas = [
    ('config.json.example', '.'),
    ('README.md', '.'),
]

a = Analysis(
    ['run.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='xiaochen_agent',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

