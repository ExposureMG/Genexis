# gxbuild3 NAND Modernization Design

## Goal

Make Genexis an effective Qt frontend for gxbuild3 without duplicating NAND
inspection, donor extraction, asset resolution, patch selection, or build
validation logic.

## Scope

This migration covers both of Genexis's NAND-facing flows:

- `Nand` / NAND Info inspection.
- `GxBuild3Adapter` / NAND Builder preparation and assembly.

Genexis remains responsible for Qt state, QML-facing presentation values, file
selection, asynchronous work, progress signals, and output-path selection.
gxbuild3 remains responsible for NAND domain behavior.

## Architecture

```text
QML
  -> Nand or NandBuilderController
  -> Genexis gxbuild3 adapter / QML mapper
  -> gxbuild3 public API
```

The boundary uses only gxbuild3 public headers:

- `GxBuild::ExtractSomeInfo` for no-CPU-key NAND inspection.
- `GxBuild::ExtractAllInfo` for CPU-key-backed NAND inspection.
- `gxbuild3::cli::BuildInputResolver` to resolve a build request.
- `GxBuild::RunBuild` to assemble the resolved NAND.

Genexis must not directly parse `FlashImage`, decrypt SMC or Keyvault data,
build patch sets, or manually discover and layer build assets after this
migration.

## NAND Info Flow

1. `Nand::openFile` reads the selected NAND on a worker thread.
2. Without a CPU key, it calls `ExtractSomeInfo` and maps its `AllNandInfo`
   result into the existing QML properties and component cards.
3. When the user supplies a valid CPU key, it calls `ExtractAllInfo` for the
   same raw NAND. The result enriches the existing state with decrypted
   Keyvault values, LDVs, pairing data, and FlashFS information.
4. Invalid images and invalid CPU keys leave the UI in a consistent partial
   state and report a useful error instead of silently swallowing exceptions.

The existing QML property names stay stable for this pass. A focused mapper in
the NAND page layer converts gxbuild3 types and field names to those
presentation strings. This keeps QML stable while making gxbuild3 the sole
source of NAND facts.

## NAND Builder Flow

1. `NandBuilderController` converts QML configuration into
   `NandBuildConfig`, retaining its UI-facing API.
2. `GxBuild3Adapter` prepares a per-build staging directory containing the
   requested donor and explicit UI overrides in gxbuild3's expected source
   layout.
3. The adapter translates the selection to `gxbuild3::cli::BuildArgs` and
   calls `BuildInputResolver::Resolve`.
4. On success, the adapter calls `GxBuild::RunBuild` with the resolver's
   `BuildRequest::input` and writes the returned bytes to the selected output.
5. On failure, it propagates gxbuild3's error message plus relevant path/item
   information through `BuildResult` and existing Qt progress signals.

The resolver becomes the source of truth for CPU-key validation, donor versus
loose-donor requirements, geometry, INI and asset discovery, options,
patches, KV/SMC handling, and input validation. The adapter must not manually
construct `Input` or use gxbuild3 internal headers.

## Platform Integration

Linux builds use gxbuild3's native CMake target. The MSVC wrapper must include
the same gxbuild3 library sources as upstream, including the CLI resolver,
command-line support, and input validation sources. Both platforms must build
against the same public gxbuild3 API.

## Testing

Add executable C++ integration tests that exercise real gxbuild3 fixture NANDs
and Genexis mapping/translation code:

- No-key NAND inspection returns public header, geometry, bootloader, and SMC
  information.
- CPU-key inspection enriches the state with decrypted Keyvault and bootloader
  metadata.
- Retail and glitch build configurations resolve and build through the adapter.
- Invalid CPU keys, missing assets, and invalid donors return actionable errors.
- The adapter cannot compile against removed gxbuild3 `Input` members or
  internal NAND parser types.

## Non-goals

- Redesigning the QML NAND pages.
- Adding a second builder backend.
- Reimplementing gxbuild3 error classification in Genexis.
