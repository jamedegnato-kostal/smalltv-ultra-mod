#!/usr/bin/env python3
"""Build an STH1 .stheme package from theme.json and optional pre-encoded assets."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

MAX_PACKAGE = 3 * 1024 * 1024
MAX_ENTRIES = 256
MAX_PATH_BYTES = 120


def package_bytes(manifest_path: Path, assets_dir: Path | None) -> bytes:
    manifest = manifest_path.read_bytes()
    try:
        parsed = json.loads(manifest)
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError(f"invalid theme.json: {exc}") from exc
    if not isinstance(parsed, dict) or parsed.get("spec") not in (1, 2):
        raise ValueError("theme.json must be an object with spec 1 or 2")

    entries: list[tuple[str, bytes]] = [("theme.json", manifest)]
    if assets_dir is not None:
        if not assets_dir.is_dir():
            raise ValueError(f"asset directory does not exist: {assets_dir}")
        for path in sorted(assets_dir.rglob("*")):
            if path.is_symlink():
                raise ValueError(f"symlinks are not allowed: {path}")
            if path.is_file():
                rel = path.relative_to(assets_dir).as_posix()
                entries.append((rel, path.read_bytes()))

    if len(entries) > MAX_ENTRIES:
        raise ValueError(f"too many package entries (max {MAX_ENTRIES})")

    output = bytearray(b"STH1")
    output.extend(len(entries).to_bytes(2, "little"))
    output.extend((0).to_bytes(2, "little"))

    seen: set[str] = set()
    for name, payload in entries:
        try:
            encoded_name = name.encode("ascii")
        except UnicodeEncodeError as exc:
            raise ValueError(f"package path must be ASCII: {name!r}") from exc
        parts = name.split("/")
        if (
            not encoded_name
            or len(encoded_name) > MAX_PATH_BYTES
            or name.startswith("/")
            or "\\" in name
            or any(part in ("", ".", "..") for part in parts)
            or name in seen
        ):
            raise ValueError(f"invalid or duplicate package path: {name!r}")
        if len(payload) > 0xFFFFFFFF:
            raise ValueError(f"package entry is too large: {name}")
        seen.add(name)
        output.extend(len(encoded_name).to_bytes(2, "little"))
        output.extend(len(payload).to_bytes(4, "little"))
        output.extend(encoded_name)
        output.extend(payload)
        if len(output) > MAX_PACKAGE:
            raise ValueError(f"package exceeds {MAX_PACKAGE} bytes")

    return bytes(output)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path, help="theme.json manifest")
    parser.add_argument("--assets", type=Path, help="directory of .sti/.stf package assets")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--output", type=Path, help="write the .stheme package")
    group.add_argument("--check", type=Path, help="verify an existing package is up to date")
    args = parser.parse_args()

    try:
        packed = package_bytes(args.manifest, args.assets)
        if args.check is not None:
            existing = args.check.read_bytes()
            if existing != packed:
                raise ValueError(f"{args.check} does not match {args.manifest}; rebuild the package")
            print(f"verified {args.check} ({len(packed)} bytes)")
        else:
            assert args.output is not None
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(packed)
            print(f"wrote {args.output} ({len(packed)} bytes)")
    except (OSError, ValueError) as exc:
        print(f"pack_stheme: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
