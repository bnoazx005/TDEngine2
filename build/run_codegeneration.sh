echo off

INTROSPECTOR_DIR="../TDEngine2/tools/Introspector/"

OUTPUT_DIR="../TDEngine2/include/"

chmod +x $INTROSPECTOR_DIR"/tde2_introspector"
$INTROSPECTOR_DIR"/tde2_introspector" "../TDEngine2/include/" "../TDEngine2/plugins/*/include/*" -O $OUTPUT_DIR --force --tagged-only  --emit-enums --emit-classes --emit-structs --exclude-paths="/platform/win32/;/deps/" --exclude-typenames="(.*?)(Win|D3D11)(.*?)"
