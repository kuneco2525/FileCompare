# CMakeList.txt : FileCompare の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.10)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FileCompare")

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (FileCompare "FileCompare.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET FileCompare PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。

# MSVC でソースと実行文字列を UTF-8 として扱う（C4819 を防止）。
if (MSVC)
	target_compile_options(FileCompare PRIVATE /utf-8)
endif()
