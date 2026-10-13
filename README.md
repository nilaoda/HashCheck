# HashCheck Shell Extension #

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/github/gurnec/HashCheck?branch=master&svg=true)](https://ci.appveyor.com/project/gurnec/HashCheck)

### Installation ###

The latest installer for Windows (Vista and later) can be found here:  
<https://github.com/gurnec/HashCheck/releases/latest>

#### Contributors ####

Kai Liu  
Christopher Gurnee  
David B. Trout  
Tim Schlueter


## Building from source ##

#### Compiler ####

Microsoft Visual Studio 2015 (the free Community edition works well).

#### CRC64 acceleration ####

CRC64 remains compatible with Go's ECMA table / CRC-64/XZ (`123456789` = `995DC9BBDF1939FA`). Buffers of at least 256 bytes use 128-bit polynomial folding when the CPU supports it:

- x86/x64: SSE2 + PCLMULQDQ, detected with CPUID and the Windows SSE2 feature flag.
- ARM64: PMULL, detected with `IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE)`.
- Unsupported CPUs and smaller buffers automatically use the existing byte-table algorithm. Lookup table initialization and CPU detection are synchronized with Windows `InitOnceExecuteOnce`.

No global AVX or crypto compiler option is required. ARM64 builds require a modern Visual Studio ARM64 toolset and Windows SDK; the existing build workflow uses Visual Studio 2022 (`v143`). To build a scalar-only version for comparison, pass `/p:TempCDefines=CRC64_NO_SIMD` to MSBuild. Validate both builds on Windows, including empty files, block tails and incremental updates; the Windows DLL cannot be built or run on macOS directly.

#### Localizations ####

Translation strings are stored as string table resources. These tables can be modified by editing [HashCheckTranslations.rc](HashCheckTranslations.rc).

#### Translation Contributors ####

Català: [@Hiro5](https://github.com/Hiro5)  
中文 (简体): "yumeyao"  
中文 (繁體): Jack Chang and [@Chocobo1](https://github.com/Chocobo1)  
Čeština: Václav Veselý  
Deutsch: "Rolf"  
Ελληνικά: "XhmikosR"  
Español: "Phare"  
Français: "mooms" and "user_hidden"  
Italiano: "Botta" and [@scara](https://github.com/scara)  
日本語: "yumeyao"  
한국어: JaeHyung Lee  
Nederlands: "Edwin"  
Polski: "RedWine"  
Português (Brasil): "0d14r3"  
Português (Portugal): "LPCA"  
Română: Oprea Nicolae, a.k.a. "Jaff"  
Pусский: Yurii Petrashko  
Svenska: Stefan Friman  
Türkçe: M. Ömer Gölgeli  
Yкраїнська: Yurii Petrashko

#### License and miscellanea ####

Standard 3-Clause BSD License.

Please refer to [license.txt](license.txt) for details about distribution and modification.

This software is based on software originally distributed at:  
<http://code.kliu.org/hashcheck/>
