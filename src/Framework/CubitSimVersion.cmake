include_guard(GLOBAL)

# Preserve the existing year.month release numbering for the whole application.
string(TIMESTAMP CUBITSIM_YEAR "%Y")
string(TIMESTAMP CUBITSIM_MONTH "%m")
string(REGEX REPLACE "^0" "" CUBITSIM_MONTH "${CUBITSIM_MONTH}")
set(CUBITSIM_VERSION "${CUBITSIM_YEAR}.${CUBITSIM_MONTH}")
add_compile_definitions(CUBITSIM_VERSION="${CUBITSIM_VERSION}")
