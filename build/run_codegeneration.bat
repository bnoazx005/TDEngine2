echo off

set INTROSPECTOR_DIR="../TDEngine2/tools/Introspector/"

set ENGINE_HEADERS_DIR="../TDEngine2/include/"
set OUTPUT_DIR=%ENGINE_HEADERS_DIR%

%INTROSPECTOR_DIR%"/tde2_introspector.exe" %ENGINE_HEADERS_DIR% -O %OUTPUT_DIR% --force --tagged-only --emit-enums --emit-classes --emit-structs --exclude-paths="/platform/unix/" --exclude-typenames="(.*?)Unix(.*?)"
