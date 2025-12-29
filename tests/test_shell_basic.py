import os
from pathlib import Path
import platform

from conftest import run_shell_script, ROOT
import pytest


def test_login_and_help():
    lines = [
        'root',
        'root',
        'help',
        'whoami',
        'exit',
    ]
    out, err, code = run_shell_script(lines)
    if code == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code == 0
    assert out is not None
    assert '可用命令' in out
    assert 'mycat' in out and 'myls' in out and 'passwd' in out
    assert 'root' in out  # 验证 whoami 输出

def test_file_ops_touch_echo_cat_cp_rm(tmp_path: Path):
    a = tmp_path / 'a.txt'
    b = tmp_path / 'b.txt'
    lines = [
        'root',
        'root',
        f'mytouch {a}',
        f'myecho Hello > {a}',
        f'myecho World >> {a}',
        f'mycat {a}',
        f'mycp {a} {b}',
        f'mycat {b}',
        f'myrm {b}',
        'exit',
    ]
    out, err, code = run_shell_script(lines)
    if code == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code == 0
    assert a.exists()
    assert 'Hello' in out and 'World' in out
    assert '已复制文件' in out
    assert not b.exists()


def test_dir_ops_mkdir_cd_ls(tmp_path: Path):
    d = tmp_path / 'sub' / 'dir'
    f = d / 'here.txt'
    lines = [
        'root',
        'root',
        f'mymkdir -p {d}',
        f'mycd {d}',
        'myls',
        'mytouch here.txt',
        'myls',
        'exit',
    ]
    out, err, code = run_shell_script(lines)
    if code == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code == 0
    assert d.exists()
    assert f.exists()