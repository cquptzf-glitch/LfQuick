#!/usr/bin/env python3

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"


class ToolingTests(unittest.TestCase):
    def run_tool(self, name: str, *args: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [sys.executable, str(TOOLS / name), *args],
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )

    def test_view_model_generator(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            spec = temp / "vm.json"
            spec.write_text(
                json.dumps(
                    {
                        "namespace": "Demo",
                        "class": "SampleViewModel",
                        "qmlElement": True,
                        "properties": [
                            {"name": "sampleId", "type": "QString"},
                            {"name": "busy", "type": "bool", "default": "false"},
                        ],
                    }
                ),
                encoding="utf-8",
            )
            result = self.run_tool("lfquick_gen_vm.py", str(spec), "--out", str(temp))
            self.assertEqual(result.returncode, 0, result.stdout)
            header = (temp / "SampleViewModel.h").read_text(encoding="utf-8")
            source = (temp / "SampleViewModel.cpp").read_text(encoding="utf-8")
            self.assertIn("QML_ELEMENT", header)
            self.assertIn("Q_PROPERTY(QString sampleId", header)
            self.assertIn("setBusy", source)

    def test_token_generator(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            spec = temp / "tokens.json"
            spec.write_text(
                json.dumps(
                    {
                        "namespace": "Demo",
                        "class": "GeneratedTokens",
                        "tokens": {
                            "surface": {"type": "color", "light": "#FFFFFF", "dark": "#202020"},
                            "radiusSmall": {"type": "int", "value": 4},
                        },
                    }
                ),
                encoding="utf-8",
            )
            qml = temp / "Tokens.qml"
            cpp = temp / "GeneratedTokens.h"
            result = self.run_tool(
                "lfquick_gen_token.py",
                str(spec),
                "--qml", str(qml),
                "--cpp", str(cpp),
            )
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertIn("Theme.dark", qml.read_text(encoding="utf-8"))
            self.assertIn("surface(bool dark)", cpp.read_text(encoding="utf-8"))

    def test_deploy_checker(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            (temp / "bin").mkdir()
            (temp / "bin" / "app.exe").write_bytes(b"demo")
            manifest = temp / "manifest.json"
            manifest.write_text(
                json.dumps({"required": ["bin/app.exe"], "forbidden": ["**/*.lib"]}),
                encoding="utf-8",
            )
            result = self.run_tool(
                "lfquick_deploy_check.py",
                str(temp),
                "--manifest", str(manifest),
            )
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertIn("Deploy check: PASS", result.stdout)


if __name__ == "__main__":
    unittest.main()
