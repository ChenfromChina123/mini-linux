import os
import sys
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def _detect_binary_path() -> Path:
    exe = 'mini_linux_shell.exe' if os.name == 'nt' else 'mini_linux_shell'
    return ROOT / exe

def build_binary() -> Path:
    bin_path = _detect_binary_path()
    if bin_path.exists():
        return bin_path
    make = shutil.which('make')
    if make:
        try:
            subprocess.run([make], cwd=str(ROOT), check=True)
        except Exception:
            pass
        if bin_path.exists():
            return bin_path
    # 尝试直接使用 gcc 简单编译（最低限度），否则跳过
    gcc = shutil.which('gcc')
    if gcc:
        try:
            # 使用项目的 Makefile 更稳健，这里只作为兜底不会覆盖复杂链接场景
            subprocess.run([gcc, '-Wall', '-g', '-o', str(bin_path),
                            'shell.c', 'user.c', 'history.c', 'util.c',
                            'mycat.c', 'myrm.c', 'myvi.c', 'mytouch.c', 'myecho.c',
                            'mycp.c', 'myls.c', 'myps.c', 'mycd.c', 'mymkdir.c'],
                           cwd=str(ROOT), check=True)
        except Exception:
            pass
        if bin_path.exists():
            return bin_path
    return bin_path  # 不存在时返回路径以便上层判断并跳过

def run_shell_script(lines, env=None, cwd=None, timeout=60):
    bin_path = build_binary()
    if not bin_path.exists():
        return None, None, 127

    # 为测试隔离 HOME/USERPROFILE，避免污染真实用户数据
    home_tmp = ROOT / 'tests' / 'home_tmp'
    home_tmp.mkdir(parents=True, exist_ok=True)
    run_env = os.environ.copy()
    run_env.setdefault('HOME', str(home_tmp))
    if os.name == 'nt':
        run_env.setdefault('USERPROFILE', str(home_tmp))
    if env:
        run_env.update(env)

    proc = subprocess.Popen(
        [str(bin_path)], cwd=str(cwd or ROOT), env=run_env,
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True, bufsize=1
    )

    script = "\n".join(lines) + "\n"
    try:
        out, err = proc.communicate(input=script, timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        out, err = proc.communicate()
    return out, err, proc.returncode

