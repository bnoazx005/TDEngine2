echo off

INTROSPECTOR_DIR="../TDEngine2/tools/Introspector/"

ENGINE_HEADERS_DIR="../TDEngine2/include/"
OUTPUT_DIR=$ENGINE_HEADERS_DIR

chmod +x $INTROSPECTOR_DIR"/tde2_introspector"
$INTROSPECTOR_DIR"/tde2_introspector" $ENGINE_HEADERS_DIR -O $OUTPUT_DIR --emit-enums --emit-classes --emit-structs --exclude-paths="/platform/win32/" --exclude-typenames="(.*?)Win(.*?)"
