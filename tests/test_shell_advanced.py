import os
import platform
from pathlib import Path

from conftest import run_shell_script, ROOT
import pytest


def test_user_management_and_login(tmp_path: Path):
    # 以 root 创建用户 alice 并设置密码，修改密码后退出重登验证
    lines_root = [
        'root',
        'root',
        'users',
        'useradd alice alicepw',
        'users',
        'passwd alice alicepw2',
        'exit',
    ]
    out_root, err_root, code_root = run_shell_script(lines_root)
    if code_root == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code_root == 0
    assert '用户创建成功' in out_root
    assert '密码已更新' in out_root

    # 使用 alice 新密码登录并执行简单命令
    lines_alice = [
        'alice',
        'alicepw2',
        'help',
        'exit',
    ]
    out_alice, err_alice, code_alice = run_shell_script(lines_alice)
    if code_alice == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code_alice == 0
    assert '@mini-linux' in out_alice
    assert '可用命令' in out_alice


def test_myps_linux_only():
    if platform.system() != 'Linux':
        return
    # 针对当前 shell 进程进行查询
    # 无法在同一进程中拿到准确 pid，这里查询全部
    lines = [
        'root',
        'root',
        'myps -p 1',
        'exit',
    ]
    out, err, code = run_shell_script(lines)
    if code == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code == 0
    assert 'PID' in out and 'COMMAND' in out


def test_myvi_edit_and_save(tmp_path: Path):
    f = tmp_path / 'vi.txt'
    # 进入 myvi，插入文本并保存退出
    lines = [
        'root',
        'root',
        f'myvi {f}',
        'iHello MiniVi\x1b:wq',
        f'mycat {f}',
        'exit',
    ]
    out, err, code = run_shell_script(lines)
    if code == 127:
        pytest.skip('mini_linux_shell 未构建，跳过测试')
    assert code == 0
    assert f.exists()
    assert 'Hello MiniVi' in out
