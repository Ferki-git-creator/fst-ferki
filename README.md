# fst — File System Statistics Utility

`fst` is a compact utility for quickly gathering file system statistics.  
Implemented in C, lean and standalone, with no external dependencies when statically linked via musl, or minimal dynamic linking with glibc.

---

## Features

- Recursive directory traversal
- Count files and directories (empty/non-empty)
- File type classification:
  - Text files
  - Binary files
  - Script files
  - Large files (>100MB)
- File size statistics:
  - Minimum, maximum, and average sizes
  - Human-readable format support
- File dates:
  - Oldest and newest files
- Links:
  - Symbolic and hard links
- Executable files
- Fully autonomous in static mode (musl)

---

## Why fst?

`fst` was designed with **efficiency, portability, and clarity** in mind. Unlike scripts or combinations of standard utilities, `fst` provides a **single, coherent tool** for comprehensive file system statistics:

1. **Lean and fast**  
   Implemented in C with minimal runtime overhead, `fst` executes quickly even on resource-constrained systems such as ARM devices.

2. **Portable and autonomous**  
   Static compilation via musl ensures the binary runs on any compatible Linux system without external dependencies, supporting both x86_64 and ARM architectures.

3. **Comprehensive yet simple**  
   Combines multiple common statistics—file types, sizes, dates, links, executables—into one tool, eliminating the need for complex shell pipelines.

4. **Designed for automation**  
   The tool is suitable for scripting, monitoring, and automated reports, making it practical for both personal and professional workflows.

5. **GNU-friendly philosophy**  
   `fst` respects the Unix philosophy of small, composable utilities: it does one job well, can be easily integrated into scripts or pipelines, and complements existing GNU tools rather than replacing them.

By combining efficiency, portability, and simplicity, `fst` fills a niche for developers and sysadmins who need **quick, reliable insights into their file systems** without the overhead of multiple utilities.

---

## Installation

### Compile a static binary via musl (recommended)

```bash
sudo apt install musl-tools
musl-gcc -static -O2 -o fst fst.c
```

Compile a dynamic binary via glibc

```bash
gcc -O2 -o fst fst.c
```

Make fst available system-wide

```bash
sudo cp fst /usr/local/bin/
```

After this, you can run fst from any directory:

```bash
fst -a -h
```

---

## Usage

```bash
fst [directory] [options]
```

If no directory is specified, fst automatically scans the current directory ..

Example: scan current directory with full statistics in human-readable format:

```bash
fst -a -h
```

Example: scan another directory:

```bash
fst /home/user/projects -a -h
```

### Options

Short	Long	Description

-t	--types	Show file type statistics
-s	--size	Show file size statistics
-p	--permissions	Show file permissions
-d	--dates	Show file dates (oldest/newest)
-l	--links	Show symbolic and hard links
-v	--verbose	Print errors while scanning directories
-h	--human	Human-readable file sizes
-a	--all	Show all statistics



---

Examples

1. Full statistics of the current directory, human-readable sizes:


```bash
fst -a -h
```

2. Only file types and sizes:


```bash
fst -t -s
```

3. Statistics of another directory:


```bash
fst /var/log -a
```

---

## Supported Systems

Linux x86_64 / ARM64

Statically compiled via musl for maximum portability

Dynamically compiled via glibc also supported



---

## License

fst is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
See https://www.gnu.org/licenses/lgpl-3.0.html for details.


---

Author: Ferki