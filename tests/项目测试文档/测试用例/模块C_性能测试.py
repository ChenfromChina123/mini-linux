from __future__ import annotations

from pathlib import Path
import time

import pytest

from conftest import run_shell_script


def _skip_if_not_built(code: int) -> None:
    """当主程序未构建时跳过测试。"""
    if code == 127:
        pytest.skip("mini_linux_shell 未构建，跳过测试")


def test_批量命令执行_耗时可控(tmp_path: Path):
    """验证：批量执行简单命令在合理时间内完成（粗粒度回归）。"""
    f = tmp_path / "perf.txt"
    lines = ["root", "root"]
    for i in range(80):
        lines.append(f"myecho line{i} >> {f}")
    lines += [f"mycat {f}", "exit"]

    start = time.perf_counter()
    out, err, code = run_shell_script(lines, cwd=tmp_path, timeout=120)
    elapsed = time.perf_counter() - start

    _skip_if_not_built(code)
    assert code == 0
    assert out is not None
    assert elapsed < 15.0
    assert "line0" in out and "line79" in out


