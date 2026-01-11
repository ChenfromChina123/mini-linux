from __future__ import annotations

from pathlib import Path

import pytest

from conftest import run_shell_script


def _skip_if_not_built(code: int) -> None:
    """当主程序未构建时跳过测试（保持在不同环境下可运行）。"""
    if code == 127:
        pytest.skip("mini_linux_shell 未构建，跳过测试")


def test_登录_help_whoami():
    """验证：能够登录、查看帮助、查询当前用户。"""
    lines = [
        "root",
        "root",
        "help",
        "whoami",
        "exit",
    ]
    out, err, code = run_shell_script(lines)
    _skip_if_not_built(code)
    assert code == 0
    assert out is not None
    assert "可用命令" in out
    assert "root" in out


def test_文件链路_创建写入读取复制删除(tmp_path: Path):
    """验证：文件创建、追加写入、读取、复制、删除的端到端链路。"""
    a = tmp_path / "a.txt"
    b = tmp_path / "b.txt"
    lines = [
        "root",
        "root",
        f"mytouch {a}",
        f"myecho Hello > {a}",
        f"myecho World >> {a}",
        f"mycat {a}",
        f"mycp {a} {b}",
        f"mycat {b}",
        f"myrm {b}",
        "exit",
    ]
    out, err, code = run_shell_script(lines)
    _skip_if_not_built(code)
    assert code == 0
    assert a.exists()
    assert "Hello" in out and "World" in out
    assert "已复制文件" in out
    assert not b.exists()


def test_目录链路_mkdir_cd_ls(tmp_path: Path):
    """验证：递归建目录、切换目录、列目录与文件可见性。"""
    d = tmp_path / "sub" / "dir"
    lines = [
        "root",
        "root",
        f"mymkdir -p {d}",
        f"mycd {d}",
        "myls",
        "mytouch here.txt",
        "myls",
        "exit",
    ]
    out, err, code = run_shell_script(lines, cwd=tmp_path)
    _skip_if_not_built(code)
    assert code == 0
    assert d.exists()
    assert (d / "here.txt").exists()
    assert "here.txt" in out


