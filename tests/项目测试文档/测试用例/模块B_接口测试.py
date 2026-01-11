from __future__ import annotations

from pathlib import Path

import pytest

from conftest import run_shell_script


def _skip_if_not_built(code: int) -> None:
    """当主程序未构建时跳过测试。"""
    if code == 127:
        pytest.skip("mini_linux_shell 未构建，跳过测试")


def test_未知命令_返回提示():
    """验证：未知命令会给出友好提示，且进程可继续交互。"""
    lines = [
        "root",
        "root",
        "this_command_should_not_exist_123",
        "help",
        "exit",
    ]
    out, err, code = run_shell_script(lines)
    _skip_if_not_built(code)
    assert code == 0
    assert out is not None
    assert ("command not found" in out) or ("未找到" in out) or ("无效" in out)
    assert "可用命令" in out


def test_myecho_重定向接口一致性(tmp_path: Path):
    """验证：myecho 的内置重定向接口行为与 mycat 读取一致。"""
    f = tmp_path / "redir.txt"
    lines = [
        "root",
        "root",
        f"myecho A > {f}",
        f"myecho B >> {f}",
        f"mycat {f}",
        "exit",
    ]
    out, err, code = run_shell_script(lines, cwd=tmp_path)
    _skip_if_not_built(code)
    assert code == 0
    assert out is not None
    assert "A" in out and "B" in out


def test_mycat_不存在文件_错误提示(tmp_path: Path):
    """验证：读取不存在文件会输出错误信息（不要求具体措辞）。"""
    missing = tmp_path / "missing.txt"
    lines = [
        "root",
        "root",
        f"mycat {missing}",
        "exit",
    ]
    out, err, code = run_shell_script(lines, cwd=tmp_path)
    _skip_if_not_built(code)
    assert code == 0
    combined = (out or "") + "\n" + (err or "")
    assert ("不存在" in combined) or ("No such file" in combined) or ("错误" in combined)


