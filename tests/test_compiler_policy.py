"""Exercise the CMake compiler policy without requiring Qt or KDE.

Run with: python -m unittest discover -s tests -v
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
CMAKE = shutil.which("cmake")
POLICY = ROOT / "cmake" / "GenexisCompiler.cmake"


@unittest.skipUnless(CMAKE, "cmake must be on PATH")
class CompilerPolicyTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.bin = self.directory / "bin"
        self.bin.mkdir()
        self.result = self.directory / "selected.txt"

    def compiler(self, name):
        # Discovery only: these files are never executed. Actual compiler
        # identification remains CMake's responsibility during project().
        path = self.bin / (name + (".exe" if os.name == "nt" else ""))
        path.touch()
        path.chmod(0o755)
        return path.as_posix()

    def run_policy(self, body, env=None):
        script = self.directory / "check.cmake"
        script.write_text(
            'cmake_minimum_required(VERSION 3.28)\n'
            'set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)\n'
            'set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)\n'
            'set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH FALSE)\n'
            f'set(CMAKE_PROGRAM_PATH "{self.bin.as_posix()}")\n'
            f'include("{POLICY.as_posix()}")\n' + body,
            encoding="utf-8",
        )
        environment = os.environ.copy()
        for name in ("CC", "CXX", "CMAKE_TOOLCHAIN_FILE"):
            environment.pop(name, None)
        environment.update(env or {})
        return subprocess.run(
            [CMAKE, "-P", str(script)], env=environment,
            capture_output=True, text=True, timeout=15,
        )

    def select(self, setup="", env=None):
        process = self.run_policy(
            setup + '\ngenexis_select_compilers()\n'
            f'file(WRITE "{self.result.as_posix()}" '
            '"${CMAKE_C_COMPILER}\n${CMAKE_CXX_COMPILER}")\n', env,
        )
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        return self.result.read_text(encoding="utf-8").split("\n")

    def test_prefers_complete_clang_pair_over_gcc(self):
        self.compiler("gcc")
        self.compiler("g++")
        clang = self.compiler("clang")
        clangxx = self.compiler("clang++")
        self.assertEqual(self.select(), [clang, clangxx])

    def test_falls_back_to_gcc(self):
        gcc = self.compiler("gcc")
        gxx = self.compiler("g++")
        self.assertEqual(self.select(), [gcc, gxx])

    def test_incomplete_clang_pair_does_not_mix_compiler_families(self):
        self.compiler("clang")
        gcc = self.compiler("gcc")
        gxx = self.compiler("g++")
        self.assertEqual(self.select(), [gcc, gxx])

    def test_no_supported_pair_fails(self):
        process = self.run_policy("genexis_select_compilers()\n")
        self.assertNotEqual(process.returncode, 0)
        self.assertIn("Clang", process.stderr)
        self.assertIn("GCC", process.stderr)

    def test_explicit_compilers_are_preserved(self):
        self.compiler("clang")
        self.compiler("clang++")
        self.assertEqual(self.select(
            'set(CMAKE_C_COMPILER /custom/gcc)\n'
            'set(CMAKE_CXX_COMPILER /custom/g++)\n'
        ), ["/custom/gcc", "/custom/g++"])

    def test_environment_selection_is_left_to_cmake(self):
        self.compiler("clang")
        self.compiler("clang++")
        self.assertEqual(self.select(env={"CC": "gcc", "CXX": "g++"}), ["", ""])

    def test_toolchain_selection_is_left_to_cmake(self):
        self.assertEqual(self.select('set(CMAKE_TOOLCHAIN_FILE custom.cmake)'), ["", ""])
        self.assertEqual(self.select(env={"CMAKE_TOOLCHAIN_FILE": "custom.cmake"}), ["", ""])

    def test_supported_compiler_ids_are_accepted(self):
        for compiler_id in ("Clang", "AppleClang", "GNU", "MSVC"):
            with self.subTest(compiler_id=compiler_id):
                process = self.run_policy(
                    f'set(CMAKE_C_COMPILER_ID {compiler_id})\n'
                    f'set(CMAKE_CXX_COMPILER_ID {compiler_id})\n'
                    'genexis_validate_compilers()\n'
                )
                self.assertEqual(process.returncode, 0, process.stderr)

    def test_unsupported_compiler_is_rejected_for_either_language(self):
        for language in ("C", "CXX"):
            with self.subTest(language=language):
                process = self.run_policy(
                    'set(CMAKE_C_COMPILER_ID Clang)\n'
                    'set(CMAKE_CXX_COMPILER_ID Clang)\n'
                    f'set(CMAKE_{language}_COMPILER_ID Intel)\n'
                    'genexis_validate_compilers()\n'
                )
                self.assertNotEqual(process.returncode, 0)
                self.assertIn("Intel", process.stderr)


if __name__ == "__main__":
    unittest.main()
