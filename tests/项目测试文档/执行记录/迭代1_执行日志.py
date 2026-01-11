from __future__ import annotations

from datetime import datetime
from pathlib import Path
import subprocess
import sys


def 运行迭代1测试并生成日志(*, project_root: Path | None = None) -> Path:
    """运行 pytest 并生成一次迭代的执行日志（Markdown）。"""
    root = project_root or Path(__file__).resolve().parents[3]
    log_dir = root / "tests" / "项目测试文档" / "执行记录"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_path = log_dir / "迭代1_执行日志.md"

    cmd = [sys.executable, "-m", "pytest", "-q", "tests"]
    started_at = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    proc = subprocess.run(cmd, cwd=str(root), capture_output=True, text=True, encoding="utf-8")

    content = (
        f"# 迭代1 执行日志\n\n"
        f"- 开始时间：{started_at}\n"
        f"- 命令：`{' '.join(cmd)}`\n"
        f"- 退出码：{proc.returncode}\n\n"
        f"## 标准输出\n\n```text\n{proc.stdout}\n```\n\n"
        f"## 标准错误\n\n```text\n{proc.stderr}\n```\n"
    )
    log_path.write_text(content, encoding="utf-8")
    return log_path


if __name__ == "__main__":
    path = 运行迭代1测试并生成日志()
    print(str(path))

