from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import random
import string


@dataclass(frozen=True)
class GeneratedUser:
    username: str
    password: str


def 生成随机字符串(length: int, *, seed: int | None = None) -> str:
    """生成指定长度的可预测随机字符串（用于测试数据）。"""
    if length <= 0:
        raise ValueError("length 必须为正整数")
    rng = random.Random(seed)
    alphabet = string.ascii_letters + string.digits
    return "".join(rng.choice(alphabet) for _ in range(length))


def 生成测试用户列表(count: int, *, seed: int = 1) -> list[GeneratedUser]:
    """生成一组可复现的测试用户（用户名/密码）。"""
    if count < 0:
        raise ValueError("count 不能为负数")
    rng = random.Random(seed)
    users: list[GeneratedUser] = []
    for i in range(count):
        username = f"user{i}_{rng.randint(1000, 9999)}"
        password = 生成随机字符串(10, seed=rng.randint(1, 1_000_000))
        users.append(GeneratedUser(username=username, password=password))
    return users


def 写入文本文件(path: Path, content: str) -> None:
    """以 UTF-8 写入测试文本文件，覆盖已有内容。"""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


